#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QtConcurrent/QtConcurrent>
#include <fftanalyzer.h>
#include <harmonictablemodel.h>
#include <QTime>
#include <filemanager.h>
#include <harmonic.h>

bool isLoadCache;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dataThread(new QThread(this))
    , simulationDataThread(new QThread(this))
    , fftThread(new QThread(this))
    , configSetting()
{
    ui->setupUi(this);
    singalConvert = new SingalConvert;

    //加载软件基本配置
    configSetting = ConfigLoader::loadConfigFromJson("Config.Default.json");
    //加载时基配置
    timeBaseList = TimeBaseLoader::loadFromJson("TimeBase.json");

    for(const TimeBase &tb : timeBaseList){
        ui->cb_Timebase->addItem(tb.scope);
    }
    //默认频域图表单位
    frequencyUnit = 0;
    MaxCacheCount = configSetting.dataCacheCount;
    ui->harmonicParam->setValue(configSetting.defaultBaseFrequency);



    //采集卡参数
    picoParam = new PicoParam(
        timeBaseList.at(0) ,
        PS2000A_CHANNEL_A ,
        PS2000A_1V ,
        PS2000A_DC ,
        ui->sb_timebase->value(),
        1.0,
        MaxCacheCount,
        0.0
    );
    //绑定参数
    binderVoltage =  EnumMap::getVoltageBuilder(ui->cb_Voltage);
    binderChannel = EnumMap::getChannelBuilder(ui->cb_Channel);
    binderCoupling = EnumMap::getCouplingBuilder(ui->cb_Couping);

    picohandle = new PicoScopeHandler();
    picohandle->setPicoParam(picoParam);

    //绑定缓存的listView
    itemModel = new QStandardItemModel(ui->listView);
    ui->listView->setModel(itemModel);

    //链接线程槽
    connect(dataThread ,&QThread::started, picohandle ,&PicoScopeHandler::startAcquisition);
    connect(simulationDataThread ,&QThread::started ,picohandle ,&PicoScopeHandler::startSimulation);
    connect(fftThread ,&QThread::started,this,[](){
    });

    connect(picohandle, &PicoScopeHandler::dataUpdated,this, [this]() {
        if(configSetting.realTimeWaveform){
            updateGraph(picohandle->getSamples());
        }
        int ps = static_cast<int>((static_cast<double>(picohandle->getCacheData().size()) / MaxCacheCount) * 100);
        ui->progressBar->setValue(ps);
        ui->lbl_CacheCount->setText(QString::number(picohandle->getCacheData().size()));
    });

    connect(picohandle,&PicoScopeHandler::cacheDataUpdated ,this ,[](){
        //qDebug()<<"cacheDataUpdated";
    });

    ui->chartView->setTitle("时域图");
    ui->chartView->setXAxisTitle("Time (us)");
    ui->chartView->setYAxisTitle("Voltage (V)");
    ui->chartView->setLineColor(Qt::blue);
    ui->chartView->setLineWidth(0.5);
    ui->chartView->setXAxisRange(0,1);
    ui->chartView->setYAxisRange(-5,5);

    ui->cb_Timebase->setCurrentIndex(configSetting.defaultTimeBase);
    //binderVoltage->setCurrentEnumValue(PS2000A_10V);
    ui->cb_Voltage->setCurrentIndex(configSetting.defaultVoltage);
    //binderCoupling->setCurrentEnumValue(PS2000A_DC);
    ui->cb_Couping->setCurrentIndex(configSetting.defaultCoupling);
    //binderChannel->setCurrentEnumValue(PS2000A_CHANNEL_A);
    ui->cb_Channel->setCurrentIndex(configSetting.defaultChannel);

    ui->chartView_2->setTitle("频域图");
    ui->chartView_2->setXAxisTitle("MHz");
    ui->chartView_2->setYAxisTitle("dBu");
    ui->chartView_2->setLineColor(Qt::blue);
    ui->chartView_2->setLineWidth(0.5);
    ui->chartView_2->setXAxisRange(0,5);
    // ui->chartView_2->setYAxisRange(-200,100);
    ui->chartView_2->setYAxisScale(-100 ,100 ,10);

    ui->chartView_2->setXAxisTitle(picoParam->timeBaseObj.frequencyUnit);
    ui->chartView_2->setXAxisRange(0 ,picoParam->timeBaseObj.frequencyScope);

    ui->chartView_3->setTitle("谐波占比");
    ui->chartView_3->setXAxisTitle("kHz");
    ui->chartView_3->setYAxisTitle("Harmonic Ratio (%)");
    ui->chartView_3->setLineColor(Qt::blue);
    ui->chartView_3->setLineWidth(1);


    ui->chartView_4->setTitle("谐波能量");
    ui->chartView_4->setXAxisTitle("kHz");
    ui->chartView_4->setYAxisTitle("Harmonic Energy");
    ui->chartView_4->setLineColor(Qt::blue);
    ui->chartView_4->setLineWidth(1);

    picohandle->initialize();

    isLoadCache = false;
}

