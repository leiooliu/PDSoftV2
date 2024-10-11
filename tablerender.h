#ifndef TABLERENDER_H
#define TABLERENDER_H
#include <QTableView>
#include <QStandardItem>
#include <QPointF>
#include <QVector>
#include <QStandardItemModel>

class tablerender
{
public:
    tablerender(QTableView *tableView);
    void render(QVector<QPointF> datas ,QVector<QString> headers,int beginValue ,int endValue);
private:
    QTableView *views;
    QStandardItemModel *model;
};

#endif // TABLERENDER_H
