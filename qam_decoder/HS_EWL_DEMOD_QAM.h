//
// File: HS_EWL_DEMOD_QAM.h
//
// MATLAB Coder version            : 5.1
// C/C++ source code generated on  : 20-Jul-2022 11:04:49
//
#ifndef HS_EWL_DEMOD_QAM_H
#define HS_EWL_DEMOD_QAM_H

// Include Files
#include "rtwtypes.h"
#include "omp.h"
#include <cstddef>
#include <cstdlib>
#include "stdint.h"
#include "qam_init.h"
//#include <qfile.h>
//#include <QTextStream>

// Function Declarations
extern int HS_EWL_DEMOD_QAM(const double *data, double len_data, double
  f_est, double Fs,qam *qam_str, double *qam_symbols_real, double *qam_symbols_imag,
  double *byte_data, double *start_inf_data);
void HS_EWL_DEMOD_QAM_free();
void HS_EWL_DEMOD_QAM_init();

uint8_t* qam_256_demodulator(creal_T* filt_data, uint16_T len, double re_norm_coef, double im_norm_coef);

uint8_t* qam_64_demodulator(creal_T* filt_data, uint16_t len, double re_norm_coef, double im_norm_coef);
void qam64_sym_to_bin(const uint8_t *input_bytes, uint8_t *output_bits, uint32_t size_bytes);

uint8_t* qam4_qpsk_demodulator(creal_T* filt_data, uint16_t len, double re_norm_coef, double im_norm_coef);
void qam4_qpsk_sym_to_bin(const uint8_t *input_bytes, uint8_t *output_bits, uint32_t size_bytes);

bool bin_to_byte(uint8_t *input_bits, uint8_t *output_dec, uint32_t size_bits);

#endif

//
// File trailer for HS_EWL_DEMOD_QAM.h
//
// [EOF]
//
