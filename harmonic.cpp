#include "harmonic.h"
#include "ui_harmonic.h"
#include "QString"
#include "QMessageBox"
#include "filemanager.h"
#include "QFileDialog.h"
#include <fftw3.h> // 使用已有的FFTW库
#include <chrono>
#include <algorithm.h>
#include <singletest.h>

//加载配置项
void harmonic::loadSettings(){
    //加载软件基本配置
    configSetting = ConfigLoader::loadConfigFromJson("Config.Default.json");
    recvLog("Config.Default.json加载完成");
    //加载时基配置
    timeBaseList = TimeBaseLoader::loadFromJson("TimeBase.json");
    recvLog("TimeBase.json加载完成");

    if(ui->cb_Timebase->count() > 0){
        ui->cb_Timebase->clear();
    }

    for(const TimeBase &tb : timeBaseList){
        ui->cb_Timebase->addItem(tb.scope);
    }
}

harmonic::harmonic(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::harmonic),currentTimebase()
{
    //UI项先加载
    ui->setupUi(this);

    ui->pushButton_13->setEnabled(false);
    ui->pushButton_14->setEnabled(false);
    ui->pushButton->setEnabled(false);

    //加载配置项
    loadSettings();

    peakParam = PeakParam(ui->dsb_upLimit->value() ,ui->dsb_downLimit->value() ,
                                      ui->cb_userThresholdTigger->checkState());

    isRunning = false;

    // 处理数据，这里可以更新 UI 或进行其他操作
    timeChart = new RenderTimeChart(ui->timeChart);
    //连接信号和槽
    connect(timeChart, &RenderTimeChart::renderFinished, this, &harmonic::renderTimeChartFinash);
    connect(timeChart, &RenderTimeChart::sendLog ,this ,&harmonic::recvLog);

    renderFrequncyChart = new RenderFrequencyChart(ui->fChart);
    connect(renderFrequncyChart, &RenderFrequencyChart::sendLog ,this ,&harmonic::recvLog);
    // 连接信号和槽
    connect(renderFrequncyChart, &RenderFrequencyChart::renderFinished, this,  &harmonic::renderFrequencyChartFinash);

    polartChart = new RenderPolartChart(ui->polartChart);

    //绑定tableView
    tableModel = new HarmonicTableModel();

    analyzer = new FFTAnalyzer(configSetting.harmonicCalculateCount);
    connect(analyzer ,&FFTAnalyzer::dataReady,this,&harmonic::harmonicRunReady);
    connect(analyzer ,&FFTAnalyzer::progressUpdated,this,&harmonic::updateProgress);

    fftHandle = new  FFTHandle();
    connect(fftHandle ,&FFTHandle::fftReady,this,&harmonic::fftReady);
    connect(fftHandle ,&FFTHandle::porgressUpdated,this,&harmonic::updateProgress);
    connect(fftHandle ,&FFTHandle::samplingRateReady,this,&harmonic::samplingRateLoad);
    connect(fftHandle ,&FFTHandle::sendLog,this,&harmonic::recvLog);

    pulseFftHandle = new FFTHandle();
    connect(pulseFftHandle ,&FFTHandle::fftPluseReady,this,&harmonic::fftPluseReady);
    connect(pulseFftHandle ,&FFTHandle::porgressUpdated,this,&harmonic::updateProgress);
    connect(pulseFftHandle ,&FFTHandle::samplingRateReady,this,&harmonic::samplingRateLoad);
    connect(pulseFftHandle ,&FFTHandle::sendLog,this,&harmonic::recvLog);


    // 创建 SegmentHandle 线程对象
    segmentHandle = new SegmentHandle(PS2000A_5V, PS2000A_DC, PS2000A_CHANNEL_A,
                                      ui->le_segmentcount->text().toInt(), ui->le_samplecount->text().toInt(), 1, this);

    // 连接信号和槽
    connect(segmentHandle, &SegmentHandle::progressUpdated, this, &harmonic::updateProgress);
    //connect(segmentHandle, &SegmentHandle::finished, segmentHandle, &QObject::deleteLater);  // 清理资源
    connect(segmentHandle, &SegmentHandle::rawDataReady, this, &harmonic::onRawDataReady);
    connect(segmentHandle, &SegmentHandle::testDataReady, this, &harmonic::onTestDataReady);
    connect(segmentHandle, &SegmentHandle::sendLog, this, &harmonic::recvLog);

    setWin = new Settings;
    //链接设置窗口信号
    connect(setWin ,&Settings::setFinished ,this, &harmonic::settingFinshed);

    currentTimebase = timeBaseList.at(configSetting.defaultTimeBase);
    ui->cb_Timebase->setCurrentIndex(configSetting.defaultTimeBase);
    currentTimebaseIndex = configSetting.defaultTimeBase;

    //绑定参数
    binderVoltage =  EnumMap::getVoltageBuilder(ui->cb_Voltage);
    binderChannel = EnumMap::getChannelBuilder(ui->cb_Channel);
    binderCoupling = EnumMap::getCouplingBuilder(ui->cb_Couping);

    ui->cb_Voltage->setCurrentIndex(configSetting.defaultVoltage);
    cunnentRange = binderVoltage->getCurrentEnumValue();

    ui->cb_Couping->setCurrentIndex(configSetting.defaultCoupling);
    ui->cb_Channel->setCurrentIndex(configSetting.defaultChannel);
}

