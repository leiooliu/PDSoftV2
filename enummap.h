#ifndef ENUMMAP_H
#define ENUMMAP_H

#include <QObject>
#include <ps2000aApi.h>
#include <QMap>
#include <enumbinder.h>
class EnumMap : public QObject{
    Q_OBJECT
public:
    static EnumBinder<PS2000A_RANGE> *getVoltageBuilder(QComboBox *comboBox){
        EnumBinder<PS2000A_RANGE> *builder = new EnumBinder<PS2000A_RANGE>(comboBox ,EnumMap::getVoltageMap());
        return builder;
    }

    static EnumBinder<enPS2000AChannel> *getChannelBuilder(QComboBox *comboBox){
        EnumBinder<enPS2000AChannel> *builder = new EnumBinder<enPS2000AChannel>(comboBox ,EnumMap::getChannelMap());
        return builder;
    }

    static EnumBinder<enPS2000ACoupling> *getCouplingBuilder(QComboBox *comboBox){
        EnumBinder<enPS2000ACoupling> *builder = new EnumBinder<enPS2000ACoupling>(comboBox ,EnumMap::getCoupling());
        return builder;
    }

    static QMap<PS2000A_RANGE, QString> getVoltageMap(){
        QMap<PS2000A_RANGE, QString> rangeMap = {
            {PS2000A_10MV, "10 mV"},
            {PS2000A_20MV, "20 mV"},
            {PS2000A_50MV, "50 mV"},
            {PS2000A_100MV, "100 mV"},
            {PS2000A_200MV, "200 mV"},
            {PS2000A_500MV, "500 mV"},
            {PS2000A_1V, "1 V"},
            {PS2000A_2V, "2 V"},
            {PS2000A_5V, "5 V"},
            {PS2000A_10V, "10 V"},
            {PS2000A_20V, "20 V"}
        };
        return rangeMap;
    };

    static QMap<enPS2000AChannel, QString> getChannelMap(){
        QMap<enPS2000AChannel, QString> rangeChannelMap = {
            {PS2000A_CHANNEL_A, "CHANNEL_A"},
            {PS2000A_CHANNEL_B, "CHANNEL_B"},
            {PS2000A_CHANNEL_C, "CHANNEL_C"},
            {PS2000A_CHANNEL_D, "CHANNEL_D"}
        };
        return rangeChannelMap;
    };

    static QMap<enPS2000ACoupling ,QString> getCoupling(){
        QMap<enPS2000ACoupling, QString> couplingMap = {
            {PS2000A_AC, "AC"},
            {PS2000A_DC, "DC"}
        };
        return couplingMap;
    };

};

#endif // ENUMMAP_H