MainWindow::~MainWindow()
{
    picohandle->stopSimulation();
    dataThread->quit();
    if(!dataThread->wait(3000)){
        dataThread->terminate();
        dataThread->wait();
    }

    delete ui;
}
//渲染数据
void MainWindow::updateGraph(const QVector<QPointF> bufferedData){
    ui->chartView->setData(bufferedData,picoParam->timeBaseObj.unit);
    // for(int i=0;i<10;++i){
    //     qDebug()<<bufferedData[i].x();
    // }
    // qDebug()<<"-------------";

    double samplingRate = singalConvert->calculateFrequency(bufferedData ,picoParam->samplingRate);
    ui->lbl_fvalues->setText(QString::number(samplingRate/1000));

    double amplitudeValue = singalConvert->measurePeakValue(bufferedData);
    ui->lbl_vvalues->setText(QString::number(amplitudeValue));

    // FFT 数据的处理移到后台线程
    QtConcurrent::run([=] {
        int unittype = 2;
        if(picoParam->timeBaseObj.frequencyUnit == "kHz"){
            unittype = 1;
        }else if(picoParam->timeBaseObj.frequencyUnit == "Hz"){
            unittype = 0;
        }else if(picoParam->timeBaseObj.frequencyUnit == "mHz"){
            unittype = 2;
        }

        // QTime startTime = QTime::currentTime(); // 获取当前时间
        const QVector<QPointF> fftResult = singalConvert->performFFT(bufferedData ,unittype,picoParam->timeBaseObj.conversion);
        // QTime endTime = QTime::currentTime();   // 获取结束时间
        // // 计算时间差（以毫秒为单位）
        // int elapsedMilliseconds = startTime.msecsTo(endTime);
        //qDebug() << "fft 时间:"<< elapsedMilliseconds << "毫秒";
        ui->chartView_2->setFData(fftResult);
    });

    // QtConcurrent::run([=]{
    //     FFTAnalyzer analyzer(2000);
    //     double baseFreq = ui->harmonicParam->value();
    //     QVector<QVector<QVariant>> results = analyzer.analyze(bufferedData,baseFreq);

    //     // 初始化最大值和最小值
    //     double maxFreq = std::numeric_limits<double>::lowest();
    //     double minFreq = std::numeric_limits<double>::max();
    //     double maxRatio = std::numeric_limits<double>::lowest();
    //     double minRatio = std::numeric_limits<double>::max();
    //     double maxEnergy = std::numeric_limits<double>::lowest();
    //     double minEnergy = std::numeric_limits<double>::max();

    //     //绑定tableView
    //     HarmonicTableModel *tableModel = new HarmonicTableModel();
    //     tableModel->setData(results);
    //     ui->tableView2->setModel(tableModel);
    //     ui->tableView2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //     ui->tableView2->verticalHeader()->hide();

    //     QVector<QPointF> harmonicFreqList;
    //     QVector<QPointF> harmonicEnergyList;
    //     // 从 results 中提取数据并添加到系列
    //     for (const auto& row : results) {
    //         double harmonicFreq = row[1].toDouble()/1000; // 谐波频率 (X 轴)
    //         double harmonicEnergy = row[2].toDouble()/1000; // 谐波占比 (Y 轴)
    //         double harmonicRatio = row[3].toDouble(); // 谐波占比 (Y 轴)

    //         // 更新最大值和最小值
    //         if (harmonicFreq > maxFreq) maxFreq = harmonicFreq;
    //         if (harmonicFreq < minFreq) minFreq = harmonicFreq;
    //         if (harmonicRatio > maxRatio) maxRatio = harmonicRatio;
    //         if (harmonicRatio < minRatio) minRatio = harmonicRatio;
    //         if(harmonicEnergy > maxEnergy) maxEnergy = harmonicEnergy;
    //         if(harmonicEnergy < minEnergy) minEnergy = harmonicEnergy;

    //         harmonicFreqList.append(QPointF(harmonicFreq, harmonicRatio));
    //         harmonicEnergyList.append(QPointF(harmonicFreq, harmonicEnergy));
    //     }

    //     ui->chartView_3->setXAxisRange(2,maxFreq);
    //     ui->chartView_3->setYAxisRange(minRatio,60);
    //     ui->chartView_3->setHData(harmonicFreqList);

    //     ui->chartView_4->setXAxisRange(0,maxFreq);
    //     ui->chartView_4->setYAxisScale(minEnergy,maxEnergy,maxEnergy/5);
    //     ui->chartView_4->setHData(harmonicEnergyList);

    // });
}

