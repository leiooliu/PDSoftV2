#ifndef FFTANALYZER_H
#define FFTANALYZER_H

#include <QThread>
#include <QVector>
#include <QPointF>
#include <QVariant>
#include <QTableView>
#include <fftw3.h>
#include <omp.h>
class FFTAnalyzer:public QThread
{
    Q_OBJECT

public:
    // 构造函数和析构函数
    explicit FFTAnalyzer(int maxHarmonics = 10); // 最大谐波阶数
    ~FFTAnalyzer();
    // FFT 分析函数
    void analyze(QVector<QPointF>* _data, double _customFundamentalFrequency = 0.0);
    void setData(std::vector<double> frequencies,const std::vector<double> magnitudes,double targetFreq);
    // 数据导出函数
    void exportTableToCSV(QTableView* tableView, const QString& filePath);
    void run() override;

signals:
    void dataReady(const QVector<QVector<QVariant>> result);
    void fftReady(const std::vector<double> frequencies,std::vector<double> magnitudes);
    // 进度更新信号
    void progressUpdated(int percentage);

private:
    // 成员变量
    int maxHarmonics;           // 最大谐波阶数
    double* fft_in;             // FFT 输入数据缓存
    fftw_complex* fft_out;      // FFT 输出数据缓存
    std::vector<double> _frequencies;
    std::vector<double> _magnitudes;
    // 辅助函数
    int findClosestIndex(const std::vector<double>& frequencies, double targetFreq);
    const QVector<QPointF> *data;
    double customFundamentalFrequency;
};

#endif // FFTANALYZER_H
