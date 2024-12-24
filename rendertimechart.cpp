#include "rendertimechart.h"

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
void RenderTimeChart::render(const QVector<QPointF>& datas,QString unit){
    _datas = datas;
    _unit = unit;
}
void RenderTimeChart::run(){
    _pdChart->setData(_datas ,_unit);
    emit renderFinished();  // 线程结束信号
}
