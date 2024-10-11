#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <picoparam.h>
#include <picoscopehandler.h>
#include <TimeBaseLoader.h>
#include <enummap.h>
#include <QThread>
#include <pdchart.h>
#include <singalconvert.h>
#include <tablerender.h>
#include <filehandle.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_cb_Timebase_currentIndexChanged(int index);
    void on_cb_Voltage_currentIndexChanged(int index);
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_cb_HZUnit_currentIndexChanged(int index);
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_sb_timebase_valueChanged(int arg1);

    void on_listView_clicked(const QModelIndex &index);

    void on_pushButton_12_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

private:
    Ui::MainWindow *ui;
    PicoParam *picoParam;
    PicoScopeHandler *picohandle;
    QVector<TimeBase> timeBaseList;
    EnumBinder<PS2000A_RANGE> *binderVoltage;
    EnumBinder<enPS2000AChannel> *binderChannel;
    EnumBinder<enPS2000ACoupling> *binderCoupling;

    //数据线程
    //时域线程
    QThread *dataThread;
    QThread *simulationDataThread;
    //频域线程
    QThread *fftThread;

    //图标
    PDChart *chartView;
    //FFT和数据转化
    SingalConvert *singalConvert;

    QVector<QPointF> bufferedData;

    int frequencyUnit;

    tablerender *tbRander;

    QVector<QPointF> fftData;

    QVector<QString> headers;

    QStandardItemModel *itemModel;

    void appendCacheList();
    void updateGraph(const QVector<QPointF> bufferedData);
};
#endif // MAINWINDOW_H
