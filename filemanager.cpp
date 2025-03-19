#include "filemanager.h"

FileManager::FileManager() {}
FileManager::~FileManager() {}

// 显示错误信息
void FileManager::showError(const QString &message) {
    QMessageBox::critical(nullptr, "Error", message);
}

// 加载 CSV 文件
bool FileManager::loadCSV(const QString &fileName, QVector<QPointF> &dataPoints) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showError("Cannot open file for reading: " + fileName);
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
    return true;
}

// 保存 CSV 文件
bool FileManager::saveCSV(const QString &fileName, const QVector<QPointF> &dataPoints) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("Cannot open file for writing: " + fileName);
        return false;
    }

    QTextStream out(&file);
    out << "Time (ms),Voltage (V)\n";
    for (const QPointF &point : dataPoints) {
        out << point.x() << "," << point.y() << "\n";
    }

    file.close();
    return true;
}

//保存原始格式的数据到csv
bool FileManager::saveRawCSV(const QString &fileName ,const QVector<double> &rawData){
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("Cannot open file for writing: " + fileName);
        return false;
    }

    QTextStream out(&file);
    out << "adc";
    for (const double &point : rawData) {
        out << point << "\n";
    }

    file.close();
    return true;
}

bool FileManager::loadRawCSV(const QString &fileName ,QVector<double> &rawData){
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showError("Cannot open file for reading: " + fileName);
        return false;
    }

    QTextStream in(&file);
    QString line = in.readLine(); // 读取表头

    // 检查表头是否为 "adc"
    if (line != "adc") {
        showError("Invalid file format: Missing or incorrect header 'adc'.");
        file.close();
        return false;
    }

    // 读取数据行
    while (!in.atEnd()) {
        line = in.readLine();
        bool ok;
        qreal value = line.toDouble(&ok); // 解析每一行的单个数据

        if (ok) {
            rawData.append(value); // 将有效的值添加到 dataPoints
        } else {
            qDebug() << "Invalid data line:" << line; // 打印无效数据行
        }
    }

    file.close();
    return true;
}

// 序列化数据到二进制文件
bool FileManager::serializeToBinary(const QString &fileName, const QVector<QPointF> &data) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical("Failed to open file for writing: %s", qPrintable(fileName));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

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

// 从二进制文件反序列化数据
QVector<QPointF> FileManager::deserializeFromBinary(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Failed to open file for reading: %s", qPrintable(fileName));
        return {}; // 返回空的 QVector<QPointF>
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    int size;
    in >> size; // 读取数据点的数量
    if (in.status() != QDataStream::Ok) {
        qCritical("Error reading the number of points from the file.");
        file.close();
        return {}; // 返回空的 QVector<QPointF>
    }

    //qDebug() << "Number of points read:" << size;

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
    //qDebug() << "Deserialization complete.";

    return data;
}

bool FileManager::serializeToBinary(const QString &fileName, const QVector<double> &data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical("Failed to open file for writing: %s", qPrintable(fileName));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    // 写入数据点的数量
    int size = data.size();
    out << size;

    // 写入数据点
    for (const double& point : data) {
        out << point;
    }

    file.close();
    qDebug() << "Serialization complete.";

    return true;
}
QVector<double> FileManager::deserializeRawFromBinary(const QString &fileName){
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Failed to open file for reading: %s", qPrintable(fileName));
        return {}; // 返回空的 QVector<QPointF>
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    int size;
    in >> size; // 读取数据点的数量
    if (in.status() != QDataStream::Ok) {
        qCritical("Error reading the number of points from the file.");
        file.close();
        return {}; // 返回空的 QVector<QPointF>
    }

    //qDebug() << "Number of points read:" << size;

    QVector<double> data(size);

    for (int i = 0; i < size; ++i) {
        qreal x;
        in >> x ;
        if (in.status() != QDataStream::Ok) {
            qCritical("Error reading point data from the file at index %d.", i);
            file.close();
            return {}; // 返回空的 QVector<QPointF>
        }
        data[i] = x;
    }

    file.close();
    //qDebug() << "Deserialization complete.";

    return data;
}

// 创建文本文件
bool FileManager::createTextFile(const QString &fileName, const QString &content) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("Failed to create text file: " + fileName);
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

