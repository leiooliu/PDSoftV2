#include "ffthandle.h"
#include <fftw3.h>
#include <omp.h>
#include <iostream>
#include <QDebug>
#include <QFile>
#include <chrono>
#include <QCoreApplication>

FFTHandle::FFTHandle(QObject *parent)
    : QThread{parent},fft_in(nullptr), fft_out(nullptr)
{
    // 启用FFTW多线程支持
    fftw_init_threads();
    //fftw_plan_with_nthreads(QThread::idealThreadCount()); // 使用最佳线程数
    fftw_plan_with_nthreads(8);
    // 加载任务计划缓存文件
    wisdomFileName = QCoreApplication::applicationDirPath() + "/fft_wisdom.dat";
    loadWFileSuccess = loadWisdomFromFile(wisdomFileName);
    // // 预分配 FFT 缓存，假设最大支持 1M 数据
    // const int MAX_FFT_SIZE = 2048 * 2048; // 可根据需求调整
    // fft_in = (double*)fftw_malloc(sizeof(double) * MAX_FFT_SIZE);
    // fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MAX_FFT_SIZE);
    // plan = fftw_plan_dft_r2c_1d(N, fft_in, fft_out, FFTW_MEASURE);
}
// 析构函数
FFTHandle::~FFTHandle() {
    fftw_cleanup_threads(); // 清理多线程环境
}
void FFTHandle::clearData(){
 // 释放未使用的内存
}

// 加载 Wisdom 文件
bool FFTHandle::loadWisdomFromFile(const QString& fileName) {
    if (QFile::exists(fileName)) {
        fftw_import_wisdom_from_filename(fileName.toStdString().c_str());
        return true;
    }
    return false;
}


// 保存 Wisdom 到文件
void FFTHandle::saveWisdomToFile(const QString& fileName) {
    fftw_export_wisdom_to_filename(fileName.toStdString().c_str());
}


void FFTHandle::setRawDatas(const QVector<double>* _rawData ,double _timeIntervalNanoseconds){
    rawData = _rawData;
    timeIntervalNanoseconds = _timeIntervalNanoseconds;
}

void FFTHandle::setDatas(QVector<QPointF>* _data ,double _timeMultiplier){
    data = _data;
    timeMultiplier = _timeMultiplier;
}

void FFTHandle::calculateNew(){

}

