#ifndef FFTHANDLE_H
#define FFTHANDLE_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include <QVariant>
#include <QTableView>
#include <fftw3.h>
#include <omp.h>
class FFTHandle : public QThread
{
    Q_OBJECT
public:
    explicit FFTHandle(QObject *parent = nullptr);
    ~FFTHandle();
    void setDatas(QVector<QPointF>* _data);
    void setRawDatas(const QVector<double>* _rawData,double _timeIntervalNanoseconds);
    void run() override;
    void clearData();
signals:
    void fftReady(const std::vector<double> frequencies,std::vector<double> magnitudes);
    void porgressUpdated(int percentage);
    void samplingRateReady(double samplingRate);
    void sendLog(QString logs);
private:
    // 成员变量
    int maxHarmonics;           // 最大谐波阶数
    double* fft_in;             // FFT 输入数据缓存
    fftw_complex* fft_out;      // FFT 输出数据缓存
    const QVector<QPointF> *data;
    const QVector<double> *rawData;
    double timeIntervalNanoseconds;
    QString wisdomFileName;
    bool loadWFileSuccess;
    fftw_plan plan;
    int N; // 样本数
    double samplingRate; // 采样率

    void calculate();
    void calculateWitRawData();

     int fft_in_size = 0;  // 用来记录当前 fft_in 的大小

    bool loadWisdomFromFile(const QString& fileName);
    // 保存 Wisdom 到文件
    void saveWisdomToFile(const QString& fileName);
};

#endif // FFTHANDLE_H