harmonic::~harmonic()
{
    delete ui;
}

void harmonic::settingFinshed(){
    loadSettings();
    ui->cb_Timebase->setCurrentIndex(configSetting.defaultTimeBase);
    ui->cb_Channel->setCurrentIndex(configSetting.defaultChannel);
    ui->cb_Voltage->setCurrentIndex(configSetting.defaultVoltage);
}

//开始后台采集
void harmonic::on_pushButton_clicked()
{
    updateProgress(0);
    on_pushButton_9_clicked();
    if(segmentHandle!=nullptr)
    {
        try{
            segmentHandle->changeTimebase(ui->le_timebase->text().toInt());
            segmentHandle->changeSamplesCount(ui->le_samplecount->text().toInt());
            segmentHandle->changeRange(cunnentRange);
            // 启动线程
            //segmentHandle->run();
            segmentHandle->loadData();
        }catch(const std::exception &e){
            recvLog(e.what());
        }
    }else{
        recvLog("segmentHandle is nullptr");
    }
}

void harmonic::recvLog(QString log){
    QTextCursor cursor = ui->logTxt->textCursor();
    cursor.movePosition(QTextCursor::Start ,QTextCursor::MoveAnchor);
    cursor.insertText(PDTools::createLogMsg(log)+"\n");
    ui->logTxt->setTextCursor(cursor);
    ui->logTxt->ensureCursorVisible();
    //ui->logTxt->append(PDTools::createLogMsg(log));
}

void harmonic::updateProgress(int percentage)
{
    ui->pBar->setValue(percentage);  // 更新进度条
}

void harmonic::onTestDataReady(const QVector<double> &rawdata,double timeIntervalNanoseconds){
    recvLog("开启采样检测 ... ... ");
    QVector<QPointF> testData;
    double minVolts;
    double maxVolts;
    Algorithm::calculateTimeData(rawdata ,cunnentRange ,currentTimebase ,testData ,minVolts ,maxVolts);
    double ns = Algorithm::calculateFrequency(rawdata,timeIntervalNanoseconds,ui->dsb_offset->value());
    singletest *s = new singletest();
    recvLog("预设值 ... ...");
    s->setData(ns ,minVolts ,maxVolts);
    s->show();

    connect(s, &singletest::setReady, this, &harmonic::onTestReady);
}

void harmonic::onTestReady(double ns ,double minVolts ,double maxVolts){
    ui->dsb_downLimit->setValue(std::abs(minVolts));
    ui->dsb_upLimit->setValue(std::abs(maxVolts));
    ui->spinBox->setValue(ns);
    ui->cb_userThresholdTigger->setChecked(true);
}

