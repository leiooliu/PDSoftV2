#ifndef PICOSCOPEHANDLER_H
#define PICOSCOPEHANDLER_H

#include <QObject>
#include <ps2000aApi.h>
#include <picoparam.h>
#include <TimeBaseLoader.h>
#include <QQueue>
#include <QTimer>

class PicoScopeHandler :public QObject
{
    Q_OBJECT
public:
    explicit PicoScopeHandler(QObject* parent = nullptr);
    ~PicoScopeHandler();
    void setPicoParam(PicoParam *param);
    void initialize();
    void setChannel();
    void configureTimebase();
    void runBlock();
    // 开始数据采集
    void startAcquisition();
    // 开始模拟
    void startSimulation();
    // 单帧模拟
    void startSimulationSingle();
    // 停止采集
    void stopAcquisition();
    //停止模拟
    void stopSimulation();
    //得到采集数据
    QVector<QPointF> getSamples() const;
    //得到缓存数据
    QQueue<QVector<QPointF>> getCacheData() const;
    //加载缓存文件
    void addCacheData(QVector<QPointF> datas);
    //清除缓存
    void removeCacheData();
    //分段采集
    void runSegmentedAcquisition(uint32_t nSegments);
signals:
    //数据更新信号
    void dataUpdated();
    //缓存更新信号
    void cacheDataUpdated();

private slots:
    //计时器开始方法
    void onTimerUpdate();
    void onSimulationTimerUpdate();

private:
    PicoParam *picoParam;
    PICO_STATUS status;
    int16_t handle;
    //采样数
    uint32_t numSamples;
    //采集间隔
    float timeIntervalNanoseconds;
    int32_t maxSamples;
    //Timer计时器
    QTimer* timer;
    QTimer* simulationTimer;
    QVector<QPointF> displayData;
    QQueue<QVector<QPointF>> cacheData;
    uint32_t nSegments;
    int32_t maxSamplesPerSegment;
    //缓存
    std::unique_ptr<int16_t[]> bufferA;

    bool acquisitionInProgress;

    //运行采集
    void runAcquisition();
    //采集进程
    void processSamples(PICO_STATUS& status);
    //模拟采集进程
    void simulationProcessSamples();
    //adc转换为电压
    double adcToVolts(int16_t adcValue ,PS2000A_RANGE range) const;

    void configureMemorySegments(uint32_t nSegments);
};

#endif // PICOSCOPEHANDLER_H
