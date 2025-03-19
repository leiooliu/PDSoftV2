#ifndef RENDERPOLARTCHART_H
#define RENDERPOLARTCHART_H

#include <QThread>
#include <pdpolartchart.h>

class RenderPolartChart : public QThread
{
    Q_OBJECT
public:
    explicit RenderPolartChart(PDPolartChart *pdChart, QObject *parent = nullptr);
    void run() override;
    void setRenderData(std::vector<double> magnitudes ,std::vector<double> phases);
    void clear();
private:
    PDPolartChart *_pdChart;
    std::vector<double> _magnitudes ;
    std::vector<double> _phases;

};

#endif // RENDERPOLARTCHART_H
