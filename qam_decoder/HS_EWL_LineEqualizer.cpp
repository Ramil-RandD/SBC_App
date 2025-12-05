/*
 * File: HS_EWL_LineEqualizer.c
 *
 * MATLAB Coder version            : 5.1
 * C/C++ source code generated on  : 28-Mar-2025 10:30:41
 */

 /* Include Files */
#include "HS_EWL_LineEqualizer.h"
#include "HS_EWL_LineEqualizer_data.h"
#include "HS_EWL_LineEqualizer_initialize.h"
#include "minOrMax.h"
#include "rt_nonfinite.h"
#include "rt_nonfinite.h"
#include <math.h>
#include <string.h>

/* Type Definitions */
#ifndef typedef_cell_wrap_3
#define typedef_cell_wrap_3

typedef struct {
    unsigned int f1[8];
} cell_wrap_3;

#endif                                 /*typedef_cell_wrap_3*/

#ifndef typedef_struct_T
#define typedef_struct_T

typedef struct {
    double Length;
    creal_T Buffer[3];
    double Pointer;
} struct_T;

#endif                                 /*typedef_struct_T*/

#ifndef typedef_comm_LinearEqualizer
#define typedef_comm_LinearEqualizer

typedef struct {
    int isInitialized;
    cell_wrap_3 inputVarSize[3];
    double ForgettingFactor;
    creal_T TapDelayLine[13];
    creal_T ReferenceSymbol;
    creal_T Weights[13];
    creal_T InverseCorrelationMatrix[169];
    boolean_T PreviousTrainingFlag;
    double ActiveDelay;
    struct_T TrainingDelayBuffer;
    double SymbolCounter;
    double NumTrainingSymbols;
    double WeightUpdateCounter;
} comm_LinearEqualizer;

#endif                                 /*typedef_comm_LinearEqualizer*/

/* Variable Definitions */
static comm_LinearEqualizer LineEqualizerStr;
static boolean_T LineEqualizerStr_not_empty;
static uint8_T LineEqualizer_crc_cnt = 0;

/* Function Declarations */
static double rt_hypotd_snf(double u0, double u1);

/* Function Definitions */
/*
 * Arguments    : double u0
 *                double u1
 * Return Type  : double
 */
static double rt_hypotd_snf(double u0, double u1)
{
    double a;
    double y;
    a = fabs(u0);
    y = fabs(u1);
    if (a < y) {
        a /= y;
        y *= sqrt(a * a + 1.0);
    }
    else if (a > y) {
        y /= a;
        y = a * sqrt(y * y + 1.0);
    }
    else {
        if (!rtIsNaN(y)) {
            y = a * 1.4142135623730951;
        }
    }

    return y;
}

/*
 * Arguments    : const creal_T input_buf[525]
 *                const creal_T training_buf[525]
 *                boolean_T trainingFlag
 *                creal_T EqualizedData[525]
 *                creal_T channel_resp[13]
 * Return Type  : void
 */
