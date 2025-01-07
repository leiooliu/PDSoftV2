#ifndef RENDERTIMECHART_H
#define RENDERTIMECHART_H

#include <QThread>
#include <pdchart.h>
#include <picoparam.h>
#include "ps2000aApi.h"
#include <tools.h>
#include <TimeBaseLoader.h>
class RenderTimeChart : public QThread
{
    Q_OBJECT
public:
    explicit RenderTimeChart(PDChart *pdChart, QObject *parent = nullptr);
    void render(const QVector<QPointF> &datas,QString unit);
    void render(const QVector<double> soucedata , PS2000A_RANGE range, TimeBase timebase);
    void run() override;
    void clear();
signals:
    void progressUpdated(int percentage);
    void dataRendey();
    void renderFinished(const QVector<QPointF> &datas);
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
};
#endif // RENDERTIMECHART_H
