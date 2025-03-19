#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QVector>
#include <fftw3.h>
#include <TimeBaseLoader.h>
#include <ps2000aApi.h>
#include <tools.h>

class Algorithm{
public :
    static double calculateFrequency(const QVector<double>& data, double sampleInterval,double offset) {
        sampleInterval *= 1e-9; // 纳秒转秒
        int N = data.size();
        if (data.isEmpty() || N <= 1 || sampleInterval <= 0) {
            return 0.0;
        }

        if(offset <= 0)
        {
            offset = 0.5;
        }

        // 分配内存
        int outSize = N/2 + 1;
        double* in = fftw_alloc_real(N);
        fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * outSize);

        // 使用汉宁窗，计算窗函数总和
        double sumWindow = 0.0;
        for (int i = 0; i < N; ++i) {
            double window = 0.5 * (1 - cos(2 * M_PI * i / (N-1)));

            //对瞬态信号改用平顶窗口
            // double window = 0.21557895
            //                 - 0.41663158 * cos(2*M_PI*i/(N-1))
            //                 + 0.277263158 * cos(4*M_PI*i/(N-1))
            //                 - 0.083578947 * cos(6*M_PI*i/(N-1))
            //                 + 0.006947368 * cos(8*M_PI*i/(N-1));

            in[i] = data[i] * window;
            sumWindow += window;
        }

        // 创建并执行FFT计划
        fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
        fftw_execute(plan);

