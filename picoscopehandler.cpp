#include "picoscopehandler.h"
#include <QMessageBox>
#include "PicoStatus.h"
PicoScopeHandler::PicoScopeHandler(QObject *parent):
    QObject(parent),
    timer(new QTimer(this)),
    simulationTimer(new QTimer(this))
{
    //链接计时器
    connect(timer, &QTimer::timeout, this, &PicoScopeHandler::onTimerUpdate);
    //链接模拟计时器
    connect(simulationTimer ,&QTimer::timeout ,this ,&PicoScopeHandler::onSimulationTimerUpdate);
    acquisitionInProgress = false;
}
//初始化设备
void PicoScopeHandler::initialize(){
    status = ps2000aOpenUnit(&handle ,NULL);
    if(status != PICO_OK){
        QMessageBox::information(nullptr ,"Error" ,"设备未打开，错误代码：" + QString::number(status));
        return;
    }

    //setChannel();
    //configureTimebase();
}
//设置频道
void PicoScopeHandler::setChannel(){
    if(picoParam != nullptr){
        status = ps2000aSetChannel(handle ,picoParam->Channel ,1 ,picoParam->Coupling ,picoParam->VoltageRange ,0);
        if(status != PICO_OK){
            QMessageBox::information(nullptr ,"Error" ,"频道未设置，错误代码：" + QString::number(status));
            return;
        }
    }else{
        QMessageBox::information(nullptr ,"Error" ,"参数类型未空，请检查参数对象");
        return;
    }
}
//设置时基础和数据缓存
void PicoScopeHandler::configureTimebase(){
    if(picoParam != nullptr){
        //status = ps2000aGetTimebase(handle ,picoParam->timeBaseObj.timebasevalue ,0 ,&timeIntervalNanoseconds, 0,&maxSamples ,0);
        int16_t oversample = 1;
        status = ps2000aGetTimebase2(handle,
                                     picoParam->timeBaseObj.timebasevalue,
                                     picoParam->timeBaseObj.sampleCount,
                                     &timeIntervalNanoseconds,
                                     oversample,
                                     &maxSamples,
                                     0);
        if(status != PICO_OK){
            QMessageBox::information(nullptr ,"Error" ,"时基获取错误，错误代码：" + QString::number(status));
            return;
        }

        //计算采样率，单位Hz
        picoParam->samplingRate = 1e9 / timeIntervalNanoseconds;

        //设置采样数
        bufferA = std::make_unique<int16_t[]>(picoParam->timeBaseObj.sampleCount);
        status = ps2000aSetDataBuffer(handle ,picoParam->Channel ,bufferA.get() ,picoParam->timeBaseObj.sampleCount ,0 ,PS2000A_RATIO_MODE_NONE);
        if(status != PICO_OK){
            QMessageBox::information(nullptr ,"Error" ,"数据缓存设置错误，错误代码：" + QString::number(status));
            return;
        }
    }else{
        QMessageBox::information(nullptr ,"Error" ,"参数类型未空，请检查参数对象");
        return;
    }
}
void PicoScopeHandler::runBlock(){
    if(picoParam != nullptr){
        status = ps2000aRunBlock(handle ,0 ,picoParam->timeBaseObj.sampleCount ,picoParam->timeBaseObj.timebasevalue ,NULL ,0 ,NULL ,NULL ,NULL);
        if(status != PICO_OK){
            QMessageBox::information(nullptr ,"Error" ,"设置运行块出错，错误代码：" + QString::number(status));
            acquisitionInProgress = false;
            return;
        }
    }else{
        QMessageBox::information(nullptr ,"Error" ,"参数类型未空，请检查参数对象");
        acquisitionInProgress = false;
        return;
    }
}
void PicoScopeHandler::runAcquisition(){
    runBlock();
    int16_t ready = 0;
    while(!ready){
        status = ps2000aIsReady(handle ,&ready);
        if(status != PICO_OK){
            QMessageBox::information(nullptr ,"Error" ,"ready错误，错误代码：" + QString::number(status));
            acquisitionInProgress = false;
            return;
        }
    }
}
//模拟采集进程
void PicoScopeHandler::simulationProcessSamples(){
    if(picoParam != nullptr){
        int32_t noOfSamples = picoParam->timeBaseObj.sampleCount;
        //清理数据对象
        displayData.clear();
        // 给定参数
        //double timePerDiv = picoParam->timeBaseObj.scale; // 每格代表的时间（微秒）
        int maxDivisions = picoParam->timeBaseObj.maxScale; // 最大刻度（格子数）
        double sampleInterval = picoParam->timeBaseObj.interval; // 采样间隔（纳秒）
        // 计算每个样本的时间间隔（单位：微秒）
        double timeInterval = sampleInterval; //totalTime / noOfSamples;

        // 计算总时间范围
        //double totalTime =  maxDivisions; // 总时间为 1000 微秒
        if(picoParam->timeBaseObj.intervalUnit == "ps" && picoParam->timeBaseObj.unit == "ns"){
            timeInterval = timeInterval / 1000;
        }
        if(picoParam->timeBaseObj.intervalUnit == "ps" && picoParam->timeBaseObj.unit == "us"){
            //totalTime = totalTime / 1000;
            timeInterval = timeInterval / 1000000;
            if(picoParam->timeBaseObj.conversion){
                timeInterval = timeInterval / 1000;
            }
        }
        if(picoParam->timeBaseObj.intervalUnit == "ns" && picoParam->timeBaseObj.unit == "us"){
            timeInterval = timeInterval / 1000;
            if(picoParam->timeBaseObj.conversion){
                timeInterval = timeInterval / 1000;
            }
        }
        if(picoParam->timeBaseObj.intervalUnit == "ns" && picoParam->timeBaseObj.unit == "ms"){
            timeInterval = timeInterval / 1000000;
        }
        if(picoParam->timeBaseObj.intervalUnit == "us" && picoParam->timeBaseObj.unit == "ms"){
            timeInterval = timeInterval / 1000;
            if(picoParam->timeBaseObj.conversion){
                timeInterval = timeInterval / 1000;
            }
        }
        if(picoParam->timeBaseObj.intervalUnit == "ms" && picoParam->timeBaseObj.unit == "s"){
            timeInterval = timeInterval / 1000000;
            if(picoParam->timeBaseObj.conversion){
                timeInterval = timeInterval / 1000;
            }
        }

        if(picoParam->timeBaseObj.conversion == 1){
            maxDivisions = maxDivisions / 1000 ;
        }

        double frequency = 1.0;
        double amlitude = 1.0;

        //noOfSamples = maxDivisions / timeInterval;

        //int32_t bufferData[noOfSamples];
        float startTime = 0;

        // 填充 buffer 数组（示例数据）
        for (int32_t i = 0; i < noOfSamples; ++i) {
            float time = startTime + i * timeInterval;
            double voltage = amlitude * sin(2 * M_PI * frequency * time);
            displayData.append(QPointF(time, voltage)); // 将时间和电压数据添加到显示数据中
        }

        if(picoParam->timeBaseObj.unit == "ms")
        {
            timeInterval = timeInterval / 1000;
        }
        if(picoParam->timeBaseObj.unit == "us")
        {
            timeInterval = timeInterval / 1000000;
        }
        if(picoParam->timeBaseObj.unit == "ns")
        {
            timeInterval = timeInterval / 1000000000;
        }
        if(picoParam->timeBaseObj.unit == "ps")
        {
            timeInterval = timeInterval / 1000000000000;
        }

        //单位HZ
        picoParam->samplingRate = displayData.size() / 1;

        //加入缓存
        cacheData.enqueue(displayData);
        if(cacheData.size() > picoParam->maxCacheCount){
            cacheData.dequeue();
        }

        //发送信号
        emit cacheDataUpdated();
        emit dataUpdated();

        //qDebug() << displayData.size();

        // 标记采集不再进行
        acquisitionInProgress = false;
    }else{
        QMessageBox::information(nullptr ,"Error" ,"参数类型未空，请检查参数对象");
        acquisitionInProgress = false;
    }
}
//分段采集
void PicoScopeHandler::configureMemorySegments(uint32_t nSegments) {
    if (handle == 0) {
        QMessageBox::information(nullptr, "Error", "设备未初始化");
        return;
    }

    this->nSegments = nSegments;
    status = ps2000aMemorySegments(handle, nSegments, &maxSamplesPerSegment);
    if (status != PICO_OK) {
        QMessageBox::information(nullptr, "Error", "分段内存配置错误，错误代码：" + QString::number(status));
        return;
    }
}