void harmonic::onRawDataReady(const QVector<double> &rawdata,double timeIntervalNanoseconds){
    recvLog("采样时间间隔：" + QString::number(timeIntervalNanoseconds) + " ns");

    _timeIntervalNanoseconds = timeIntervalNanoseconds;
    currentTimebase.interval = _timeIntervalNanoseconds;

    static bool alreadyProcessed = false;
    if (alreadyProcessed) {
        return; // 如果已处理数据，则直接返回，防止重复绘制
    }
    alreadyProcessed = true;
    bufferedRawData = rawdata;

    peakParam.up_volts_threshold = ui->dsb_upLimit->value();
    peakParam.down_volts_threshold = ui->dsb_downLimit->value();
    peakParam.isShow = ui->cb_userThresholdTigger->checkState();

    timeChart->render(rawdata,cunnentRange,currentTimebase,peakParam);
    timeChart->run();

    if(configSetting.autoCalculateSingalFreq){
        double ns = Algorithm::calculateFrequency(bufferedRawData,timeIntervalNanoseconds,ui->dsb_offset->value());
        //过0点
        //double nsZero = calculateFrequencyByZero(bufferedRawData ,timeIntervalNanoseconds);

        recvLog("信号频率：" + QString::number(ns));
        ui->spinBox->setValue(ns);
    }

    // if(configSetting.autoRenderFrequency){
    //     fftHandle->setRawDatas(&rawdata ,timeIntervalNanoseconds);
    //     fftHandle->run();
    // }

    if(configSetting.autoSaveRawData){
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
        // 将时间戳和日志内容拼接
        QString logMessage = configSetting.autoSaveFolder + timestamp + "_rawdata.rawpd";

        //等待测试
        //FileManager::serializeToBinary(logMessage ,rawdata);
        FileManager::serializeToBinary(logMessage ,rawdata ,cunnentRange ,
                                       ui->le_timebase->text().toInt() ,timeIntervalNanoseconds,ui->cb_Timebase->currentIndex(),
                                       ui->dsb_upLimit->value() ,
                                       ui->dsb_downLimit->value() ,
                                       ui->cb_userThresholdTigger->checkState());
    }

    alreadyProcessed = false; // 允许下次处理
}

void harmonic::samplingRateLoad(double samplingRate){
    static bool alreadyProcessed = false;
    if (alreadyProcessed) {
        return; // 如果已处理数据，则直接返回，防止重复绘制
    }
    alreadyProcessed = true;
    ui->spinBox->setValue(samplingRate);
    alreadyProcessed = false; // 允许下次处理
}

void harmonic::onDataReady(const QVector<QPointF> &data){
}
//时域图表渲染完成
void harmonic::renderTimeChartFinash(const QVector<QPointF> &data,const QVector<QPointF> &pulseDatas ,double timeMultiplier){
    bufferedData = data;
    recvLog("时域图表渲染完成");

    if(configSetting.autoRenderFrequency){
        fftHandle->setDatas(&bufferedData ,timeMultiplier);
        //fftHandle->setRawDatas(&rawdata ,timeIntervalNanoseconds);
        fftHandle->run();
    }

    // //只绘制两个点
    // QVector<QPointF> tempDatas;
    // for(int i=0;i<2;++i)
    // {
    //     tempDatas.append(pulseDatas.at(i));
    // }
    // //绘制相位图
    // pulseFftHandle->setPulseDatas(&tempDatas,timeMultiplier);

    //绘制相位图
    pulseFftHandle->setPulseDatas(&pulseDatas,timeMultiplier);

    //polartChart->setRenderData(magnitudes ,phases);
    //polartChart->run();

    // if(!configSetting.autoCalculateHarmonicResult &&
    //     !configSetting.autoRenderFrequency)
    // {
    //     //延迟执行采集代码
    //     QTimer::singleShot(configSetting.autoLoadDelay, [this]() {
    //         //qDebug() << "两秒后执行的代码";
    //         on_pushButton_clicked();
    //     });
    // }
    //fftHandle->setRawDatas(&bufferedRawData ,_timeIntervalNanoseconds);
    //fftHandle->run();
}

void harmonic::renderFrequencyChartFinash(const QVector<QPointF> &data){
    recvLog("频域图表渲染完成");
    if(configSetting.autoCalculateHarmonicResult){
        analyzer->run();
    }else{
        //延迟执行采集代码
        QTimer::singleShot(configSetting.autoLoadDelay, [this]() {
            //qDebug() << "两秒后执行的代码";
            on_pushButton_clicked();
        });
    }
}