//开始模拟数据
void MainWindow::on_pushButton_clicked()
{
    picohandle->moveToThread(simulationDataThread);
    ui->textEdit->append(picoParam->timeBaseObj.scope);
    ui->textEdit->append("时基值：" + QString::number(picoParam->timebaseValue));
    ui->textEdit->append("采样数：" + QString::number(picoParam->timeBaseObj.sampleCount));
    ui->textEdit->append("采样时间间隔：" + QString::number(picoParam->timeBaseObj.interval) + " " + picoParam->timeBaseObj.intervalUnit);
    ui->textEdit->append("开始模拟。");
    ui->textEdit->append("--------------");
    // 启动数据线程
    simulationDataThread->start();
}

//停止模拟数据
void MainWindow::on_pushButton_2_clicked()
{
    simulationDataThread->quit();
    if(!simulationDataThread->wait(3000)){
        simulationDataThread->terminate();
        simulationDataThread->wait();
        //ui->chartView->clearData();
    }
    picohandle->stopSimulation();
    appendCacheList();
    ui->chartView->clearData();
    ui->chartView_2->clearData();
    //ui->chartView->clearData();
    qDebug() << "stop simulation.";
    ui->textEdit->append("停止模拟数据。");
    ui->textEdit->append("--------------");
}

//选择时基
void MainWindow::on_cb_Timebase_currentIndexChanged(int index)
{
    if(ui->cb_Timebase->count() > 1){
        if(timeBaseList.size() > 0){
            TimeBase timeBaseObj = timeBaseList.at(index);
            double values = timeBaseObj.scale;
            ui->chartView->setXAxisScale(0 ,values*10 ,values);
            if(timeBaseObj.conversion){
                values = values / 1000;
                ui->chartView->setXAxisScale(0 ,values*10 ,values);
            }
            if(timeBaseObj.unit == "ns"){
                if(timeBaseObj.conversion){
                    ui->chartView->setXAxisTitle("Time (us)");
                }else{
                    ui->chartView->setXAxisTitle("Time (ns)");
                }
            }
            if(timeBaseObj.unit == "us"){
                if(timeBaseObj.conversion){
                    ui->chartView->setXAxisTitle("Time (ms)");
                }else{
                    ui->chartView->setXAxisTitle("Time (us)");
                }
            }
            if(timeBaseObj.unit == "ms"){
                if(timeBaseObj.conversion){
                    ui->chartView->setXAxisTitle("Time (s)");
                }else{
                    ui->chartView->setXAxisTitle("Time (ms)");
                }
            }
            if(timeBaseObj.unit == "s"){
                ui->chartView->setXAxisTitle("Time (s)");
            }
            picoParam->timeBaseObj = timeBaseObj;
            ui->sb_timebase->setValue(timeBaseObj.timebasevalue);
            //picoParam->timeBaseObj.timebasevalue = ui->sb_timebase->value();
            ui->textEdit->append("选择时基：");
            ui->textEdit->append(timeBaseObj.scope);
            ui->textEdit->append("------------------------------");

            ui->chartView_2->setXAxisTitle(picoParam->timeBaseObj.frequencyUnit);
            ui->chartView_2->setXAxisRange(0 ,picoParam->timeBaseObj.frequencyScope);
        }
    }
}
//选择电压
void MainWindow::on_cb_Voltage_currentIndexChanged(int index)
{
    if (ui->cb_Voltage->count() > 1) {
        double interval = 100;       // 默认间隔，单位为 mV
        double voltageRange = 500;    // 默认电压范围，单位为 mV
        QString AxisTitle = "Voltage (mV)";
        PS2000A_RANGE range = binderVoltage->getEnumValueAtIndex(index);

        switch (range) {
        case PS2000A_10MV:
            voltageRange = 10;    // 10 mV
            interval = 2.0;       // 2 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_20MV:
            voltageRange = 20;
            interval = 4.0;
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_50MV:
            voltageRange = 50;
            interval = 10.0;
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_100MV:
            voltageRange = 100;
            interval = 20.0;
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_200MV:
            voltageRange = 200;
            interval = 40.0;
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_500MV:
            voltageRange = 500;
            interval = 100.0;
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_1V:
            voltageRange = 1000;  // 1 V 转换为 mV
            interval = 200.0;     // 200 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_2V:
            voltageRange = 2000;  // 2 V 转换为 mV
            interval = 400.0;     // 400 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_5V:
            voltageRange = 5000;  // 5 V 转换为 mV
            interval = 1000.0;    // 1000 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_10V:
            voltageRange = 10000; // 10 V 转换为 mV
            interval = 2000.0;    // 2000 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_20V:
            voltageRange = 20000; // 20 V 转换为 mV
            interval = 4000.0;    // 4000 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        case PS2000A_50V:
            voltageRange = 50000; // 50 V 转换为 mV
            interval = 10000.0;   // 10000 mV 间隔
            AxisTitle = "Voltage (mV)";
            break;
        default:
            voltageRange = 500;    // 默认 500 mV
            interval = 100.0;
            AxisTitle = "Voltage (mV)";
            break;
        }

        ui->chartView->setYAxisTitle(AxisTitle);
        ui->chartView->setYAxisScale(-voltageRange, voltageRange, interval);

        picoParam->VoltageRange = range;

        ui->textEdit->append("选择幅值：");
        ui->textEdit->append(QString::number(voltageRange) + " mV");
        ui->textEdit->append("------------------------------");
    }
}

