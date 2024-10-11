#include "filehandle.h"
#include <QPointF>
FileHandle::FileHandle() {}
FileHandle::~FileHandle() {}

// 实现函数
bool FileHandle::createFolderByDate(const QString &baseDir) {
    QString folderName = getFolderNameByDate();
    QDir dir(baseDir);
    if (!dir.exists(folderName)) {
        return dir.mkdir(folderName);
    }
    return true;
}

QString FileHandle::getFolderNameByDate() {
    QDate date = QDate::currentDate();
    return date.toString("yyyy-MM-dd");
}

bool FileHandle::createTextFile(const QString &filename, const QString &content) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

// 序列化数据到二进制文件
bool FileHandle::serializeToFile(const QString &filename, const QVector<QPointF> &data)
{
    qDebug() << "Serializing to file:" << filename;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical("Failed to open file for writing: %s", qPrintable(filename));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_7); // 设置版本以确保兼容性

    // 写入数据点的数量
    int size = data.size();
    out << size;

    // 写入数据点
    for (const QPointF& point : data) {
        out << point.x() << point.y();
    }

    file.close();
    qDebug() << "Serialization complete.";

    return true;
}

QString FileHandle::readTextFile(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

// 从二进制文件反序列化数据
QVector<QPointF> FileHandle::deserializeFromFile(const QString &filename)
{
    qDebug() << "Trying to open file:" << filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Failed to open file for reading: %s", qPrintable(filename));
        return {}; // 返回空的 QVector<QPointF>
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_7); // 设置版本以确保兼容性

    int size;
    in >> size; // 读取数据点的数量
    if (in.status() != QDataStream::Ok) {
        qCritical("Error reading the number of points from the file.");
        file.close();
        return {}; // 返回空的 QVector<QPointF>
    }

    qDebug() << "Number of points read:" << size;

    QVector<QPointF> data(size);

    for (int i = 0; i < size; ++i) {
        qreal x, y;
        in >> x >> y;
        if (in.status() != QDataStream::Ok) {
            qCritical("Error reading point data from the file at index %d.", i);
            file.close();
            return {}; // 返回空的 QVector<QPointF>
        }
        data[i] = QPointF(x, y);
    }

    file.close();
    qDebug() << "Deserialization complete.";

    return data;
}
