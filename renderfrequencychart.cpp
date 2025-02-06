#include "renderfrequencychart.h"
#include <chrono>
RenderFrequencyChart::RenderFrequencyChart(PDChart *pdChart ,QObject *parent)
    : QThread{parent}
{
    _chart = pdChart;
    _chart->setTitle("频域图");
    _chart->setXAxisTitle("MHz");
    _chart->setYAxisTitle("dBu");
    _chart->setLineColor(Qt::blue);
    _chart->setLineWidth(0.5);
    _chart->setXAxisRange(0,5);
    // ui->chartView_2->setYAxisRange(-200,100);
    _chart->setYAxisScale(-100 ,100 ,10);
}

// 创建LOD缓存
void RenderFrequencyChart::createLOD(const QVector<QPointF>& points) {
    lodData.clear(); // 清空LOD数据
    lodData.append(points); // 原始数据作为最高精度

    int totalSize = points.size();
    for (int level = 1; level < maxLevels; ++level) {
        int step = qMax(1, totalSize / (targetPointsPerLevel << level)); // 抽稀倍数
        QVector<QPointF> reduced;
        for (int i = 0; i < totalSize; i += step) {
            reduced.append(points[i]);
        }
        lodData.append(reduced); // 添加到LOD缓存
    }
}

// 更新LOD显示
void RenderFrequencyChart::updateLOD(int zoomLevel) {
    int lodIndex = qMin(zoomLevel, lodData.size() - 1);
    //series->replace(lodData[lodIndex]); // 使用对应LOD数据
    _chart->setFData(lodData[lodIndex]);
}

// 设置数据并创建LOD
void RenderFrequencyChart::setFData(const QVector<QPointF>& points) {
    // createLOD(points); // 预处理LOD数据
    // updateLOD(maxLevels - 1); // 初始显示最低精度

    //_chart->setFData(points);
}

void RenderFrequencyChart::render(std::vector<double> frequencies ,
                                  std::vector<double> magnitudes ,
                                  QString unit){
    // 记录起始时间
    //auto start = std::chrono::high_resolution_clock::now();
    int unittype = 2;
    if(unit == "KHz"){
        unittype = 1;
        _chart->setXAxisTitle("KHz");
    }else if(unit == "Hz"){
        unittype = 0;
        _chart->setXAxisTitle("Hz");
    }else if(unit == "MHz"){
        unittype = 2;
        _chart->setXAxisTitle("MHz");
    }

    const double unitConversion[] = {1.0, 1e-3, 1e-6};
    maxFreq = std::numeric_limits<double>::lowest();
    minFreq = std::numeric_limits<double>::max();
    maxEnergy = std::numeric_limits<double>::lowest();
    minEnergy = std::numeric_limits<double>::max();
    _datas.clear();
    for(int i=0;i<frequencies.size();++i){
        double frequency = frequencies[i] * unitConversion[unittype];

        if (frequency > maxFreq) maxFreq = frequency;
        if (frequency < minFreq) minFreq = frequency;
        if(magnitudes[i] > maxEnergy) maxEnergy = magnitudes[i];
        if(magnitudes[i] < minEnergy) minEnergy = magnitudes[i];
        _datas.append(QPointF(frequency ,magnitudes[i]));
    }

    // // 记录结束时间
    // auto end = std::chrono::high_resolution_clock::now();
    // // 计算时间差，并转换为毫秒
    // std::chrono::duration<double, std::milli> elapsed = end - start;

    // qDebug()<<"频域数据计算时间："<<QString::number(elapsed.count())<<"毫秒";
}

void RenderFrequencyChart::run(){
    _chart->clearData();
    // 记录起始时间
    auto start = std::chrono::high_resolution_clock::now();
    _chart->setXAxisRange(0,20);
    _chart->setYAxisRange(minEnergy,5);
    createLOD(_datas); // 预处理LOD数据
    updateLOD(maxLevels - 1); // 初始显示最低精度
    emit renderFinished(_datas);
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算时间差，并转换为毫秒
    std::chrono::duration<double, std::milli> elapsed = end - start;
    emit sendLog("频域图表渲染时间：" + QString::number(elapsed.count()) + " ms");
}

void RenderFrequencyChart::clear(){
    _chart->clearData();
}

void RenderFrequencyChart::setXRange(double min ,double max){
    minFreq = min;
    maxFreq = max;
    _chart->setXAxisRange(min,max);
}

void RenderFrequencyChart::setYRange(double min ,double max){
    minEnergy = min;
    maxEnergy = max;
    _chart->setYAxisRange(min,max);
}

