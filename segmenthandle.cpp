#include "segmenthandle.h"
#include <filemanager.h>
// 构造函数
SegmentHandle::SegmentHandle(PS2000A_RANGE range, PS2000A_COUPLING coupling, PS2000A_CHANNEL channel,
                             int segmentCount, int samplesPerSegment, int timebase, QObject *parent)
    : _range(range), _coupling(coupling), _channel(channel),
    _segmentCount(segmentCount), _samplesPerSegment(samplesPerSegment), _timebase(timebase) {
    isUseTriggers = true;
}

//设置触发器
void SegmentHandle::useTriggers(bool isUsed){
    isUseTriggers = isUsed;
}

bool SegmentHandle::open(){
    status = ps2000aOpenUnit(&handle, NULL);
    bool success = false;
    if(status != PICO_OK){
        emit sendLog("设备打开失败，错误代码：" + QString::number(status));
    }else{
        success = true;
    }
    return success;
}
bool SegmentHandle::close(){
    status = ps2000aCloseUnit(handle);
    bool success = false;
    if(status != PICO_OK){
        emit sendLog("设备关闭失败，错误代码：" + QString::number(status));
    }else{
        success = true;
    }
    return success;
}

SegmentHandle::~SegmentHandle(){
    datas.clear();
    datas.squeeze();
    rawdatas.clear();
    rawdatas.clear();
    close();
}

void SegmentHandle::clearData(){
    datas.clear();
    datas.squeeze();
    rawdatas.clear();
    rawdatas.squeeze();
}


void SegmentHandle::changeTimebase(int timebaseValue){
    _timebase = timebaseValue;
}

//更换采样数
void SegmentHandle::changeSamplesCount(int samplesCount){
    _samplesPerSegment = samplesCount;
}

//更换电压幅值
void SegmentHandle::changeRange(PS2000A_RANGE range){
    _range = range;
}

// 新增计算采样率的方法
QString SegmentHandle::calculateSamplingRate(double timeIntervalSeconds, int totalSamples) {
    if (timeIntervalSeconds <= 0 || totalSamples < 2) {
        emit sendLog("采样率计算出错。");
        return "Error";
    }

    // 计算采样率 (Hz)
    double rate = 1.0 / timeIntervalSeconds;

    // 转换成人类可读格式
    QString rateStr;
    if (rate >= 1e6) {
        rateStr = QString::number(rate / 1e6, 'f', 2) + "M/s"; // 显示为 MHz
    } else if (rate >= 1e3) {
        rateStr = QString::number(rate / 1e3, 'f', 2) + "K/s"; // 显示为 kHz
    } else {
        rateStr = QString::number(rate, 'f', 2) + " Hz"; // 显示为 Hz
    }

    emit sendLog("采样率: " + rateStr);
    return rateStr;
}

void SegmentHandle::loadData(){

    // 设置通道
    status = ps2000aSetChannel(handle, _channel, 1, _coupling, _range, 0);
    if (status != PICO_OK) {
        emit sendLog("频道设置失败， 错误代码: " + QString::number(status));
        emit finished();  // 线程结束信号
        return;
    }
    emit sendLog("已设置通道 ... ");

    int16_t triggersEnable = isUseTriggers == true ? 1 : 0;
    // 设置触器发条件
    status = ps2000aSetSimpleTrigger(handle,         // 设备句柄
                                     triggersEnable, // 启用触发
                                     _channel,       // 使用通道A触发
                                     0, // 阈值 0 V (ADC单位：0)
                                     PS2000A_RISING, // 上升沿触发
                                     0,              // 延迟采样数
                                     0 // 自动触发延迟时间(毫秒)
                                     );

    if (status != PICO_OK) {
        emit sendLog("出发条件设置失败，错误代码 " + QString::number(status));
        emit finished(); // 线程结束信号
        return;
    }

    // 配置分段内存
    uint32_t segmentCount = _segmentCount;
    int32_t maxSamplesPerSegment = 0;
    status = ps2000aMemorySegments(handle, segmentCount, &maxSamplesPerSegment);
    if (status != PICO_OK) {
        emit sendLog("设置内存分段失败，错误代码: " + QString::number(status));
        emit finished();  // 线程结束信号
        return;
    }
    emit sendLog("已设置分段内存 ... ");

    // 确定时间基准和采样设置
    uint32_t timebase = _timebase;
    int32_t samplesPerSegment = _samplesPerSegment;
    float timeIntervalNanoseconds = 0;
    int16_t oversample = 1;

    status = ps2000aGetTimebase2(handle, timebase, samplesPerSegment,
                                 &timeIntervalNanoseconds, oversample, &maxSamplesPerSegment, 0);

    if (status != PICO_OK || samplesPerSegment > maxSamplesPerSegment) {
        emit sendLog("时间基准无效或样本数超出设备支持范围");
        emit finished();  // 线程结束信号
        return;
    }
    emit sendLog("已确定时间基准，设置采样 ... ");
    emit sendLog("采样率："+calculateSamplingRate(timeIntervalNanoseconds * 1e-9 ,samplesPerSegment));

    // 设置数据缓存
    int16_t* buffer = new int16_t[samplesPerSegment];

    // 对每个段单独采集
    for (uint32_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
        status = ps2000aSetDataBuffer(handle, _channel, buffer, samplesPerSegment, segmentIndex, PS2000A_RATIO_MODE_NONE);
        if (status != PICO_OK) {
            delete[] buffer;
            emit sendLog("设置缓存失败: " + QString::number(status));
            emit finished();  // 线程结束信号
            return;
        }

        status = ps2000aRunBlock(handle, 0, samplesPerSegment, timebase, oversample, nullptr, segmentIndex, nullptr, nullptr);
        if (status != PICO_OK) {
            delete[] buffer;
            emit sendLog("采集第: " + QString::number(segmentIndex)+" 段失败，错误代码:" + QString::number(segmentIndex));
            emit finished();  // 线程结束信号
            return;
        }

        // 等待采集完成
        int16_t ready = 0;
        while (!ready) {
            status = ps2000aIsReady(handle, &ready);
            if (status != PICO_OK) {
                delete[] buffer;
                emit sendLog("检查采集状态失败，错误代码:  " + QString::number(status));
                emit finished();  // 线程结束信号
                return;
            }
        }
    }
    rawdatas.clear();
    for (uint32_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
        uint32_t noOfSamples = samplesPerSegment;
        status = ps2000aGetValues(handle, 0, &noOfSamples, 1, PS2000A_RATIO_MODE_NONE, segmentIndex, nullptr);
        if (status != PICO_OK) {
            emit sendLog("读取第:  " + QString::number(segmentIndex)+ " 段数据失败，错误代码: " +QString::number(status));
            continue;
        }

        for(uint32_t i = 0; i<noOfSamples;++i){
            rawdatas.append(buffer[i]);
            // 更新进度条
            if (i % 1000 == 0 || i == noOfSamples - 1) {
                int progress = static_cast<int>((i / static_cast<float>(noOfSamples)) * 100);
                emit progressUpdated(progress);  // 更新进度信号
            }
        }
    }

    // 确保最后更新进度条为 100%
    emit progressUpdated(100);  // 发出进度信号，确保到达100%
    emit rawDataReady(rawdatas,timeIntervalNanoseconds);
    emit finished();  // 线程结束信号
    delete[] buffer;
}

// // 重写run方法，线程的工作将在此执行
// void SegmentHandle::run()
// {
//     loadData();
// }
