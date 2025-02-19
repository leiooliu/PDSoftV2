#include "harmonic.h"
#include "ui_harmonic.h"
#include "QString"
#include "QMessageBox"
#include "filemanager.h"
#include "QFileDialog.h"
#include <fftw3.h> // 使用已有的FFTW库
#include <chrono>

double harmonic::calculateFrequencyByZero(const QVector<double>& data, double samplingInterval)
{
    samplingInterval *= 1e-9; // 纳秒转秒
    QVector<int> zeroCrossings; // 用于存储过零点的索引

    // 遍历信号数据，找到过零点
    for (int i = 1; i < data.size(); ++i) {
        if ((data[i-1] > 0 && data[i] <= 0) || (data[i-1] < 0 && data[i] >= 0)) {
            // 检测到过零点（从正到负，或从负到正）
            zeroCrossings.append(i);
        }

        // 如果已经找到三个过零点，则停止查找
        if (zeroCrossings.size() >= 3) {
            break;
        }
    }

    // 如果没有找到三个过零点，返回0
    if (zeroCrossings.size() < 3) {
        recvLog("没有足够的过零点。");
        return 0.0;
    }

    // 计算前三个过零点之间的周期，并计算频率
    double totalPeriod = 0.0;
    for (int i = 0; i < 2; ++i) {
        int deltaN = zeroCrossings[i+1] - zeroCrossings[i];
        double period = deltaN * samplingInterval;  // 计算周期
        totalPeriod += period;
    }

    // 计算平均周期
    double averagePeriod = totalPeriod / 2; // 取两段周期的平均值
    double frequency = 1.0 / averagePeriod;  // 频率是周期的倒数

    return frequency;
}

