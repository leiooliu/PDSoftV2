#include "renderpolartchart.h"

RenderPolartChart::RenderPolartChart(PDPolartChart *pdChart, QObject *parent)
    : QThread{parent}
{
    _pdChart = pdChart;

}

void RenderPolartChart::run(){
    _pdChart->setData(_magnitudes ,_phases);
    //_pdChart->testData();
}

void RenderPolartChart::setRenderData(std::vector<double> magnitudes ,std::vector<double> phases){
    _magnitudes = magnitudes;
    _phases = phases;
}

void RenderPolartChart::clear(){
    _pdChart->clear();
}