void FFTHandle::calculate(){
    if (data->size() < 2) {
        std::cerr << "输入数据不足，无法进行FFT分析。" << std::endl;
        emit sendLog("输入数据不足，无法进行FFT分析。");
        return;
    }

    // 提取时间和电压数据
    std::vector<double> time, voltage;
    for (const auto& point : *data) {
        time.push_back(point.x()*timeMultiplier);
        voltage.push_back(point.y());
    }

    // 确保数据是按时间排序的（如果有必要）
    if (!std::is_sorted(time.begin(), time.end())) {
        std::cerr << "输入数据未按时间排序，请检查输入数据。" << std::endl;
        return;
    }

    double delta = time[1] - time[0];
    // for (size_t i = 2; i < time.size(); ++i) {
    //     if (std::abs((time[i] - time[i-1]) - delta) > 1e-6) {
    //         std::cerr << "非均匀采样，FFT结果无效。" << std::endl;
    //         emit sendLog("错误：非均匀采样数据");
    //         return;
    //     }
    // }

    // 计算采样率
    double samplingRate = 1.0 / delta;
    qDebug()<<"采样率："<<samplingRate;

    // 准备 FFT 数据
    int N = voltage.size();
    int output_length = N/2 + 1;

    fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * output_length);
    fft_in = (double*)fftw_malloc(sizeof(double) * N);

    for (int i = 0; i < N; ++i) {
        fft_in[i] = voltage[i];
    }

    // 执行 FFT
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, fft_in, fft_out, FFTW_ESTIMATE);
    fftw_execute(plan);


    double referenceVoltage = 0.775;

    // 计算频率和幅值
    std::vector<double> frequencies(output_length);
    std::vector<double> magnitudes(output_length);

    for (int i = 0; i < output_length; ++i) {

        double real = fft_out[i][0];
        double imag = fft_out[i][1];
        double mag = std::sqrt(real * real + imag * imag) / N;

        //单边普新政（直流和Nyquist分量不 * 2）
        if(i>0 && i != output_length - 1){
            mag *= 2;
        }

        //转换RMS振幅
        double amplitude_rms = mag / std::sqrt(2);

        //转换为dBu
        double amplitude_dBu = 20 * std::log10(amplitude_rms / referenceVoltage);

        magnitudes[i] = amplitude_dBu;
        frequencies[i] = i * samplingRate / N;
        // 更新 FFT 数据计算进度（0% - 100%）
        if (i % 1000 == 0 || i == N / 2 - 1) {
            int progress = static_cast<int>((i / static_cast<float>(N / 2)) * 100); // 更新到100%
            emit porgressUpdated(progress); // 发送进度信号
        }
    }

    // // 每50Hz计算一次分量
    // for (int i = 0; i * samplingRate / N <= 5000; i += 50) {
    //     int idx = i * N / samplingRate;
    //     if (idx >= output_length) break;

    //     double real = fft_out[idx][0];
    //     double imag = fft_out[idx][1];
    //     double mag = std::sqrt(real * real + imag * imag) / N;

    //     //单边谱密度调整（直流和Nyquist分量不需要乘2）
    //     if(i > 0 && i != output_length - 1) {
    //         mag *= 2;
    //     }

    //     magnitudes.push_back(mag);
    //     frequencies.push_back(i * samplingRate / N);
    // }

    emit porgressUpdated(100);
    emit fftReady(frequencies ,magnitudes);
}
void FFTHandle::calculateWitRawData(){
    int N = rawData->size();
    //N = 5000000;
    // 将数据长度填充到2的幂次方
    int optimalN = 1;
    while (optimalN < N) {
        optimalN *= 2;
    }
    QVector<double> adjustedData = *rawData;
    adjustedData.resize(optimalN, 0.0); // 填充0

    double timeIntervalSeconds = timeIntervalNanoseconds / 1e9;
    double samplingRate = 1.0 / timeIntervalSeconds;

    // 去除直流偏移
    // double mean = std::accumulate(adjustedData.begin(), adjustedData.end(), 0.0) / optimalN;
    // for (int i = 0; i < optimalN; ++i) {
    //     adjustedData[i] -= mean;
    // }

    // FFT 初始化
    fftw_complex* fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    double* fft_in = (double*)fftw_malloc(sizeof(double) * N);

    // for (int i = 0; i < N; ++i) {
    //     fft_in[i] = adjustedData[i];
    // }
    std::memcpy(fft_in ,adjustedData.data() ,sizeof(double)*N);
    auto startA = std::chrono::high_resolution_clock::now();

    // 尝试加载 Wisdom 文件
    if (!loadWFileSuccess) {
        emit sendLog("未找到 Wisdom 文件，使用默认计划...");
    }

    fftw_plan plan = fftw_plan_dft_r2c_1d(N, fft_in, fft_out, FFTW_ESTIMATE);

    if(!loadWFileSuccess){
        // 保存 Wisdom 文件
        saveWisdomToFile(wisdomFileName); // 保存优化后的计划，供下次使用
    }

    auto endA = std::chrono::high_resolution_clock::now();
    // 计算时间差，并转换为毫秒
    std::chrono::duration<double, std::milli> elapsedA = endA - startA;
    emit sendLog("FFT PLAN 初始化：" + QString::number(elapsedA.count()) + " ms");

    // 执行FFT
    fftw_execute(plan);

    std::vector<double> frequencies;
    std::vector<double> magnitudes;

    auto start = std::chrono::high_resolution_clock::now();
    // 只计算20MHz以内的数据
    double maxFrequency = 20e6; // 20 MHz
    // 预计算因子
    double reciprocal = 1.0 / samplingRate; // 预计算倒数
    double scaleFactor = N * reciprocal; // 优化的缩放因子


    // 直接使用乘法计算最大索引
    int maxIndex = static_cast<int>(maxFrequency * scaleFactor);

    for (int i = 0; i <= maxIndex && i < N / 2; ++i) {
        double frequency = i * samplingRate / N;
        double magnitude = sqrt(fft_out[i][0] * fft_out[i][0] + fft_out[i][1] * fft_out[i][1]) / N;
        frequencies.push_back(frequency);
        magnitudes.push_back(magnitude);
    }

    auto end = std::chrono::high_resolution_clock::now();
    // 计算时间差，并转换为毫秒
    std::chrono::duration<double, std::milli> elapsed = end - start;
    emit sendLog("FFT结果运算及填充：" + QString::number(elapsed.count()) + " ms");

    // 释放资源
    fftw_destroy_plan(plan);
    fftw_free(fft_in);
    fftw_free(fft_out);

    emit fftReady(frequencies, magnitudes);
}

void FFTHandle::run(){
    //calculate();
    // 记录起始时间
    auto start = std::chrono::high_resolution_clock::now();
    //calculateWitRawData();
    calculate();
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算时间差，并转换为毫秒
    std::chrono::duration<double, std::milli> elapsed = end - start;
    emit sendLog("FFT计算时间：" + QString::number(elapsed.count()) + " ms");

}
