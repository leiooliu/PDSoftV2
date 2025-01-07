#include "harmonic.h"
#include "ui_harmonic.h"
#include "QString"
#include "QMessageBox"
#include "filemanager.h"
#include "QFileDialog.h"
#include <fftw3.h> // 使用已有的FFTW库
#include <chrono>

// 函数：计算频率
// 参数：
// - data: QVector<double>，存储原始的ADC数据
// - sample_rate: double，采样率（单位 Hz）
// - sampleCount: int，采样点数
// - sampleInterval: double，采样间隔（单位 ns）
// 返回值：double，计算得到的主频率（单位 Hz）
double harmonic::calculateFrequency(const QVector<double>& data, double sampleInterval) {
    // 将采样间隔从纳秒转换为秒
    sampleInterval *= 1e-9;
    int sampleCount = data.size();
    // 检查输入有效性
    if (data.isEmpty() || sampleCount <= 1 || sampleInterval <= 0) {
        return 0.0; // 无效输入返回0
    }

    // 使用 FFTW 进行快速傅里叶变换
    int N = sampleCount;
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, const_cast<double*>(data.data()), out, FFTW_ESTIMATE);

    // 执行 FFT
    fftw_execute(plan);

    // 计算频谱幅值并寻找最大频率分量
    double maxMagnitude = 0.0;
    int peakIndex = 0;
    for (int i = 0; i < N / 2; ++i) { // 只检查正频率
        double magnitude = out[i][0] * out[i][0] + out[i][1] * out[i][1]; // 实部和虚部平方和
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            peakIndex = i;
        }
    }

    // 释放 FFT 资源
    fftw_destroy_plan(plan);
    fftw_free(out);

    // 计算主频率
    double frequency = (double)peakIndex / (N * sampleInterval);
    return frequency;
}

harmonic::harmonic(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::harmonic),currentTimebase()
{
    ui->setupUi(this);

    //加载时基配置
    timeBaseList = TimeBaseLoader::loadFromJson("TimeBase.json");
    currentTimebase = timeBaseList.at(17);

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

    analyzer = new FFTAnalyzer(2000);
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
}

harmonic::~harmonic()
{
    delete ui;
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
    //qDebug() << log;
    // 获取当前时间
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    // 将时间戳和日志内容拼接
    QString logMessage = timestamp + " - " + log;
    ui->logTxt->append(logMessage);
}

void harmonic::updateProgress(int percentage)
{
    ui->pBar->setValue(percentage);  // 更新进度条
}

void harmonic::onRawDataReady(const QVector<double> &rawdata,double timeIntervalNanoseconds){
    recvLog("采样时间间隔：" + QString::number(timeIntervalNanoseconds) + " ns");
    _timeIntervalNanoseconds = timeIntervalNanoseconds;
    static bool alreadyProcessed = false;
    if (alreadyProcessed) {
        return; // 如果已处理数据，则直接返回，防止重复绘制
    }
    alreadyProcessed = true;
    bufferedRawData = rawdata;

    timeChart->render(rawdata,PS2000A_5V,currentTimebase);
    timeChart->run();

    double ns = calculateFrequency(bufferedRawData,timeIntervalNanoseconds);
    recvLog("信号频率：" + QString::number(ns));
    int roundedValue = static_cast<int>(std::round(ns));
    recvLog("信号频率(四舍五入)：" + QString::number(roundedValue));
    ui->spinBox->setValue(roundedValue);

    fftHandle->setRawDatas(&rawdata ,timeIntervalNanoseconds);
    fftHandle->run();

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
    // 将时间戳和日志内容拼接
    QString logMessage = timestamp + "_rawdata.rawpd";
    FileManager::serializeToBinary(logMessage ,rawdata);

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
void harmonic::renderTimeChartFinash(const QVector<QPointF> &data){
    bufferedData = data;
    recvLog("时域图表渲染完成");
    //fftHandle->setRawDatas(&bufferedRawData ,_timeIntervalNanoseconds);
    //fftHandle->run();
}

void harmonic::renderFrequencyChartFinash(const QVector<QPointF> &data){
    recvLog("频域图表渲染完成");
    analyzer->run();
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
        QVector<double> rawdata = FileManager::deserializeRawFromBinary(fileName);
        // 记录结束时间
        auto end = std::chrono::high_resolution_clock::now();
        // 计算时间差，并转换为毫秒
        std::chrono::duration<double, std::milli> elapsed = end - start;
        recvLog("反序列化原始数据文件时间："+QString::number(elapsed.count())+" ms");
        if(rawdata.size() > 0){
            timeChart->render(rawdata,PS2000A_5V,currentTimebase);
            timeChart->run();

            bufferedRawData = rawdata;

            double ns = calculateFrequency(bufferedRawData,3);
            int roundedValue = static_cast<int>(std::round(ns));
            recvLog("信号频率："+QString::number(roundedValue));
            ui->spinBox->setValue(roundedValue);

            //fftHandle->setDatas(bufferedRawData)
            fftHandle->setRawDatas(&rawdata ,timeBaseList.at(18).interval);
            fftHandle->run();
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
    if(!success){

    }
}
//关闭设备
void harmonic::on_pushButton_13_clicked()
{
    bool success = segmentHandle->close();
    if(!success){

    }
}

