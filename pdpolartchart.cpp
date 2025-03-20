#include "pdpolartchart.h"

PDPolartChart::PDPolartChart(QWidget *parent)
    : QWidget{parent}
{
    chart = new QPolarChart();

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    // chart->setTitle("信号相位");



    // chartView = new PDChartView(chart);
    // chartView->setRenderHint(QPainter::Antialiasing);

    // QVBoxLayout *layout = new QVBoxLayout(this);
    // layout->addWidget(chartView);
}

void PDPolartChart::testData(){
    // 创建极坐标数据序列
    QLineSeries *series = new QLineSeries();

    // 参数设置
    const double frequency = 50.0;                // 50Hz
    const double omega = 2 * M_PI * frequency;      // 角频率
    const double sampleRate = 1000.0;               // 采样率：1000次采样/秒
    const double dt = 1.0 / sampleRate;             // 采样时间间隔
    const int totalSamples = sampleRate / frequency; // 每周期采样点数

    // 为了显示多个周期，可以采样若干个周期（例如 5 个周期）
    int cycles = 2;
    int totalPoints = totalSamples * cycles;

    // 延时相位：使相位图不再只是一个常数半径的圆
    const double phaseShift = M_PI / 4;

    // 生成数据：先计算状态空间（x, y），再转换到极坐标（θ, r）
    for (int i = 0; i < totalPoints; i++) {
        double t = i * dt;
        double x = sin(omega * t);
        double y = sin(omega * t + phaseShift);
        double r = std::sqrt(x * x + y * y);
        // 角度单位转换为度（Qt 极坐标默认角度单位为度）
        double theta = std::atan2(y, x) * 180.0 / M_PI;
        series->append(theta, r);
    }

    // 创建极坐标图表
    chart->addSeries(series);
    chart->setTitle("50Hz 正弦波延时相位图（极坐标）");
    chart->legend()->hide();

    // 创建角度（极角）坐标轴（使用 QValueAxis 设置连续刻度）
    QValueAxis *angularAxis = new QValueAxis();
    angularAxis->setTickCount(9);  // 可根据需要调整
    angularAxis->setLabelFormat("%.0f");
    angularAxis->setRange(-180, 180);
    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
    series->attachAxis(angularAxis);

    // 创建径向坐标轴
    QValueAxis *radialAxis = new QValueAxis();
    radialAxis->setTickCount(6);
    radialAxis->setRange(0, 1.5);
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);
    series->attachAxis(radialAxis);
}

void PDPolartChart::setData(const std::vector<double> magnitudes ,
                            const std::vector<double> phases){

    QVector<QPointF> phasesData;

    QScatterSeries *series = new QScatterSeries();
    series->setUseOpenGL(true);
    series->setColor(Qt::red);
    series->setMarkerSize(8);
    chart->addSeries(series);

    for (size_t i = 0; i < magnitudes.size(); ++i) {
        double angleDeg = phases[i] * 180 / M_PI;
        if (angleDeg < 0) angleDeg += 360; // 归一化角度到0-360°
        //series->append(angleDeg, magnitudes[i]);
        //phasesData.append(QPointF(std::abs(angleDeg) ,std::abs(magnitudes[i])));
        phasesData.append(QPointF(angleDeg ,magnitudes[i]));
    }

    QValueAxis *angularAxis = new QValueAxis();
    angularAxis->setRange(0, 360);
    angularAxis->setTickCount(9);
    angularAxis->setLabelFormat("%d");

    QValueAxis *radialAxis = new QValueAxis();
    radialAxis->setRange(0, *std::max_element(magnitudes.begin(), magnitudes.end()));
    radialAxis->setTickCount(6);

    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

    series->attachAxis(angularAxis);
    series->attachAxis(radialAxis);
    series->replace(phasesData);

    // for(int i=0;i<100;++i){
    //     qDebug() << phasesData[i].rx() << " : " << phasesData[i].ry();
    // }

}

void PDPolartChart::clear(){
    chart->removeAllSeries();

}
