#include "singalconvert.h"
#include <fftw3.h>
#include <QDebug>
SingalConvert::SingalConvert() {}

//方波THD
double SingalConvert::calculateTHDForSquareWave(const QList<QPointF>& frequencyData, double fundamentalFrequency, int maxHarmonic) {
    double thd = 0.0;
    double fundamentalAmplitude = 0.0;

    // 找到基波幅值
    for (const QPointF& point : frequencyData) {
        if (qAbs(point.x() - fundamentalFrequency) < 1e-6) {
            fundamentalAmplitude = point.y();
            break;
        }
    }

    // 计算各次谐波幅值的平方和，并考虑谐波截断
    double harmonicPower = 0.0;
    for (const QPointF& point : frequencyData) {
        if (qAbs(point.x()) > fundamentalFrequency && qAbs(point.x()) <= maxHarmonic * fundamentalFrequency) {
            harmonicPower += qPow(point.y(), 2);
        }
    }

    // 计算THD
    if (fundamentalAmplitude != 0.0) {
        thd = qSqrt(harmonicPower) / fundamentalAmplitude;
    }

    return thd;
}

//正弦波THD
double SingalConvert::calculateTHD(const QList<QPointF>& frequencyData, double fundamentalFrequency){
    double thd = 0.0;
    double fundamentalAmplitude = 0.0;

    // 找到基波幅值
    for (const QPointF& point : frequencyData) {
        if (qAbs(point.x() - fundamentalFrequency) < 1e-6) {
            fundamentalAmplitude = point.y();
            break;
        }
    }
    // 计算各次谐波幅值的平方和
    double harmonicPower = 0.0;
    for (const QPointF& point : frequencyData) {
        if (qAbs(point.x()) > fundamentalFrequency) {
            harmonicPower += qPow(point.y(), 2);
        }
    }
    // 计算THD
    if (fundamentalAmplitude != 0.0) {
        thd = qSqrt(harmonicPower) / fundamentalAmplitude;
    }

    return thd;
}

QVector<QPointF> SingalConvert::performFFT(QVector<QPointF> dataPoints ,int unit){
    int N = dataPoints.size();
    double T = dataPoints[1].x() - dataPoints[0].x(); // 采样间隔

    // 分配输入和输出数组的内存
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // 填充输入数组
    for (int i = 0; i < N; ++i) {
        in[i][0] = dataPoints[i].y(); // 实部
        in[i][1] = 0.0; // 虚部
    }

    // 执行FFT
    fftw_execute(p);

    // 创建QVector来存储结果
    QVector<QPointF> results;

    // 计算频率和dBu单位的振幅，然后存储在QVector中
    for (int i = 0; i < N / 2; ++i) {
        double frequency_Hz = i / (T * N); // 计算频率 (Hz)
        // 将频率转换为 MHz
        double frequency_MHz = frequency_Hz / 1000000.0; // 或者写为 frequency_Hz / 1e6;
        double frequency_KHz = frequency_Hz / 1000.0; //千赫
        double amplitude = 2.0 / N * sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        double amplitude_dBu = 20 * log10(amplitude / 1e-6); // 转换为dBu
        if(unit == 0){
            results.append(QPointF(frequency_Hz, amplitude_dBu));
        }
        if(unit == 1){
            // 存储结果
            results.append(QPointF(frequency_KHz, amplitude_dBu));
        }
        if(unit == 2){
            // 存储结果
            results.append(QPointF(frequency_MHz, amplitude_dBu));
        }
    }

    // 清理
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return results;
}

qreal SingalConvert::findMaxY(const QVector<QPointF>& dataPoints) {
    if (dataPoints.isEmpty()) {
        return 0.0; // 或者返回一个合适的默认值
    }

    qreal maxY = dataPoints[0].y(); // 初始化为第一个点的 x 值

    for (const QPointF& point : dataPoints) {
        if (point.y() > maxY) {
            maxY = point.y();
        }
    }

    return maxY;
}

qreal SingalConvert::findMaxX(const QVector<QPointF>& dataPoints) {
    if (dataPoints.isEmpty()) {
        return 0.0; // 或者返回一个合适的默认值
    }

    qreal maxX = dataPoints[0].x(); // 初始化为第一个点的 x 值

    for (const QPointF& point : dataPoints) {
        if (point.x() > maxX) {
            maxX = point.x();
        }
    }

    return maxX;
}




