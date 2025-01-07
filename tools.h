#ifndef TOOLS_H
#define TOOLS_H

#include <ps2000aApi.h>
class PDTools{
public:
    // adc值转换为电压
    static double adcToVolts(int16_t adcValue, PS2000A_RANGE range)
    {
        double maxVoltage = 0;
        switch (range) {
        case PS2000A_10MV:   maxVoltage = 0.01; break;    // ±10 mV
        case PS2000A_20MV:   maxVoltage = 0.02; break;    // ±20 mV
        case PS2000A_50MV:   maxVoltage = 0.05; break;    // ±50 mV
        case PS2000A_100MV:  maxVoltage = 0.1; break;     // ±100 mV
        case PS2000A_200MV:  maxVoltage = 0.2; break;     // ±200 mV
        case PS2000A_500MV:  maxVoltage = 0.5; break;     // ±500 mV
        case PS2000A_1V:     maxVoltage = 1.0; break;     // ±1 V
        case PS2000A_2V:     maxVoltage = 2.0; break;     // ±2 V
        case PS2000A_5V:     maxVoltage = 5.0; break;     // ±5 V
        case PS2000A_10V:    maxVoltage = 10.0; break;    // ±10 V
        case PS2000A_20V:    maxVoltage = 20.0; break;    // ±20 V
        case PS2000A_50V:    maxVoltage = 50.0; break;    // ±50 V
        default:             maxVoltage = 1.0; break;     // 默认1 V
        }
        int16_t adcMaxValue = 32767; // 16位ADC的最大值
        return (static_cast<double>(adcValue) / adcMaxValue) * maxVoltage;
    }
};

#endif // TOOLS_H
