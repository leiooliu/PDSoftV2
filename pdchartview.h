#ifndef PDCHARTVIEW_H
#define PDCHARTVIEW_H

#include <QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QToolTip>
#include <QMouseEvent>
#include "singalconvert.h"

class PDChartView : public QChartView {
public:
    PDChartView(QChart *chart, QWidget *parent = nullptr) : QChartView(chart, parent) {
        setMouseTracking(true);  // 启用鼠标追踪
        //setRubberBand(QChartView::HorizontalRubberBand);
        setRubberBand(QChartView::RectangleRubberBand);
        axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).constFirst());
        convert = new SingalConvert;
    }
private:
    SingalConvert *convert;
    QValueAxis *axisX;
    // 获取水平轴
    QValueAxis* getAxisX(QChart* chart) {
        QList<QAbstractAxis*> axes = chart->axes(Qt::Horizontal);
        if (!axes.isEmpty()) {
            return qobject_cast<QValueAxis*>(axes.first());
        }
        return nullptr;
    }

    // 获取垂直轴
    QValueAxis* getAxisY(QChart* chart) {
        QList<QAbstractAxis*> axes = chart->axes(Qt::Vertical);
        if (!axes.isEmpty()) {
            return qobject_cast<QValueAxis*>(axes.first());
        }
        return nullptr;
    }
protected:

    void mouseMoveEvent(QMouseEvent *event) override {
        QChartView::mouseMoveEvent(event);

        // 获取当前鼠标的位置
        QPointF mousePos = chart()->mapToValue(event->pos());
        //qDebug() << "mouse pos x :" << mousePos.x() << " y :" << mousePos.y();

        // 获取图表中的所有 series（假设我们只处理 QLineSeries）
        foreach (QAbstractSeries *series, chart()->series()) {
            QLineSeries *lineSeries = qobject_cast<QLineSeries *>(series);
            if (lineSeries) {
                // 遍历所有数据点，寻找与鼠标位置最接近的点
                for (const QPointF &point : lineSeries->points()) {

                    if (qAbs(point.x() - mousePos.x()) < 0.1 && qAbs(point.y() - mousePos.y()) < 0.1) {

                        //double thdValue = convert->calculateTHD(lineSeries->points() ,1);
                        //double thdValue = convert->calculateTHD(lineSeries->points() ,point.x());
                        // 显示 Tooltip
                        //QString tooltip = QString("X: %1\nY: %2\nTHD: \%3").arg(point.x()).arg(point.y());//.arg(thdValue);
                        QString tooltip = QString("X: %1\nY: %2\n").arg(point.x()).arg(point.y());//.arg(thdValue);
                        QToolTip::showText(event->globalPosition().toPoint(), tooltip, this);
                        return;  // 找到点后立即返回
                    }
                }
            }
        }

        // 如果没有找到合适的数据点，隐藏提示
        QToolTip::hideText();
    }

    // 重写鼠标双击事件
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        QChartView::mouseDoubleClickEvent(event);

        if (event->button() == Qt::LeftButton) {
            // 恢复缩放
            chart()->zoomReset();

            // 获取水平轴和垂直轴
            QValueAxis *axisX = getAxisX(chart());
            QValueAxis *axisY = getAxisY(chart());

            // 恢复坐标轴范围
            if (axisX && axisY) {
                chart()->zoomReset();
            }
        }
    }

    // 重写滚轮事件
    void wheelEvent(QWheelEvent *event) override {
        if (axisX) {
            // 获取当前 X 轴的最小值和最大值
            qreal minX = axisX->min();
            qreal maxX = axisX->max();

            // 计算滚动步长，步长越大滚动越快
            qreal scrollStep = (maxX - minX) * 0.02;  // 10% 的范围滚动

            // 根据滚轮的滚动方向调整 X 轴的范围
            if (event->angleDelta().y() > 0) {
                // 向前滚动，向左移动图表
                axisX->setRange(minX - scrollStep, maxX - scrollStep);
            } else {
                // 向后滚动，向右移动图表
                axisX->setRange(minX + scrollStep, maxX + scrollStep);
            }
        }
        // 调用父类的滚轮事件
        QChartView::wheelEvent(event);
    }
};

#endif // PDCHARTVIEW_H