//单帧渲染
void MainWindow::on_pushButton_3_clicked()
{
    //picohandle->startSimulationSingle();
    picohandle->runSegmentedAcquisition(10);
    ui->textEdit->append(QString::number(picohandle->getSamples().size()));
    //fftData = singalConvert->performFFT(bufferedData ,frequencyUnit);
    //ui->chartView_2->setData(fftData);
}

//导出CSV
void MainWindow::on_pushButton_4_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.csv);;All Files (*)"));

    if (!filePath.isEmpty())
    {
        // 确保文件扩展名是.txt
        if (!filePath.endsWith(".csv", Qt::CaseInsensitive))
            filePath.append(".csv");

        //bool success = file.serializeToFile()
        //CSVLoader::saveDataToCSV(filePath,bufferedData);
        FileManager::saveCSV(filePath ,bufferedData);
    }
}
//加载CSV
void MainWindow::on_pushButton_5_clicked()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open CSV File", "", "pd Files (*.csv)");
    if (!fileName.isEmpty())
    {
        bufferedData.clear();
        FileManager::loadCSV(fileName ,bufferedData);
        // CSVLoader::loadDataFromCSV(fileName ,bufferedData);
        updateGraph(bufferedData);
    }

}

//保存计算结果
void MainWindow::on_pushButton_6_clicked()
{
    // 获取文件保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.csv);;All Files (*)"));
    if (!filePath.isEmpty())
    {
        FFTAnalyzer analyzer(1);
        analyzer.exportTableToCSV(ui->tableView2, filePath);
    }
}

