#include "tablerender.h"
#include <QStandardItemModel>
#include <singalconvert.h>
tablerender::tablerender(QTableView *tableView) {
    views = tableView;
}
void tablerender::render(QVector<QPointF> datas ,QVector<QString> headers ,int beginValue ,int endValue){
    model = new QStandardItemModel(0, 0, nullptr); // 3 行 2 列
    for(int i=0;i<headers.size();++i){
        QStandardItem *headerItem1 = new QStandardItem(headers.at(i));
        model->setHorizontalHeaderItem(i ,headerItem1);
    }

    views->setModel(model);
    SingalConvert convert;
    int rowCount = 0;
    for(int i=0;i<datas.size();i++){
        if(datas[i].x()>=beginValue && datas[i].x() <= endValue){
            double xValue = datas[i].x();
            model->setItem(rowCount, 0, new QStandardItem(QString::number(xValue)));
            model->setItem(rowCount, 1, new QStandardItem(QString::number(convert.calculateTHD(datas, 1000))));
            rowCount++;
        }
    }
    model->setRowCount(rowCount);
}
