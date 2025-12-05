//
// File: HS_EWL_TR_FUN_EST.h
//
// MATLAB Coder version            : 5.1
// C/C++ source code generated on  : 20-Jul-2022 11:04:49
//
#ifndef HS_EWL_TR_FUN_EST_H
#define HS_EWL_TR_FUN_EST_H

// Include Files
#include "rtwtypes.h"
#include "omp.h"
#include "qam_init.h"
#include <cstddef>
#include <cstdlib>

#define KERNEL_SIZE 7     // для медианного фильтра (должен быть нечётным)
#define PI 3.14159265358979323846

// Function Declarations
extern void HS_EWL_TR_FUN_EST(const double sweep_data[450000], const double
  sweep_math[57820], double Fs, double f_opt, double f_sine, double PreSPP,
  double gain[2048], double phase[2048], double *shift_for_qam_data, double
  *sweep_warning_status, qam qam_str);

#endif

//
// File trailer for HS_EWL_TR_FUN_EST.h
//
// [EOF]
//