//加载文件
void MainWindow::on_pushButton_7_clicked()
{
    bufferedData.clear();
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open PD File", "", "pd Files (*.pd)");
    if (!fileName.isEmpty())
    {
        bufferedData = FileManager::deserializeFromBinary(fileName);
        if(bufferedData.size() > 0){
            updateGraph(bufferedData);
        }else{
            QMessageBox::critical(this, tr("Error"), tr("文件打开错误。"));
        }
    }
}

//打开示波器
void MainWindow::on_pushButton_8_clicked()
{
    picohandle->moveToThread(dataThread);
    ui->textEdit->append(picoParam->timeBaseObj.scope);
    ui->textEdit->append("时基值：" + QString::number(picoParam->timebaseValue));
    ui->textEdit->append("采样数：" + QString::number(picoParam->timeBaseObj.sampleCount));
    ui->textEdit->append("采样时间间隔：" + QString::number(picoParam->timeBaseObj.interval) + " " + picoParam->timeBaseObj.intervalUnit);
    ui->textEdit->append("开始采集。");
    ui->textEdit->append("------------------------------");
    // 启动数据线程
    dataThread->start();
}

//停止采集
void MainWindow::on_pushButton_9_clicked()
{
    dataThread->quit();
    if(!dataThread->wait(3000)){
        dataThread->terminate();
        dataThread->wait();
        //ui->chartView->clearData();
    }
    picohandle->stopAcquisition();
    appendCacheList();
    //ui->chartView->clearData();
    qDebug() << "stop simulation.";
    ui->textEdit->append("停止采集。");
    ui->textEdit->append("------------------------------");
}

//加载数据到缓存列表
void MainWindow::appendCacheList(){
    isLoadCache = true;
    itemModel->clear();
    for(int i=0; i<picohandle->getCacheData().size(); ++i){
        QStandardItem *item = new QStandardItem("采样缓存" + QString::number(i + 1));
        itemModel->appendRow(item);
    }
    isLoadCache = false;
}

//选择时基调整值
void MainWindow::on_sb_timebase_valueChanged(int arg1)
{
    picoParam->timeBaseObj.timebasevalue = ui->sb_timebase->value();
    ui->textEdit->append("时基参数调整：");
    ui->textEdit->append(QString::number(picoParam->timeBaseObj.timebasevalue));
    ui->textEdit->append("------------------------------");
}

