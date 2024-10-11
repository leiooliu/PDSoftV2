#ifndef PDCHART_H
#define PDCHART_H

#include <QWidget>
#include <QChartView>
#include <zoomchartview.h>
#include <QLineSeries>
#include <QValueAxis>
#include <QMouseEvent>
#include <pdchartview.h>
class PDChart:public QWidget
{
public:
    PDChart(QWidget *parent = nullptr);
    ~PDChart();
    void setData(const QList<QPointF> &points);
    void setTitle(const QString &title);
    void setXAxisTitle(const QString &title);
    void setYAxisTitle(const QString &title);
    void setLineColor(const QColor &color);
    void setLineWidth(qreal width);
    void setXAxisRange(qreal min, qreal max);
    void setYAxisRange(qreal min, qreal max);
    void setMargins(int left, int top, int right, int bottom);
    void setYAxisScale(qreal min, qreal max, qreal interval);
    void setXAxisScale(qreal min, qreal max, qreal interval);
    void clearData();
signals:
    void sgl_recoverRange(PDChart *p);

protected:
    //鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *pEvent) override;

private:
    PDChartView *chartView;
    QChart *chart;
    QLineSeries *series;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QGraphicsTextItem *m_tooltip;
    bool isMousePressed;  // 标志鼠标左键是否被按住
    void updateTooltip(const QPointF &point);
    bool isPointNearDataPoint(const QPointF &point, const QPointF &dataPoint, qreal tolerance = 10.0);
    bool altPressed;
    bool isAltPressed() const { return altPressed; }

    bool is_Pressed_;
    bool m_bMiddleButtonPressed;
    QPoint m_oPrePos;
};

#endif // PDCHART_H