void PicoScopeHandler::runSegmentedAcquisition(uint32_t nSegments) {
    if (acquisitionInProgress) {
        QMessageBox::information(nullptr, "Error", "采集正在进行中，请稍后再试");
        return;
    }

    acquisitionInProgress = true;

    // 配置分段内存
    configureMemorySegments(nSegments);

    // 验证时基和样本数
    float timeIntervalNs;
    int32_t maxSamples;
    status = ps2000aGetTimebase2(handle, picoParam->timeBaseObj.timebasevalue, maxSamplesPerSegment * nSegments, &timeIntervalNs, 1, &maxSamples, 0);
    if (status != PICO_OK) {
        QMessageBox::information(nullptr, "Error", "无效的时基值或样本数超出范围，错误代码：" + QString::number(status));
        return;
    }

    // 确保样本数在范围内
    if (maxSamplesPerSegment * nSegments > maxSamples) {
        QMessageBox::information(nullptr, "Error", "样本数超出设备支持范围");
        return;
    }


    // 设置数据缓存
    bufferA = std::make_unique<int16_t[]>(maxSamplesPerSegment);
    for (uint32_t segmentIndex = 0; segmentIndex < nSegments; ++segmentIndex) {
        status = ps2000aSetDataBuffer(handle, picoParam->Channel, bufferA.get(), maxSamplesPerSegment, segmentIndex, PS2000A_RATIO_MODE_NONE);
        if (status != PICO_OK) {
            QMessageBox::information(nullptr, "Error", "数据缓存设置错误，错误代码：" + QString::number(status));
            acquisitionInProgress = false;
            return;
        }
    }

    // 运行采集块
    status = ps2000aRunBlock(handle, 0, maxSamplesPerSegment * nSegments, picoParam->timeBaseObj.timebasevalue, NULL, 0, NULL, NULL,NULL);
    if (status != PICO_OK) {
        QMessageBox::information(nullptr, "Error", "运行采集块错误，错误代码：" + QString::number(status));
        acquisitionInProgress = false;
        return;
    }

    // 等待采集完成
    int16_t ready = 0;
    while (!ready) {
        status = ps2000aIsReady(handle, &ready);
        if (status != PICO_OK) {
            QMessageBox::information(nullptr, "Error", "采集未准备好，错误代码：" + QString::number(status));
            acquisitionInProgress = false;
            return;
        }
    }

    // 逐段读取数据
    for (uint32_t segmentIndex = 0; segmentIndex < nSegments; ++segmentIndex) {
        uint32_t noOfSamples = maxSamplesPerSegment;
        status = ps2000aGetValues(handle, 0, &noOfSamples, 1, PS2000A_RATIO_MODE_NONE, segmentIndex, NULL);
        if (status != PICO_OK) {
            QMessageBox::information(nullptr, "Error", "读取分段数据错误，错误代码：" + QString::number(status));
            acquisitionInProgress = false;
            return;
        }

        // 处理数据
        QVector<QPointF> segmentData;
        for (uint32_t i = 0; i < noOfSamples; ++i) {
            double time = i * picoParam->timeBaseObj.interval / 1e9; // 转换为秒
            double voltage = adcToVolts(bufferA[i], picoParam->VoltageRange) * 1000; // 转换为毫伏
            segmentData.append(QPointF(time, voltage));
        }

        // 更新缓存
        cacheData.enqueue(segmentData);
        if (cacheData.size() > picoParam->maxCacheCount) {
            cacheData.dequeue();
        }

        // 发射信号更新数据
        emit dataUpdated();
    }

    acquisitionInProgress = false;
}