//点击缓存列表加载数据
void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    if(!isLoadCache){
        ui->tableView2->setModel(nullptr);
        if(picohandle->getCacheData().size() > 0){
            bufferedData = picohandle->getCacheData().at(index.row());
            updateGraph(bufferedData);

            QtConcurrent::run([=]{
                // FFTAnalyzer analyzer(2000);
                // double baseFreq = ui->harmonicParam->value();
                // QVector<QVector<QVariant>> results = analyzer.analyze(bufferedData,baseFreq);

                // // 初始化最大值和最小值
                // double maxFreq = std::numeric_limits<double>::lowest();
                // double minFreq = std::numeric_limits<double>::max();
                // double maxRatio = std::numeric_limits<double>::lowest();
                // double minRatio = std::numeric_limits<double>::max();
                // double maxEnergy = std::numeric_limits<double>::lowest();
                // double minEnergy = std::numeric_limits<double>::max();

                // //绑定tableView
                // HarmonicTableModel *tableModel = new HarmonicTableModel();
                // tableModel->setData(results);
                // ui->tableView2->setModel(tableModel);
                // ui->tableView2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                // ui->tableView2->verticalHeader()->hide();

                // QVector<QPointF> harmonicFreqList;
                // QVector<QPointF> harmonicEnergyList;
                // // 从 results 中提取数据并添加到系列
                // for (const auto& row : results) {
                //     double harmonicFreq = row[1].toDouble()/1000; // 谐波频率 (X 轴)
                //     double harmonicEnergy = row[2].toDouble()/1000; // 谐波占比 (Y 轴)
                //     double harmonicRatio = row[3].toDouble(); // 谐波占比 (Y 轴)

                //     // 更新最大值和最小值
                //     if (harmonicFreq > maxFreq) maxFreq = harmonicFreq;
                //     if (harmonicFreq < minFreq) minFreq = harmonicFreq;
                //     if (harmonicRatio > maxRatio) maxRatio = harmonicRatio;
                //     if (harmonicRatio < minRatio) minRatio = harmonicRatio;
                //     if(harmonicEnergy > maxEnergy) maxEnergy = harmonicEnergy;
                //     if(harmonicEnergy < minEnergy) minEnergy = harmonicEnergy;

                //     harmonicFreqList.append(QPointF(harmonicFreq, harmonicRatio));
                //     harmonicEnergyList.append(QPointF(harmonicFreq, harmonicEnergy));
                // }

                // ui->chartView_3->setXAxisRange(0,maxFreq);
                // ui->chartView_3->setYAxisRange(minRatio,60);
                // ui->chartView_3->setHData(harmonicFreqList);

                // ui->chartView_4->setXAxisRange(0,maxFreq);
                // ui->chartView_4->setYAxisScale(minEnergy,maxEnergy,maxEnergy/5);
                // ui->chartView_4->setHData(harmonicEnergyList);

            });
        }
    }
}

//保存文件
void MainWindow::on_pushButton_12_clicked()
{
    // 获取文件保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.pd);;All Files (*)"));
    if (!filePath.isEmpty())
    {
        // 确保文件扩展名是.txt
        if (!filePath.endsWith(".pd", Qt::CaseInsensitive))
            filePath.append(".pd");


        bool success = FileManager::serializeToBinary(filePath ,bufferedData);
        //bool success = fileHandle.serializeToFile(filePath ,bufferedData);
        if(success){
            QMessageBox::information(nullptr, "SAVE", "文件保存成功");
        }else{
            QMessageBox::critical(this, tr("Error"), tr("Failed to open file for writing."));
        }
    }
}

//加载缓存文件
void MainWindow::on_pushButton_10_clicked()
{
    // 弹出文件选择对话框，允许用户选择多个文件
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Select Files"),
                                                          "/",
                                                          tr("pd Files (*)"));

    if (!fileNames.isEmpty())
    {
        // 打开文件选择对话框
        picohandle->getCacheData().clear();
        for(QString fileName:fileNames){
            //qDebug() << "file :" << fileName;
            // QVector<QPointF> datas = fileHandle.deserializeFromFile(fileName);
            QVector<QPointF> datas = FileManager::deserializeFromBinary(fileName);
            picohandle->addCacheData(datas);
        }

        bufferedData = picohandle->getCacheData().at(0);
        updateGraph(bufferedData);
        itemModel->clear();

        for(int i = 0; i< picohandle->getCacheData().size(); ++i){
            QStandardItem *item = new QStandardItem("采样缓存" + QString::number(i + 1));
            itemModel->appendRow(item);
        }
    }
}

