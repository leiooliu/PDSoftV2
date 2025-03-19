#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QString>
#include <QVector>
#include <QPointF>
#include <QDir>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QMessageBox>
#include <ps2000aApi.h>
class FileManager
{
public:
    FileManager();
    ~FileManager();

    // CSV 文件操作
    static bool loadCSV(const QString &fileName, QVector<QPointF> &dataPoints);
    static bool saveCSV(const QString &fileName, const QVector<QPointF> &dataPoints);

    static bool saveRawCSV(const QString &fileName ,const QVector<double> &rawData);
    static bool loadRawCSV(const QString &fileName ,QVector<double> &rawData);


    // 二进制文件操作
    static bool serializeToBinary(const QString &fileName, const QVector<QPointF> &data);
    static QVector<QPointF> deserializeFromBinary(const QString &fileName);

    // 二进制原始数据操作
    static bool serializeToBinary(const QString &fileName, const QVector<double> &data);
    static QVector<double> deserializeRawFromBinary(const QString &fileName);

    // 文本文件操作
    static bool createTextFile(const QString &fileName, const QString &content);
    static QString readTextFile(const QString &fileName);

    // 创建基于日期的文件夹
    static bool createFolderByDate(const QString &baseDir = ".");
    static QString getFolderNameByDate();

    static bool deserializeFromBinary(const QString &fileName, QVector<double> &data,
                                      PS2000A_RANGE &range, int &timeBase,
                                      double &sampleInterval ,int &divTagIndex);
    static bool serializeToBinary(const QString &fileName, const QVector<double> &data,
                                  PS2000A_RANGE range, int timeBase,
                                  double sampleInterval ,int divTagIndex);

    static bool serializeToBinary(const QString &fileName, const QVector<double> &data,
                                  PS2000A_RANGE range, int timeBase,
                                  double sampleInterval ,int divTagIndex ,
                                  double up_volts_threshold ,double down_volts_threshold ,bool isShow);
    static bool deserializeFromBinary(const QString &fileName, QVector<double> &data,
                                      PS2000A_RANGE &range, int &timeBase,
                                      double &sampleInterval ,int &divTagIndex,
                                      double &up_volts_threshold ,double &down_volts_threshold ,bool &isShow);

private:
    static void showError(const QString &message);
};

#endif // FILEMANAGER_H
