#include "pdchart.h"
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QPen>
#include <QVBoxLayout>
#include <QChartView>
#include <QToolTip>
#include <QMouseEvent>
#include <QXYSeries>
#include <customlineitem.h>

PDChart::PDChart(QWidget *parent):
    QWidget(parent),
    chartView(nullptr),
    chart(nullptr),
    series(nullptr),
    axisX(nullptr),
    axisY(nullptr),
    m_bMiddleButtonPressed(false),
    m_oPrePos(0,0)
{

    //setMouseTracking(true);
    //setRubberBand(PDChart::NoRubberBand);
    chart = new QChart();
    series = new QLineSeries();
    //使用OpenGL渲染
    series->setUseOpenGL(true);

    axisX = new QValueAxis();
    axisY = new QValueAxis();

    chart->addSeries(series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->setZValue(20);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView = new PDChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);

    // 将 tooltip 添加到场景中
    m_tooltip = new QGraphicsTextItem(chart);
    chartView->scene()->addItem(m_tooltip);
    //m_tooltip->setZValue(11); // 确保 tooltip 在图表上方
    m_tooltip->setDefaultTextColor(Qt::black); // 设置 tooltip 文本颜色
    m_tooltip->hide(); // 初始隐藏 tooltip

    is_Pressed_ = false;
    // 启用鼠标追踪
    //setMouseTracking(true);
    //chartView->setMouseTracking(true);

    //chartView->setRubberBand(QChartView::HorizontalRubberBand);
    //chartView->setAttribute(Qt::WA_UnderMouse, true);
}



void PDChart::updateTooltip(const QPointF &point)
{
    QString tooltipText = QString("X: %1\nY: %2").arg(point.x()).arg(point.y());
    m_tooltip->setPlainText(tooltipText);

    QPointF position = chart->mapToPosition(point, series);
    m_tooltip->setPos(position.x() + 10, position.y() - 30);  // 调整 tooltip 位置
    m_tooltip->show();
}

bool PDChart::isPointNearDataPoint(const QPointF &point, const QPointF &dataPoint, qreal tolerance)
{
    qreal distance = qSqrt(qPow(point.x() - dataPoint.x(), 2) + qPow(point.y() - dataPoint.y(), 2));
    return distance <= tolerance;
}

PDChart::~PDChart()
{
    delete chartView;
    if(chart!=nullptr)
        delete chart;
    if(series!=nullptr)
        delete series;
    if(axisX!=nullptr)
        delete axisX;
    if(axisY!=nullptr)
        delete axisY;
}

void PDChart::setFData(const QVector<QPointF> &points){
    series->clear();
    series->replace(points);
}

void PDChart::setHData(const QVector<QPointF> &points){
    series->clear();
    //chart->createDefaultAxes();
    series->replace(points);
}

void PDChart::setData(const QList<QPointF> &points,QString unit)
{
    series->clear();

    // 确定转换因子，将所有单位转换为秒
    double timeScaleFactor = 1.0;
    if (unit == "ps") {
        timeScaleFactor = 1e12;
    } else if (unit == "ns") {
        timeScaleFactor = 1e9;
    } else if (unit == "us") {
        timeScaleFactor = 1e6;
    } else if (unit == "ms") {
        timeScaleFactor = 1e3;
    } else if (unit == "s") {
        timeScaleFactor = 1.0;
    }


    //qDebug() << "timeScaleFactor : " << timeScaleFactor;

    QVector<QPointF> convertedPoints;
    for (const QPointF &point : points) {
        double timeInSeconds = point.x() * timeScaleFactor;  // 将时间值转换为秒
        convertedPoints.append(QPointF(timeInSeconds, point.y()));
    }

    series->replace(convertedPoints);
}



void PDChart::setTitle(const QString &title)
{
    chart->setTitle(title);
}

void PDChart::setXAxisTitle(const QString &title)
{
    axisX->setTitleText(title);
}

void PDChart::setYAxisTitle(const QString &title)
{
    axisY->setTitleText(title);
}

void PDChart::setLineColor(const QColor &color)
{
    QPen pen = series->pen();
    pen.setColor(color);
    series->setPen(pen);
}

void PDChart::setLineWidth(qreal width)
{
    QPen pen = series->pen();
    pen.setWidthF(width);
    series->setPen(pen);
}

void PDChart::setXAxisRange(qreal min, qreal max)
{
    axisX->setRange(min, max);
}

void PDChart::setYAxisRange(qreal min, qreal max)
{
    axisY->setRange(min, max);
}

void PDChart::setMargins(int left, int top, int right, int bottom)
{
    chart->setMargins(QMargins(left, top, right, bottom));
}

void PDChart::setXAxisScale(qreal min, qreal max, qreal interval)
{
    axisX->setRange(min, max);
    axisX->setTickCount((max - min) / interval + 1); // 设置刻度数量
    axisX->setLabelFormat("%g"); // 格式化标签
}

void PDChart::setYAxisScale(qreal min, qreal max, qreal interval)
{
    axisY->setRange(min, max);
    axisY->setTickCount((max - min) / interval + 1); // 设置刻度数量
    axisY->setLabelFormat("%g"); // 格式化标签
}

void PDChart::clearData()
{
    series->clear();
    chart->update();
}

void PDChart::mousePressEvent(QMouseEvent *event)  {
    if (event->button() == Qt::MiddleButton) {
        m_bMiddleButtonPressed = true;
        m_oPrePos = event->pos();
        setCursor(Qt::OpenHandCursor);
    } else {
        // 如果不是中键按下，调用父类的默认处理（橡皮筋选择）
        QWidget::mousePressEvent(event);
    }
}


void PDChart::mouseReleaseEvent(QMouseEvent *event)  {
    if (event->button() == Qt::MiddleButton) {
        m_bMiddleButtonPressed = false;
        setCursor(Qt::ArrowCursor);
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void PDChart::wheelEvent(QWheelEvent *pEvent)
{

    //qreal rVal = std::pow(0.999 ,pEvent->pixelDelta().x());

    QRectF oPlotAreaRect = chart->plotArea();
    QPointF oCenterPoint = oPlotAreaRect.center();

    oPlotAreaRect.setWidth(oPlotAreaRect.width() );
    oPlotAreaRect.setHeight(oPlotAreaRect.height() );

    QPointF oNewCenterPoint(2*oCenterPoint - pEvent->position()-(oCenterPoint - pEvent->position()));
    oPlotAreaRect.moveCenter(oNewCenterPoint);
    chart->zoomIn(oPlotAreaRect);
}