//保存数据
void harmonic::on_pushButton_4_clicked()
{
    if(bufferedData.size() > 0){
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
    }else{
        QMessageBox::information(nullptr, "error", "数据缓存为空，请重新采集");
    }
}

//计算谐波数据
void harmonic::on_pushButton_5_clicked()
{
    // 记录起始时间
    auto start = std::chrono::high_resolution_clock::now();
    //analyzer->analyze(&bufferedData,baseFreq);

    analyzer->run();
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算时间差，并转换为毫秒
    std::chrono::duration<double, std::milli> elapsed = end - start;
    // QMessageBox::information(nullptr, "", "谐波计算时间：" + QString::number(elapsed.count()));
    qDebug()<<"谐波计算时间："<<elapsed.count()<<"毫秒";
}

void harmonic::harmonicRunReady(const QVector<QVector<QVariant>> result){
    tableModel->setData(result);
    ui->tableView->setModel(tableModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->hide();
    recvLog("谐波计算完成");

    //如果自动采集被打开
    if(isRunning){
        //延迟执行采集代码
        QTimer::singleShot(configSetting.autoLoadDelay, [this]() {
            //qDebug() << "两秒后执行的代码";
            on_pushButton_clicked();
        });
    }
}

void harmonic::fftReady(std::vector<double> frequencies,std::vector<double> magnitudes,std::vector<double> phases){
    static bool alreadyProcessed = false;

    if (alreadyProcessed) {
        return; // 如果已处理数据，则直接返回，防止重复绘制
    }
    alreadyProcessed = true;
    recvLog("FFT运算完成");
    renderFrequncyChart->render(frequencies ,magnitudes ,"MHz");
    renderFrequncyChart->start();
    alreadyProcessed = false; // 允许下次处理

    double baseFreq = ui->spinBox->value();
    analyzer->setData(frequencies ,magnitudes ,baseFreq);
}

void harmonic::fftPluseReady(std::vector<double> frequencies,std::vector<double> magnitudes,std::vector<double> phases){
    static bool alreadyProcessed = false;

    if (alreadyProcessed) {
        return; // 如果已处理数据，则直接返回，防止重复绘制
    }
    alreadyProcessed = true;

    polartChart->setRenderData(magnitudes ,phases);
    polartChart->run();

    alreadyProcessed = false;
}

//上一页
void harmonic::on_pushButton_2_clicked()
{
    tableModel->previousPage();
}

//下一页
void harmonic::on_pushButton_3_clicked()
{
    tableModel->nextPage();
}

//加载数据
void harmonic::on_pushButton_7_clicked()
{
    bufferedData.clear();
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open PD File", "", "pd Files (*.pd)");
    if (!fileName.isEmpty())
    {
        bufferedData = FileManager::deserializeFromBinary(fileName);
        if(bufferedData.size() > 0){
            timeChart->render(bufferedData ,currentTimebase.unit);
            timeChart->start();
        }else{
            QMessageBox::critical(this, tr("Error"), tr("文件打开错误。"));
        }
    }
}

//导出表格数据到csv
void harmonic::on_pushButton_8_clicked()
{
    // 获取文件保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.csv);;All Files (*)"));
    if (!filePath.isEmpty())
    {
        //analyzer->exportTableToCSV()
        analyzer->exportTableToCSV(ui->tableView, filePath);
    }
}

//清空数据
void harmonic::on_pushButton_9_clicked()
{
    bufferedData.clear();
    ui->tableView->setModel(nullptr);
    renderFrequncyChart->clear();
    timeChart->clear();
}

void harmonic::clearPolartChart(){
    polartChart->clear();
}

//加载原始数据（测试）
void harmonic::on_pushButton_6_clicked()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open PD File", "", "pd Files (*.rawpd)");
    if (!fileName.isEmpty())
    {
        on_pushButton_9_clicked();
        // 记录起始时间
        auto start = std::chrono::high_resolution_clock::now();
        //QVector<double> rawdata = FileManager::deserializeRawFromBinary(fileName);
        //等待测试
        QVector<double> rawdata ;
        int timebaseValue = 0;
        double sampleInterval = 0.0;
        int cb_Timebase_index = 0;

        double up_limit;
        double down_limit;
        bool userThresholdTigger;

        FileManager::deserializeFromBinary(fileName ,rawdata ,cunnentRange ,timebaseValue ,sampleInterval,cb_Timebase_index,
                                           up_limit ,down_limit ,userThresholdTigger);

        ui->cb_Timebase->setCurrentIndex(cb_Timebase_index);
        currentTimebaseIndex = cb_Timebase_index;

        currentTimebase = timeBaseList.at(currentTimebaseIndex);
        currentTimebase.interval = sampleInterval;

        binderVoltage->setCurrentEnumValue(cunnentRange);
        //ui->le_timebase->setText(QString::number(timebaseValue));
        ui->le_timebase->setValue(timebaseValue);
        ui->le_samplecount->setText(QString::number(rawdata.size()));

        // 记录结束时间
        auto end = std::chrono::high_resolution_clock::now();
        // 计算时间差，并转换为毫秒
        std::chrono::duration<double, std::milli> elapsed = end - start;
        recvLog("反序列化原始数据文件时间："+QString::number(elapsed.count())+" ms");
        if(rawdata.size() > 0){

            peakParam.up_volts_threshold = up_limit;
            peakParam.down_volts_threshold = down_limit;
            peakParam.isShow = userThresholdTigger;

            //ui->cb_userThresholdTigger->setCheckState(userThresholdTigger);
            ui->dsb_downLimit->setValue(down_limit);
            ui->dsb_upLimit->setValue(up_limit);
            ui->cb_userThresholdTigger->setChecked(userThresholdTigger);

            timeChart->render(rawdata,cunnentRange,currentTimebase,peakParam);
            timeChart->run();

            bufferedRawData = rawdata;

            //判断信号里是否存在谐波
            //detectHarmonics(bufferedRawData,0.05);

            //double nsZero = calculateFrequencyByZero(bufferedRawData ,currentTimebase.interval);
            //recvLog("过零点信号频率：" + QString::number(nsZero));
            double ns = Algorithm::calculateFrequency(bufferedRawData,sampleInterval,ui->dsb_offset->value());

            //double ns = Algorithm::calculateFrequency(bufferedRawData ,sampleInterval);
            recvLog("信号频率："+QString::number(ns));
            ui->spinBox->setValue(ns);

            //fftHandle->setDatas(bufferedRawData)
            //fftHandle->setRawDatas(&rawdata ,currentTimebase.interval);
            //fftHandle->run();
        }else{
            QMessageBox::critical(this, tr("Error"), tr("文件打开错误。"));
        }
    }
}

