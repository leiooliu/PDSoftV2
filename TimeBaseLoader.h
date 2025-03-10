#ifndef TIMEBASELOADER_H
#define TIMEBASELOADER_H

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

class TimeBase {
public:
    QString scope;  // 示波器
    int sampleCount; // 样本数
    QString unit;
    int scale;//刻度;
    int maxScale;//最大刻度
    int conversion; //是否换算
    double interval;//采样间隔
    QString intervalUnit; //采样间隔单位
    uint32_t timebasevalue;
    QString frequencyUnit;//频率单位
    double frequencyScope;//频率范围
    int gridValue;
    TimeBase(){

    };
    // 构造函数
    TimeBase(const QString &scope,
             int sampleCount ,
             QString &_unit ,
             int _scale ,
             int _maxScale ,
             int  _conversion ,
             double _interval ,
             QString &_intervalUnit,
             uint32_t _timebasevalue,
             QString &_frequencyUnit,
             double _frequencyScope,
             int _gridValue)
        : scope(scope),
        sampleCount(sampleCount),
        unit(_unit),
        scale(_scale),
        maxScale(_maxScale) ,
        conversion(_conversion) ,
        interval(_interval) ,
        intervalUnit(_intervalUnit),
        timebasevalue(_timebasevalue),
        frequencyUnit(_frequencyUnit),
        frequencyScope(_frequencyScope),
        gridValue(_gridValue)
    {}
};

class TimeBaseLoader {
public:
    // 假设 adcToVolts 函数已经定义好了
    static double adcToVolts(int16_t adcValue) {
        // 示例转换函数
        return adcValue * (2.0 / 32767.0);
    }

    static QVector<QPointF> testTimeBaseData(TimeBase timebase){
        // 给定参数
        double timePerDiv = timebase.scale; // 每格代表的时间（微秒）
        int maxDivisions = timebase.maxScale; // 最大刻度（格子数）
        double sampleInterval = timebase.interval; // 采样间隔（纳秒）
        // 计算每个样本的时间间隔（单位：微秒）
        double timeInterval = sampleInterval; //totalTime / noOfSamples;

        // 计算总时间范围
        double totalTime =  maxDivisions; // 总时间为 1000 微秒
        if(timebase.intervalUnit == "ps" && timebase.unit == "ns"){
            timeInterval = timeInterval / 1000;
        }
        if(timebase.intervalUnit == "ps" && timebase.unit == "us"){
            //totalTime = totalTime / 1000;
            timeInterval = timeInterval / 1000000;
        }
        if(timebase.intervalUnit == "ns" && timebase.unit == "us"){
            timeInterval = timeInterval / 1000;
        }
        if(timebase.intervalUnit == "ns" && timebase.unit == "ms"){
            timeInterval = timeInterval / 1000000;
        }
        if(timebase.intervalUnit == "us" && timebase.unit == "ms"){
            timeInterval = timeInterval / 1000;
        }
        if(timebase.intervalUnit == "ms" && timebase.unit == "s"){
            timeInterval = timeInterval / 1000000;
        }

        // 计算总样本数
        int noOfSamples = timebase.sampleCount;



        // 创建一个存储数据点的向量
        QVector<QPointF> displayData;

        qDebug() << noOfSamples;
        // 假设 bufferA 是一个包含 noOfSamples 个样本的数组
        int32_t bufferA[noOfSamples]; // 示例数组

        // 填充 bufferA 数组（示例数据）
        for (int i = 0; i < noOfSamples; ++i) {
            bufferA[i] = -32768 + (rand() % 65536); // 随机生成的 ADC 值
        }

        // 计算并填充 displayData 向量
        for (int i = 0; i < noOfSamples; ++i) {
            float time = i * timeInterval; // 当前样本对应的时间（单位：微秒）
            double voltage = adcToVolts(bufferA[i]); // 转换电压
            displayData.append(QPointF(time, voltage)); // 将时间和电压数据添加到显示数据中
        }

        qDebug()<< timePerDiv << timebase.unit << "/div" ;
        qDebug()<< "总时间范围：" << totalTime;
        qDebug()<< "采样间隔：" << sampleInterval;
        qDebug()<<"样本总数："<<noOfSamples;
        qDebug()<< "=================================";

        return displayData;
    }

    static QVector<TimeBase> loadFromJson(const QString &fileName) {
        QVector<TimeBase> timeBaseList;

        // 获取当前目录路径
        QString currentDir = QCoreApplication::applicationDirPath();
        QString filePath = QDir(currentDir).filePath(fileName);

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "无法打开文件" << filePath;
            return timeBaseList;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isArray()) {
            qWarning() << "JSON格式错误或不是数组";
            return timeBaseList;
        }

        QJsonArray jsonArray = doc.array();
        for (const QJsonValue &value : jsonArray) {
            QJsonObject obj = value.toObject();
            QString scope = obj["scope"].toString();
            int sampleCount = obj["sampleCount"].toInt();
            QString unit = obj["unit"].toString();
            int scale = obj["scale"].toInt();
            int maxScale = obj["maxScale"].toInt();
            bool conversion = obj["conversion"].toInt();
            double interval = obj["interval"].toDouble();
            QString intervalUnit = obj["intervalUnit"].toString();
            uint32_t timebasevalue = obj["timebasevalue"].toInt();
            QString frequencyUnit = obj["frequencyUnit"].toString();
            double frequencyScope = obj["frequencyScope"].toDouble();
            int gridValue = obj["gridValue"].toInt();
            timeBaseList.append(TimeBase(
                scope, sampleCount ,unit ,scale,
                maxScale ,conversion ,interval ,intervalUnit,timebasevalue,frequencyUnit,frequencyScope,gridValue
            ));
        }

        return timeBaseList;
    }
};

#endif // TIMEBASELOADER_H
