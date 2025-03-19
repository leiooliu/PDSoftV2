#ifndef SEGMENTHANDLE_H
#define SEGMENTHANDLE_H

#include <QThread>
#include <QPointF>
#include <io.h>
#include <fcntl.h>
#include "ps2000aApi.h"
#include <tools.h>
class SegmentHandle : public QObject
{
    Q_OBJECT

public:
    // 构造函数：传入范围、耦合、通道、段数、每段样本数、时间基准
    SegmentHandle(PS2000A_RANGE range, PS2000A_COUPLING coupling, PS2000A_CHANNEL channel,
                  int segmentCount, int samplesPerSegment, int timebase, QObject *parent = nullptr);

    // 重写run方法，线程的工作将在此执行
    //void run() override;
    bool open();
    bool close();

    void loadTestData();

    ~SegmentHandle();
    void clearData();
    void loadData();
    //更换时基参数
    void changeTimebase(int timebaseValue);
    //更换采样数
    void changeSamplesCount(int samplesCount);
    //更换电压幅值
    void changeRange(PS2000A_RANGE range);
    //计算采样率
    QString calculateSamplingRate(double timeIntervalSeconds, int totalSamples);
    //设置触发器
    void useTriggers(bool isUsed);

signals:
    // 进度更新信号
    void progressUpdated(int percentage);

    // 数据准备完成信号
    void dataReady(const QVector<QPointF>& data);

    // 原始数据准备完成信号
    void rawDataReady(const QVector<double>& data,double timeIntervalNanoseconds);

    //测试数据采集完成
    void testDataReady(const QVector<double>& data,double timeIntervalNanoseconds);

    // 发送日志输出
    void sendLog(QString log);

    // 线程结束信号
    void finished();

private:
    PS2000A_RANGE _range;
    PS2000A_COUPLING _coupling;
    PS2000A_CHANNEL _channel;
    int _segmentCount;
    int _samplesPerSegment;
    int _timebase;
    QVector<QPointF> datas;
    QVector<double> rawdatas;
    int16_t handle;
    PICO_STATUS status;
    bool isUseTriggers;
};

#endif // SEGMENTHANDLE_H