//原始数据导出cvs格式的文本
void harmonic::on_pushButton_10_clicked()
{
    // 获取文件保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.csv);;All Files (*)"));
    if (!filePath.isEmpty())
    {
        // 确保文件扩展名是.txt
        if (!filePath.endsWith(".csv", Qt::CaseInsensitive))
            filePath.append(".csv");

        if(bufferedRawData.size() > 0){
            bool success = FileManager::saveRawCSV(filePath ,bufferedRawData);
            if(success){
                QMessageBox::information(nullptr, "SAVE", "文件保存成功");
            }else{
                QMessageBox::critical(this, tr("Error"), tr("Failed to open file for writing."));
            }
        }else{
             QMessageBox::critical(this, tr("Error"), tr("并无原始缓存数据。"));
        }
    }
}

//数据保存为csv
void harmonic::on_pushButton_11_clicked()
{
    // 获取文件保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.csv);;All Files (*)"));
    if (!filePath.isEmpty())
    {
        // 确保文件扩展名是.txt
        if (!filePath.endsWith(".csv", Qt::CaseInsensitive))
            filePath.append(".csv");

        if(bufferedData.size() > 0){
            bool success = FileManager::saveCSV(filePath ,bufferedData);
            if(success){
                QMessageBox::information(nullptr, "SAVE", "文件保存成功");
            }else{
                QMessageBox::critical(this, tr("Error"), tr("Failed to open file for writing."));
            }
        }else{
            QMessageBox::critical(this, tr("Error"), tr("并无原始缓存数据。"));
        }
    }
}


