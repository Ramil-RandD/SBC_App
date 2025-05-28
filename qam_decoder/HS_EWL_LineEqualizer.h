/*
 * File: HS_EWL_LineEqualizer.h
 *
 * MATLAB Coder version            : 5.1
 * C/C++ source code generated on  : 06-Mar-2025 08:43:50
 */

#ifndef HS_EWL_LINEEQUALIZER_H
#define HS_EWL_LINEEQUALIZER_H

/* Include Files */
#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>
#ifdef __cplusplus

extern "C" {

#endif

  /* Function Declarations */
  extern void HS_EWL_LineEqualizer(const creal_T *input_buf, uint32_T buf_size, const creal_T
    *training_buf, boolean_T trainingFlag, creal_T *EqualizedData,
    creal_T channel_resp[13]);
  void HS_EWL_LineEqualizer_init(void);

#ifdef __cplusplus

}
#endif
#endif

/*
 * File trailer for HS_EWL_LineEqualizer.h
 *
 * [EOF]
 */
