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

QVector<QPointF> SingalConvert::convertToFrequencyDomain(const QVector<QPointF>& dataPoints) {
    int N = dataPoints.size();
    if (N == 0) return QVector<QPointF>();

    // 提取时域信号的 y 值
    QVector<double> timeSignal(N);
    for (int i = 0; i < N; ++i) {
        timeSignal[i] = dataPoints[i].y();
    }

    // 准备 FFT 输入输出数组
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    double* in = (double*)fftw_malloc(sizeof(double) * N);

    // 将数据复制到 FFT 输入
    for (int i = 0; i < N; ++i) {
        in[i] = timeSignal[i];
    }

    // 创建 FFT 计划并执行
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    // 计算频域幅值，并存入 QVector<QPointF>
    QVector<QPointF> frequencyDomain;
    double freqResolution = 1.0;  // 假设采样频率为 1 Hz，可根据实际采样率调整
    for (int i = 0; i < N / 2 + 1; ++i) {
        double frequency = i * freqResolution;
        double magnitude = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        frequencyDomain.append(QPointF(frequency, magnitude));
    }

    // 清理 FFTW 资源
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return frequencyDomain;
}

QVector<QPointF> SingalConvert::performFFT(QVector<QPointF> dataPoints ,int unit,bool isConvert){
    int N = dataPoints.size();
    if (N < 2) {
        qWarning() << "Insufficient data points for FFT.";
        return {};
    }

    // 计算采样间隔
    double T = dataPoints[1].x() - dataPoints[0].x();
    if (isConvert) {
        T *= 1e3;  // 单位转换（纳秒转微秒等）
    }
    if (T <= 0) {
        qWarning() << "Invalid sampling interval (T):" << T;
        return {};
    }

    // 检查 unit 参数
    if (unit < 0 || unit >= 3) {
        qWarning() << "Invalid unit parameter. Must be 0 (Hz), 1 (kHz), or 2 (MHz).";
        return {};
    }

    // FFT 输入数据（填充 real 和 imag 部分）
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // 填充输入数据（仅使用 y 值作为实部，虚部为零）
    for (int i = 0; i < N; ++i) {
        in[i][0] = dataPoints[i].y(); // 实部
        in[i][1] = 0.0;              // 虚部
    }

    fftw_execute(p);  // 执行 FFT

    QVector<QPointF> results;
    const double unitConversion[] = {1.0, 1e-3, 1e-6};  // Hz, kHz, MHz 单位转换
    double referenceVoltage = 0.775;  // dBu 的参考电压
    double bandwidth = 1.0 / (T * N);  // 频率分辨率

    // 提取频率和幅值
    for (int i = 0; i < N / 2; ++i) {
        double frequency = i * bandwidth * unitConversion[unit];
        double amplitude = (i == 0 || i == N / 2) ?
                               sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]) / N :
                               2.0 * sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]) / N;

        double amplitude_rms = amplitude / sqrt(2.0);
        double amplitude_dBu = 20 * log10(amplitude_rms / referenceVoltage);
        results.append(QPointF(frequency, amplitude_dBu));
    }

    fftw_destroy_plan(p);  // 销毁 FFT 计划
    fftw_free(in);         // 释放输入数据
    fftw_free(out);        // 释放输出数据

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


double SingalConvert::calculateFrequency(const QVector<QPointF>& samples, double sampling_rate) {
    size_t N = samples.size();
    if (N < 2) return 0;

    // 计算信号均值
    double mean = std::accumulate(samples.begin(), samples.end(), 0.0,
                                  [](double sum, const QPointF& p) { return sum + p.y(); }) / N;

    // 计算采样率
    double total_time = samples.back().x() - samples.front().x();
    if (total_time <= 0) return 0;  // 检查时间跨度
    double sampling_rate_corrected = (N - 1) / total_time;

    // 零交叉点计数
    size_t zero_crossings = 0;
    for (size_t i = 1; i < N; ++i) {
        double y1 = samples[i - 1].y() - mean;
        double y2 = samples[i].y() - mean;

        if (y1 * y2 < 0) { // 检测零交叉
            zero_crossings++;
        }
    }

    // 检查零交叉数
    if (zero_crossings < 2) return 0;

    // 计算频率
    double frequency = sampling_rate_corrected * (zero_crossings / 2.0) / N;

    return frequency;
}

