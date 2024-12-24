#ifndef SEGMENTHANDLE_H
#define SEGMENTHANDLE_H

#include <QThread>
#include <QPointF>
#include <io.h>
#include <fcntl.h>
#include "ps2000aApi.h"
class SegmentHandle : public QThread
{
    Q_OBJECT

public:
    // 构造函数：传入范围、耦合、通道、段数、每段样本数、时间基准
    SegmentHandle(PS2000A_RANGE range, PS2000A_COUPLING coupling, PS2000A_CHANNEL channel,
                  int segmentCount, int samplesPerSegment, int timebase, QObject *parent = nullptr);

    // adc值转换为电压
    double adcToVolts(int16_t adcValue, PS2000A_RANGE range);

    // 重写run方法，线程的工作将在此执行
    void run() override;

signals:
    // 进度更新信号
    void progressUpdated(int percentage);
    // 数据准备完成信号
    void dataReady(const QVector<QPointF>& data);
    // 线程结束信号
    void finished();

private:
    PS2000A_RANGE _range;
    PS2000A_COUPLING _coupling;
    PS2000A_CHANNEL _channel;
    int _segmentCount;
    int _samplesPerSegment;
    int _timebase;
};

#endif // SEGMENTHANDLE_H