// 函数：计算频率
// 参数：
// - data: QVector<double>，存储原始的ADC数据
// - sample_rate: double，采样率（单位 Hz）
// - sampleCount: int，采样点数
// - sampleInterval: double，采样间隔（单位 ns）
// 返回值：double，计算得到的主频率（单位 Hz）
double harmonic::calculateFrequency(const QVector<double>& data, double sampleInterval) {
    sampleInterval *= 1e-9; // 纳秒转秒
    int N = data.size();
    if (data.isEmpty() || N <= 1 || sampleInterval <= 0) {
        return 0.0;
    }

    // 分配内存
    int outSize = N/2 + 1;
    double* in = fftw_alloc_real(N);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * outSize);

    // 使用汉宁窗，计算窗函数总和
    double sumWindow = 0.0;
    for (int i = 0; i < N; ++i) {
        double window = 0.5 * (1 - cos(2 * M_PI * i / (N-1)));

        //对瞬态信号改用平顶窗口
        // double window = 0.21557895
        //                 - 0.41663158 * cos(2*M_PI*i/(N-1))
        //                 + 0.277263158 * cos(4*M_PI*i/(N-1))
        //                 - 0.083578947 * cos(6*M_PI*i/(N-1))
        //                 + 0.006947368 * cos(8*M_PI*i/(N-1));

        in[i] = data[i] * window;
        sumWindow += window;
    }

    // 创建并执行FFT计划
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    // 跳直流，找最大幅值
    double maxMagnitude = 0.0;
    int peakIndex = 0;
    for (int i = 1; i < outSize; ++i) { // 从1开始跳直流
        // 计算幅值并补偿（汉宁窗补偿）
        double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]) * 2.0 / sumWindow;

        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            peakIndex = i;
        }
    }

    // 抛物线插值优化
    double Fs = 1.0 / sampleInterval;
    double frequency = peakIndex * Fs / N;

    if (peakIndex > 0 && peakIndex < outSize-1) {
        // 获取相邻频点幅值（需重新计算补偿）
        double magPrev = sqrt(out[peakIndex-1][0]*out[peakIndex-1][0] +
                              out[peakIndex-1][1]*out[peakIndex-1][1]) * 2.0 / sumWindow;
        double magNext = sqrt(out[peakIndex+1][0]*out[peakIndex+1][0] +
                              out[peakIndex+1][1]*out[peakIndex+1][1]) * 2.0 / sumWindow;

        // 算频率偏移量
        double delta = 0.5 * (magPrev - magNext) / (magPrev - 2*maxMagnitude + magNext);
        frequency = (peakIndex + delta) * Fs / N;
    }

    // 释放资源
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return frequency;
}
//判断信号中是否存在谐波
bool harmonic::detectHarmonics(const QVector<double>& adcData, double threshold) {
    int n = adcData.size();
    if (n == 0) return false;

    // 创建FFT输入输出数据
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n);

    // 将QVector中的数据复制到FFT输入
    for (int i = 0; i < n; ++i) {
        in[i][0] = adcData[i];  // 实部
        in[i][1] = 0.0;  // 虚部
    }

    // 创建FFT计划
    fftw_plan p = fftw_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // 执行FFT
    fftw_execute(p);

    // 分析频域数据
    double maxMagnitude = 0.0;  // 记录最大幅值
    double averageMagnitude = 0.0; // 记录平均幅值
    int harmonicDetected = false;

    for (int i = 1; i < n / 2; ++i) {
        double real = out[i][0];
        double imag = out[i][1];
        double magnitude = std::sqrt(real * real + imag * imag) / n;

        maxMagnitude = std::max(maxMagnitude, magnitude);
        averageMagnitude += magnitude;

        // 检测基频和谐波成分
        if (magnitude > threshold) {
            recvLog("Potential harmonic detected at frequency index: "+QString::number(i)+"  with magnitude: "+QString::number(magnitude));
            harmonicDetected = true;
        }
    }

    // 计算平均幅值
    //averageMagnitude /= (n / 2);

    // 输出调试信息
    //recvLog("Max Magnitude: " + QString::number(maxMagnitude) + ", Average Magnitude: " + QString::number(averageMagnitude));

    // 清理FFT资源
    fftw_free(in);
    fftw_free(out);
    fftw_destroy_plan(p);

    return harmonicDetected;
}

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

    // 创建 SegmentHandle 线程对象
    segmentHandle = new SegmentHandle(PS2000A_5V, PS2000A_DC, PS2000A_CHANNEL_A,
                                      ui->le_segmentcount->text().toInt(), ui->le_samplecount->text().toInt(), 1, this);

    // 连接信号和槽
    connect(segmentHandle, &SegmentHandle::progressUpdated, this, &harmonic::updateProgress);
    //connect(segmentHandle, &SegmentHandle::finished, segmentHandle, &QObject::deleteLater);  // 清理资源
    connect(segmentHandle, &SegmentHandle::rawDataReady, this, &harmonic::onRawDataReady);
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


    timeChart->render(rawdata,cunnentRange,currentTimebase);
    timeChart->run();

    if(configSetting.autoCalculateSingalFreq){
        double ns = calculateFrequency(bufferedRawData,timeIntervalNanoseconds);
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
                                       ui->le_timebase->text().toInt() ,timeIntervalNanoseconds,ui->cb_Timebase->currentIndex());
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
void harmonic::renderTimeChartFinash(const QVector<QPointF> &data ,double timeMultiplier){
    bufferedData = data;
    recvLog("时域图表渲染完成");


    if(configSetting.autoRenderFrequency){
        fftHandle->setDatas(&bufferedData ,timeMultiplier);
        //fftHandle->setRawDatas(&rawdata ,timeIntervalNanoseconds);
        fftHandle->run();
    }

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

void harmonic::fftReady(std::vector<double> frequencies,std::vector<double> magnitudes){
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

        FileManager::deserializeFromBinary(fileName ,rawdata ,cunnentRange ,timebaseValue ,sampleInterval,cb_Timebase_index);

        ui->cb_Timebase->setCurrentIndex(cb_Timebase_index);
        currentTimebaseIndex = cb_Timebase_index;

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
            timeChart->render(rawdata,cunnentRange,currentTimebase);
            timeChart->run();

            bufferedRawData = rawdata;

            //判断信号里是否存在谐波
            //detectHarmonics(bufferedRawData,0.05);

            //double nsZero = calculateFrequencyByZero(bufferedRawData ,currentTimebase.interval);
            //recvLog("过零点信号频率：" + QString::number(nsZero));
            double ns = calculateFrequency(bufferedRawData,sampleInterval);
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
                timeChart->render(bufferedRawData,cunnentRange,currentTimebase);
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

//筛选时域图的x轴
void harmonic::on_pushButton_15_clicked()
{
    double xMin = ui->dsb_time_x_min->value();
    double xMax = ui->dsb_time_x_max->value();
    timeChart->setXRange(xMin ,xMax);
}

//筛选频率图的x轴
void harmonic::on_pushButton_16_clicked()
{
    double xMin = ui->dsb_frequency_x_min->value();
    double xMax = ui->dsb_frequency_x_max->value();
    renderFrequncyChart->setXRange(xMin ,xMax);
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
    setWin->SetConfig(&configSetting,timeBaseList);
    setWin->show();
}

