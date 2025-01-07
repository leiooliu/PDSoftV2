#ifndef HARMONIC_H
#define HARMONIC_H

#include <QWidget>
#include "SegmentHandle.h"  // 引入SegmentHandle类
#include <TimeBaseLoader.h>
#include "rendertimechart.h"
#include <harmonictablemodel.h>
#include <fftanalyzer.h>
#include "renderfrequencychart.h"
#include "ffthandle.h"

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
    void on_pushButton_6_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

private:
    Ui::harmonic *ui;
    void onDataReady(const QVector<QPointF> &data);
    void onRawDataReady(const QVector<double> &rawdata,double timeIntervalNanoseconds);
    void updateProgress(int percentage);
    void renderTimeChartFinash(const QVector<QPointF> &data);
    void renderFrequencyChartFinash(const QVector<QPointF> &data);
    SegmentHandle *segmentHandle;  // 用于启动数据采集的线程
    QVector<TimeBase> timeBaseList;
    RenderTimeChart *timeChart;
    RenderFrequencyChart *renderFrequncyChart;
    QVector<QPointF> bufferedData;
    QVector<double> bufferedRawData;
    //绑定tableView
    HarmonicTableModel *tableModel;
    QVector<QVector<QVariant>> results ;
    void harmonicRunReady(const QVector<QVector<QVariant>> result);
    FFTAnalyzer *analyzer;
    FFTHandle *fftHandle;
    void fftReady(std::vector<double> frequencies,std::vector<double> magnitudes);
    void samplingRateLoad(double samplingRate);
    void recvLog(QString log);
    TimeBase currentTimebase;
    double calculateFrequency(const QVector<double>& data, double sampleInterval);
    double _timeIntervalNanoseconds;
};

#endif // HARMONIC_H
