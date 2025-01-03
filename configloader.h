#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVector>
#include <QDebug>
#include <QDir>
#include <QPointF>
#include <QVector>
class ConfigSetting{
public:
    bool realTimeWaveform;
    int dataCacheCount;
    int defaultTimeBase;
    int defaultCoupling;
    int defaultVoltage;
    int defaultChannel;
    int defaultBaseFrequency;
    bool segmentedRenderingChart;
    ConfigSetting(){}
    ConfigSetting(bool _realTimeWaveform ,
           int _dataCacheCount ,
           int _defaultTimeBase ,
           int _defaultCoupling ,
           int _defaultVoltage ,
           int _defaultChannel ,
           int _defaultBaseFrequency ,
           bool _segmentedRenderingChart):
        realTimeWaveform(_realTimeWaveform),
        dataCacheCount(_dataCacheCount),
        defaultTimeBase(_defaultTimeBase),
        defaultCoupling(_defaultCoupling),
        defaultVoltage(_defaultVoltage),
        defaultChannel(_defaultChannel),
        defaultBaseFrequency(_defaultBaseFrequency),
        segmentedRenderingChart(_segmentedRenderingChart){}
};

class ConfigLoader{
public:
    static ConfigSetting loadConfigFromJson(const QString &fileName) {
        // 获取当前目录路径
        QString currentDir = QCoreApplication::applicationDirPath();
        QString filePath = QDir(currentDir).filePath(fileName);

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "无法打开文件" << filePath;
            throw std::runtime_error("无法打开配置文件");
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isObject()) {
            qWarning() << "JSON格式错误或不是对象";
            throw std::runtime_error("配置文件格式错误");
        }

        QJsonObject obj = doc.object();

        // 解析 JSON 数据
        bool realTimeWaveform = obj["realTimeWaveform"].toBool();
        int dataCacheCount = obj["dataCacheCount"].toInt();
        int defaultTimeBase = obj["defaultTimeBase"].toInt();
        int defaultCoupling = obj["defaultCoupling"].toInt();
        int defaultVoltage = obj["defaultVoltage"].toInt();
        int defaultChannel = obj["defaultChannel"].toInt();
        int defaultBaseFrequency = obj["defaultBaseFrequency"].toInt();
        bool segmentedRenderingChart = obj["segmentedRenderingChart"].toBool();

        // 创建 Config 对象并返回
        return ConfigSetting(realTimeWaveform, dataCacheCount, defaultTimeBase,
                      defaultCoupling, defaultVoltage, defaultChannel,
                      defaultBaseFrequency, segmentedRenderingChart);
    }
};

#endif // CONFIGLOADER_H
