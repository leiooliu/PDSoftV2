#include "rendertimechart.h"
#include "QMessageBox"
#include <chrono>
#include <algorithm.h>
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
    _pdChart->setXAxisScale(_xMin,_xMax,1);
    _pdChart->setYAxisRange(_yMin ,_yMax);
}

void RenderTimeChart::changeY(PS2000A_RANGE range){
    double intervals = 100;       // 默认间隔，单位为 mV
    double voltageRange = 500;    // 默认电压范围，单位为 mV
    QString AxisTitle = "Voltage (mV)";
    //计算幅值坐标
    switch (range) {
    case PS2000A_10MV:
        voltageRange = 10;    // 10 mV
        intervals = 2.0;       // 2 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_20MV:
        voltageRange = 20;
        intervals = 4.0;
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_50MV:
        voltageRange = 50;
        intervals = 10.0;
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_100MV:
        voltageRange = 100;
        intervals = 20.0;
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_200MV:
        voltageRange = 200;
        intervals = 40.0;
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_500MV:
        voltageRange = 500;
        intervals = 100.0;
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_1V:
        voltageRange = 1000;  // 1 V 转换为 mV
        intervals = 200.0;     // 200 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_2V:
        voltageRange = 2000;  // 2 V 转换为 mV
        intervals = 400.0;     // 400 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_5V:
        voltageRange = 5000;  // 5 V 转换为 mV
        intervals = 1000.0;    // 1000 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_10V:
        voltageRange = 10000; // 10 V 转换为 mV
        intervals = 2000.0;    // 2000 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_20V:
        voltageRange = 20000; // 20 V 转换为 mV
        intervals = 4000.0;    // 4000 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    case PS2000A_50V:
        voltageRange = 50000; // 50 V 转换为 mV
        intervals = 10000.0;   // 10000 mV 间隔
        AxisTitle = "Voltage (mV)";
        break;
    default:
        voltageRange = 500;    // 默认 500 mV
        intervals = 100.0;
        AxisTitle = "Voltage (mV)";
        break;
    }
    _pdChart->setYAxisScale(-voltageRange, voltageRange, intervals);
    _yIntervals = intervals;
}

void RenderTimeChart::changeX(TimeBase timebase){
    constexpr int DivsCount = 10; // 总刻度10个div
    double totalTime = DivsCount * timebase.gridValue; // 总时间跨度
    if(timebase.conversion){
        totalTime = DivsCount * (timebase.gridValue * 1e-3);
    }
    QString unit = timebase.unit;
    QString xTitle = "Time (ns)";
    if (unit == "ns") {
        if(timebase.conversion){
            xTitle = "Time (us)";
        }else{
            xTitle = "Time (ns)";
        }
    } else if (unit == "us") {
        if(timebase.conversion){
            xTitle = "Time (ms)";
        }else{
            xTitle = "Time (us)";
        }
    } else if (unit == "ms") {
        if(timebase.conversion){
            xTitle = "Time (s)";
        }else{
            xTitle = "Time (ms)";
        }
    } else if (unit == "s") {
        _pdChart->setXAxisTitle("Time (s)");
    } else {
        QMessageBox::information(nullptr, "error", "无效单位: " + unit);
        return; // 无效单位直接返回
    }

    _pdChart->setXAxisTitle(xTitle);
    _xIntervals = timebase.gridValue;

    _pdChart->setXAxisScale(0,totalTime,timebase.gridValue);
}

void RenderTimeChart::render(const QVector<QPointF>& datas ,QString unit){
    _datas = datas;
    _unit = unit;
}

void RenderTimeChart::setPeakTiggerData(const QVector<QPointF> &datas){


}
void RenderTimeChart::render(const QVector<double> sourceData , PS2000A_RANGE range, TimeBase timebase,PeakParam peakParam){
    QString unit = timebase.unit;
    int unitValue = timebase.gridValue; // 这个要拆解示波器的数字
    double timeIntervalNanoseconds = timebase.interval;

    // 清空旧数据，防止重复
    _datas.clear();
    // 异常检查
    if (sourceData.isEmpty() || unitValue <= 0 || timeIntervalNanoseconds <= 0) {
        // 弹出错误信息后期可以考虑用信号槽来解决阻塞主线程的问题
        QMessageBox::information(nullptr, "error",
                                 "输入数据为空或单位值无效 : " + QString::number(unitValue) +
                                     ", 时间间隔 (ns): " + QString::number(timeIntervalNanoseconds));
        return; // 输入数据为空或单位值无效
    }

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
        _pdChart->setXAxisTitle("Time (s)");
        timeMultiplier = 1.0;   // 秒
    } else {
        QMessageBox::information(nullptr, "error", "无效单位: " + unit);
        return; // 无效单位直接返回
    }


    // 计算总显示时间跨度
    changeX(timebase);
    changeY(range);

    // 将采样间隔从纳秒转换为秒
    double interval = timeIntervalNanoseconds * 1e-9; // 单位转换为秒

    // 调试输出，检查 interval 和 timeMultiplier
    emit sendLog("Interval（秒） = " + QString::number(interval));
    emit sendLog("timeMultiplier = " + QString::number(timeMultiplier));

    // 提前分配内存
    _datas.reserve(sourceData.size());
    //_datas.reserve(timebase.sampleCount);


    // 定义断开阈值 (比如设置为 interval 的1.5倍)
    double breakThreshold = interval / timeMultiplier * 1000 ;

    for (int i = 0; i < sourceData.size(); ++i) {
        double time = i * interval / timeMultiplier;
        double volts = PDTools::adcToVolts(sourceData[i], range) * 1000;

        QPointF point = QPointF(time, volts);
        _datas.append(point);

        // bool isPeak = false;
        // if (volts > 0 && volts >= peakParam.up_volts_threshold) {
        //     isPeak = true;
        // } else if (volts < 0 && volts <= -peakParam.down_volts_threshold) {
        //     isPeak = true;
        // }

        // if (isPeak) {
        //     peakDatas.append(point);
        // }
    }

    _peakDatas = Algorithm::findSpikesBySlope(_datas ,500);

    if (peakParam.isShow) {
        _pdChart->setPeakTiggerData(_peakDatas,breakThreshold);
    }
    //peakDatas = Algorithm::getPeakPulses(_datas);

    // for(int i=0;i<peakDatas.size();++i){
    //     qDebug()<< "脉冲数据：" << peakDatas[i].rx() << "," << peakDatas[i].ry();
    // }


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
    emit renderFinished(_datas,_peakDatas ,timeMultiplier);  // 线程结束信号
}
void RenderTimeChart::clear(){
    _pdChart->clearData();
}

void RenderTimeChart::setXRange(double min ,double max){
    _xMin = min;
    _xMax = max;
    _pdChart->setXAxisScale(_xMin,_xMax,_xIntervals);
}
void RenderTimeChart::setYRange(double min ,double max){
    _yMin = min;
    _yMax = max;
    _pdChart->setYAxisScale(_yMin, _yMax, _yIntervals);
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