//导出缓存文件
void MainWindow::on_pushButton_11_clicked()
{
    bool success = false;
    QString filePath = QFileDialog::getExistingDirectory(this,tr("select folder"),"",QFileDialog::ShowDirsOnly);
    if(!filePath.isEmpty()){
        qDebug() << filePath;
        for(int i=0;i<picohandle->getCacheData().size();++i){
            QString binaryFilename = filePath + "/cache_data_" + QString::number(i)  + ".pd";
            // FileHandle fileHandle;
            // success = fileHandle.serializeToFile(binaryFilename ,picohandle->getCacheData()[i]);
            success = FileManager::serializeToBinary(binaryFilename ,picohandle->getCacheData()[i]);
            if(!success){
                QMessageBox::critical(this, tr("Error"), tr("Failed to save file for read."));
                break;
            }
        }
    }
}

// 清除缓存
void MainWindow::on_pushButton_13_clicked()
{
    picohandle->removeCacheData();
    itemModel->clear();
    ui->tableView2->setModel(nullptr);
}

//选择切换耦合方式
void MainWindow::on_cb_Couping_currentIndexChanged(int index)
{
    if (ui->cb_Couping->count() > 1) {

        enPS2000ACoupling couplint = binderCoupling->getEnumValueAtIndex(index);
        picoParam->Coupling = couplint;

        ui->textEdit->append("Couping：");
        ui->textEdit->append(QString::number(couplint));
        ui->textEdit->append("------------------------------");
    }
}


void MainWindow::on_pushButton_14_clicked()
{

}


void MainWindow::on_harmonicParam_valueChanged(int arg1)
{

}


void MainWindow::on_harmonicParam_editingFinished()
{
    // FFTAnalyzer analyzer(2000);
    // QVector<QVector<QVariant>> results = analyzer.analyze(bufferedData,ui->harmonicParam->value());

    // // 初始化最大值和最小值
    // double maxFreq = std::numeric_limits<double>::lowest();
    // double minFreq = std::numeric_limits<double>::max();
    // double maxRatio = std::numeric_limits<double>::lowest();
    // double minRatio = std::numeric_limits<double>::max();
    // double maxEnergy = std::numeric_limits<double>::lowest();
    // double minEnergy = std::numeric_limits<double>::max();

    // //绑定tableView
    // HarmonicTableModel *tableModel = new HarmonicTableModel();
    // tableModel->setData(results);
    // ui->tableView2->setModel(tableModel);
    // ui->tableView2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // ui->tableView2->verticalHeader()->hide();

    // QVector<QPointF> harmonicFreqList;
    // QVector<QPointF> harmonicEnergyList;
    // // 从 results 中提取数据并添加到系列
    // for (const auto& row : results) {
    //     double harmonicFreq = row[1].toDouble()/1000; // 谐波频率 (X 轴)
    //     double harmonicEnergy = row[2].toDouble()/1000; // 谐波占比 (Y 轴)
    //     double harmonicRatio = row[3].toDouble(); // 谐波占比 (Y 轴)

    //     // 更新最大值和最小值
    //     if (harmonicFreq > maxFreq) maxFreq = harmonicFreq;
    //     if (harmonicFreq < minFreq) minFreq = harmonicFreq;
    //     if (harmonicRatio > maxRatio) maxRatio = harmonicRatio;
    //     if (harmonicRatio < minRatio) minRatio = harmonicRatio;
    //     if(harmonicEnergy > maxEnergy) maxEnergy = harmonicEnergy;
    //     if(harmonicEnergy < minEnergy) minEnergy = harmonicEnergy;

    //     harmonicFreqList.append(QPointF(harmonicFreq, harmonicRatio));
    //     harmonicEnergyList.append(QPointF(harmonicFreq, harmonicEnergy));
    // }

    // ui->chartView_3->setXAxisRange(2,maxFreq);
    // ui->chartView_3->setYAxisRange(minRatio,60);
    // ui->chartView_3->setHData(harmonicFreqList);

    // ui->chartView_4->setXAxisRange(0,maxFreq);
    // ui->chartView_4->setYAxisScale(minEnergy,maxEnergy,maxEnergy/5);
    // ui->chartView_4->setHData(harmonicEnergyList);
}

//弹出后台采集窗口

void MainWindow::on_pushButton_15_clicked()
{
    harmonic *harmonicWin = new  harmonic();
    harmonicWin->show();
}

