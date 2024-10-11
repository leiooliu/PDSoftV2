#ifndef SINGALCONVERT_H
#define SINGALCONVERT_H
#include <QVector>
#include <QPointF>
class SingalConvert
{
public:
    SingalConvert();
    QVector<QPointF> performFFT(QVector<QPointF> dataPoints,int unit);
    qreal findMaxX(const QVector<QPointF>& dataPoints);
    qreal findMaxY(const QVector<QPointF>& dataPoints);
    double calculateTHD(const QList<QPointF>& frequencyData, double fundamentalFrequency);
    double calculateTHDForSquareWave(const QList<QPointF>& frequencyData, double fundamentalFrequency, int maxHarmonic);

};

#endif // SINGALCONVERT_H
