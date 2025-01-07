#ifndef CUSTOMLINEITEM_H
#define CUSTOMLINEITEM_H

#include <QGraphicsItem>
#include <QPointF>
#include <QPen>
#include <QPainter>

class CustomLineItem : public QGraphicsItem {
public:
    CustomLineItem(const QVector<QPointF>& points)
        : points(points) {
        setFlag(QGraphicsItem::ItemHasNoContents, false);
    }

    QRectF boundingRect() const override {
        // 计算图形项的边界框
        qreal minX = points.first().x(), minY = points.first().y();
        qreal maxX = points.first().x(), maxY = points.first().y();
        for (const auto& point : points) {
            minX = qMin(minX, point.x());
            minY = qMin(minY, point.y());
            maxX = qMax(maxX, point.x());
            maxY = qMax(maxY, point.y());
        }
        return QRectF(minX, minY, maxX - minX, maxY - minY);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
        // 绘制点或线
        QPen pen(Qt::blue, 1);
        painter->setPen(pen);

        for (int i = 1; i < points.size(); ++i) {
            painter->drawLine(points[i - 1], points[i]);
        }
    }

private:
    QVector<QPointF> points;  // 存储所有的点
};

#endif // CUSTOMLINEITEM_H
