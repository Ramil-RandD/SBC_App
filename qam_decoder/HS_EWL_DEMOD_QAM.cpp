//
// File: HS_EWL_DEMOD_QAM.cpp
//
// MATLAB Coder version            : 5.1
// C/C++ source code generated on  : 20-Jul-2022 11:04:49
//

// Include Files
#include "HS_EWL_DEMOD_QAM.h"
#include "FIRDecimator.h"
#include "HS_EWL_DEMOD_QAM_data.h"
#include "HS_EWL_DEMOD_QAM_initialize.h"
#include "HS_EWL_DEMOD_QAM_rtwutil.h"
#include "RaisedCosineReceiveFilter.h"
#include "qammod.h"
#include "rat.h"
#include "rt_nonfinite.h"
#include "rt_nonfinite.h"
#include <cmath>
#include <cstring>


//static void fir_filter(creal_T* input_buf, uint32_t buf_size, creal_T* filter_coeff, creal_T* output_buf);
// Variable Definitions
static coder::comm::RaisedCosineReceiveFilter rxFilter1;
static boolean_T rxFilter1_not_empty;
static uint8_t demod_qam_data[525] = {0};
static uint8_t data_byte[222] = {0};
const double qam_256_table_real[256] = { -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };
const double qam_256_table_imag[256] = { 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15 };
const double qam_64_table_real[64] = {-7,-7,-7,-7,-7,-7,-7,-7,-5,-5,-5,-5,-5,-5,-5,-5,-3,-3,-3,-3,-3,-3,-3,-3,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7};
const double qam_64_table_imag[64] = {7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7, 7, 5, 3, 1,-1,-3,-5,-7};
const double qpsk_table_real[4] = {-1, -1, 1, 1};
const double qpsk_table_imag[4] = {1, -1, 1, -1};

const uint8_t table_gray_decode[256] = { 0, 1, 3, 2, 7, 6, 4, 5, 15, 14, 12, 13, 8, 9, 11, 10, 16, 17, 19, 18, 23, 22, 20, 21, 31, 30, 28, 29, 24, 25, 27, 26, 48, 49, 51, 50, 55, 54, 52, 53, 63, 62, 60, 61, 56, 57, 59, 58, 32, 33, 35, 34, 39, 38, 36, 37, 47, 46, 44, 45, 40, 41, 43, 42, 112, 113, 115, 114, 119, 118, 116, 117, 127, 126, 124, 125, 120, 121, 123, 122, 96, 97, 99, 98, 103, 102, 100, 101, 111, 110, 108, 109, 104, 105, 107, 106, 64, 65, 67, 66, 71, 70, 68, 69, 79, 78, 76, 77, 72, 73, 75, 74, 80, 81, 83, 82, 87, 86, 84, 85, 95, 94, 92, 93, 88, 89, 91, 90, 240, 241, 243, 242, 247, 246, 244, 245, 255, 254, 252, 253, 248, 249, 251, 250, 224, 225, 227, 226, 231, 230, 228, 229, 239, 238, 236, 237, 232, 233, 235, 234, 192, 193, 195, 194, 199, 198, 196, 197, 207, 206, 204, 205, 200, 201, 203, 202, 208, 209, 211, 210, 215, 214, 212, 213, 223, 222, 220, 221, 216, 217, 219, 218, 128, 129, 131, 130, 135, 134, 132, 133, 143, 142, 140, 141, 136, 137, 139, 138, 144, 145, 147, 146, 151, 150, 148, 149, 159, 158, 156, 157, 152, 153, 155, 154, 176, 177, 179, 178, 183, 182, 180, 181, 191, 190, 188, 189, 184, 185, 187, 186, 160, 161, 163, 162, 167, 166, 164, 165, 175, 174, 172, 173, 168, 169, 171, 170 };
const uint8_t table_gray_decode_qam64[64] = {0, 1, 3, 2, 7, 6, 4, 5, 8, 9,11,10,15,14,12,13,24,25,27,26,31,30,28,29,16,17,19,18,23,22,20,21,56,57,59,58,63,62,60,61,48,49,51,50,55,54,52,53,32,33,35,34,39,38,36,37,40,41,43,42,47,46,44,45};
const uint8_t table_gray_decode_qpsk[4] = {0, 1, 3, 2};