// 读取文本文件
QString FileManager::readTextFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showError("Failed to read text file: " + fileName);
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

// 创建基于日期的文件夹
bool FileManager::createFolderByDate(const QString &baseDir) {
    QString folderName = getFolderNameByDate();
    QDir dir(baseDir);
    if (!dir.exists(folderName)) {
        return dir.mkdir(folderName);
    }
    return true;
}

QString FileManager::getFolderNameByDate() {
    return QDate::currentDate().toString("yyyy-MM-dd");
}

bool FileManager::deserializeFromBinary(const QString &fileName, QVector<double> &data,
                                        PS2000A_RANGE &range, int &timeBase, double &sampleInterval, int &divTagIndex)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Failed to open file for reading: %s", qPrintable(fileName));
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    // 读取 PS2000A_RANGE
    int rangeInt;
    in >> rangeInt;
    range = static_cast<PS2000A_RANGE>(rangeInt);

    // 读取 TimeBase
    in >> timeBase;

    // 读取采样时间间隔
    in >> sampleInterval;

    // 读取div索引
    in >> divTagIndex;

    // 读取数据点的数量
    int size;
    in >> size;

    // 读取数据点
    data.resize(size);
    for (int i = 0; i < size; ++i) {
        in >> data[i];
    }

    file.close();
    qDebug() << "Deserialization complete.";

    return true;
}


bool FileManager::serializeToBinary(const QString &fileName, const QVector<double> &data,
                                    PS2000A_RANGE range, int timeBase, double sampleInterval ,int divTagIndex)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical("Failed to open file for writing: %s", qPrintable(fileName));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    // 写入 PS2000A_RANGE（假设是一个枚举，按整数存储）
    out << static_cast<int>(range);

    // 写入 TimeBase
    out << timeBase;

    // 写入采样时间间隔
    out << sampleInterval;

    //写入div索引
    out << divTagIndex;

    // 写入数据点的数量
    int size = data.size();
    out << size;

    // 写入数据点
    for (const double& point : data) {
        out << point;
    }

    file.close();

    qDebug() << "Serialization complete.";

    return true;
}

bool FileManager::serializeToBinary(const QString &fileName, const QVector<double> &data,
                              PS2000A_RANGE range, int timeBase,
                              double sampleInterval ,int divTagIndex,
                              double up_volts_threshold ,double down_volts_threshold ,bool isShow){
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical("Failed to open file for writing: %s", qPrintable(fileName));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    // 写入 PS2000A_RANGE（假设是一个枚举，按整数存储）
    out << static_cast<int>(range);

    // 写入 TimeBase
    out << timeBase;

    // 写入采样时间间隔
    out << sampleInterval;

    //写入div索引
    out << divTagIndex;

    // 写入数据点的数量
    int size = data.size();
    out << size;

    // 写入数据点
    for (const double& point : data) {
        out << point;
    }

    out << up_volts_threshold;

    out << down_volts_threshold;

    out << isShow;

    file.close();

    qDebug() << "Serialization complete.";

    return true;
}

bool FileManager::deserializeFromBinary(const QString &fileName, QVector<double> &data,
                                  PS2000A_RANGE &range, int &timeBase,
                                  double &sampleInterval ,int &divTagIndex,
                                  double &up_volts_threshold ,double &down_volts_threshold ,bool &isShow){
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Failed to open file for reading: %s", qPrintable(fileName));
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_6); // 设置版本以确保兼容性

    // 读取 PS2000A_RANGE
    int rangeInt;
    in >> rangeInt;
    range = static_cast<PS2000A_RANGE>(rangeInt);

    // 读取 TimeBase
    in >> timeBase;

    // 读取采样时间间隔
    in >> sampleInterval;

    // 读取div索引
    in >> divTagIndex;

    // 读取数据点的数量
    int size;
    in >> size;

    // 读取数据点
    data.resize(size);
    for (int i = 0; i < size; ++i) {
        in >> data[i];
    }

    in >> up_volts_threshold;
    in >> down_volts_threshold;
    in >> isShow;

    file.close();
    qDebug() << "Deserialization complete.";

    return true;
}







