        // 跳直流，找最大幅值
        double maxMagnitude = 0.0;
        int peakIndex = 0;
        for (int i = 1; i < outSize; ++i) { // 从1开始跳直流
            // 计算幅值并补偿（汉宁窗补偿）
            double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]) * 2.0 / sumWindow;

            if (magnitude > maxMagnitude) {
                maxMagnitude = magnitude;
                peakIndex = i;
            }
        }

        // 抛物线插值优化
        double Fs = 1.0 / sampleInterval;
        double frequency = peakIndex * Fs / N;

        if (peakIndex > 0 && peakIndex < outSize-1) {
            // 获取相邻频点幅值（需重新计算补偿）
            double magPrev = sqrt(out[peakIndex-1][0]*out[peakIndex-1][0] +
                                  out[peakIndex-1][1]*out[peakIndex-1][1]) * 2.0 / sumWindow;
            double magNext = sqrt(out[peakIndex+1][0]*out[peakIndex+1][0] +
                                  out[peakIndex+1][1]*out[peakIndex+1][1]) * 2.0 / sumWindow;

            // 算频率偏移量
            double delta = offset * (magPrev - magNext) / (magPrev - 2*maxMagnitude + magNext);
            frequency = (peakIndex + delta) * Fs / N;
        }

        // 释放资源
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);

        return frequency;
    }

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
    static double calculateFrequencyByCubicInterpolation(const QVector<double>& data, double sampleInterval) {
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


    //计算时域数据，并得到当前信号的最大和最小幅值
    static void calculateTimeData(const QVector<double> sourceData ,
                                    PS2000A_RANGE range,
                                    TimeBase timebase ,
                                    QVector<QPointF> &_datas ,
                                    double &minVoltsValue ,
                                    double &maxVoltsValue){
        QString unit = timebase.unit;
        //int unitValue = timebase.gridValue; // 这个要拆解示波器的数字
        double timeIntervalNanoseconds = timebase.interval;
        double timeMultiplier;

        // 清空旧数据，防止重复
        _datas.clear();

        // 定义时间倍率
        timeMultiplier = 1.0;
        if (unit == "ns") {
            timeMultiplier = 1e-9;  // 纳秒
            if (timebase.conversion) {
                timeMultiplier = 1e-6;  // 如果转换标志为 true，将单位调整为微秒
            }
        } else if (unit == "us") {
            timeMultiplier = 1e-6;  // 微秒
            if (timebase.conversion) {
                timeMultiplier = 1e-3;  // 如果转换标志为 true，将单位调整为毫秒
            }
        } else if (unit == "ms") {
            timeMultiplier = 1e-3;  // 毫秒
            if (timebase.conversion) {
                timeMultiplier = 1.0;  // 如果转换标志为 true，将单位调整为秒
            }
        } else if (unit == "s") {
            timeMultiplier = 1.0;   // 秒
        } else {

        }

        // 将采样间隔从纳秒转换为秒
        double interval = timeIntervalNanoseconds * 1e-9; // 单位转换为秒

        // 提前分配内存
        _datas.reserve(sourceData.size());

        maxVoltsValue = std::numeric_limits<double>::lowest(); // 初始值设为最小
        minVoltsValue = std::numeric_limits<double>::max();    // 初始值设为最大

        for (int i = 0; i < sourceData.size(); ++i) {
            double time = i * interval / timeMultiplier;
            double volts = PDTools::adcToVolts(sourceData[i], range) * 1000;
            QPointF point = QPointF(time, volts);
            _datas.append(point);

            // 实时更新最大值和最小值
            if (volts > maxVoltsValue) maxVoltsValue = volts;
            if (volts < minVoltsValue) minVoltsValue = volts;
        }
    }

    //差值阈值计算
    static QVector<QPointF> getDynamicThresholdPulses(const QVector<QPointF>& datas) {
        if (datas.isEmpty()) return QVector<QPointF>();

        // 计算均值
        double sum = 0.0;
        for (const auto& point : datas) sum += point.y();
        double mean = sum / datas.size();

        // 计算标准差
        double variance = 0.0;
        for (const auto& point : datas) {
            variance += pow(point.y() - mean, 2);
        }
        double stddev = sqrt(variance / datas.size());

        // 设置动态阈值（例如均值+3倍标准差）
        double threshold = mean + 5 * stddev;

        // 提取超过阈值的点
        QVector<QPointF> pulseData;
        for (const auto& point : datas) {
            if (point.y() >= threshold) {
                pulseData.append(point);
            }
        }
        return pulseData;
    }

    static QVector<QPointF> getPeakPulses(const QVector<QPointF>& datas) {
        QVector<QPointF> pulseData;
        if (datas.size() < 3) return pulseData; // 至少需要三个点

        for (int i = 1; i < datas.size() - 1; ++i) {
            double prevY = datas[i-1].y();
            double currY = datas[i].y();
            double nextY = datas[i+1].y();

            if (currY > prevY && currY > nextY) {
                pulseData.append(datas[i]);
            }
        }
        return pulseData;
    }

    static QVector<QPointF> getPulseByThreshold(const QVector<QPointF>& datas, double threshold) {
        QVector<QPointF> pulseData;
        for (const QPointF& point : datas) {
            if (point.y() >= threshold) {
                pulseData.append(point);
            }
        }
        return pulseData;
    }

    static QVector<QPointF> extractPulseData(const QVector<QPointF>& _data, double threshold)
    {
        QVector<QPointF> pulseDatas;
        bool inPulse = false;
        QVector<QPointF> currentPulse;

        for (int i = 0; i < _data.size(); ++i) {
            if (_data[i].y() > threshold) {
                // 当前数据点处于脉冲中
                if (!inPulse) {
                    // 刚刚进入脉冲区域，清空当前脉冲数据
                    inPulse = true;
                    currentPulse.clear();
                }
                currentPulse.append(_data[i]);
            } else {
                if (inPulse) {
                    // 脉冲结束，根据需求处理 currentPulse
                    // 例如，这里取当前脉冲中间的点作为代表
                    int midIndex = currentPulse.size() / 2;
                    pulseDatas.append(currentPulse[midIndex]);
                    inPulse = false;
                }
            }
        }

        // 如果最后一个数据点仍处于脉冲中，处理最后的脉冲数据
        if (inPulse && !currentPulse.isEmpty()) {
            int midIndex = currentPulse.size() / 2;
            pulseDatas.append(currentPulse[midIndex]);
        }

        return pulseDatas;
    }

    static QVector<QPointF> findSpikesBySlope(const QVector<QPointF>& data, double slopeThreshold)
    {
        QVector<QPointF> spikes;
        if (data.size() < 2) {
            return spikes;
        }

        for (int i = 1; i < data.size(); ++i) {
            double dx = data[i].x() - data[i - 1].x();
            // 如果 x 坐标是等间隔采样，也可以直接认为 dx=1.0 或 dx=采样周期
            if (dx == 0) {
                continue; // 避免除 0
            }

            double dy = data[i].y() - data[i - 1].y();
            //double slope = dy / dx;

            if (std::fabs(dy) > slopeThreshold) {
                spikes.append(data[i]);
            }

            //qDebug() << "Point" << i << "slope:" << slope;  // 打印调试信息

            // if (std::fabs(slope) > slopeThreshold) {
            //     // 认为在 data[i] 附近存在明显突起
            //     spikes.append(data[i]);
            // }
        }

        return spikes;
    }


};

#endif // ALGORITHM_H