//打开设备
void harmonic::on_pushButton_12_clicked()
{
    bool success = segmentHandle->open();

    segmentHandle->changeTimebase(ui->le_timebase->text().toInt());
    segmentHandle->changeSamplesCount(ui->le_samplecount->text().toInt());
    segmentHandle->changeRange(cunnentRange);
    segmentHandle->loadTestData();

    if(success){
        ui->pushButton_12->setEnabled(false);
        ui->pushButton_13->setEnabled(true);

        ui->pushButton_14->setEnabled(true);
        ui->pushButton->setEnabled(true);
    }
}
//关闭设备
void harmonic::on_pushButton_13_clicked()
{
    bool success = segmentHandle->close();
    if(success){
        ui->pushButton_12->setEnabled(true);
        ui->pushButton_13->setEnabled(false);

        ui->pushButton_14->setEnabled(false);
        ui->pushButton->setEnabled(false);
    }
}

//开始，触发自动采集
void harmonic::on_pushButton_14_clicked()
{
    if(!isRunning){
        isRunning = true;
        //出发采集功能
        on_pushButton_clicked();
        ui->pushButton_14->setText("停止采集");
    }else{
        isRunning = false;
        ui->pushButton_14->setText("自动采集");
    }
}

//选择div/time 显示参数
void harmonic::on_cb_Timebase_currentIndexChanged(int index)
{
    if(ui->cb_Timebase->count() > 1){
        if(timeBaseList.size() > 0){
            currentTimebase = timeBaseList.at(index);
            //ui->le_timebase->setText(QString::number(currentTimebase.timebasevalue));
            ui->le_timebase->setValue(currentTimebase.timebasevalue);
            ui->le_samplecount->setText(QString::number(currentTimebase.sampleCount));

            if(!isRunning && bufferedRawData.size() > 0){
                timeChart->render(bufferedRawData,cunnentRange,currentTimebase,peakParam);
                timeChart->run();
            }else{
                timeChart->changeX(currentTimebase);
            }
            currentTimebaseIndex = index;
        }
    }
}

//选择电压幅值范围
void harmonic::on_cb_Voltage_currentIndexChanged(int index)
{
    if(ui->cb_Voltage->count() > 1){
        ui->cb_Voltage->setCurrentIndex(index);
        cunnentRange = binderVoltage->getCurrentEnumValue();
        timeChart->changeY(cunnentRange);
        // if(!isRunning && bufferedRawData.size() > 0){
        //     //timeChart->changeY(cunnentRange);
        //     timeChart->render(bufferedRawData,cunnentRange,currentTimebase);
        //     timeChart->run();
        // }else{
        //     timeChart->changeY(cunnentRange);
        // }
    }
}


void harmonic::on_dsb_time_x_max_valueChanged(double arg1)
{
    double xMin = ui->dsb_time_x_min->value();
    double xMax = arg1;
    timeChart->setXRange(xMin ,xMax);
}


void harmonic::on_dsb_time_x_min_valueChanged(double arg1)
{
    double xMin = arg1;
    double xMax = ui->dsb_time_x_max->value();;
    timeChart->setXRange(xMin ,xMax);
}


void harmonic::on_dsb_frequency_x_max_valueChanged(double arg1)
{
    double xMin = ui->dsb_frequency_x_min->value();
    double xMax = arg1;
    renderFrequncyChart->setXRange(xMin ,xMax);
}


void harmonic::on_dsb_frequency_x_min_valueChanged(double arg1)
{
    double xMin = arg1;
    double xMax = ui->dsb_frequency_x_max->value();
    renderFrequncyChart->setXRange(xMin ,xMax);
}

//重新加载配置项
void harmonic::on_reloadConfig_btn_clicked()
{
    loadSettings();
}

//点击是否使用触发器
void harmonic::on_checkBox_stateChanged(int arg1)
{
    qDebug() << arg1;
    if(arg1 == 0){
        segmentHandle->useTriggers(false);
    }else{
        segmentHandle->useTriggers(true);
    }

}

//打开系统设置
void harmonic::on_pushButton_17_clicked()
{
    // QVector<QPointF> datas;
    // timeChart->setPeakTiggerData(datas);
    setWin->SetConfig(&configSetting,timeBaseList);
    setWin->show();
}

//清空相位图图表
void harmonic::on_pushButton_15_clicked()
{
    clearPolartChart();
}

