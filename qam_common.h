#ifndef QAM_COMMON_H
#define QAM_COMMON_H

#include <QCoreApplication>
#include "qam_decoder/qam_decoder.h"

namespace QAM_Common
{
    const double Fs = 1832061;//280000;//ADC sample rate
    //const double f0 = 35000;//carrier freq
//    double sps = round(Fs/f0);//sample per symbol
    //const double mode = 1;//1-both stages enabled, 0-only sevond stage
    const double preamble_len = 20;//preamble length
    const double message_len = 255;
    const double QAM_order = 256;
    const double preamble_QAM_symbol = 128;//QAM symbol used in preamble

    //  input var for sweep
    const double f_sine = 35000;
    const double period_amount = 500;
    const double sine_sps = Fs/f_sine;
    const double f_pream = 2000;
    const double Fs_math_sweep = 280000;
    const double pream_sps = Fs_math_sweep/f_pream;
}

#endif // QAM_COMMON_H
