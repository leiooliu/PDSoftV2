#include "rendertimechart.h"
#include "QMessageBox"
#include <chrono>
RenderTimeChart::RenderTimeChart(PDChart *pdChart,QObject *parent)
    : QThread{parent}
{
    _pdChart = pdChart;
    _pdChart->setTitle("时域图");
    _pdChart->setXAxisTitle("Time (ms)");
    _pdChart->setYAxisTitle("Voltage (mV)");
    _pdChart->setLineColor(Qt::blue);
    _pdChart->setLineWidth(0.5);
    //_pdChart->setXAxisRange(0,10);
    _pdChart->setXAxisScale(0,10,1);
    _pdChart->setYAxisRange(-5000 ,5000);
}
void RenderTimeChart::render(const QVector<QPointF>& datas ,QString unit){
    _datas = datas;
    _unit = unit;
}
void RenderTimeChart::render(const QVector<double> soucedata , PS2000A_RANGE range, TimeBase timebase){

    QString unit = timebase.unit;
    int unitValue = timebase.gridValue; //这个要拆解示波器的数字
    double timeIntervalNanoseconds  = timebase.interval;

    // 清空旧数据，防止重复
    _datas.clear();
    // 异常检查
    if (soucedata.isEmpty() || unitValue <= 0 || timeIntervalNanoseconds <= 0) {
        QMessageBox::information(nullptr, "error",
                                 "输入数据为空或单位值无效 : " + QString::number(unitValue) +
                                     ", 时间间隔 (ns): " + QString::number(timeIntervalNanoseconds));
        return; // 输入数据为空或单位值无效
    }
    // 定义时间倍率
    double timeMultiplier = 1.0;
    if (unit == "ns") {
        timeMultiplier = 1e-9;  // 纳秒
        _pdChart->setXAxisTitle("Time (ns)");
    } else if (unit == "us") {
        timeMultiplier = 1e-6;  // 微秒
        _pdChart->setXAxisTitle("Time (us)");
    } else if (unit == "ms") {
        timeMultiplier = 1e-3;  // 毫秒
        _pdChart->setXAxisTitle("Time (ms)");
    } else if (unit == "s") {
        timeMultiplier = 1.0;   // 秒
        _pdChart->setXAxisTitle("Time (s)");
    } else {
        QMessageBox::information(nullptr, "error", "无效单位: " + unit);
        return; // 无效单位直接返回
    }
    // 计算总显示时间跨度
    constexpr int DivsCount = 10; // 总刻度10个div
    double totalTime = DivsCount * unitValue; // 总时间跨度
    _pdChart->setXAxisScale(0,totalTime,unitValue);
    // 将采样间隔从纳秒转换为秒
    double interval = timeIntervalNanoseconds * 1e-9; // 单位转换为秒

    // 提前分配内存
    _datas.reserve(soucedata.size());
    //_datas.reserve(timebase.sampleCount);

    // 填充数据
    for (int i = 0; i < soucedata.size(); ++i) {
    //for (int i = 0; i < timebase.sampleCount; ++i) {
        // 计算当前采样点时间（单位：秒）
        double time = i * interval / timeMultiplier;
        // if(time>totalTime){
        //     break;
        // }
        // 转换电压为mV
        double volts = PDTools::adcToVolts(soucedata[i], range) * 1000; // 电压统一为mV
        // 添加到结果集中
        _datas.append(QPointF(time, volts));
    }
    // 设置单位
    _unit = unit;
    qDebug() << _datas.size();
}
void RenderTimeChart::run(){
    // 记录起始时间
    auto start = std::chrono::high_resolution_clock::now();
    _pdChart->setFData(_datas);
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    // 计算时间差，并转换为毫秒
    std::chrono::duration<double, std::milli> elapsed = end - start;
    emit sendLog("时域图表渲染时间：" + QString::number(elapsed.count()) + " ms");
    //createLOD(_datas); // 预处理LOD数据
    //updateLOD(maxLevels - 1); // 初始显示最低精度
    emit renderFinished(_datas);  // 线程结束信号
}
void RenderTimeChart::clear(){
    _pdChart->clearData();
}

// 创建LOD缓存
void RenderTimeChart::createLOD(const QVector<QPointF>& points) {
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
void RenderTimeChart::updateLOD(int zoomLevel) {
    int lodIndex = qMin(zoomLevel, lodData.size() - 1);
    //series->replace(lodData[lodIndex]); // 使用对应LOD数据
    _pdChart->setFData(lodData[lodIndex]);
}

// 设置数据并创建LOD
void RenderTimeChart::setFData(const QVector<QPointF>& points) {
    createLOD(points); // 预处理LOD数据
    updateLOD(maxLevels - 1); // 初始显示最低精度
}
