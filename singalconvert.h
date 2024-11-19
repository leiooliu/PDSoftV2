#ifndef SINGALCONVERT_H
#define SINGALCONVERT_H
#include <QVector>
#include <QPointF>
#include <cmath>
#include <complex>
#include <vector>
class SingalConvert
{
public:
    SingalConvert();
    QVector<QPointF> performFFT(QVector<QPointF> dataPoints,int unit,bool isConvert);
    qreal findMaxX(const QVector<QPointF>& dataPoints);
    qreal findMaxY(const QVector<QPointF>& dataPoints);
    double calculateTHD(const QList<QPointF>& frequencyData, double fundamentalFrequency);
    double calculateTHDForSquareWave(const QList<QPointF>& frequencyData, double fundamentalFrequency, int maxHarmonic);
    //计算信号数据频率
    double calculateFrequency(const QVector<QPointF>& samples, double sampling_rate);
    //测量信号峰峰值
    double measureAmplitude(const QVector<QPointF> data);
    //测量信号峰值
    double measurePeakValue(const QVector<QPointF> data);
    //测量基波幅值
    double calculateFundamentalAmplitude(const QVector<QPointF>& data);
    std::vector<std::complex<double>> fft(const QVector<QPointF>& data);

    QVector<QPointF> convertToFrequencyDomain(const QVector<QPointF>& dataPoints);
};

#endif // SINGALCONVERT_H
