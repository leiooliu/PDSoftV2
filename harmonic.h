#ifndef HARMONIC_H
#define HARMONIC_H

#include <QWidget>
#include "SegmentHandle.h"  // 引入SegmentHandle类
#include <TimeBaseLoader.h>
#include "rendertimechart.h"
#include <harmonictablemodel.h>
#include <fftanalyzer.h>
namespace Ui {
class harmonic;
}

class harmonic : public QWidget
{
    Q_OBJECT

public:
    explicit harmonic(QWidget *parent = nullptr);
    ~harmonic();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

private:
    Ui::harmonic *ui;
    void onDataReady(const QVector<QPointF> &data);
    void updateProgress(int percentage);
    void renderTimeChartFinash();
    SegmentHandle *segmentHandle;  // 用于启动数据采集的线程
    QVector<TimeBase> timeBaseList;
    RenderTimeChart *timeChart;
    QVector<QPointF> bufferedData;
    //绑定tableView
    HarmonicTableModel *tableModel;
    QVector<QVector<QVariant>> results ;
    void harmonicRunReady(const QVector<QVector<QVariant>> result);
    FFTAnalyzer *analyzer;

};

#endif // HARMONIC_H
