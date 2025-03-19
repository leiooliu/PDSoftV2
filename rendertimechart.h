#ifndef RENDERTIMECHART_H
#define RENDERTIMECHART_H

#include <QThread>
#include <pdchart.h>
#include <picoparam.h>
#include "ps2000aApi.h"
#include <tools.h>
#include <TimeBaseLoader.h>
#include <peakparam.h>

class RenderTimeChart : public QThread
{
    Q_OBJECT
public:
    explicit RenderTimeChart(PDChart *pdChart, QObject *parent = nullptr);
    void render(const QVector<QPointF> &datas,QString unit);
    void render(const QVector<double> sourceData , PS2000A_RANGE range, TimeBase timebase,PeakParam peakParam);
    void changeY(PS2000A_RANGE range);
    void changeX(TimeBase timebase);
    void run() override;
    void clear();

    void setXRange(double min ,double max);
    void setYRange(double min ,double max);

    void setPeakTiggerData(const QVector<QPointF> &datas);

signals:
    void progressUpdated(int percentage);
    void dataRendey();
    void renderFinished(const QVector<QPointF> &datas ,double timeMultiplier);
    void sendLog(QString logs);
private:
    PDChart *_pdChart;
    QVector<QPointF> _datas;
    QString _unit;
    // LOD相关变量
    QVector<QVector<QPointF>> lodData; // 存储LOD数据
    int maxLevels = 6;                 // 最大LOD级别
    int targetPointsPerLevel = 5000;   // 每级LOD最大点数
    // 创建LOD缓存
    void createLOD(const QVector<QPointF>& points) ;

    // 更新LOD显示
    void updateLOD(int zoomLevel);

    // 设置数据并创建LOD
    void setFData(const QVector<QPointF>& points);

    double timeMultiplier;

    double _xMin = 0;
    double _xMax = 10;
    double _xIntervals = 1.0;
    double _yMin = -5000;
    double _yMax = 5000;
    double _yIntervals = 1.0;

    PeakParam *peakParam;

};
#endif // RENDERTIMECHART_H
