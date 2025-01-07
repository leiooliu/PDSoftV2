#ifndef RENDERFREQUENCYCHART_H
#define RENDERFREQUENCYCHART_H

#include <QThread>
#include <pdchart.h>
class RenderFrequencyChart : public QThread
{
    Q_OBJECT
public:
    explicit RenderFrequencyChart(PDChart *pdChart ,QObject *parent = nullptr);
    void render(std::vector<double> frequencies ,std::vector<double> magnitudes ,QString unit);
    void run() override;
    void clear();
    void setFData(const QVector<QPointF>& points);
signals:
    void dataRendy();
    void renderFinished(const QVector<QPointF> &data);
    void sendLog(QString logs);

private:
    PDChart *_chart;
    QVector<QPointF> _datas;
    QString _unit;
    double maxFreq;
    double minFreq;
    double maxEnergy;
    double minEnergy;

    // LOD相关变量
    QVector<QVector<QPointF>> lodData; // 存储LOD数据
    int maxLevels = 6;                 // 最大LOD级别
    int targetPointsPerLevel = 5000;   // 每级LOD最大点数
    void updateLOD(int zoomLevel);     // 更新LOD显示
    void createLOD(const QVector<QPointF>& points); // 生成LOD缓存
};

#endif // RENDERFREQUENCYCHART_H