//运行采集线程
void PicoScopeHandler::processSamples(PICO_STATUS& status){

    if (picoParam != nullptr) {
        uint32_t noOfSamples = picoParam->timeBaseObj.sampleCount;

        status = ps2000aGetValues(handle, 0, &noOfSamples, 1, PS2000A_RATIO_MODE_NONE, 0, NULL);
        if (status != PICO_OK) {
            qDebug() << "Failed to get values. Error code: " << status;
            QMessageBox::information(nullptr, "Error", "Failed to get values. Error code:" + QString::number(status));
            acquisitionInProgress = false; // 无论成功与否，都要设置为 false
            return;
        }

        // 清理数据对象
        displayData.clear();

        // 设置采样间隔并转换为秒
        double sampleInterval = picoParam->timeBaseObj.interval;
        if (picoParam->timeBaseObj.intervalUnit == "ps") {
            sampleInterval *= 1e-12;
        } else if (picoParam->timeBaseObj.intervalUnit == "ns") {
            sampleInterval *= 1e-9;
        } else if (picoParam->timeBaseObj.intervalUnit == "us") {
            sampleInterval *= 1e-6;
        } else if (picoParam->timeBaseObj.intervalUnit == "ms") {
            sampleInterval *= 1e-3;
        }
        if(picoParam->timeBaseObj.conversion){
            sampleInterval *= 1e-3;
        }

        float startTime = 0;

        // 获取电压范围
        PS2000A_RANGE range = picoParam->VoltageRange;

        // 填充 displayData 数据，统一将电压转换为 mV
        for (int32_t i = 0; i < noOfSamples; ++i) {
            float time = startTime + i * sampleInterval;
            double voltage = adcToVolts(bufferA[i] ,range) * 1000;  // 将所有电压转换为 mV
            displayData.append(QPointF(time, voltage));  // 将时间和电压数据添加到显示数据中
        }

        // 更新缓存
        cacheData.enqueue(displayData);
        if (cacheData.size() > picoParam->maxCacheCount) {
            cacheData.dequeue();
        }

        // 发送信号
        emit cacheDataUpdated();
        emit dataUpdated();

        acquisitionInProgress = false;
    } else {
        QMessageBox::information(nullptr, "Error", "参数类型为空，请检查参数对象");
        acquisitionInProgress = false;
    }
}
void PicoScopeHandler::setPicoParam(PicoParam *param)
{
    picoParam = param;
}
//开始采集
void PicoScopeHandler::startAcquisition(){
    if(acquisitionInProgress){
      return;
    }
    acquisitionInProgress = true;
    setChannel();
    configureTimebase();
    runBlock();
    runAcquisition();
    if(status == PICO_OK){
        processSamples(status);
    }
    timer->start(160);
}
// 开始模拟
void PicoScopeHandler::startSimulation(){
    if(acquisitionInProgress){
        return;
    }
    acquisitionInProgress = true;
    simulationProcessSamples();
    simulationTimer->start(200);
}
// 单帧模拟模式
void PicoScopeHandler::startSimulationSingle(){
    if(acquisitionInProgress){
        return;
    }
    acquisitionInProgress = true;
    simulationProcessSamples();
}
//停止采集
void PicoScopeHandler::stopAcquisition(){
    if(!acquisitionInProgress){
        return;
    }
    timer->stop();
    status = ps2000aStop(handle);
    acquisitionInProgress = false;
    if(status != PICO_OK){
        QMessageBox::information(nullptr ,"Error" ,"关闭设备出错，错误代码：" + QString::number(status));
        return;
    }
}
//停止模拟
void PicoScopeHandler::stopSimulation(){
    if(!acquisitionInProgress){
        return;
    }
    simulationTimer->stop();
    acquisitionInProgress = false;
}
//槽函数
void PicoScopeHandler::onTimerUpdate()
{
    if (acquisitionInProgress) {
        qDebug() << "Acquisition is already in progress or paused, skipping update.";
        return;
    }
    startAcquisition();
}
//模拟计时器槽函数
void PicoScopeHandler::onSimulationTimerUpdate(){
    if(acquisitionInProgress){
        return;
    }
    startSimulation();
}
//得到采集数据
QVector<QPointF> PicoScopeHandler::getSamples() const{
    return displayData;
}
//得到缓存数据
QQueue<QVector<QPointF>> PicoScopeHandler::getCacheData() const{
    return cacheData;
}
//加载缓存文件
void PicoScopeHandler::addCacheData(QVector<QPointF> datas){
    cacheData.enqueue(datas);
}
//adc值换算成电压
double PicoScopeHandler::adcToVolts(int16_t adcValue,PS2000A_RANGE range) const{
    double maxVoltage = 0;
    switch (range) {
        case PS2000A_10MV:   maxVoltage = 0.01; break;    // ±10 mV
        case PS2000A_20MV:   maxVoltage = 0.02; break;    // ±20 mV
        case PS2000A_50MV:   maxVoltage = 0.05; break;    // ±50 mV
        case PS2000A_100MV:  maxVoltage = 0.1; break;     // ±100 mV
        case PS2000A_200MV:  maxVoltage = 0.2; break;     // ±200 mV
        case PS2000A_500MV:  maxVoltage = 0.5; break;     // ±500 mV
        case PS2000A_1V:     maxVoltage = 1.0; break;     // ±1 V
        case PS2000A_2V:     maxVoltage = 2.0; break;     // ±2 V
        case PS2000A_5V:     maxVoltage = 5.0; break;     // ±5 V
        case PS2000A_10V:    maxVoltage = 10.0; break;    // ±10 V
        case PS2000A_20V:    maxVoltage = 20.0; break;    // ±20 V
        case PS2000A_50V:    maxVoltage = 50.0; break;    // ±50 V
        default:             maxVoltage = 1.0; break;     // 默认1 V
    }

    int16_t adcMaxValue = 32767; // 16位ADC的最大值
    return (static_cast<double>(adcValue) / adcMaxValue) * maxVoltage;
}
//清除缓存
void PicoScopeHandler::removeCacheData(){
    cacheData.clear();
}

//析构函数
PicoScopeHandler::~PicoScopeHandler() {
    if (handle != 0) {
        ps2000aStop(handle);
        ps2000aCloseUnit(handle);
        removeCacheData();
    }
}
