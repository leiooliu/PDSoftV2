#ifndef CSVLOADER_H
#define CSVLOADER_H

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QVector>
#include <QPointF>
class CSVLoader {
public:
    static bool loadDataFromCSV(const QString& fileName, QVector<QPointF>& dataPoints) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file for reading: " << fileName;
            QMessageBox::information(nullptr, "Error", "Cannot open file for reading: " + fileName);
            return false;
        }

        QTextStream in(&file);
        QString line = in.readLine(); // 跳过标题行
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList parts = line.split(',');

            if (parts.size() == 2) {
                bool okX, okY;
                qreal x = parts[0].toDouble(&okX);
                qreal y = parts[1].toDouble(&okY);

                if (okX && okY) {
                    dataPoints.append(QPointF(x, y));
                } else {
                    qDebug() << "Invalid data line:" << line;
                }
            } else {
                qDebug() << "Invalid data line:" << line;
            }
        }

        file.close();
        qDebug() << "Data loaded from: " << fileName;
        return true;
    }

    static void saveDataToCSV(const QString &fileName ,const QVector<QPointF> bufferedData){
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file for writing: " << fileName;
            QMessageBox::information(nullptr, "Error", "Cannot open file for writing: " + fileName);
            return;
        }

        QTextStream out(&file);
        out << "Time (ms),Voltage (V)\n"; // CSV文件的标题

        for (uint32_t i = 0; i < bufferedData.size(); ++i) {
            out << bufferedData[i].x() << "," << bufferedData[i].y() << "\n"; // 写入时间和电压
        }

        file.close();
        qDebug() << "Data saved to: " << fileName;
        QMessageBox::information(nullptr, "SAVE", "文件导出成功");
    }
};


#endif // CSVLOADER_H
