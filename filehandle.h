#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <QDir>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QString>
#include <QVector>

class FileHandle
{
public:
    FileHandle();
    ~FileHandle();
    // 创建基于日期的文件夹
    bool createFolderByDate(const QString &baseDir = ".");

    // 创建文本文件
    bool createTextFile(const QString &filename, const QString &content);

    // 序列化数据到二进制文件
    bool serializeToFile(const QString &filename,const QVector<QPointF> &data);

    // 读取文本文件
    QString readTextFile(const QString &filename);

    // 反序列化从二进制文件中读取的数据
    QVector<QPointF> deserializeFromFile(const QString &filename);

    QString getFolderNameByDate();


};

#endif // FILEHANDLE_H