void HS_EWL_LineEqualizer(const creal_T *input_buf, uint32_T buf_size, const creal_T
    *training_buf, boolean_T trainingFlag, creal_T *EqualizedData, creal_T
    channel_resp[13])
{
    static const cint8_T icv[256] = { { -15,/* re */
        15                               /* im */
      }, { -15,                          /* re */
        13                               /* im */
      }, { -15,                          /* re */
        9                                /* im */
      }, { -15,                          /* re */
        11                               /* im */
      }, { -15,                          /* re */
        1                                /* im */
      }, { -15,                          /* re */
        3                                /* im */
      }, { -15,                          /* re */
        7                                /* im */
      }, { -15,                          /* re */
        5                                /* im */
      }, { -15,                          /* re */
        -15                              /* im */
      }, { -15,                          /* re */
        -13                              /* im */
      }, { -15,                          /* re */
        -9                               /* im */
      }, { -15,                          /* re */
        -11                              /* im */
      }, { -15,                          /* re */
        -1                               /* im */
      }, { -15,                          /* re */
        -3                               /* im */
      }, { -15,                          /* re */
        -7                               /* im */
      }, { -15,                          /* re */
        -5                               /* im */
      }, { -13,                          /* re */
        15                               /* im */
      }, { -13,                          /* re */
        13                               /* im */
      }, { -13,                          /* re */
        9                                /* im */
      }, { -13,                          /* re */
        11                               /* im */
      }, { -13,                          /* re */
        1                                /* im */
      }, { -13,                          /* re */
        3                                /* im */
      }, { -13,                          /* re */
        7                                /* im */
      }, { -13,                          /* re */
        5                                /* im */
      }, { -13,                          /* re */
        -15                              /* im */
      }, { -13,                          /* re */
        -13                              /* im */
      }, { -13,                          /* re */
        -9                               /* im */
      }, { -13,                          /* re */
        -11                              /* im */
      }, { -13,                          /* re */
        -1                               /* im */
      }, { -13,                          /* re */
        -3                               /* im */
      }, { -13,                          /* re */
        -7                               /* im */
      }, { -13,                          /* re */
        -5                               /* im */
      }, { -9,                           /* re */
        15                               /* im */
      }, { -9,                           /* re */
        13                               /* im */
      }, { -9,                           /* re */
        9                                /* im */
      }, { -9,                           /* re */
        11                               /* im */
      }, { -9,                           /* re */
        1                                /* im */
      }, { -9,                           /* re */
        3                                /* im */
      }, { -9,                           /* re */
        7                                /* im */
      }, { -9,                           /* re */
        5                                /* im */
      }, { -9,                           /* re */
        -15                              /* im */
      }, { -9,                           /* re */
        -13                              /* im */
      }, { -9,                           /* re */
        -9                               /* im */
      }, { -9,                           /* re */
        -11                              /* im */
      }, { -9,                           /* re */
        -1                               /* im */
      }, { -9,                           /* re */
        -3                               /* im */
      }, { -9,                           /* re */
        -7                               /* im */
      }, { -9,                           /* re */
        -5                               /* im */
      }, { -11,                          /* re */
        15                               /* im */
      }, { -11,                          /* re */
        13                               /* im */
      }, { -11,                          /* re */
        9                                /* im */
      }, { -11,                          /* re */
        11                               /* im */
      }, { -11,                          /* re */
        1                                /* im */
      }, { -11,                          /* re */
        3                                /* im */
      }, { -11,                          /* re */
        7                                /* im */
      }, { -11,                          /* re */
        5                                /* im */
      }, { -11,                          /* re */
        -15                              /* im */
      }, { -11,                          /* re */
        -13                              /* im */
      }, { -11,                          /* re */
        -9                               /* im */
      }, { -11,                          /* re */
        -11                              /* im */
      }, { -11,                          /* re */
        -1                               /* im */
      }, { -11,                          /* re */
        -3                               /* im */
      }, { -11,                          /* re */
        -7                               /* im */
      }, { -11,                          /* re */
        -5                               /* im */
      }, { -1,                           /* re */
        15                               /* im */
      }, { -1,                           /* re */
        13                               /* im */
      }, { -1,                           /* re */
        9                                /* im */
      }, { -1,                           /* re */
        11                               /* im */
      }, { -1,                           /* re */
        1                                /* im */
      }, { -1,                           /* re */
        3                                /* im */
      }, { -1,                           /* re */
        7                                /* im */
      }, { -1,                           /* re */
        5                                /* im */
      }, { -1,                           /* re */
        -15                              /* im */
      }, { -1,                           /* re */
        -13                              /* im */
      }, { -1,                           /* re */
        -9                               /* im */
      }, { -1,                           /* re */
        -11                              /* im */
      }, { -1,                           /* re */
        -1                               /* im */
      }, { -1,                           /* re */
        -3                               /* im */
      }, { -1,                           /* re */
        -7                               /* im */
      }, { -1,                           /* re */
        -5                               /* im */
      }, { -3,                           /* re */
        15                               /* im */
      }, { -3,                           /* re */
        13                               /* im */
      }, { -3,                           /* re */
        9                                /* im */
      }, { -3,                           /* re */
        11                               /* im */
      }, { -3,                           /* re */
        1                                /* im */
      }, { -3,                           /* re */
        3                                /* im */
      }, { -3,                           /* re */
        7                                /* im */
      }, { -3,                           /* re */
        5                                /* im */
      }, { -3,                           /* re */
        -15                              /* im */
      }, { -3,                           /* re */
        -13                              /* im */
      }, { -3,                           /* re */
        -9                               /* im */
      }, { -3,                           /* re */
        -11                              /* im */
      }, { -3,                           /* re */
        -1                               /* im */
      }, { -3,                           /* re */
        -3                               /* im */
      }, { -3,                           /* re */
        -7                               /* im */
      }, { -3,                           /* re */
        -5                               /* im */
      }, { -7,                           /* re */
        15                               /* im */
      }, { -7,                           /* re */
        13                               /* im */
      }, { -7,                           /* re */
        9                                /* im */
      }, { -7,                           /* re */
        11                               /* im */
      }, { -7,                           /* re */
        1                                /* im */
      }, { -7,                           /* re */
        3                                /* im */
      }, { -7,                           /* re */
        7                                /* im */
      }, { -7,                           /* re */
        5                                /* im */
      }, { -7,                           /* re */
        -15                              /* im */
      }, { -7,                           /* re */
        -13                              /* im */
      }, { -7,                           /* re */
        -9                               /* im */
      }, { -7,                           /* re */
        -11                              /* im */
      }, { -7,                           /* re */
        -1                               /* im */
      }, { -7,                           /* re */
        -3                               /* im */
      }, { -7,                           /* re */
        -7                               /* im */
      }, { -7,                           /* re */
        -5                               /* im */
      }, { -5,                           /* re */
        15                               /* im */
      }, { -5,                           /* re */
        13                               /* im */
      }, { -5,                           /* re */
        9                                /* im */
      }, { -5,                           /* re */
        11                               /* im */
      }, { -5,                           /* re */
        1                                /* im */
      }, { -5,                           /* re */
        3                                /* im */
      }, { -5,                           /* re */
        7                                /* im */
      }, { -5,                           /* re */
        5                                /* im */
      }, { -5,                           /* re */
        -15                              /* im */
      }, { -5,                           /* re */
        -13                              /* im */
      }, { -5,                           /* re */
        -9                               /* im */
      }, { -5,                           /* re */
        -11                              /* im */
      }, { -5,                           /* re */
        -1                               /* im */
      }, { -5,                           /* re */
        -3                               /* im */
      }, { -5,                           /* re */
        -7                               /* im */
      }, { -5,                           /* re */
        -5                               /* im */
      }, { 15,                           /* re */
        15                               /* im */
      }, { 15,                           /* re */
        13                               /* im */
      }, { 15,                           /* re */
        9                                /* im */
      }, { 15,                           /* re */
        11                               /* im */
      }, { 15,                           /* re */
        1                                /* im */
      }, { 15,                           /* re */
        3                                /* im */
      }, { 15,                           /* re */
        7                                /* im */
      }, { 15,                           /* re */
        5                                /* im */
      }, { 15,                           /* re */
        -15                              /* im */
      }, { 15,                           /* re */
        -13                              /* im */
      }, { 15,                           /* re */
        -9                               /* im */
      }, { 15,                           /* re */
        -11                              /* im */
      }, { 15,                           /* re */
        -1                               /* im */
      }, { 15,                           /* re */
        -3                               /* im */
      }, { 15,                           /* re */
        -7                               /* im */
      }, { 15,                           /* re */
        -5                               /* im */
      }, { 13,                           /* re */
        15                               /* im */
      }, { 13,                           /* re */
        13                               /* im */
      }, { 13,                           /* re */
        9                                /* im */
      }, { 13,                           /* re */
        11                               /* im */
      }, { 13,                           /* re */
        1                                /* im */
      }, { 13,                           /* re */
        3                                /* im */
      }, { 13,                           /* re */
        7                                /* im */
      }, { 13,                           /* re */
        5                                /* im */
      }, { 13,                           /* re */
        -15                              /* im */
      }, { 13,                           /* re */
        -13                              /* im */
      }, { 13,                           /* re */
        -9                               /* im */
      }, { 13,                           /* re */
        -11                              /* im */
      }, { 13,                           /* re */
        -1                               /* im */
      }, { 13,                           /* re */
        -3                               /* im */
      }, { 13,                           /* re */
        -7                               /* im */
      }, { 13,                           /* re */
        -5                               /* im */
      }, { 9,                            /* re */
        15                               /* im */
      }, { 9,                            /* re */
        13                               /* im */
      }, { 9,                            /* re */
        9                                /* im */
      }, { 9,                            /* re */
        11                               /* im */
      }, { 9,                            /* re */
        1                                /* im */
      }, { 9,                            /* re */
        3                                /* im */
      }, { 9,                            /* re */
        7                                /* im */
      }, { 9,                            /* re */
        5                                /* im */
      }, { 9,                            /* re */
        -15                              /* im */
      }, { 9,                            /* re */
        -13                              /* im */
      }, { 9,                            /* re */
        -9                               /* im */
      }, { 9,                            /* re */
        -11                              /* im */
      }, { 9,                            /* re */
        -1                               /* im */
      }, { 9,                            /* re */
        -3                               /* im */
      }, { 9,                            /* re */
        -7                               /* im */
      }, { 9,                            /* re */
        -5                               /* im */
      }, { 11,                           /* re */
        15                               /* im */
      }, { 11,                           /* re */
        13                               /* im */
      }, { 11,                           /* re */
        9                                /* im */
      }, { 11,                           /* re */
        11                               /* im */
      }, { 11,                           /* re */
        1                                /* im */
      }, { 11,                           /* re */
        3                                /* im */
      }, { 11,                           /* re */
        7                                /* im */
      }, { 11,                           /* re */
        5                                /* im */
      }, { 11,                           /* re */
        -15                              /* im */
      }, { 11,                           /* re */
        -13                              /* im */
      }, { 11,                           /* re */
        -9                               /* im */
      }, { 11,                           /* re */
        -11                              /* im */
      }, { 11,                           /* re */
        -1                               /* im */
      }, { 11,                           /* re */
        -3                               /* im */
      }, { 11,                           /* re */
        -7                               /* im */
      }, { 11,                           /* re */
        -5                               /* im */
      }, { 1,                            /* re */
        15                               /* im */
      }, { 1,                            /* re */
        13                               /* im */
      }, { 1,                            /* re */
        9                                /* im */
      }, { 1,                            /* re */
        11                               /* im */
      }, { 1,                            /* re */
        1                                /* im */
      }, { 1,                            /* re */
        3                                /* im */
      }, { 1,                            /* re */
        7                                /* im */
      }, { 1,                            /* re */
        5                                /* im */
      }, { 1,                            /* re */
        -15                              /* im */
      }, { 1,                            /* re */
        -13                              /* im */
      }, { 1,                            /* re */
        -9                               /* im */
      }, { 1,                            /* re */
        -11                              /* im */
      }, { 1,                            /* re */
        -1                               /* im */
      }, { 1,                            /* re */
        -3                               /* im */
      }, { 1,                            /* re */
        -7                               /* im */
      }, { 1,                            /* re */
        -5                               /* im */
      }, { 3,                            /* re */
        15                               /* im */
      }, { 3,                            /* re */
        13                               /* im */
      }, { 3,                            /* re */
        9                                /* im */
      }, { 3,                            /* re */
        11                               /* im */
      }, { 3,                            /* re */
        1                                /* im */
      }, { 3,                            /* re */
        3                                /* im */
      }, { 3,                            /* re */
        7                                /* im */
      }, { 3,                            /* re */
        5                                /* im */
      }, { 3,                            /* re */
        -15                              /* im */
      }, { 3,                            /* re */
        -13                              /* im */
      }, { 3,                            /* re */
        -9                               /* im */
      }, { 3,                            /* re */
        -11                              /* im */
      }, { 3,                            /* re */
        -1                               /* im */
      }, { 3,                            /* re */
        -3                               /* im */
      }, { 3,                            /* re */
        -7                               /* im */
      }, { 3,                            /* re */
        -5                               /* im */
      }, { 7,                            /* re */
        15                               /* im */
      }, { 7,                            /* re */
        13                               /* im */
      }, { 7,                            /* re */
        9                                /* im */
      }, { 7,                            /* re */
        11                               /* im */
      }, { 7,                            /* re */
        1                                /* im */
      }, { 7,                            /* re */
        3                                /* im */
      }, { 7,                            /* re */
        7                                /* im */
      }, { 7,                            /* re */
        5                                /* im */
      }, { 7,                            /* re */
        -15                              /* im */
      }, { 7,                            /* re */
        -13                              /* im */
      }, { 7,                            /* re */
        -9                               /* im */
      }, { 7,                            /* re */
        -11                              /* im */
      }, { 7,                            /* re */
        -1                               /* im */
      }, { 7,                            /* re */
        -3                               /* im */
      }, { 7,                            /* re */
        -7                               /* im */
      }, { 7,                            /* re */
        -5                               /* im */
      }, { 5,                            /* re */
        15                               /* im */
      }, { 5,                            /* re */
        13                               /* im */
      }, { 5,                            /* re */
        9                                /* im */
      }, { 5,                            /* re */
        11                               /* im */
      }, { 5,                            /* re */
        1                                /* im */
      }, { 5,                            /* re */
        3                                /* im */
      }, { 5,                            /* re */
        7                                /* im */
      }, { 5,                            /* re */
        5                                /* im */
      }, { 5,                            /* re */
        -15                              /* im */
      }, { 5,                            /* re */
        -13                              /* im */
      }, { 5,                            /* re */
        -9                               /* im */
      }, { 5,                            /* re */
        -11                              /* im */
      }, { 5,                            /* re */
        -1                               /* im */
      }, { 5,                            /* re */
        -3                               /* im */
      }, { 5,                            /* re */
        -7                               /* im */
      }, { 5,                            /* re */
        -5                               /* im */
      } };

    static const cint8_T sigConst[256] = { { -15,/* re */
        15                               /* im */
      }, { -15,                          /* re */
        13                               /* im */
      }, { -15,                          /* re */
        9                                /* im */
      }, { -15,                          /* re */
        11                               /* im */
      }, { -15,                          /* re */
        1                                /* im */
      }, { -15,                          /* re */
        3                                /* im */
      }, { -15,                          /* re */
        7                                /* im */
      }, { -15,                          /* re */
        5                                /* im */
      }, { -15,                          /* re */
        -15                              /* im */
      }, { -15,                          /* re */
        -13                              /* im */
      }, { -15,                          /* re */
        -9                               /* im */
      }, { -15,                          /* re */
        -11                              /* im */
      }, { -15,                          /* re */
        -1                               /* im */
      }, { -15,                          /* re */
        -3                               /* im */
      }, { -15,                          /* re */
        -7                               /* im */
      }, { -15,                          /* re */
        -5                               /* im */
      }, { -13,                          /* re */
        15                               /* im */
      }, { -13,                          /* re */
        13                               /* im */
      }, { -13,                          /* re */
        9                                /* im */
      }, { -13,                          /* re */
        11                               /* im */
      }, { -13,                          /* re */
        1                                /* im */
      }, { -13,                          /* re */
        3                                /* im */
      }, { -13,                          /* re */
        7                                /* im */
      }, { -13,                          /* re */
        5                                /* im */
      }, { -13,                          /* re */
        -15                              /* im */
      }, { -13,                          /* re */
        -13                              /* im */
      }, { -13,                          /* re */
        -9                               /* im */
      }, { -13,                          /* re */
        -11                              /* im */
      }, { -13,                          /* re */
        -1                               /* im */
      }, { -13,                          /* re */
        -3                               /* im */
      }, { -13,                          /* re */
        -7                               /* im */
      }, { -13,                          /* re */
        -5                               /* im */
      }, { -9,                           /* re */
        15                               /* im */
      }, { -9,                           /* re */
        13                               /* im */
      }, { -9,                           /* re */
        9                                /* im */
      }, { -9,                           /* re */
        11                               /* im */
      }, { -9,                           /* re */
        1                                /* im */
      }, { -9,                           /* re */
        3                                /* im */
      }, { -9,                           /* re */
        7                                /* im */
      }, { -9,                           /* re */
        5                                /* im */
      }, { -9,                           /* re */
        -15                              /* im */
      }, { -9,                           /* re */
        -13                              /* im */
      }, { -9,                           /* re */
        -9                               /* im */
      }, { -9,                           /* re */
        -11                              /* im */
      }, { -9,                           /* re */
        -1                               /* im */
      }, { -9,                           /* re */
        -3                               /* im */
      }, { -9,                           /* re */
        -7                               /* im */
      }, { -9,                           /* re */
        -5                               /* im */
      }, { -11,                          /* re */
        15                               /* im */
      }, { -11,                          /* re */
        13                               /* im */
      }, { -11,                          /* re */
        9                                /* im */
      }, { -11,                          /* re */
        11                               /* im */
      }, { -11,                          /* re */
        1                                /* im */
      }, { -11,                          /* re */
        3                                /* im */
      }, { -11,                          /* re */
        7                                /* im */
      }, { -11,                          /* re */
        5                                /* im */
      }, { -11,                          /* re */
        -15                              /* im */
      }, { -11,                          /* re */
        -13                              /* im */
      }, { -11,                          /* re */
        -9                               /* im */
      }, { -11,                          /* re */
        -11                              /* im */
      }, { -11,                          /* re */
        -1                               /* im */
      }, { -11,                          /* re */
        -3                               /* im */
      }, { -11,                          /* re */
        -7                               /* im */
      }, { -11,                          /* re */
        -5                               /* im */
      }, { -1,                           /* re */
        15                               /* im */
      }, { -1,                           /* re */
        13                               /* im */
      }, { -1,                           /* re */
        9                                /* im */
      }, { -1,                           /* re */
        11                               /* im */
      }, { -1,                           /* re */
        1                                /* im */
      }, { -1,                           /* re */
        3                                /* im */
      }, { -1,                           /* re */
        7                                /* im */
      }, { -1,                           /* re */
        5                                /* im */
      }, { -1,                           /* re */
        -15                              /* im */
      }, { -1,                           /* re */
        -13                              /* im */
      }, { -1,                           /* re */
        -9                               /* im */
      }, { -1,                           /* re */
        -11                              /* im */
      }, { -1,                           /* re */
        -1                               /* im */
      }, { -1,                           /* re */
        -3                               /* im */
      }, { -1,                           /* re */
        -7                               /* im */
      }, { -1,                           /* re */
        -5                               /* im */
      }, { -3,                           /* re */
        15                               /* im */
      }, { -3,                           /* re */
        13                               /* im */
      }, { -3,                           /* re */
        9                                /* im */
      }, { -3,                           /* re */
        11                               /* im */
      }, { -3,                           /* re */
        1                                /* im */
      }, { -3,                           /* re */
        3                                /* im */
      }, { -3,                           /* re */
        7                                /* im */
      }, { -3,                           /* re */
        5                                /* im */
      }, { -3,                           /* re */
        -15                              /* im */
      }, { -3,                           /* re */
        -13                              /* im */
      }, { -3,                           /* re */
        -9                               /* im */
      }, { -3,                           /* re */
        -11                              /* im */
      }, { -3,                           /* re */
        -1                               /* im */
      }, { -3,                           /* re */
        -3                               /* im */
      }, { -3,                           /* re */
        -7                               /* im */
      }, { -3,                           /* re */
        -5                               /* im */
      }, { -7,                           /* re */
        15                               /* im */
      }, { -7,                           /* re */
        13                               /* im */
      }, { -7,                           /* re */
        9                                /* im */
      }, { -7,                           /* re */
        11                               /* im */
      }, { -7,                           /* re */
        1                                /* im */
      }, { -7,                           /* re */
        3                                /* im */
      }, { -7,                           /* re */
        7                                /* im */
      }, { -7,                           /* re */
        5                                /* im */
      }, { -7,                           /* re */
        -15                              /* im */
      }, { -7,                           /* re */
        -13                              /* im */
      }, { -7,                           /* re */
        -9                               /* im */
      }, { -7,                           /* re */
        -11                              /* im */
      }, { -7,                           /* re */
        -1                               /* im */
      }, { -7,                           /* re */
        -3                               /* im */
      }, { -7,                           /* re */
        -7                               /* im */
      }, { -7,                           /* re */
        -5                               /* im */
      }, { -5,                           /* re */
        15                               /* im */
      }, { -5,                           /* re */
        13                               /* im */
      }, { -5,                           /* re */
        9                                /* im */
      }, { -5,                           /* re */
        11                               /* im */
      }, { -5,                           /* re */
        1                                /* im */
      }, { -5,                           /* re */
        3                                /* im */
      }, { -5,                           /* re */
        7                                /* im */
      }, { -5,                           /* re */
        5                                /* im */
      }, { -5,                           /* re */
        -15                              /* im */
      }, { -5,                           /* re */
        -13                              /* im */
      }, { -5,                           /* re */
        -9                               /* im */
      }, { -5,                           /* re */
        -11                              /* im */
      }, { -5,                           /* re */
        -1                               /* im */
      }, { -5,                           /* re */
        -3                               /* im */
      }, { -5,                           /* re */
        -7                               /* im */
      }, { -5,                           /* re */
        -5                               /* im */
      }, { 15,                           /* re */
        15                               /* im */
      }, { 15,                           /* re */
        13                               /* im */
      }, { 15,                           /* re */
        9                                /* im */
      }, { 15,                           /* re */
        11                               /* im */
      }, { 15,                           /* re */
        1                                /* im */
      }, { 15,                           /* re */
        3                                /* im */
      }, { 15,                           /* re */
        7                                /* im */
      }, { 15,                           /* re */
        5                                /* im */
      }, { 15,                           /* re */
        -15                              /* im */
      }, { 15,                           /* re */
        -13                              /* im */
      }, { 15,                           /* re */
        -9                               /* im */
      }, { 15,                           /* re */
        -11                              /* im */
      }, { 15,                           /* re */
        -1                               /* im */
      }, { 15,                           /* re */
        -3                               /* im */
      }, { 15,                           /* re */
        -7                               /* im */
      }, { 15,                           /* re */
        -5                               /* im */
      }, { 13,                           /* re */
        15                               /* im */
      }, { 13,                           /* re */
        13                               /* im */
      }, { 13,                           /* re */
        9                                /* im */
      }, { 13,                           /* re */
        11                               /* im */
      }, { 13,                           /* re */
        1                                /* im */
      }, { 13,                           /* re */
        3                                /* im */
      }, { 13,                           /* re */
        7                                /* im */
      }, { 13,                           /* re */
        5                                /* im */
      }, { 13,                           /* re */
        -15                              /* im */
      }, { 13,                           /* re */
        -13                              /* im */
      }, { 13,                           /* re */
        -9                               /* im */
      }, { 13,                           /* re */
        -11                              /* im */
      }, { 13,                           /* re */
        -1                               /* im */
      }, { 13,                           /* re */
        -3                               /* im */
      }, { 13,                           /* re */
        -7                               /* im */
      }, { 13,                           /* re */
        -5                               /* im */
      }, { 9,                            /* re */
        15                               /* im */
      }, { 9,                            /* re */
        13                               /* im */
      }, { 9,                            /* re */
        9                                /* im */
      }, { 9,                            /* re */
        11                               /* im */
      }, { 9,                            /* re */
        1                                /* im */
      }, { 9,                            /* re */
        3                                /* im */
      }, { 9,                            /* re */
        7                                /* im */
      }, { 9,                            /* re */
        5                                /* im */
      }, { 9,                            /* re */
        -15                              /* im */
      }, { 9,                            /* re */
        -13                              /* im */
      }, { 9,                            /* re */
        -9                               /* im */
      }, { 9,                            /* re */
        -11                              /* im */
      }, { 9,                            /* re */
        -1                               /* im */
      }, { 9,                            /* re */
        -3                               /* im */
      }, { 9,                            /* re */
        -7                               /* im */
      }, { 9,                            /* re */
        -5                               /* im */
      }, { 11,                           /* re */
        15                               /* im */
      }, { 11,                           /* re */
        13                               /* im */
      }, { 11,                           /* re */
        9                                /* im */
      }, { 11,                           /* re */
        11                               /* im */
      }, { 11,                           /* re */
        1                                /* im */
      }, { 11,                           /* re */
        3                                /* im */
      }, { 11,                           /* re */
        7                                /* im */
      }, { 11,                           /* re */
        5                                /* im */
      }, { 11,                           /* re */
        -15                              /* im */
      }, { 11,                           /* re */
        -13                              /* im */
      }, { 11,                           /* re */
        -9                               /* im */
      }, { 11,                           /* re */
        -11                              /* im */
      }, { 11,                           /* re */
        -1                               /* im */
      }, { 11,                           /* re */
        -3                               /* im */
      }, { 11,                           /* re */
        -7                               /* im */
      }, { 11,                           /* re */
        -5                               /* im */
      }, { 1,                            /* re */
        15                               /* im */
      }, { 1,                            /* re */
        13                               /* im */
      }, { 1,                            /* re */
        9                                /* im */
      }, { 1,                            /* re */
        11                               /* im */
      }, { 1,                            /* re */
        1                                /* im */
      }, { 1,                            /* re */
        3                                /* im */
      }, { 1,                            /* re */
        7                                /* im */
      }, { 1,                            /* re */
        5                                /* im */
      }, { 1,                            /* re */
        -15                              /* im */
      }, { 1,                            /* re */
        -13                              /* im */
      }, { 1,                            /* re */
        -9                               /* im */
      }, { 1,                            /* re */
        -11                              /* im */
      }, { 1,                            /* re */
        -1                               /* im */
      }, { 1,                            /* re */
        -3                               /* im */
      }, { 1,                            /* re */
        -7                               /* im */
      }, { 1,                            /* re */
        -5                               /* im */
      }, { 3,                            /* re */
        15                               /* im */
      }, { 3,                            /* re */
        13                               /* im */
      }, { 3,                            /* re */
        9                                /* im */
      }, { 3,                            /* re */
        11                               /* im */
      }, { 3,                            /* re */
        1                                /* im */
      }, { 3,                            /* re */
        3                                /* im */
      }, { 3,                            /* re */
        7                                /* im */
      }, { 3,                            /* re */
        5                                /* im */
      }, { 3,                            /* re */
        -15                              /* im */
      }, { 3,                            /* re */
        -13                              /* im */
      }, { 3,                            /* re */
        -9                               /* im */
      }, { 3,                            /* re */
        -11                              /* im */
      }, { 3,                            /* re */
        -1                               /* im */
      }, { 3,                            /* re */
        -3                               /* im */
      }, { 3,                            /* re */
        -7                               /* im */
      }, { 3,                            /* re */
        -5                               /* im */
      }, { 7,                            /* re */
        15                               /* im */
      }, { 7,                            /* re */
        13                               /* im */
      }, { 7,                            /* re */
        9                                /* im */
      }, { 7,                            /* re */
        11                               /* im */
      }, { 7,                            /* re */
        1                                /* im */
      }, { 7,                            /* re */
        3                                /* im */
      }, { 7,                            /* re */
        7                                /* im */
      }, { 7,                            /* re */
        5                                /* im */
      }, { 7,                            /* re */
        -15                              /* im */
      }, { 7,                            /* re */
        -13                              /* im */
      }, { 7,                            /* re */
        -9                               /* im */
      }, { 7,                            /* re */
        -11                              /* im */
      }, { 7,                            /* re */
        -1                               /* im */
      }, { 7,                            /* re */
        -3                               /* im */
      }, { 7,                            /* re */
        -7                               /* im */
      }, { 7,                            /* re */
        -5                               /* im */
      }, { 5,                            /* re */
        15                               /* im */
      }, { 5,                            /* re */
        13                               /* im */
      }, { 5,                            /* re */
        9                                /* im */
      }, { 5,                            /* re */
        11                               /* im */
      }, { 5,                            /* re */
        1                                /* im */
      }, { 5,                            /* re */
        3                                /* im */
      }, { 5,                            /* re */
        7                                /* im */
      }, { 5,                            /* re */
        5                                /* im */
      }, { 5,                            /* re */
        -15                              /* im */
      }, { 5,                            /* re */
        -13                              /* im */
      }, { 5,                            /* re */
        -9                               /* im */
      }, { 5,                            /* re */
        -11                              /* im */
      }, { 5,                            /* re */
        -1                               /* im */
      }, { 5,                            /* re */
        -3                               /* im */
      }, { 5,                            /* re */
        -7                               /* im */
      }, { 5,                            /* re */
        -5                               /* im */
      } };

    static const double dv[169] = { 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
      0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1 };

    struct_T r;
    creal_T dcv[169];
    creal_T K[13];
    creal_T UP[13];
    double varargin_1[256];
    double UP_im;
    double UP_re;
    double b_channel_resp_im;
    double channel_resp_im;
    double channel_resp_re;
    double d;
    double d1;
    double dRef_im;
    double dRef_re;
    double im;
    double lambdaInverse;
    double numTrainSymbols;
    double re;
    double ref_im;
    double ref_re;
    double symbolCounter;
    double weightUpdateCounter;
    int i;
    int i1;
    int idx;
    int p;
    if (!isInitialized_HS_EWL_LineEqualizer) {
        HS_EWL_LineEqualizer_initialize();
    }

    if (!LineEqualizerStr_not_empty) {
        LineEqualizerStr.ForgettingFactor = 0.99;
        LineEqualizerStr.isInitialized = 0;
        LineEqualizerStr_not_empty = true;

        /* [ -15.0000 -15.0000i -15.0000 -13.0000i -15.0000 - 9.0000i -15.0000 -11.0000i -15.0000 - 1.0000i -15.0000 - 3.0000i -15.0000 - 7.0000i -15.0000 - 5.0000i -15.0000 +15.0000i -15.0000 +13.0000i -15.0000 + 9.0000i -15.0000 +11.0000i -15.0000 + 1.0000i -15.0000 + 3.0000i -15.0000 + 7.0000i -15.0000 + 5.0000i -13.0000 -15.0000i -13.0000 -13.0000i -13.0000 - 9.0000i -13.0000 -11.0000i -13.0000 - 1.0000i -13.0000 - 3.0000i -13.0000 - 7.0000i -13.0000 - 5.0000i -13.0000 +15.0000i -13.0000 +13.0000i -13.0000 + 9.0000i -13.0000 +11.0000i -13.0000 + 1.0000i -13.0000 + 3.0000i -13.0000 + 7.0000i -13.0000 + 5.0000i -9.0000 -15.0000i -9.0000 -13.0000i -9.0000 - 9.0000i -9.0000 -11.0000i -9.0000 - 1.0000i -9.0000 - 3.0000i -9.0000 - 7.0000i -9.0000 - 5.0000i -9.0000 +15.0000i -9.0000 +13.0000i -9.0000 + 9.0000i -9.0000 +11.0000i -9.0000 + 1.0000i -9.0000 + 3.0000i -9.0000 + 7.0000i -9.0000 + 5.0000i -11.0000 -15.0000i -11.0000 -13.0000i -11.0000 - 9.0000i -11.0000 -11.0000i -11.0000 - 1.0000i -11.0000 - 3.0000i -11.0000 - 7.0000i -11.0000 - 5.0000i -11.0000 +15.0000i -11.0000 +13.0000i -11.0000 + 9.0000i -11.0000 +11.0000i -11.0000 + 1.0000i -11.0000 + 3.0000i -11.0000 + 7.0000i -11.0000 + 5.0000i -1.0000 -15.0000i -1.0000 -13.0000i -1.0000 - 9.0000i -1.0000 -11.0000i -1.0000 - 1.0000i -1.0000 - 3.0000i -1.0000 - 7.0000i -1.0000 - 5.0000i -1.0000 +15.0000i -1.0000 +13.0000i -1.0000 + 9.0000i -1.0000 +11.0000i -1.0000 + 1.0000i -1.0000 + 3.0000i -1.0000 + 7.0000i -1.0000 + 5.0000i -3.0000 -15.0000i -3.0000 -13.0000i -3.0000 - 9.0000i -3.0000 -11.0000i -3.0000 - 1.0000i -3.0000 - 3.0000i -3.0000 - 7.0000i -3.0000 - 5.0000i -3.0000 +15.0000i -3.0000 +13.0000i -3.0000 + 9.0000i -3.0000 +11.0000i -3.0000 + 1.0000i -3.0000 + 3.0000i -3.0000 + 7.0000i -3.0000 + 5.0000i -7.0000 -15.0000i -7.0000 -13.0000i -7.0000 - 9.0000i -7.0000 -11.0000i -7.0000 - 1.0000i -7.0000 - 3.0000i -7.0000 - 7.0000i -7.0000 - 5.0000i -7.0000 +15.0000i -7.0000 +13.0000i -7.0000 + 9.0000i -7.0000 +11.0000i -7.0000 + 1.0000i -7.0000 + 3.0000i -7.0000 + 7.0000i -7.0000 + 5.0000i -5.0000 -15.0000i -5.0000 -13.0000i -5.0000 - 9.0000i -5.0000 -11.0000i -5.0000 - 1.0000i -5.0000 - 3.0000i -5.0000 - 7.0000i -5.0000 - 5.0000i -5.0000 +15.0000i -5.0000 +13.0000i -5.0000 + 9.0000i -5.0000 +11.0000i -5.0000 + 1.0000i -5.0000 + 3.0000i -5.0000 + 7.0000i -5.0000 + 5.0000i 15.0000 -15.0000i 15.0000 -13.0000i 15.0000 - 9.0000i 15.0000 -11.0000i 15.0000 - 1.0000i 15.0000 - 3.0000i 15.0000 - 7.0000i 15.0000 - 5.0000i 15.0000 +15.0000i 15.0000 +13.0000i 15.0000 + 9.0000i 15.0000 +11.0000i 15.0000 + 1.0000i 15.0000 + 3.0000i 15.0000 + 7.0000i 15.0000 + 5.0000i 13.0000 -15.0000i 13.0000 -13.0000i 13.0000 - 9.0000i 13.0000 -11.0000i 13.0000 - 1.0000i 13.0000 - 3.0000i 13.0000 - 7.0000i 13.0000 - 5.0000i 13.0000 +15.0000i 13.0000 +13.0000i 13.0000 + 9.0000i 13.0000 +11.0000i 13.0000 + 1.0000i 13.0000 + 3.0000i 13.0000 + 7.0000i 13.0000 + 5.0000i 9.0000 -15.0000i 9.0000 -13.0000i 9.0000 - 9.0000i 9.0000 -11.0000i 9.0000 - 1.0000i 9.0000 - 3.0000i 9.0000 - 7.0000i 9.0000 - 5.0000i 9.0000 +15.0000i 9.0000 +13.0000i 9.0000 + 9.0000i 9.0000 +11.0000i 9.0000 + 1.0000i 9.0000 + 3.0000i 9.0000 + 7.0000i 9.0000 + 5.0000i 11.0000 -15.0000i 11.0000 -13.0000i 11.0000 - 9.0000i 11.0000 -11.0000i 11.0000 - 1.0000i 11.0000 - 3.0000i 11.0000 - 7.0000i 11.0000 - 5.0000i 11.0000 +15.0000i 11.0000 +13.0000i 11.0000 + 9.0000i 11.0000 +11.0000i 11.0000 + 1.0000i 11.0000 + 3.0000i 11.0000 + 7.0000i 11.0000 + 5.0000i 1.0000 -15.0000i 1.0000 -13.0000i 1.0000 - 9.0000i 1.0000 -11.0000i 1.0000 - 1.0000i 1.0000 - 3.0000i 1.0000 - 7.0000i 1.0000 - 5.0000i 1.0000 +15.0000i 1.0000 +13.0000i 1.0000 + 9.0000i 1.0000 +11.0000i 1.0000 + 1.0000i 1.0000 + 3.0000i 1.0000 + 7.0000i 1.0000 + 5.0000i 3.0000 -15.0000i 3.0000 -13.0000i 3.0000 - 9.0000i 3.0000 -11.0000i 3.0000 - 1.0000i 3.0000 - 3.0000i 3.0000 - 7.0000i 3.0000 - 5.0000i 3.0000 +15.0000i 3.0000 +13.0000i 3.0000 + 9.0000i 3.0000 +11.0000i 3.0000 + 1.0000i 3.0000 + 3.0000i 3.0000 + 7.0000i 3.0000 + 5.0000i 7.0000 -15.0000i 7.0000 -13.0000i 7.0000 - 9.0000i 7.0000 -11.0000i 7.0000 - 1.0000i 7.0000 - 3.0000i 7.0000 - 7.0000i 7.0000 - 5.0000i 7.0000 +15.0000i 7.0000 +13.0000i 7.0000 + 9.0000i 7.0000 +11.0000i 7.0000 + 1.0000i 7.0000 + 3.0000i 7.0000 + 7.0000i 7.0000 + 5.0000i 5.0000 -15.0000i 5.0000 -13.0000i 5.0000 - 9.0000i 5.0000 -11.0000i 5.0000 - 1.0000i 5.0000 - 3.0000i 5.0000 - 7.0000i 5.0000 - 5.0000i 5.0000 +15.0000i 5.0000 +13.0000i 5.0000 + 9.0000i 5.0000 +11.0000i 5.0000 + 1.0000i 5.0000 + 3.0000i 5.0000 + 7.0000i 5.0000 + 5.0000i]); */
    }

    /* EqualizedData = zeros(buf_size, 1); */
    if (LineEqualizerStr.isInitialized != 1) {
        LineEqualizerStr.isInitialized = 1;
        memset(&LineEqualizerStr.Weights[0], 0, 13U * sizeof(creal_T));
        memset(&LineEqualizerStr.TapDelayLine[0], 0, 13U * sizeof(creal_T));
        LineEqualizerStr.TrainingDelayBuffer.Length = 3.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[0].re = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[0].im = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[1].re = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[1].im = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[2].re = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[2].im = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Pointer = 1.0;
        for (i = 0; i < 169; i++) {
            LineEqualizerStr.InverseCorrelationMatrix[i].re = dv[i];
            LineEqualizerStr.InverseCorrelationMatrix[i].im = 0.0;
        }

        LineEqualizerStr.ActiveDelay = 3.0;
        memset(&LineEqualizerStr.TapDelayLine[0], 0, 13U * sizeof(creal_T));
        LineEqualizerStr.SymbolCounter = 0.0;
        LineEqualizerStr.NumTrainingSymbols = 0.0;
        LineEqualizerStr.WeightUpdateCounter = 0.0;
        LineEqualizerStr.PreviousTrainingFlag = false;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[0].re = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[0].im = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[1].re = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[1].im = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[2].re = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Buffer[2].im = 0.0;
        LineEqualizerStr.TrainingDelayBuffer.Pointer = 1.0;
        memset(&LineEqualizerStr.Weights[0], 0, 13U * sizeof(creal_T));
        for (i = 0; i < 169; i++) {
            LineEqualizerStr.InverseCorrelationMatrix[i].re = dv[i];
            LineEqualizerStr.InverseCorrelationMatrix[i].im = 0.0;
        }

        LineEqualizerStr.ReferenceSymbol.re = 2.2204460492503131E-16;
        LineEqualizerStr.ReferenceSymbol.im = 0.0;
    }

    //memcpy(&LineEqualizerStr.Weights[0], ref_channel_resp, 13U * sizeof(creal_T));

    symbolCounter = LineEqualizerStr.SymbolCounter;
    weightUpdateCounter = LineEqualizerStr.WeightUpdateCounter;
    numTrainSymbols = LineEqualizerStr.NumTrainingSymbols;
    if (trainingFlag) {
        if (!LineEqualizerStr.PreviousTrainingFlag) {
            numTrainSymbols = LineEqualizerStr.ActiveDelay + (float)buf_size;
            symbolCounter = 0.0;
        }
        else {
            numTrainSymbols = LineEqualizerStr.NumTrainingSymbols + (float)buf_size;
        }
    }

    LineEqualizerStr.PreviousTrainingFlag = trainingFlag;
    ref_re = LineEqualizerStr.ReferenceSymbol.re;
    ref_im = LineEqualizerStr.ReferenceSymbol.im;
    memcpy(&channel_resp[0], &LineEqualizerStr.Weights[0], 13U * sizeof(creal_T));
    r = LineEqualizerStr.TrainingDelayBuffer;
    lambdaInverse = 1.0 / LineEqualizerStr.ForgettingFactor;
    for (p = 0; p < (int)buf_size; p++) {
        for (idx = 0; idx < 12; idx++) {
            LineEqualizerStr.TapDelayLine[12 - idx] = LineEqualizerStr.TapDelayLine[11
                - idx];
        }

        LineEqualizerStr.TapDelayLine[0] = input_buf[p];
        symbolCounter++;
        weightUpdateCounter++;
        channel_resp_re = 0.0;
        channel_resp_im = 0.0;
        for (i = 0; i < 13; i++) {
            dRef_im = channel_resp[i].re;
            b_channel_resp_im = -channel_resp[i].im;
            d = LineEqualizerStr.TapDelayLine[i].re;
            d1 = LineEqualizerStr.TapDelayLine[i].im;
            channel_resp_re += dRef_im * d - b_channel_resp_im * d1;
            channel_resp_im += dRef_im * d1 + b_channel_resp_im * d;
        }

        if (symbolCounter <= numTrainSymbols) {
            if (LineEqualizerStr.TrainingDelayBuffer.Length > 0.0) {
                ref_re = r.Buffer[(int)r.Pointer - 1].re;
                ref_im = r.Buffer[(int)r.Pointer - 1].im;
                r.Buffer[(int)r.Pointer - 1] = training_buf[p];
                r.Pointer++;
                if (r.Pointer > LineEqualizerStr.TrainingDelayBuffer.Length) {
                    r.Pointer = 1.0;
                }
            }
            else {
                ref_re = training_buf[p].re;
                ref_im = training_buf[p].im;
            }

            if (symbolCounter <= LineEqualizerStr.ActiveDelay) {
                for (idx = 0; idx < 256; idx++) {
                    varargin_1[idx] = rt_hypotd_snf((double)sigConst[idx].re -
                        channel_resp_re, (double)sigConst[idx].im - channel_resp_im);
                }

                coder::internal::minimum_for_equalizer(varargin_1, &dRef_im, &idx);
                ref_re = icv[idx - 1].re;
                ref_im = icv[idx - 1].im;
            }
        }
        else {
            for (idx = 0; idx < 256; idx++) {
                varargin_1[idx] = rt_hypotd_snf((double)sigConst[idx].re -
                    channel_resp_re, (double)sigConst[idx].im - channel_resp_im);
            }

            coder::internal::minimum_for_equalizer(varargin_1, &dRef_im, &idx);
            ref_re = icv[idx - 1].re;
            ref_im = icv[idx - 1].im;
        }

        if (trainingFlag){// || (weightUpdateCounter == 1.0)) {
            weightUpdateCounter = 0.0;
            UP_re = 0.0;
            UP_im = 0.0;
            for (i = 0; i < 13; i++) {
                re = 0.0;
                im = 0.0;
                for (i1 = 0; i1 < 13; i1++) {
                    dRef_im = LineEqualizerStr.TapDelayLine[i1].re;
                    b_channel_resp_im = -LineEqualizerStr.TapDelayLine[i1].im;
                    idx = i1 + 13 * i;
                    re += dRef_im * LineEqualizerStr.InverseCorrelationMatrix[idx].re -
                        b_channel_resp_im * LineEqualizerStr.InverseCorrelationMatrix[idx].
                        im;
                    im += dRef_im * LineEqualizerStr.InverseCorrelationMatrix[idx].im +
                        b_channel_resp_im * LineEqualizerStr.InverseCorrelationMatrix[idx].
                        re;
                }

                UP[i].re = re;
                UP[i].im = im;
                d = LineEqualizerStr.TapDelayLine[i].re;
                d1 = LineEqualizerStr.TapDelayLine[i].im;
                UP_re += re * d - im * d1;
                UP_im += re * d1 + im * d;
            }

            dRef_re = LineEqualizerStr.ForgettingFactor + UP_re;
            for (i = 0; i < 13; i++) {
                re = 0.0;
                im = 0.0;
                for (i1 = 0; i1 < 13; i1++) {
                    idx = i + 13 * i1;
                    re += LineEqualizerStr.InverseCorrelationMatrix[idx].re *
                        LineEqualizerStr.TapDelayLine[i1].re -
                        LineEqualizerStr.InverseCorrelationMatrix[idx].im *
                        LineEqualizerStr.TapDelayLine[i1].im;
                    im += LineEqualizerStr.InverseCorrelationMatrix[idx].re *
                        LineEqualizerStr.TapDelayLine[i1].im +
                        LineEqualizerStr.InverseCorrelationMatrix[idx].im *
                        LineEqualizerStr.TapDelayLine[i1].re;
                }

                if (UP_im == 0.0) {
                    if (im == 0.0) {
                        K[i].re = re / dRef_re;
                        K[i].im = 0.0;
                    }
                    else if (re == 0.0) {
                        K[i].re = 0.0;
                        K[i].im = im / dRef_re;
                    }
                    else {
                        K[i].re = re / dRef_re;
                        K[i].im = im / dRef_re;
                    }
                }
                else if (dRef_re == 0.0) {
                    if (re == 0.0) {
                        K[i].re = im / UP_im;
                        K[i].im = 0.0;
                    }
                    else if (im == 0.0) {
                        K[i].re = 0.0;
                        K[i].im = -(re / UP_im);
                    }
                    else {
                        K[i].re = im / UP_im;
                        K[i].im = -(re / UP_im);
                    }
                }
                else {
                    UP_re = fabs(dRef_re);
                    b_channel_resp_im = fabs(UP_im);
                    if (UP_re > b_channel_resp_im) {
                        b_channel_resp_im = UP_im / dRef_re;
                        dRef_im = dRef_re + b_channel_resp_im * UP_im;
                        K[i].re = (re + b_channel_resp_im * im) / dRef_im;
                        K[i].im = (im - b_channel_resp_im * re) / dRef_im;
                    }
                    else if (b_channel_resp_im == UP_re) {
                        if (dRef_re > 0.0) {
                            b_channel_resp_im = 0.5;
                        }
                        else {
                            b_channel_resp_im = -0.5;
                        }

                        if (UP_im > 0.0) {
                            dRef_im = 0.5;
                        }
                        else {
                            dRef_im = -0.5;
                        }

                        K[i].re = (re * b_channel_resp_im + im * dRef_im) / UP_re;
                        K[i].im = (im * b_channel_resp_im - re * dRef_im) / UP_re;
                    }
                    else {
                        b_channel_resp_im = dRef_re / UP_im;
                        dRef_im = UP_im + b_channel_resp_im * dRef_re;
                        K[i].re = (b_channel_resp_im * re + im) / dRef_im;
                        K[i].im = (b_channel_resp_im * im - re) / dRef_im;
                    }
                }
            }

            dRef_re = ref_re - channel_resp_re;
            dRef_im = -(ref_im - channel_resp_im);
            for (i = 0; i < 13; i++) {
                d = K[i].re;
                d1 = K[i].im;
                channel_resp[i].re += d * dRef_re - d1 * dRef_im;
                channel_resp[i].im += d * dRef_im + d1 * dRef_re;
                d = UP[i].re;
                d1 = UP[i].im;
                for (i1 = 0; i1 < 13; i1++) {
                    idx = i1 + 13 * i;
                    b_channel_resp_im = K[i1].re;
                    UP_re = K[i1].im;
                    dcv[idx].re = LineEqualizerStr.InverseCorrelationMatrix[idx].re -
                        (b_channel_resp_im * d - UP_re * d1);
                    dcv[idx].im = LineEqualizerStr.InverseCorrelationMatrix[idx].im -
                        (b_channel_resp_im * d1 + UP_re * d);
                }
            }

            for (i = 0; i < 169; i++) {
                LineEqualizerStr.InverseCorrelationMatrix[i].re = lambdaInverse * dcv[i]
                    .re;
                    LineEqualizerStr.InverseCorrelationMatrix[i].im = lambdaInverse * dcv[i]
                        .im;
            }
        }

        EqualizedData[p].re = channel_resp_re;
        EqualizedData[p].im = channel_resp_im;
    }
//    for(int i = 0; i < 3; i++)
//    {
//        EqualizedData[buf_size + i].re = r.Buffer[2-i].re;
//        EqualizedData[buf_size + i].im = r.Buffer[2-i].im;
//    }
    LineEqualizerStr.TrainingDelayBuffer = r;
    memcpy(&LineEqualizerStr.Weights[0], &channel_resp[0], 13U * sizeof(creal_T));
    LineEqualizerStr.ReferenceSymbol.re = ref_re;
    LineEqualizerStr.ReferenceSymbol.im = ref_im;
    LineEqualizerStr.SymbolCounter = symbolCounter;
    LineEqualizerStr.WeightUpdateCounter = weightUpdateCounter;
    LineEqualizerStr.NumTrainingSymbols = numTrainSymbols;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void HS_EWL_LineEqualizer_init(void)
{
    LineEqualizerStr_not_empty = false;
}

void LineEqualizer_crc_cnt_reset()
{
    LineEqualizer_crc_cnt = 0;
}

boolean_T LineEqualizer_is_error_sequence_of_CRC()
{
    LineEqualizer_crc_cnt++;
    if(LineEqualizer_crc_cnt < ERROR_CRC_THRESHOLD)
        return false;
    else
        return true;
}

boolean_T LineEqualizer_qam_diagram_distance_check(creal_T* buf1, creal_T* buf2, creal_T* ref_buf)
{
    double distance[2] = {0};
    int16_T distance_cnt[2] = {0};
    for(int i = 0; i < 256; i++)
    {
        distance[0] = sqrt(pow(ref_buf[i].re - buf1[i].re,2) + pow(ref_buf[i].im - buf1[i].im,2));
        distance[1] = sqrt(pow(ref_buf[i].re - buf2[i].re,2) + pow(ref_buf[i].im - buf2[i].im,2));

        if(distance[0] > distance[1])
            distance_cnt[0]++;
        else
            distance_cnt[1]++;
    }

    if(distance_cnt[0] - 50 > distance_cnt[1])
        return 0; // don't update impulse responce
    else
        return 1; // update impulse responce
}

/*
 * File trailer for HS_EWL_LineEqualizer.c
 *
 * [EOF]
 */
