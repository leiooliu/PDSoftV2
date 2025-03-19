#ifndef PEAKPARAM_H
#define PEAKPARAM_H

class PeakParam{
public :
    double up_volts_threshold;
    double down_volts_threshold;
    bool isShow;

    PeakParam(){}

    PeakParam(double _up_volts_threshold ,
              double _down_volts_threshold ,
              bool _isShow)
        :up_volts_threshold(_up_volts_threshold) ,
        down_volts_threshold(_down_volts_threshold) ,
        isShow(_isShow)
    {

    }

};

#endif // PEAKPARAM_H
