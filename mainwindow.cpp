#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <csvloader.h>
#include <QStandardItemModel>
#include <QStandardItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dataThread(new QThread(this))
    , simulationDataThread(new QThread(this))
    , fftThread(new QThread(this))
{
    ui->setupUi(this);
    singalConvert = new SingalConvert;

    //加载时基配置
    timeBaseList = TimeBaseLoader::loadFromJson("TimeBase.json");
    for(const TimeBase &tb : timeBaseList){
        ui->cb_Timebase->addItem(tb.scope);
    }
    //默认频域图表单位
    frequencyUnit = 0;
    //频率单位
    ui->cb_HZUnit->addItem("Hz");
    ui->cb_HZUnit->addItem("KHz");
    ui->cb_HZUnit->addItem("MHz");
    ui->cb_HZUnit->currentIndexChanged(0);
    //采集卡参数
    picoParam = new PicoParam(
        timeBaseList.at(0) ,
        PS2000A_CHANNEL_A ,
        PS2000A_1V ,
        PS2000A_DC ,
        ui->sb_timebase->value(),
        1.0,
        64
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
        updateGraph(picohandle->getSamples());
    });

    connect(picohandle,&PicoScopeHandler::cacheDataUpdated ,this ,[](){
        //qDebug()<<"cacheDataUpdated";
    });

    ui->chartView->setTitle("时域图");
    ui->chartView->setXAxisTitle("Time (us)");
    ui->chartView->setYAxisTitle("Voltage (V)");
    ui->chartView->setLineColor(Qt::blue);
    ui->chartView->setLineWidth(1);
    ui->chartView->setXAxisRange(0,1);
    ui->chartView->setYAxisRange(-5,5);

    ui->cb_Timebase->setCurrentIndex(10);
    binderVoltage->setCurrentEnumValue(PS2000A_1V);
    binderCoupling->setCurrentEnumValue(PS2000A_DC);
    binderChannel->setCurrentEnumValue(PS2000A_CHANNEL_A);

    ui->chartView_2->setTitle("频域图");
    ui->chartView_2->setXAxisTitle("Hz");
    ui->chartView_2->setYAxisTitle("dBu");
    ui->chartView_2->setLineColor(Qt::blue);
    ui->chartView_2->setLineWidth(1);
    ui->chartView_2->setXAxisRange(0,350);
    ui->chartView_2->setYAxisRange(-200,200);

    tbRander = new tablerender(ui->tableView);
    headers.append("Frequency");
    headers.append("THD");

    picohandle->initialize();

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
    ui->chartView->setData(bufferedData);
    fftData = singalConvert->performFFT(bufferedData ,frequencyUnit);
    // qreal maxX = singalConvert->findMaxX(fftData);
    // qreal maxY = singalConvert->findMaxY(fftData);
    // ui->chartView_2->setXAxisScale(0,maxX,100.0);
    // ui->chartView_2->setYAxisScale(-(maxY+10),maxY+10,100.0);
    ui->chartView_2->setData(fftData);
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
    ui->textEdit->append("------------------------------");
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
    ui->textEdit->append("------------------------------");
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
        }
    }
}
//选择电压
void MainWindow::on_cb_Voltage_currentIndexChanged(int index)
{
    if(ui->cb_Voltage->count() > 1){
        double interval = 100;
        QString AxisTitle = "Voltage (mV)";
        PS2000A_RANGE range = binderVoltage->getEnumValueAtIndex(index);
        double voltageRange = 0;

        switch (range) {
        case PS2000A_10MV:
            voltageRange = 10;
            interval = 2.0;
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
            voltageRange = 1;
            interval = 0.2;
            AxisTitle = "Voltage (V)";
            break;
        case PS2000A_2V:
            voltageRange = 2;
            interval = 0.4;
            AxisTitle = "Voltage (V)";
            break;
        case PS2000A_5V:
            voltageRange = 5;
            interval = 1.0;
            AxisTitle = "Voltage (V)";
            break;
        case PS2000A_10V:
            voltageRange = 10;
            interval = 2.0;
            AxisTitle = "Voltage (V)";
            break;
        case PS2000A_20V:
            voltageRange = 20;
            interval = 4.0;
            AxisTitle = "Voltage (V)";
            break;
        case PS2000A_50V:
            voltageRange = 50;
            interval = 10.0;
            AxisTitle = "Voltage (V)";
            break;
        default:
            voltageRange = 500;// 默认500mV
            interval = 100.0;
            AxisTitle = "Voltage (mV)";
            break;
        }

        ui->chartView->setYAxisTitle(AxisTitle);
        ui->chartView->setYAxisScale(-voltageRange ,voltageRange ,interval);

        picoParam->VoltageRange = range;

        ui->textEdit->append("选择幅值：");
        ui->textEdit->append(QString::number(voltageRange));
        ui->textEdit->append("------------------------------");
    }
}

//单帧渲染
void MainWindow::on_pushButton_3_clicked()
{
    picohandle->startSimulationSingle();
    ui->textEdit->append(QString::number(picohandle->getSamples().size()));
    fftData = singalConvert->performFFT(bufferedData ,frequencyUnit);
    ui->chartView_2->setData(fftData);
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
        CSVLoader::saveDataToCSV(filePath,bufferedData);
    }
}
//加载CSV
void MainWindow::on_pushButton_5_clicked()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open CSV File", "", "pd Files (*.csv)");
    bufferedData.clear();
    CSVLoader::loadDataFromCSV(fileName ,bufferedData);
    updateGraph(bufferedData);
}

//选择频域单位
void MainWindow::on_cb_HZUnit_currentIndexChanged(int index)
{
    frequencyUnit = index;
    if(bufferedData.size() > 0){
        QVector<QPointF> fftData = singalConvert->performFFT(bufferedData ,frequencyUnit);
        ui->chartView_2->setData(fftData);
    }
}

//筛选频率单位
void MainWindow::on_pushButton_6_clicked()
{
    tbRander->render(fftData ,headers ,ui->beginBox->value() ,ui->endBox->value());
}

//加载文件
void MainWindow::on_pushButton_7_clicked()
{
    bufferedData.clear();

    FileHandle fileHandle;
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open PD File", "", "pd Files (*.pd)");
    if (!fileName.isEmpty())
    {
        bufferedData = fileHandle.deserializeFromFile(fileName);
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
    itemModel->clear();
    for(int i=0; i<picohandle->getCacheData().size(); ++i){
        QStandardItem *item = new QStandardItem("采样缓存" + QString::number(i + 1));
        itemModel->appendRow(item);
    }
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
    if(picohandle->getCacheData().size() > 0){
        bufferedData = picohandle->getCacheData().at(index.row());
        updateGraph(bufferedData);
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

        //bool success = file.serializeToFile()
        FileHandle fileHandle;
        bool success = fileHandle.serializeToFile(filePath ,bufferedData);
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
        FileHandle fileHandle;
        // 打开文件选择对话框
        picohandle->getCacheData().clear();
        for(QString fileName:fileNames){
            //qDebug() << "file :" << fileName;
            QVector<QPointF> datas = fileHandle.deserializeFromFile(fileName);
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
            FileHandle fileHandle;
            success = fileHandle.serializeToFile(binaryFilename ,picohandle->getCacheData()[i]);
            if(!success){
                QMessageBox::critical(this, tr("Error"), tr("Failed to save file for read."));
                break;
            }
        }
    }
}