double SingalConvert::measurePeakValue(const QVector<QPointF> data){
    if (data.isEmpty()) {
        qDebug() << "数据为空";
        return -1;
    }

    double peakVal = 0.0;

    // 遍历数据，找到最大绝对值
    for (const QPointF& point : data) {
        double value = point.y();  // 信号幅值
        double absValue = std::abs(value);  // 获取绝对值

        if (absValue > peakVal) {
            peakVal = absValue;  // 更新峰值
        }
    }

    return peakVal;
}

//测量信号峰峰值
double SingalConvert::measureAmplitude(const QVector<QPointF> data){
    if (data.isEmpty()) {
        qDebug() << "数据为空";
        return -1;
    }

    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    // 遍历数据，找到最大值和最小值
    for (const QPointF& point : data) {
        double value = point.y();  // 信号幅值
        if (value < minVal) minVal = value;
        if (value > maxVal) maxVal = value;
    }

    // 计算峰峰值（Peak-to-Peak Amplitude）
    double peakToPeakAmplitude = maxVal - minVal;
    return peakToPeakAmplitude;
}
//测量基波幅值
double SingalConvert::calculateFundamentalAmplitude(const QVector<QPointF>& data){
    // 执行FFT，获得频谱数据
    auto spectrum = fft(data);

    // 基波幅值（第一个非零频率分量的幅值）
    double fundamentalAmplitude = std::abs(spectrum[1]) / data.size();

    // 如果需要峰峰值，可乘以2
    return fundamentalAmplitude * 2;
}

std::vector<std::complex<double>> SingalConvert::fft(const QVector<QPointF>& data) {
    int N = data.size();
    std::vector<std::complex<double>> X(N);

    for (int k = 0; k < N; ++k) {
        std::complex<double> sum(0, 0);
        for (int n = 0; n < N; ++n) {
            double angle = 2 * M_PI * k * n / N;
            sum += std::polar(data[n].y(), -angle);
        }
        X[k] = sum;
    }
    return X;
}


void SingalConvert::analyzeHarmonics(const QVector<QPointF>& fft_data,int max_harmonics) {
    int n = fft_data.size();

    // 确保数据足够处理
    if (n == 0) {
        qDebug() << "FFT data is empty!";
        return;
    }

    // 提取频率和幅度
    QVector<double> frequencies(n), magnitudes(n);
    for (int i = 0; i < n; ++i) {
        frequencies[i] = fft_data[i].x();
        magnitudes[i] = fft_data[i].y();
    }

    // 找到主频率和最大幅值
    auto max_iter = std::max_element(magnitudes.begin(), magnitudes.end());
    int peak_index = std::distance(magnitudes.begin(), max_iter);
    double fundamental_frequency = 50;
    double fundamental_magnitude = std::abs(magnitudes[peak_index]);

    // 识别谐波频率（2x, 3x, ..., max_harmonics）
    QVector<double> harmonic_frequencies;
    for (int i = 2; i <= max_harmonics; ++i) {
        harmonic_frequencies.append(fundamental_frequency * i);
    }

    // 找到与谐波最接近的实际频率
    QVector<double> harmonic_magnitudes;
    for (double harmonic : harmonic_frequencies) {
        auto closest_iter = std::min_element(frequencies.begin(), frequencies.end(),
                                             [harmonic](double a, double b) {
                                                 return std::abs(a - harmonic) < std::abs(b - harmonic);
                                             });
        int closest_index = std::distance(frequencies.begin(), closest_iter);
        harmonic_magnitudes.append(magnitudes[closest_index]);
    }

    QVector<double> harmonic_ratios;
    for (double magnitude : harmonic_magnitudes) {
        harmonic_ratios.append(std::abs(magnitude) / fundamental_magnitude);
    }

    // 打印谐波分析结果
    qDebug() << "Harmonic Order | Harmonic Frequency (Hz) | Harmonic Magnitude | Ratio to Fundamental (%)";
    for (int i = 0; i < harmonic_frequencies.size(); ++i) {
        qDebug() << i + 2 << "\t\t" // Harmonic Order
                 << harmonic_frequencies[i] << "\t\t" // Harmonic Frequency
                 << harmonic_magnitudes[i] << "\t\t" // Harmonic Magnitude
                 << harmonic_ratios[i] * 100 << "%"; // Ratio to Fundamental
    }
}
