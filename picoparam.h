#ifndef PICOPARAM_H
#define PICOPARAM_H
#include <QObject>
#include <TimeBaseLoader.h>
#include <ps2000aApi.h>
class PicoParam  : public QObject{
    Q_OBJECT

public :
    //时基对象
    TimeBase timeBaseObj;
    //频道
    enPS2000AChannel Channel;
    //电压范围
    PS2000A_RANGE VoltageRange;
    //耦合类型
    PS2000A_COUPLING Coupling;
    //时基值
    uint32_t timebaseValue;
    //电压范围值
    float voltageRangeValue;
    //最大缓存数
    int16_t maxCacheCount;
    //采样率
    double samplingRate;


    PicoParam(TimeBase timeBaseObj ,
              enPS2000AChannel Channel ,
              PS2000A_RANGE VoltageRange ,
              PS2000A_COUPLING Coupling ,
              uint32_t timebaseValue ,
              float voltageRangeValue,
              int16_t maxCacheCount,
              double _samplingRate):
        timeBaseObj(timeBaseObj) ,
        Channel(Channel) ,
        VoltageRange(VoltageRange) ,
        Coupling(Coupling) ,
        timebaseValue(timebaseValue) ,
        voltageRangeValue(voltageRangeValue),
        maxCacheCount(maxCacheCount),
        samplingRate(_samplingRate)
    {

    }

    ~PicoParam(){
    }

};

#endif // PICOPARAM_H
