#include "segmenthandle.h"
#include <iostream>

// 构造函数
SegmentHandle::SegmentHandle(PS2000A_RANGE range, PS2000A_COUPLING coupling, PS2000A_CHANNEL channel,
                             int segmentCount, int samplesPerSegment, int timebase, QObject *parent)
    : QThread(parent), _range(range), _coupling(coupling), _channel(channel),
    _segmentCount(segmentCount), _samplesPerSegment(samplesPerSegment), _timebase(timebase) {}

// adc值转换为电压
double SegmentHandle::adcToVolts(int16_t adcValue, PS2000A_RANGE range)
{
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

// 重写run方法，线程的工作将在此执行
void SegmentHandle::run()
{
    QVector<QPointF> datas;
    int16_t handle;
    PICO_STATUS status = ps2000aOpenUnit(&handle, NULL);
    if (status != PICO_OK) {
        std::cerr << "设备打开失败，错误代码: " << status << std::endl;
        emit finished();  // 线程结束信号
        return;
    }

    // 设置通道
    status = ps2000aSetChannel(handle, _channel, 1, _coupling, _range, 0);
    if (status != PICO_OK) {
        std::cerr << "频道设置失败: 错误代码" << status << std::endl;
        ps2000aCloseUnit(handle);
        emit finished();  // 线程结束信号
        return;
    }

    // 配置分段内存
    uint32_t segmentCount = _segmentCount;
    int32_t maxSamplesPerSegment = 0;
    status = ps2000aMemorySegments(handle, segmentCount, &maxSamplesPerSegment);
    if (status != PICO_OK) {
        std::cerr << "设置内存分段失败，错误代码: " << status << std::endl;
        ps2000aCloseUnit(handle);
        emit finished();  // 线程结束信号
        return;
    }

    // 确定时间基准和采样设置
    uint32_t timebase = _timebase;
    int32_t samplesPerSegment = _samplesPerSegment;
    float timeIntervalNanoseconds = 0;
    int16_t oversample = 1;
    status = ps2000aGetTimebase2(handle, timebase, samplesPerSegment, &timeIntervalNanoseconds, oversample, &maxSamplesPerSegment, 0);

    if (status != PICO_OK || samplesPerSegment > maxSamplesPerSegment) {
        std::cerr << "时间基准无效或样本数超出设备支持范围" << std::endl;
        ps2000aCloseUnit(handle);
        emit finished();  // 线程结束信号
        return;
    }

    // 设置数据缓存
    int16_t* buffer = new int16_t[samplesPerSegment];

    // 对每个段单独采集
    for (uint32_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
        status = ps2000aSetDataBuffer(handle, _channel, buffer, samplesPerSegment, segmentIndex, PS2000A_RATIO_MODE_NONE);
        if (status != PICO_OK) {
            std::cerr << "设置缓存失败，错误代码: " << status << std::endl;
            delete[] buffer;
            ps2000aCloseUnit(handle);
            emit finished();  // 线程结束信号
            return;
        }

        status = ps2000aRunBlock(handle, 0, samplesPerSegment, timebase, oversample, nullptr, segmentIndex, nullptr, nullptr);
        if (status != PICO_OK) {
            std::cerr << "采集第 " << segmentIndex << " 段失败，错误代码: " << status << std::endl;
            delete[] buffer;
            ps2000aCloseUnit(handle);
            emit finished();  // 线程结束信号
            return;
        }

        // 等待采集完成
        int16_t ready = 0;
        while (!ready) {
            status = ps2000aIsReady(handle, &ready);
            if (status != PICO_OK) {
                std::cerr << "检查采集状态失败，错误代码: " << status << std::endl;
                delete[] buffer;
                ps2000aCloseUnit(handle);
                emit finished();  // 线程结束信号
                return;
            }
        }
    }

    // 读取数据并生成时间电压曲线
    double timeIntervalInSeconds = 1.0; // 时间间隔 (1ns -> 秒)
    double timeOffset = 0.0;
    for (uint32_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
        uint32_t noOfSamples = samplesPerSegment;
        status = ps2000aGetValues(handle, 0, &noOfSamples, 1, PS2000A_RATIO_MODE_NONE, segmentIndex, nullptr);
        if (status != PICO_OK) {
            std::cerr << "读取第 " << segmentIndex << " 段数据失败，错误代码: " << status << std::endl;
            continue;
        }

        // 目标显示范围：10ms
        double maxTimeInSeconds = 10.0 / 1000.0; // 10ms 转换为秒

        // 绘图数据处理
        for (uint32_t i = 0; i < noOfSamples; ++i) {
            // 将时间转换为毫秒
            double timeInSeconds = (timeOffset + i * timeIntervalNanoseconds) / 1e9;

            // 如果时间超过 10ms，则跳出循环
            if (timeInSeconds > maxTimeInSeconds) {
                break; // 直接停止处理
            }

            // 将数据按毫秒时间轴添加
            double voltage = adcToVolts(buffer[i], _range) * 1000; // 电压转换
            datas.append(QPointF(timeInSeconds, voltage)); // 将时间和电压数据存入QVector

            // 更新进度条
            if (i % 1000 == 0 || i == noOfSamples - 1) {
                int progress = static_cast<int>((i / static_cast<float>(noOfSamples)) * 100);
                emit progressUpdated(progress);  // 更新进度信号
            }
        }
        timeOffset += noOfSamples * timeIntervalInSeconds;
    }

    // 确保最后更新进度条为 100%
    int finalProgress = 100;
    emit progressUpdated(finalProgress);  // 发出进度信号，确保到达100%

    delete[] buffer;
    ps2000aCloseUnit(handle);
    emit dataReady(datas);  // 数据准备完成信号
    emit finished();  // 线程结束信号
}
