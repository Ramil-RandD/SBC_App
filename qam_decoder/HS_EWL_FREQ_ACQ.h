//
// File: HS_EWL_FREQ_ACQ.h
//
// MATLAB Coder version            : 5.1
// C/C++ source code generated on  : 20-Jul-2022 11:04:49
//
#ifndef HS_EWL_FREQ_ACQ_H
#define HS_EWL_FREQ_ACQ_H

// Include Files
#include "rtwtypes.h"
#include "omp.h"
#include <cstddef>
#include <cstdlib>
#include "qam_init.h"

typedef struct{
    double bounds[2];
    double pream_from;
    double pream_to;
    double f_opt;
}stage_1_freq_est;

typedef struct{

}stage_2_freq_est;

typedef struct{
    stage_1_freq_est stage_1;
    stage_2_freq_est stage_2;
    double *data;
    double *data_len;

}freq_est;

// Function Declarations
extern int  HS_EWL_FREQ_ACQ(const double *data, double len, double Fs,
  double f_opt, int32_T sps, double mode, int32_T Pl, qam *qam_str, double *s2,
  int32_T *len_data, double *f_est, double *warningStatus);

void        HS_EWL_FREQ_ACQ_free();
void        HS_EWL_FREQ_ACQ_init();
double      absolute_min(double idx, const double FF[50]);
void bounds_find(const double *data, double len_data, int32_T sps, double *bounds);


double      dist_between_2point(creal_T point1, creal_T point2, creal_T norm_coef);
double      find_norm_coeff_for_freq(const double *dist_table, const double *freq_table, double distance);
double      qam4_qpsk_find_norm_coeff_for_freq(const double *dist_table, const double *freq_table, double distance);
int32_T     premable_from(const double *data, int data_len, int win_len);
double      find_preamble_max(const double *data, int32_T pre_start, int32_T sps, int32_T pre_len);
int32_T     cut_out_valid_signal(const double *in_data, double len, double data_max, int32_T pre_start, int32_T pre_end, double *out_data, double *warning);
int32_T     lagrange_reamp(double *in_data, int32_T *len, double *out_data, double freq, double Fs, int32_T sps);
void        pream_mult_ref_exp(double *in_data, int32_T *len, int32_T bound1, int32_T bound2, int str_pre, const double *ref_sin, const double *ref_cos, int32_T sps, creal_T *out_data);
creal_T     complex_division(creal_T dvd, creal_T dvs);
void        smooth(const double* in_buf, double* out_buf, int len, int window)

#endif

//
// File trailer for HS_EWL_FREQ_ACQ.h
//
// [EOF]
//
