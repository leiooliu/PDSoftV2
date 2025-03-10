#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QVector>
#include <fftw3.h>

class Algorithm{
public :
    //三次差值函数
    static double cubicInterpolation(const QVector<double>& x,
                                     const QVector<double>& y,
                                     double targetX) {
        int n = x.size();
        if (n < 4) return y[0];  // 三次插值至少需要4个点

        // 找到最接近的点
        int i = 0;
        while (i < n - 1 && x[i + 1] < targetX) {
            i++;
        }

        // 获取插值点
        double x0 = x[i], x1 = x[i+1], x2 = x[i+2], x3 = x[i+3];
        double y0 = y[i], y1 = y[i+1], y2 = y[i+2], y3 = y[i+3];

        double dx0 = targetX - x0, dx1 = targetX - x1, dx2 = targetX - x2, dx3 = targetX - x3;

        // 三次插值公式
        return y0 * (1 - 3 * dx0 * dx0 + 2 * dx0 * dx0 * dx0) +
               y1 * (3 * dx1 * dx1 - 2 * dx1 * dx1 * dx1) +
               y2 * (3 * dx2 * dx2 - 2 * dx2 * dx2 * dx2) +
               y3 * (1 - 3 * dx3 * dx3 + 2 * dx3 * dx3 * dx3);
    }

    //计算频率
    static double calculateFrequency(const QVector<double>& data, double sampleInterval) {
        sampleInterval *= 1e-9; // 纳秒转秒
        int N = data.size();
        if (data.isEmpty() || N <= 1 || sampleInterval <= 0) {
            return 0.0;
        }

        // 零填充，提高分辨率
        int N_new = N * 4; // 零填充至4倍的大小
        QVector<double> paddedData(N_new, 0.0);
        for (int i = 0; i < N; ++i) {
            paddedData[i] = data[i];
        }

        // 分配内存
        double* in = fftw_alloc_real(N_new);
        fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N_new / 2 + 1));

        // 使用汉宁窗，计算窗函数总和
        double sumWindow = 0.0;
        for (int i = 0; i < N_new; ++i) {
            double window = 0.5 * (1 - cos(2 * M_PI * i / (N_new-1)));
            in[i] = paddedData[i] * window;
            sumWindow += window;
        }

        // 创建并执行FFT计划
        fftw_plan plan = fftw_plan_dft_r2c_1d(N_new, in, out, FFTW_ESTIMATE);
        fftw_execute(plan);

        // 跳直流，找最大幅值
        double maxMagnitude = 0.0;
        int peakIndex = 0;
        QVector<double> freqValues;
        QVector<double> magValues;

        for (int i = 1; i < N_new / 2 + 1; ++i) { // 从1开始跳直流
            // 计算幅值并补偿（汉宁窗补偿）
            double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]) * 2.0 / sumWindow;

            // 确保幅值为正
            if (magnitude < 0) {
                magnitude = 0;
            }

            freqValues.push_back(i * (1.0 / sampleInterval) / N_new); // 计算对应频率
            magValues.push_back(magnitude);

            if (magnitude > maxMagnitude) {
                maxMagnitude = magnitude;
                peakIndex = i;
            }
        }

        // 使用三次插值方法优化频率计算
        double frequency = cubicInterpolation(freqValues, magValues, peakIndex * (1.0 / sampleInterval) / N_new);

        // 确保频率非负
        if (frequency < 0) {
            frequency = 0;
        }

        // 释放资源
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        return frequency;
    }
};

#endif // ALGORITHM_H
