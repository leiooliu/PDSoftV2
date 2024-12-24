#ifndef RENDERTIMECHART_H
#define RENDERTIMECHART_H

#include <QThread>
#include <pdchart.h>
#include <picoparam.h>
class RenderTimeChart : public QThread
{
    Q_OBJECT
public:
    explicit RenderTimeChart(PDChart *pdChart, QObject *parent = nullptr);
    void render(const QVector<QPointF> &datas,QString unit);
    void run() override;
signals:
    void progressUpdated(int percentage);
    void dataRendey();
    void renderFinished();
private:
    PDChart *_pdChart;
    QVector<QPointF> _datas;
    QString _unit;
};

#endif // RENDERTIMECHART_H
