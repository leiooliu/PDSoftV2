#ifndef HARMONIC_H
#define HARMONIC_H

#include <QWidget>
#include "SegmentHandle.h"  // 引入SegmentHandle类
#include <TimeBaseLoader.h>
#include "renderpolartchart.h"
#include "rendertimechart.h"
#include <harmonictablemodel.h>
#include <fftanalyzer.h>
#include "renderfrequencychart.h"
#include "ffthandle.h"
#include <autoexecutor.h>
#include "configloader.h"
#include <enummap.h>
#include <settings.h>
#include <peakparam.h>
#include <renderfrequencychart.h>

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
    void on_pushButton_14_clicked();
    void on_cb_Timebase_currentIndexChanged(int index);
    void on_cb_Voltage_currentIndexChanged(int index);
    void on_dsb_time_x_max_valueChanged(double arg1);
    void on_dsb_time_x_min_valueChanged(double arg1);
    void on_dsb_frequency_x_max_valueChanged(double arg1);
    void on_dsb_frequency_x_min_valueChanged(double arg1);
    void on_reloadConfig_btn_clicked();
    void on_checkBox_stateChanged(int arg1);
    void on_pushButton_17_clicked();

    void on_pushButton_15_clicked();

private:
    Ui::harmonic *ui;
    void onDataReady(const QVector<QPointF> &data);
    void onRawDataReady(const QVector<double> &rawdata,double timeIntervalNanoseconds);

    void onTestDataReady(const QVector<double> &rawdata,double timeIntervalNanoseconds);

    void updateProgress(int percentage);
    void renderTimeChartFinash(const QVector<QPointF> &data ,const QVector<QPointF> &pulseDatas ,double timeMultiplier);
    void renderFrequencyChartFinash(const QVector<QPointF> &data);
    SegmentHandle *segmentHandle;  // 用于启动数据采集的线程
    void onTestReady(double ns ,double minVolts ,double maxVolts);

    void clearPolartChart();

    QVector<TimeBase> timeBaseList;
    RenderTimeChart *timeChart;
    RenderFrequencyChart *renderFrequncyChart;
    RenderPolartChart *polartChart;
    QVector<QPointF> bufferedData;
    QVector<double> bufferedRawData;
    //绑定tableView
    HarmonicTableModel *tableModel;
    QVector<QVector<QVariant>> results;
    void harmonicRunReady(const QVector<QVector<QVariant>> result);
    FFTAnalyzer *analyzer;
    FFTHandle *fftHandle;
    FFTHandle *pulseFftHandle;
    void fftReady(std::vector<double> frequencies,std::vector<double> magnitudes,std::vector<double> phases);
    void fftPluseReady(std::vector<double> frequencies,std::vector<double> magnitudes,std::vector<double> phases);
    void samplingRateLoad(double samplingRate);
    void recvLog(QString log);
    TimeBase currentTimebase;


    double _timeIntervalNanoseconds;
    PeakParam peakParam;


    bool isRunning;
    ConfigSetting configSetting;
    int currentTimebaseIndex;

    void loadSettings();

    EnumBinder<PS2000A_RANGE> *binderVoltage;
    EnumBinder<enPS2000AChannel> *binderChannel;
    EnumBinder<enPS2000ACoupling> *binderCoupling;

    PS2000A_RANGE cunnentRange;

    Settings *setWin;
    void settingFinshed();
};

#endif // HARMONIC_H
