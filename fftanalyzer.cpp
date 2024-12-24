#include "fftanalyzer.h"
#include <fftw3.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <thread>
// 构造函数
FFTAnalyzer::FFTAnalyzer(int maxHarmonics)
    : maxHarmonics(maxHarmonics),fft_in(nullptr), fft_out(nullptr) {

    fftw_init_threads();
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // 默认线程数
    fftw_plan_with_nthreads(numThreads);

    // 预分配 FFT 缓存，假设最大支持 1M 数据
    const int MAX_FFT_SIZE = 2048 * 2048; // 可根据需求调整
    fft_in = (double*)fftw_malloc(sizeof(double) * MAX_FFT_SIZE);
    fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MAX_FFT_SIZE);
}

// 析构函数
FFTAnalyzer::~FFTAnalyzer() {
    fftw_free(fft_in);
    fftw_free(fft_out);
    fftw_cleanup_threads(); // 清理多线程资源
}

void FFTAnalyzer::run(){
    if (data->size() < 2) {
        std::cerr << "输入数据不足，无法进行FFT分析。" << std::endl;
        return;
    }

    // 提取时间和电压数据
    std::vector<double> time, voltage;
    for (const auto& point : *data) {
        time.push_back(point.x());
        voltage.push_back(point.y());
    }

    // 确保数据是按时间排序的（如果有必要）
    if (!std::is_sorted(time.begin(), time.end())) {
        std::cerr << "输入数据未按时间排序，请检查输入数据。" << std::endl;
        return;
    }

    // 计算采样率
    double samplingRate = 1.0 / (time[1] - time[0]);

    // 准备 FFT 数据
    int N = voltage.size();
    fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    fft_in = (double*)fftw_malloc(sizeof(double) * N);

    for (int i = 0; i < N; ++i) {
        fft_in[i] = voltage[i];
    }

    // 执行 FFT
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, fft_in, fft_out, FFTW_ESTIMATE);
    fftw_execute(plan);

    // 计算频率和幅值
    std::vector<double> frequencies(N / 2);
    std::vector<double> magnitudes(N / 2);
    int maxTimeCount = maxHarmonics + (N / 2);
    for (int i = 0; i < N / 2; ++i) {
        magnitudes[i] = sqrt(fft_out[i][0] * fft_out[i][0] + fft_out[i][1] * fft_out[i][1]);
        frequencies[i] = i * samplingRate / N;

        // 更新 FFT 数据计算进度（0% - 50%）
        if (i % 1000 == 0 || i == N / 2 - 1) {
            int progress = static_cast<int>((i / static_cast<float>(N / 2)) * 50); // FFT阶段更新到50%
            emit progressUpdated(progress);
        }
    }

    // 自动检测基频
    double fundamentalFrequency = 0.0;
    double fundamentalEnergy = 0.0;

    if (customFundamentalFrequency > 0) {
        // 使用用户提供的基频
        fundamentalFrequency = customFundamentalFrequency;

        // 找到最接近用户指定基频的频谱点
        auto closestIt = std::min_element(frequencies.begin(), frequencies.end(),
                                          [=](double a, double b) {
                                              return std::abs(a - customFundamentalFrequency) < std::abs(b - customFundamentalFrequency);
                                          });
        int closestIndex = std::distance(frequencies.begin(), closestIt);
        double fundamentalMagnitude = magnitudes[closestIndex];
        fundamentalEnergy = fundamentalMagnitude * fundamentalMagnitude; // 基频的能量
    } else {
        // 自动检测基频
        auto maxIt = std::max_element(magnitudes.begin(), magnitudes.end());
        int peakIndex = std::distance(magnitudes.begin(), maxIt);
        fundamentalFrequency = frequencies[peakIndex];
        double fundamentalMagnitude = magnitudes[peakIndex];
        fundamentalEnergy = fundamentalMagnitude * fundamentalMagnitude; // 基频的能量
    }

    // 计算谐波
    QVector<QVector<QVariant>> results; // 用于绑定 TableView 的结果

    // ====== 1. 预计算频率索引映射 ======
    std::vector<int> freqIndexMap(maxHarmonics + 1); // 频率索引映射表
    for (int n = 1; n <= maxHarmonics; ++n) {
        double harmonicFreq = fundamentalFrequency * n;

        // 使用二分查找找到最近的频率索引
        auto it = std::lower_bound(frequencies.begin(), frequencies.end(), harmonicFreq);
        int idx = std::distance(frequencies.begin(), it);

        // 检查前后点，选择更接近的频率点
        if (idx > 0 && (std::abs(frequencies[idx - 1] - harmonicFreq) < std::abs(frequencies[idx] - harmonicFreq))) {
            --idx;
        }

        freqIndexMap[n] = idx; // 保存索引映射
    }

    // ====== 2. 谐波分析 ======
    for (int n = 2; n <= maxHarmonics; ++n) {
        // 使用预计算的索引直接获取最近频率点
        int closestIndex = freqIndexMap[n];
        double harmonicMagnitude = magnitudes[closestIndex]; // 幅值
        double harmonicEnergy = harmonicMagnitude * harmonicMagnitude; // 能量
        double harmonicRatio = harmonicEnergy / fundamentalEnergy * 100.0; // 能量占比

        // 保存结果
        results.append({
            n,                                 // 谐波阶数
            QString::number(fundamentalFrequency * n, 'f', 1), // 谐波频率，保留6位小数
            QString::number(harmonicEnergy, 'f', 2),           // 谐波能量，保留6位小数
            QString::number(harmonicRatio, 'f', 6)             // 谐波能量占比，保留2位小数
        });

        // ====== 3. 更新进度条 ======
        if (n % 100 == 0 || n == maxHarmonics) { // 每 100 次更新一次进度条
            int progress = 50 + static_cast<int>((n / static_cast<float>(maxHarmonics)) * 50); // 映射到 50% - 100%
            emit progressUpdated(progress);
        }
    }

    // ====== 4. 最终进度更新 ======
    emit progressUpdated(100); // 确保进度条更新到 100%
    //emit progressUpdated(finalProgress);  // 发出进度信号，确保到达100%

    // 清理 FFTW
    fftw_destroy_plan(plan);

    emit dataReady(results);

    //return results;
}

// 分析函数
void FFTAnalyzer::analyze(QVector<QPointF>* _data,double _customFundamentalFrequency) {
    data = _data;
    customFundamentalFrequency = _customFundamentalFrequency;
}

void FFTAnalyzer::exportTableToCSV(QTableView* tableView, const QString& filePath) {
    if (!tableView || !tableView->model()) {
        QMessageBox::warning(nullptr, "导出失败", "没有有效的数据模型！");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "导出失败", "无法创建文件：" + filePath);
        return;
    }

    QTextStream stream(&file);
    QAbstractItemModel* model = tableView->model();

    // 写入表头
    for (int col = 0; col < model->columnCount(); ++col) {
        stream << model->headerData(col, Qt::Horizontal).toString();
        if (col < model->columnCount() - 1) {
            stream << ","; // 用逗号分隔列
        }
    }
    stream << "\n";

    // 写入数据行
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            stream << model->data(model->index(row, col)).toString();
            if (col < model->columnCount() - 1) {
                stream << ","; // 用逗号分隔列
            }
        }
        stream << "\n"; // 换行
    }

    file.close();

    QMessageBox::information(nullptr, "导出成功", "数据已成功导出到：" + filePath);
}
