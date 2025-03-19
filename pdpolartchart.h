#ifndef PDPOLARTCHART_H
#define PDPOLARTCHART_H

#include <QWidget>
#include <QChartView>
#include <zoomchartview.h>
#include <pdchartview.h>
#include <QPolarChart>
#include <QScatterSeries>
#include <QVBoxLayout>
#include <QDebug>
class PDPolartChart : public QWidget
{
    Q_OBJECT
public:
    explicit PDPolartChart(QWidget *parent = nullptr);
    QPolarChart *chart;
    void setData(const std::vector<double> magnitudes ,const std::vector<double> phases);
    void testData();
    void clear();
signals:

private:
    QChartView *chartView;
    QScatterSeries *series;
};

#endif // PDPOLARTCHART_H
