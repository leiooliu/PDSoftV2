#ifndef ZOOMCHARTVIEW_H
#define ZOOMCHARTVIEW_H

#include <QChartView>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCore/QRandomGenerator>
class ZoomChartView : public QChartView
{
public:
    ZoomChartView(QChart *chart ,QWidget *parent = nullptr):QChartView(chart ,parent){
        setRenderHint(QPainter::Antialiasing);
    }
protected:
    // void wheelEvent(QWheelEvent *event) override {
    //     if (event->angleDelta().y() > 0) {
    //         chart()->zoomIn();
    //     } else {
    //         chart()->zoomOut();
    //     }
    //     event->accept();
    // }
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            isPanning = true;
            lastMousePos = event->pos();
            setCursor(Qt::ClosedHandCursor);
        }
        QChartView::mousePressEvent(event);
    }

    // void mouseMoveEvent(QMouseEvent *event) override {
    //     if (isPanning) {
    //         QPoint delta = event->pos() - lastMousePos;
    //         chart()->scroll(-delta.x(), delta.y());
    //         lastMousePos = event->pos();
    //     }
    //     QChartView::mouseMoveEvent(event);
    // }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            isPanning = false;
            setCursor(Qt::ArrowCursor);
        }
        QChartView::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override {
        chart()->zoomReset();
        event->accept();
    }
private:
    bool isPanning;
    QPoint lastMousePos;
    QRectF initialRect;
};

#endif // ZOOMCHARTVIEW_H
