//
// File: minOrMax.h
//
// MATLAB Coder version            : 5.1
// C/C++ source code generated on  : 20-Jul-2022 11:04:49
//
#ifndef MINORMAX_H
#define MINORMAX_H

// Include Files
#include "rtwtypes.h"
#include "omp.h"
#include <cstddef>
#include <cstdlib>

// Function Declarations
namespace coder
{
  namespace internal
  {
    double maximum(const double x[57820]);
    void maximum(const double x[58820], double *ex, int *idx);
    void minimum(const double x[50], double *ex, int *idx);

    void minimum_for_equalizer(const double x[256], double *ex, int *idx);
  }
}

#endif

//
// File trailer for minOrMax.h
//
// [EOF]
//
