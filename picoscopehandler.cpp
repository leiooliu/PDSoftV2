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
        status = ps2000aGetTimebase(handle ,picoParam->timeBaseObj.timebasevalue ,0 ,&timeIntervalNanoseconds, 0,&maxSamples ,0);
        if(status != PICO_OK){
            QMessageBox::information(nullptr ,"Error" ,"时基获取错误，错误代码：" + QString::number(status));
            return;
        }



        //设置采样率


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
//运行采集线程
void PicoScopeHandler::processSamples(PICO_STATUS& status){

    if(picoParam != nullptr){
        uint32_t noOfSamples = picoParam->timeBaseObj.sampleCount;

        status = ps2000aGetValues(handle, 0, &noOfSamples, 1, PS2000A_RATIO_MODE_NONE, 0, NULL);
        if (status != PICO_OK) {
            qDebug() << "Failed to get values. Error code: " << status;
            QMessageBox::information(nullptr, "Error", "Failed to get values. Error code:"+ QString::number(status));
            acquisitionInProgress = false; // 无论成功与否，都要设置为 false
            return;
        }

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

        float startTime = 0;

        //判断电压是否需要转换单位
        bool voltsConversion = false;
        switch(picoParam->VoltageRange){
            case PS2000A_10MV:
            case PS2000A_20MV:
            case PS2000A_50MV:
            case PS2000A_100MV:
            case PS2000A_200MV:
            case PS2000A_500MV:
                voltsConversion = true;
                break;
            default:
                voltsConversion = false;
                break;

        }

        // 填充 buffer 数组（示例数据）
        for (int32_t i = 0; i < noOfSamples; ++i) {
            float time = startTime + i * timeInterval;
            double voltage = adcToVolts(bufferA[i]);
            if(voltsConversion){
                voltage = voltage * 1000;
            }
            //double voltage = bufferA[i];
            displayData.append(QPointF(time, voltage)); // 将时间和电压数据添加到显示数据中
        }

        //加入缓存
        cacheData.enqueue(displayData);
        if(cacheData.size() > picoParam->maxCacheCount){
            cacheData.dequeue();
        }

        //发送信号
        emit cacheDataUpdated();
        emit dataUpdated();

        // 标记采集不再进行
        acquisitionInProgress = false;
    }else{
        QMessageBox::information(nullptr ,"Error" ,"参数类型未空，请检查参数对象");
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
double PicoScopeHandler::adcToVolts(int16_t adcValue) const{
    //picoParam->voltageRangeValue
    return adcValue * (picoParam->voltageRangeValue / 32767.0);
    //return adcValue * (2.0f / 32767.0);
}
//析构函数
PicoScopeHandler::~PicoScopeHandler() {
    if (handle != 0) {
        ps2000aStop(handle);
        ps2000aCloseUnit(handle);
    }
}