// Function Definitions
//
// Arguments    : const double data[14040]
//                double len_data
//                double f_est
//                double Fs
//                double qam_symbols_real[275]
//                double qam_symbols_imag[275]
//                double byte_data[212]
//                double *start_inf_data
// Return Type  : void
//
int HS_EWL_DEMOD_QAM(const double *data, double len_data, double f_est,//18460-for qam64 14040-for qam256
                      double Fs, qam *qam_str, creal_T *qam_symbols, double *byte_data, double//212-for qam256 222-for qam64
                      *start_inf_data, creal_T *qam_sym_ref, creal_T *channel_resp)
{

    //return 1 input data LEN <= 0
    //return 2 incorrect input sample freq for lagrange_resamp func
    //return 0 OK
  static const double y[105] = { 1.0, 1.1132456, 1.2102575, 1.2896211, 1.3501792,
    1.3910486, 1.4116334, 1.4116334, 1.3910486, 1.3501792, 1.2896211, 1.2102575,
    1.1132456, 1.0, 0.87217220000000006, 0.7316261, 0.5804113, 0.4207328,
    0.2549192, 0.085388000000000019, -0.085388000000000019, -0.2549192,
    -0.4207328, -0.5804113, -0.7316261, -0.87217220000000006,
    -0.99999999999999989, -1.1132456, -1.2102575, -1.2896211, -1.3501792,
    -1.3910486, -1.4116334, -1.4116334, -1.3910486, -1.3501792, -1.2896211,
    -1.2102575, -1.1132456, -1.0000000000000002, -0.87217220000000006,
    -0.7316261, -0.5804113, -0.4207328, -0.2549192, -0.085388000000000019,
    0.085388000000000019, 0.2549192, 0.4207328, 0.5804113, 0.7316261,
    0.87217220000000006, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  static const double y_qpsk[105] = {-1.0        , -0.86953222, -0.72588322, -0.57123056, -0.40791864,
       -0.23842308, -0.06531327,  0.10878662,  0.28123741,  0.44942494,
        0.61079963,  0.76291523,  0.90346582,  1.03032078,  1.14155713,
        1.23548863,  1.31069139,  1.3660254 ,  1.40065186,  1.41404587,
        1.40600438,  1.3766493 ,  1.32642562,  1.25609467,  1.16672262,
        1.05966423,  0.93654242,  0.79922358,  0.64978933,  0.49050493,
        0.32378497,  0.15215677, -0.02177798, -0.19538259, -0.3660254 ,
       -0.53111964, -0.68816265, -0.83477382, -0.96873066, -1.08800254,
       -1.1907814 , -1.27550923, -1.34090165, -1.38596736, -1.41002322,
       -1.41270456, -1.39397074, -1.35410575, -1.2937139 , -1.21371066,
       -1.1153088 , -1.0 , 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0 };

  static creal_T b_y1[27300];//18640
  //static double s[18644];
  creal_T z[525];//350//270
  //z[b_i].recreal_T tmp_z[525];
  double resamp_signal[27300]={0};//18640
  double c1[209];//has the same size for qam256 and qam64
  double Q;
  double del_re;
  double x;
  int b_i;
  double mult_real[18000];
  double mult_imag[18000];
//  double intX[265];
//  unsigned char mapping[256];
//  signed char symbolI[256];
//  signed char symbolQ[256];
  uint8_T preamble_end_point[2];
  if (!isInitialized_HS_EWL_DEMOD_QAM) {
    HS_EWL_DEMOD_QAM_initialize();
  }

  std::memset(&qam_symbols[0].re, 0, 525U * sizeof(double));
  std::memset(&qam_symbols[0].im, 0, 525U * sizeof(double));
  std::memset(&qam_sym_ref[0].re, 0, 525U * sizeof(double));
  std::memset(&qam_sym_ref[0].im, 0, 525U * sizeof(double));
  std::memset(&byte_data[0], 0, 222U * sizeof(double));
  *start_inf_data = 1.0;
  if (!(len_data <= 0.0)) {
    if (!rxFilter1_not_empty) {
      rxFilter1.init();
      rxFilter1_not_empty = true;
    }

    //Fs = 17520*52;
    coder::rat((f_est * 52.0) / Fs, &del_re, &Q);
    if ((!(del_re <= 0.0)) && (!(Q <= 0.0))) {
      creal_T dc;
      double a3;
      double a3_tmp;
      double b_a3_tmp;
      double d;
      double resamp_len;
      int i;
      int idx;
      int ihi;
      int k;
      double *ref_sin;

      if(qam_str->order == 4)
      {
          ref_sin = (double *)&y_qpsk;
      }
      else
      {
          ref_sin = (double *)&y;
      }
      //  Input parameters
      //   s   - input signal vector [N x 1];
      //   p   - p paramter of samplarate conversion
      //   q   - q paramter of samplarate conversion
      //   x0  - fractional delay
      //  Ouptut parameters
      //   y   - Resampled signal
      //std::memset(&resamp_signal[0], 0, 18640U * sizeof(double));
      if (del_re > 1.0) {
        if (Q == 1.0) {
          resamp_len = std::floor(len_data * del_re) + 1.0;

          // resamp_data = zeros(floor(length(s)*p/q)+1,1);
        } else {
          resamp_len = std::floor(len_data * del_re / Q);

          // resamp_data = zeros(floor(length(s)*p/q),1);
        }
      } else {
        resamp_len = std::floor(len_data * del_re / Q);

        // resamp_data = zeros(floor(length(s)*p/q),1);
      }

      //s[0] = 0.0;
      //s[1] = 0.0;
      //std::memcpy(&s[2], &data[0], 18640U * sizeof(double));
      //s[14042] = 0.0;
      //s[14043] = 0.0;
      double sampl_freq_coef = Q / del_re;
      i = static_cast<int>(resamp_len);
      for (k = 0; k < i; k++) {
        x = (static_cast<double>(k) + 1.0) * sampl_freq_coef;
        //a3 = std::floor(x);
        a3 = (int)(x + 32768.) - 32768;
        d = (a3 + 1.0) - x;
        a3_tmp = data[static_cast<int>(a3 + 4.0) - 1];
        x = data[static_cast<int>((a3 + 4.0) - 2.0) - 1];
        b_a3_tmp = data[static_cast<int>((a3 + 4.0) - 1.0) - 1];
        a3 = 0.16666666666666666 * (a3_tmp - data[static_cast<int>((a3 + 4.0) - 3.0)
          - 1]) + 0.5 * (x - b_a3_tmp);
        x = 0.5 * (a3_tmp - x) - a3;
        resamp_signal[k] = ((b_a3_tmp - x * d) + (((a3_tmp - b_a3_tmp) - a3) - x)
                            * d * d) - a3 * d * d * d;
      }

      // cos(2*pi*f_est(1)*time(1:samples_per_period))+sin(2*pi*f_est(1)*time(1:samples_per_period)); 
      std::memset(&c1[0], 0, 209U * sizeof(double));
      for (k = 0; k < 105; k++) {
        ihi = 103 - k;
        x = 0.0;
        for (b_i = 0; b_i <= ihi; b_i++) {
          x += ref_sin[b_i] * resamp_signal[k + b_i];
        }

        c1[k + 104] = x;
      }

      for (k = 0; k < 104; k++) {
        ihi = 103 - k;
        x = 0.0;
        for (b_i = 0; b_i <= ihi; b_i++) {
          x += ref_sin[(k + b_i) + 1] * resamp_signal[b_i];
        }

        c1[103 - k] = x;
      }

      if (!rtIsNaN(c1[104])) {
        idx = 1;
      } else {
        boolean_T exitg1;
        idx = 0;
        k = 2;
        exitg1 = false;
        while ((!exitg1) && (k < 106)) {
          if (!rtIsNaN(c1[k + 103])) {
            idx = k;
            exitg1 = true;
          } else {
            k++;
          }
        }
      }

      if (idx == 0) {
        idx = 1;
      } else {
        x = c1[idx + 103];
        i = idx + 1;
        for (k = i; k < 106; k++) {
          b_a3_tmp = c1[k + 103];
          if (x < b_a3_tmp) {
            x = b_a3_tmp;
            idx = k;
          }
        }
      }

      // coder.varsize('y1', [255*52, 1]);
      std::memset(&b_y1[0], 0, 27300U * sizeof(creal_T));//18640
      ihi = 0;
      i = static_cast<int>((((std::floor(resamp_len / 52.0) - 1.0) * 52.0 +
        static_cast<double>(idx)) - 1.0) + (1.0 - static_cast<double>(idx)));
      for (b_i = 0; b_i < i; b_i++) {
        unsigned int c_i;
        c_i = static_cast<unsigned int>(idx) + b_i;

        // y1(i-Im+1) = resamp_signal(i)*exp(1j*2*pi*f_est*time(i-Im+1))*2;
        x = resamp_signal[static_cast<int>(c_i) - 1];
        k = static_cast<int>(c_i) - idx + 0;
        b_y1[k].re = x * dv[ihi] * 2.0;
        b_y1[k].im = dv1[ihi] * x * 2.0;
        ihi++;
        if (ihi + 1 >= 53) {
          ihi = 0;
        }
      }
      for(int i = 0; i < 18000; i++)
      {
          mult_real[i] = b_y1[i].re;
          mult_imag[i] = b_y1[i].im;
      }
      int sig_len = (qam_str->qam_sym_per_frame + 15)*52;
      int z_len = qam_str->qam_sym_per_frame + 15;
      rxFilter1.step(b_y1,sig_len, z, z_len);
      dc.re = qam_str->pream_qam_sym; //coder::qammod();
      dc.im = qam_str->pream_qam_sym;
      if(qam_str->order == 4)
      {
          dc.re = -1;
          dc.im = 1;
      }
      creal_T del = z[static_cast<int>(round(resamp_len/52))];
      if (del.im == 0.0) {
        if (dc.im == 0.0) {
          del_re = dc.re / del.re;
          a3 = 0.0;
        } else if (dc.re == 0.0) {
          del_re = 0.0;
          a3 = dc.im / del.re;
        } else {
          del_re = dc.re / del.re;
          a3 = dc.im / del.re;
        }
      } else if (del.re == 0.0) {
        if (dc.re == 0.0) {
          del_re = dc.im / del.im;
          a3 = 0.0;
        } else if (dc.im == 0.0) {
          del_re = 0.0;
          a3 = -(dc.re / del.im);
        } else {
          del_re = dc.im / del.im;
          a3 = -(dc.re / del.im);
        }
      } else {
        a3_tmp = std::abs(del.re);
        x = std::abs(del.im);
        if (a3_tmp > x) {
          x = del.im / del.re;
          d = del.re + x * del.im;
          del_re = (dc.re + x * dc.im) / d;
          a3 = (dc.im - x * dc.re) / d;
        } else if (x == a3_tmp) {
          if (del.re > 0.0) {
            x = 0.5;
          } else {
            x = -0.5;
          }

          if (del.im > 0.0) {
            a3 = 0.5;
          } else {
            a3 = -0.5;
          }

          del_re = (dc.re * x + dc.im * a3) / a3_tmp;
          a3 = (dc.im * x - dc.re * a3) / a3_tmp;
        } else {
          x = del.re / del.im;
          d = del.im + x * del.re;
          del_re = (x * dc.re + dc.im) / d;
          a3 = (x * dc.im - dc.re) / d;
        }
      }
      //QString filename = "testData.txt";
      //QFile file("testData.txt");

//          QTextStream stream(&file);
//          stream << "something" << endl;
//      }
      // posible start QAM-256 demodulate

      //memcpy(tmp_z,z,sizeof(creal_T)*525);
      creal_T tmp_var;
      for(int i = 0; i < 525; i++)
      {
          tmp_var.re = z[i].re * del_re - z[i].im * a3;
          tmp_var.im = z[i].re * a3 + z[i].im * del_re;
          z[i].re = tmp_var.re;
          z[i].im = tmp_var.im;
      }
      fir_filter(z, 525, channel_resp, qam_sym_ref);
      uint8_t* pointer_to_inf_byte;
//      if(qam_str->order == 256)
//        pointer_to_inf_byte = qam_256_demodulator(z, qam_str->qam_sym_per_frame+13, del_re, a3);
//      else if(qam_str->order == 64)
//        pointer_to_inf_byte = qam_64_demodulator(z, qam_str->qam_sym_per_frame+13, del_re, a3);
//      else
//          pointer_to_inf_byte = qam4_qpsk_demodulator(z, qam_str->qam_sym_per_frame+13, del_re, a3);
      if(qam_str->order == 256)
          pointer_to_inf_byte = qam_256_demodulator(&qam_sym_ref[3], qam_str->qam_sym_per_frame+16,1,0);// del_re, a3);
      else if(qam_str->order == 64)
          pointer_to_inf_byte = qam_64_demodulator(&qam_sym_ref[3], qam_str->qam_sym_per_frame+16,1,0);// del_re, a3);
      else
          pointer_to_inf_byte = qam4_qpsk_demodulator(&qam_sym_ref[3], qam_str->qam_sym_per_frame+13, 1, 0);


      for(int i = 0; i < (int)qam_str->inf_byte_amount; i++)
      {
          byte_data[i] = *(pointer_to_inf_byte+i);
      }

      if(qam_str->order == 256)
      {
          preamble_end_point[0] = 128;
          preamble_end_point[1] = 255;
      }
      else if(qam_str->order == 64)
      {
          preamble_end_point[0] = 32;
          preamble_end_point[1] = 4;
      }
      else
      {
          preamble_end_point[0] = 0;
          preamble_end_point[1] = 3;
      }

      for (int i = 0; i < 50; i++)
      {
          if (demod_qam_data[i] == preamble_end_point[0] && demod_qam_data[i + 1] == preamble_end_point[0] && demod_qam_data[i + 2] == preamble_end_point[0] && demod_qam_data[i + 3] == preamble_end_point[0] && demod_qam_data[i + 4] == preamble_end_point[0] && (demod_qam_data[i + 5] == preamble_end_point[1]))
          {
              *start_inf_data = i + 6;
              break;
          }
      }
//      if(tmp_z[(int)*start_inf_data + 5].re == 0 && tmp_z[(int)*start_inf_data + 5].im == 0)
//      {
//          tmp_z[5].re = 0;
//      }
      //get qam diagram symbols
      int border_tmp = (int)(((float)qam_str->inf_byte_amount * 8.0)/log2(qam_str->order)) + 3;
      for (b_i = *start_inf_data; b_i < *start_inf_data + border_tmp; b_i++) {
        //b_a3_tmp = tmp_z[b_i].re * del_re - tmp_z[b_i].im * a3;
        //x = tmp_z[b_i].re * a3 + tmp_z[b_i].im * del_re;
        qam_symbols[b_i - (int)*start_inf_data].re = z[b_i].re;//b_a3_tmp * del_re - x * a3;
        qam_symbols[b_i - (int)*start_inf_data].im = z[b_i].im;//b_a3_tmp * a3 + x * del_re;
      }

      if(qam_str->order == 256)
            qam256_modulator(pointer_to_inf_byte, (int)qam_str->inf_byte_amount + 3, qam_sym_ref);
      else if(qam_str-> order == 64)
            qam64_modulator(&demod_qam_data[(int)*start_inf_data], border_tmp, qam_sym_ref);
      else
          qam4_modulator(&demod_qam_data[(int)*start_inf_data], border_tmp, qam_sym_ref);
//      for (k = 0; k < 265; k++) {
//        b_a3_tmp = z[k + 5].re;
//        x = z[k + 5].im;
//        a3_tmp = rt_roundd_snf(((b_a3_tmp * del_re - x * a3) + 15.0) / 2.0);
//        if (a3_tmp < 0.0) {
//          a3_tmp = 0.0;
//        } else {
//          if (a3_tmp > 15.0) {
//            a3_tmp = 15.0;
//          }
//        }

//        b_a3_tmp = rt_roundd_snf(((b_a3_tmp * a3 + x * del_re) + 15.0) / 2.0);
//        if (b_a3_tmp < 0.0) {
//          b_a3_tmp = 0.0;
//        } else {
//          if (b_a3_tmp > 15.0) {
//            b_a3_tmp = 15.0;
//          }
//        }

//        intX[k] = ((16.0 - b_a3_tmp) - 1.0) + 16.0 * a3_tmp;
//      }

//      for (b_i = 0; b_i < 256; b_i++) {
//        mapping[b_i] = 0U;
//        symbolI[b_i] = static_cast<signed char>(b_i >> 4);//for qam64 (b_i >> 3)
//        symbolQ[b_i] = static_cast<signed char>(b_i & 15);//for qam64 (b_i & 7)
//      }

//      for (b_i = 1; b_i < 4; b_i += b_i) {//for qam_64 b_i = 3
//        for (ihi = 0; ihi < 256; ihi++) {
//          k = symbolI[ihi];
//          symbolI[ihi] = static_cast<signed char>(k ^ k >> b_i);
//          k = symbolQ[ihi];
//          symbolQ[ihi] = static_cast<signed char>(k ^ k >> b_i);
//        }
//      }

//      for (b_i = 0; b_i < 256; b_i++) {
//        mapping[(symbolI[b_i] << 4) + symbolQ[b_i]] = static_cast<unsigned char>
//          (b_i);
//      }
//      // end QAM-256 demodulate

//      //      qam_symbols = zeros(273,1,'like',0.0000 + 0.0000i);
//      for (b_i = 0; b_i < 265; b_i++) {
//        b_a3_tmp = z[b_i + 5].re;
//        x = z[b_i + 5].im;
//        qam_symbols_real[b_i] = b_a3_tmp * del_re - x * a3;
//        qam_symbols_imag[b_i] = b_a3_tmp * a3 + x * del_re;
//      }

//      for (b_i = 0; b_i < 50; b_i++) {
//        if ((mapping[static_cast<int>(intX[b_i] + 1.0) - 1] == 128) && (mapping[
//             static_cast<int>(intX[b_i + 1] + 1.0) - 1] == 128) && (mapping[
//             static_cast<int>(intX[b_i + 2] + 1.0) - 1] == 128) && (mapping[
//             static_cast<int>(intX[b_i + 3] + 1.0) - 1] == 255)) {
//          *start_inf_data = (static_cast<double>(b_i) + 1.0) + 3.0;
//        }
//      }

//      for (b_i = 0; b_i < 212; b_i++) {
//        byte_data[b_i] = mapping[static_cast<int>(intX[static_cast<int>
//          (*start_inf_data) + b_i] + 1.0) - 1];
//      }

      // byte_data = trData(start_inf_data+(1:255-43));
      return 0;
    }
    else {
    return 2;
    }
  } else {
    // error = 1;
    return 1;
  }
}

static uint8_t graydecode_fast_qam256(uint8_t gray)
{
    return table_gray_decode[gray];
}

void qam256_modulator(uint8_t* input_buf, uint32_t len, creal_T* output_buf)
{
    uint8_t gray_num;

    for (uint32_t i = 0; i < len; i++)
    {
        gray_num = graydecode_fast_qam256(input_buf[i]);
        output_buf[i].re = qam_256_table_real[gray_num];
        output_buf[i].im = qam_256_table_imag[gray_num];
    }
}

static uint8_t graydecode_fast_qam64(uint8_t gray)
{
    return table_gray_decode_qam64[gray];
}

static uint8_t graydecode_fast_qam4(uint8_t gray)
{
    return table_gray_decode_qpsk[gray];
}

void qam64_modulator(uint8_t* input_buf, uint32_t len, creal_T* output_buf)
{
    uint8_t gray_num;

    for (uint32_t i = 0; i < len; i++)
    {
        gray_num = graydecode_fast_qam64(input_buf[i]);
        output_buf[i].re = qam_64_table_real[gray_num];
        output_buf[i].im = qam_64_table_imag[gray_num];
    }
}

void qam4_modulator(uint8_t* input_buf, uint32_t len, creal_T* output_buf)
{
    uint8_t gray_num;

    for(uint32_t i = 0; i < len; i++)
    {
        gray_num = graydecode_fast_qam4(input_buf[i]);
        output_buf[i].re = qpsk_table_real[gray_num];
        output_buf[i].im = qpsk_table_imag[gray_num];
    }
}

uint8_t* qam_256_demodulator(creal_T* filt_data, uint16_t len, double re_norm_coef, double im_norm_coef) {
    double real_testData = 0;
    double imag_testData = 0;
    uint8_t rIdx = 0; //real ideal points
    uint8_t iIdx = 0; //imag ideal points
    uint8_t start_inf_data = 0;
    uint8_t mapping[256] = { 0 };
    uint8_t symbolI[256] = { 0 };
    uint8_t symbolQ[256] = { 0 };


    for (int i = 0; i < len; i++) {
        real_testData = filt_data[i].re;
        imag_testData = filt_data[i].im;

        rIdx = round(((real_testData * re_norm_coef - imag_testData * im_norm_coef) + 15) / 2);
        if (rIdx < 0.0)
            rIdx = 0.0;
        if (rIdx > 15.0)
            rIdx = 15.0;

        iIdx = round(((real_testData * im_norm_coef + imag_testData * re_norm_coef) + 15) / 2);
        if (iIdx < 0.0)
            iIdx = 0.0;
        if (iIdx > 15.0)
            iIdx = 15.0;

        demod_qam_data[i] = ((16.0 - iIdx) - 1.0) + 16.0 * rIdx;
    }

    for (int i = 0; i < 256; i++) {
        symbolI[i] = i >> 4;
        symbolQ[i] = i & 15;
    }

    for (int i = 1; i < 4; i += i) {
        for (int j = 0; j < 256; j++) {
            symbolI[j] = (symbolI[j] ^ symbolI[j] >> i);
            symbolQ[j] = (symbolQ[j] ^ symbolQ[j] >> i);
        }
    }

    for (int i = 0; i < 256; i++) {
        mapping[(symbolI[i] << 4) + symbolQ[i]] = i;
    }

    for (int i = 0; i < len; i++) {
        demod_qam_data[i] = mapping[demod_qam_data[i]];
    }

    for (int i = 0; i < 50; i++) {
        if (demod_qam_data[i] == 128 && demod_qam_data[i + 1] == 128 && demod_qam_data[i + 2] == 128 && demod_qam_data[i + 3] == 255)
        {
            start_inf_data = i + 4;
        }
    }
    return &demod_qam_data[start_inf_data];
}

uint8_t* qam_64_demodulator(creal_T* filt_data, uint16_t len, double re_norm_coef, double im_norm_coef) {
    double real_testData = 0;
    double imag_testData = 0;
    uint8_t rIdx = 0; //real ideal points
    uint8_t iIdx = 0; //imag ideal points
    uint8_t start_inf_data = 0;
    uint8_t mapping[64] = { 0 };
    uint8_t symbolI[64] = { 0 };
    uint8_t symbolQ[64] = { 0 };
    uint8_t data_bin[300*6] = {0};

    for (int i = 0; i < len; i++) {
        real_testData = filt_data[i].re;
        imag_testData = filt_data[i].im;

        rIdx = round(((real_testData * re_norm_coef - imag_testData * im_norm_coef) + 7) / 2);
        if (rIdx < 0.0)
            rIdx = 0.0;
        if (rIdx > 7.0)
            rIdx = 7.0;

        iIdx = round(((real_testData * im_norm_coef + imag_testData * re_norm_coef) + 7) / 2);
        if (iIdx < 0.0)
            iIdx = 0.0;
        if (iIdx > 7.0)
            iIdx = 7.0;

        demod_qam_data[i] = ((8.0 - iIdx) - 1.0) + 8.0 * rIdx;
    }

    for (int i = 0; i < 64; i++) {
        symbolI[i] = i >> 3;
        symbolQ[i] = i & 7;
    }

    for (int i = 1; i < 3; i += i) {
        for (int j = 0; j < 64; j++) {
            symbolI[j] = (symbolI[j] ^ symbolI[j] >> i);
            symbolQ[j] = (symbolQ[j] ^ symbolQ[j] >> i);
        }
    }

    for (int i = 0; i < 64; i++) {
        mapping[(symbolI[i] << 3) + symbolQ[i]] = i;
    }

    for (int i = 0; i < len; i++) {
        demod_qam_data[i] = mapping[demod_qam_data[i]];
    }

    for (int i = 0; i < 50; i++) {
        if (demod_qam_data[i] == 32 && demod_qam_data[i + 1] == 32 && demod_qam_data[i + 2] == 32 && demod_qam_data[i + 3] == 4)
        {
            start_inf_data = i + 4;
        }
    }

    qam64_sym_to_bin(&demod_qam_data[start_inf_data], data_bin, 300);

    if(bin_to_byte(data_bin, data_byte, 300*6))
        return &data_byte[0];
    else
        return 0;
}

void qam64_sym_to_bin(const uint8_t *input_bytes, uint8_t *output_bits, uint32_t size_bytes)
{
    uint32_t n_bit;

    for(uint32_t i = 0; i < size_bytes; ++i)
    {
        n_bit = i * 6;
        output_bits[n_bit + 5] = input_bytes[i] & 0x1;
        output_bits[n_bit + 4] = (input_bytes[i] >> 1) & 0x1;
        output_bits[n_bit + 3] = (input_bytes[i] >> 2) & 0x1;
        output_bits[n_bit + 2] = (input_bytes[i] >> 3) & 0x1;
        output_bits[n_bit + 1] = (input_bytes[i] >> 4) & 0x1;
        output_bits[n_bit + 0] = (input_bytes[i] >> 5) & 0x1;
    }
}

uint8_t* qam4_qpsk_demodulator(creal_T* filt_data, uint16_t len, double re_norm_coef, double im_norm_coef) {
    double real_testData = 0;
    double imag_testData = 0;
    uint8_t rIdx = 0; //real ideal points
    uint8_t iIdx = 0; //imag ideal points
    uint8_t start_inf_data = 0;
    uint8_t mapping[64] = { 0 };
    uint8_t symbolI[64] = { 0 };
    uint8_t symbolQ[64] = { 0 };
    uint8_t data_bin[200*2] = {0};

    uint8_t M = 4; // qam modulation oreder (qam4-4, qam64-64, qam256-256)
    uint8_t sqrt_M = 2; //sqrt(M)
    uint8_t table_gray_decode_qpsk[4] = {0x0, 0x1, 0x3, 0x2};

    for (int i = 0; i < len; i++)
    {
        real_testData = filt_data[i].re;
        imag_testData = filt_data[i].im;

        rIdx = round(((real_testData * re_norm_coef - imag_testData * im_norm_coef) + (sqrt_M - 1)) / 2);
        if (rIdx < 0.0)
            rIdx = 0.0;
        if (rIdx > (sqrt_M - 1))
            rIdx = sqrt_M - 1;

        iIdx = round(((real_testData * im_norm_coef + imag_testData * re_norm_coef) + (sqrt_M - 1)) / 2);
        if (iIdx < 0.0)
            iIdx = 0.0;
        if (iIdx > (sqrt_M - 1))
            iIdx = sqrt_M - 1;

        demod_qam_data[i] = ((sqrt_M - iIdx) - 1.0) + sqrt_M * rIdx;
    }

    for (int i = 0; i < M; i++)
    {
        symbolI[i] = i >> 1;
        symbolQ[i] = i & (sqrt_M - 1);
    }

    for (int i = 1; i < 1; i += i)
    {
        for (int j = 0; j < M; j++)
        {
            symbolI[j] = (symbolI[j] ^ symbolI[j] >> i);
            symbolQ[j] = (symbolQ[j] ^ symbolQ[j] >> i);
        }
    }

    for (int i = 0; i < M; i++)
    {
        mapping[(symbolI[i] << 1) + symbolQ[i]] = i;
    }

    for (int i = 0; i < len; i++)
    {
        demod_qam_data[i] = table_gray_decode_qpsk[demod_qam_data[i]];
    }

    for (int i = 0; i < 50; i++)
    {
        if (demod_qam_data[i] == 0 && demod_qam_data[i + 1] == 0 && demod_qam_data[i + 2] == 0 && demod_qam_data[i + 3] == 0 && demod_qam_data[i + 4] == 0 && demod_qam_data[i + 5] == 3)
        {
            start_inf_data = i + 6;
            break;
        }
    }

    qam4_qpsk_sym_to_bin(&demod_qam_data[start_inf_data], data_bin, 200);

    if(bin_to_byte(data_bin, data_byte, 200*2))
    {
        return &data_byte[0];
    }
    else
    {
        return 0;
    }
}

void qam4_qpsk_sym_to_bin(const uint8_t *input_bytes, uint8_t *output_bits, uint32_t size_bytes)
{
    uint32_t n_bit;

    for(uint32_t i = 0; i < size_bytes; ++i)
    {
        n_bit = i * 2;
        output_bits[n_bit + 1] = input_bytes[i] & 0x1;
        output_bits[n_bit + 0] = (input_bytes[i] >> 1) & 0x1;
    }
}

bool bin_to_byte(uint8_t *input_bits, uint8_t *output_dec, uint32_t size_bits)
{
    if(size_bits % 8)
        return false;

    uint32_t n_bit;

    for(uint32_t i = 0; i < size_bits/8; ++i)
    {
        n_bit = i * 8;
        output_dec[i] = input_bits[n_bit + 7];
        output_dec[i] |= input_bits[n_bit + 6] << 1;
        output_dec[i] |= input_bits[n_bit + 5] << 2;
        output_dec[i] |= input_bits[n_bit + 4] << 3;
        output_dec[i] |= input_bits[n_bit + 3] << 4;
        output_dec[i] |= input_bits[n_bit + 2] << 5;
        output_dec[i] |= input_bits[n_bit + 1] << 6;
        output_dec[i] |= input_bits[n_bit + 0] << 7;
    }

    return true;
}
void fir_filter(const creal_T* input_buf, uint32_t buf_size, creal_T* filter_coeff, creal_T* output_buf)
{
    creal_T tapDelayLine[FILTER_SIZE];
    double ref_re;
    double ref_im;
    double dRef_re;
    double dRef_im;

    std::memset(&tapDelayLine[0], 0, FILTER_SIZE * sizeof(creal_T));

    for (int i = 0; i < (int)buf_size; i++)
    {
        for (int idx = 0; idx < FILTER_SIZE - 1; idx++)
        {
            tapDelayLine[FILTER_SIZE - 1 - idx] = tapDelayLine[FILTER_SIZE - 2 - idx];
        }
        tapDelayLine[0] = input_buf[i];

        output_buf[i].re = 0.0;
        output_buf[i].im = 0.0;
        for (int idx = 0; idx < FILTER_SIZE; idx++)
        {
            ref_re = filter_coeff[idx].re;
            ref_im = -filter_coeff[idx].im;
            dRef_re = tapDelayLine[idx].re;
            dRef_im = tapDelayLine[idx].im;
            output_buf[i].re += ref_re * dRef_re - ref_im * dRef_im;
            output_buf[i].im += ref_re * dRef_im + ref_im * dRef_re;
        }
        //output_buf[i].im *= -1;
    }
}
double rmsCalculate(creal_T *input_buf, creal_T *ref_buf, uint32_T buf_size)
{
    double rms = 0;
    double err = 0;
    double vec = 0;

    for(uint32_T i = 0; i < buf_size; i++)
    {
        vec += pow(input_buf[i].re,2) + pow(input_buf[i].im,2);
        err += pow(input_buf[i].re - ref_buf[i].re,2) + pow(input_buf[i].im - ref_buf[i].im,2);
    }
    if(err == 0)
        err = 1;

    rms = 10*log10(vec/err);
    return rms;
}
//
// Arguments    : void
// Return Type  : void
//
void HS_EWL_DEMOD_QAM_free()
{
  rxFilter1.matlabCodegenDestructor();
  rxFilter1._pobj0.matlabCodegenDestructor();
}

//
// Arguments    : void
// Return Type  : void
//
void HS_EWL_DEMOD_QAM_init()
{
  rxFilter1_not_empty = false;
  rxFilter1._pobj0.matlabCodegenIsDeleted = true;
  rxFilter1.matlabCodegenIsDeleted = true;
}

//
// File trailer for HS_EWL_DEMOD_QAM.cpp
//
// [EOF]
//
