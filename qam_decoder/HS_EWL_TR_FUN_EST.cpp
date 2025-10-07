//
// File: HS_EWL_TR_FUN_EST.cpp
//
// MATLAB Coder version            : 5.1
// C/C++ source code generated on  : 20-Jul-2022 11:04:49
//

// Include Files
#include "HS_EWL_TR_FUN_EST.h"
#include "HS_EWL_DEMOD_QAM_data.h"
#include "HS_EWL_DEMOD_QAM_initialize.h"
#include "HS_EWL_DEMOD_QAM_rtwutil.h"
#include "eml_fftshift.h"
#include "fft.h"
#include "fftshift.h"
#include "ifft.h"
#include "ifftshift.h"
#include "interp1.h"
#include "minOrMax.h"
#include "movSumProdOrMean.h"
#include "rat.h"
#include "rt_nonfinite.h"
#include "xcorr.h"
#include "rt_nonfinite.h"
#include "stdio.h"
#include <cmath>
#include <cstring>

// Function Declarations
static void lagrange_resamp(const double s[450000], double p, double q, double
  x0, double resamp_data[57820]);
static double rt_hypotd_snf(double u0, double u1);

// Function Definitions
//
// Input parameters
//   s   - input signal vector [N x 1];
//   p   - p paramter of samplarate conversion
//   q   - q paramter of samplarate conversion
//   x0  - fractional delay
// Arguments    : const double s[450000]
//                double p
//                double q
//                double x0
//                double resamp_data[57820]
// Return Type  : void
//
static void lagrange_resamp(const double s[450000], double p, double q, double
  x0, double resamp_data[57820])
{
  static double b_s[450004];
  double resamp_len;
  int i;

  //  Ouptut parameters
  //   y   - Resampled signal
  //      test_var = zeros(1,1);
  //      test_var = p;
  std::memset(&resamp_data[0], 0, 57820U * sizeof(double));
  if (p > 1.0) {
    if (q == 1.0) {
      resamp_len = std::floor(450000.0 * p) + 1.0;
    } else {
      resamp_len = std::floor(450000.0 * p / q);
    }
  } else {
    resamp_len = std::floor(450000.0 * p / q);
  }

  b_s[0] = 0.0;
  b_s[1] = 0.0;
  std::memcpy(&b_s[2], &s[0], 450000U * sizeof(double));
  b_s[450002] = 0.0;
  b_s[450003] = 0.0;
  i = static_cast<int>(resamp_len);
  for (int k = 0; k < i; k++) {
    double a3;
    double a3_tmp;
    double b_a3_tmp;
    double d;
    resamp_len = (static_cast<double>(k) + 1.0) * q / p - x0;
    a3 = std::floor(resamp_len);
    d = (a3 + 1.0) - resamp_len;
    a3_tmp = b_s[static_cast<int>(a3 + 4.0) - 1];
    resamp_len = b_s[static_cast<int>((a3 + 4.0) - 2.0) - 1];
    b_a3_tmp = b_s[static_cast<int>((a3 + 4.0) - 1.0) - 1];
    a3 = 0.16666666666666666 * (a3_tmp - b_s[static_cast<int>((a3 + 4.0) - 3.0)
      - 1]) + 0.5 * (resamp_len - b_a3_tmp);
    resamp_len = 0.5 * (a3_tmp - resamp_len) - a3;
    resamp_data[k] = ((b_a3_tmp - resamp_len * d) + (((a3_tmp - b_a3_tmp) - a3)
      - resamp_len) * d * d) - a3 * d * d * d;
  }
}

//
// Arguments    : double u0
//                double u1
// Return Type  : double
//
static double rt_hypotd_snf(double u0, double u1)
{
  double a;
  double y;
  a = std::abs(u0);
  y = std::abs(u1);
  if (a < y) {
    a /= y;
    y *= std::sqrt(a * a + 1.0);
  } else if (a > y) {
    y /= a;
    y = a * std::sqrt(y * y + 1.0);
  } else {
    if (!rtIsNaN(y)) {
      y = a * 1.4142135623730951;
    }
  }

  return y;
}

//
// sweep_warning_status   = 0; %input array OK
// sweep_warning_status   = 1; %input sine singal freq or sample freq equal 0
// sweep_warning_status   = 2; %not enough len input array len < 80%
// sweep_warning_status   = 3; % len input array > 80% or < 100%
// sweep_warning_status   = 4; %no found sweep preamble(ampMod)
// sweep_warning_status   = 5; %all input array equal 0
// sweep_warning_status   = 6; %phase dosen't work with input sweep
// Arguments    : const double sweep_data[450000]
//                const double sweep_math[57820]
//                double Fs
//                double f_opt
//                double f_sine
//                double PreSPP
//                double gain[2048]
//                double phase[2048]
//                double *shift_for_qam_data
//                double *sweep_warning_status
// Return Type  : void
//
void HS_EWL_TR_FUN_EST(const double sweep_data[450000], const double sweep_math
  [57820], double Fs, double f_opt, double f_sine, double PreSPP, double gain
  [2048], double phase[2048], double *shift_for_qam_data, double
  *sweep_warning_status, qam qam_str)
{
  static creal_T channel2[57820];
  static creal_T spec_sweep[57820];
  static creal_T TrFunEst[56001];
  static creal_T y_tmp[56001];
  static double c[115639];
  static double unusedU0[115639];
  static double found_c_max[58820];
  static double Vq[57820];
  static double b_y[57820];
  static double ft[57820];
  static double resampSweepAfter23[57820];
  static double resamp_sweep[57820];
  static double y[57820];
  static double F_tr[56001];
  static double b_dv[14001];
  static double newGain[14001];
  static double newPhase[14001];
  static double c_y[13581];
  double b_gain_s[2048];//2048
  double b_dv1[4096];//1025
  double abs_freq[4096] = { 0 };
  double gain_s[1025];//1025
  double bi;
  double kd;
  double r;
  int iindx;
  if (!isInitialized_HS_EWL_DEMOD_QAM) {
    HS_EWL_DEMOD_QAM_initialize();
  }

  std::memset(&gain[0], 0, 2048U * sizeof(double));
  std::memset(&phase[0], 0, 2048U * sizeof(double));
  std::memset(&b_gain_s[0], 0, 2048U * sizeof(double));
  *shift_for_qam_data = 0.0;

  // gain_phase_impulse_cable_buf634 = load('C:\Users\ramil.ziyadiev\Desktop\Work\qam transmiter\gain_phase_impulse_cable_buf634a.mat'); 
  if ((f_opt == 0.0) || (Fs == 0.0) || (f_sine == 0.0) || (PreSPP == 0.0)) {
    *sweep_warning_status = 1.0;

    // input sine singal freq or sample freq equal 0
  } else {
    int flag_array_zero;
    int i;
    boolean_T exitg1;
    flag_array_zero = 1;
    i = 0;
    exitg1 = false;
    while ((!exitg1) && (i < 450000)) {
      if ((sweep_data[i] > 0.05) || (sweep_data[i] < -0.05)) {
        flag_array_zero = 0;
        exitg1 = true;
      } else {
        i++;
      }
    }

    if (flag_array_zero == 1) {
      *sweep_warning_status = 5.0;

      // all input array equal 0
    } else {
      // sample frequency math sweep
      // 280373;
      // %%240MHz / 131
      coder::rat(f_opt * 280000.0 / f_sine / Fs, &r, &kd);
      if ((r <= 0.0) || (kd <= 0.0)) {
        *sweep_warning_status = 6.0;

        //  incorrect input sample freq for lagrange_resamp func
      } else {
        double d;
        int b_i;
        int k;
        lagrange_resamp(sweep_data, r, kd, 0.0, resamp_sweep);
        r = coder::internal::maximum(resamp_sweep);
        for (b_i = 0; b_i < 57820; b_i++) {
          resamp_sweep[b_i] /= r;
        }

        *sweep_warning_status = 0.0;

        //  input array OK
        F_tr[0] = -140000.0;
        F_tr[56000] = 139995.00008928409;
        for (k = 0; k < 27999; k++) {
          kd = (static_cast<double>(k) + 1.0) * 4.9999107158800733;
          F_tr[k + 1] = kd + -140000.0;
          F_tr[55999 - k] = 139995.00008928409 - kd;
        }

        F_tr[28000] = -2.4999553579546046;
        r = coder::internal::maximum(resamp_sweep);
        for (b_i = 0; b_i < 57820; b_i++) {
          ft[b_i] = resamp_sweep[b_i] / r;
        }

        coder::xcorr(ft, *(double (*)[1820])&sweep_math[0], c, unusedU0);
        std::memset(&found_c_max[0], 0, 58820U * sizeof(double));
        d = 57820.0 - PreSPP * 2.0;
        b_i = static_cast<int>((1.0 - d) + 115639.0);
        for (i = 0; i < b_i; i++) {
          r = d + static_cast<double>(i);
          found_c_max[static_cast<int>(((r - 57820.0) + PreSPP * 2.0) + 1.0) - 1]
            = c[static_cast<int>(r) - 1];
        }

        coder::internal::maximum(found_c_max, &kd, &iindx);
        if (iindx >= PreSPP * 2.0) {
          *sweep_warning_status = 4.0;

          // no found sweep preamble(ampMod)
        } else {
          double ai;
          double ar;
          double br;
          double brm;
          double re;
          double shift_pream;

          // START SWEEP CODE
          //  resampSweepAfter23 = [zeros(1,PreSPP*2-Im+corr_var),resamp_sweep(1:end-(PreSPP*2-Im+corr_var))']; 
          //      TrFunEst = fftshift(fft(resampSweepAfter23(length(sweepPreamble):end))./fft(sweep_math(length(sweepPreamble):end))); 
          //
          //      newPhase        = angle(TrFunEst(firstFreq:lastFreq));
          //      newGain         = abs(TrFunEst(firstFreq:lastFreq));
          // while(1)%com
          std::memset(&resampSweepAfter23[0], 0, 57820U * sizeof(double));
          b_i = static_cast<int>(57820.0 - ((PreSPP * 2.0 - static_cast<double>
            (iindx)) + 2.0));
          for (i = 0; i < b_i; i++) {
            resampSweepAfter23[static_cast<int>((((static_cast<double>(i) + 1.0)
              + PreSPP * 2.0) - static_cast<double>(iindx)) + 2.0) - 1] =
              resamp_sweep[i];
          }

          // resampSweepAfter23 = [zeros(1,PreSPP*2-Im+corr_var),resamp_sweep(1:end-(PreSPP*2-Im+corr_var))']; 
          coder::fft(*(double (*)[56001])&sweep_math[1819], y_tmp);
          coder::fft(*(double (*)[56001])&resampSweepAfter23[1819], TrFunEst);
          for (b_i = 0; b_i < 56001; b_i++) {
            ar = TrFunEst[b_i].re;
            ai = TrFunEst[b_i].im;
            br = y_tmp[b_i].re;
            bi = y_tmp[b_i].im;
            if (bi == 0.0) {
              if (ai == 0.0) {
                re = ar / br;
                kd = 0.0;
              } else if (ar == 0.0) {
                re = 0.0;
                kd = ai / br;
              } else {
                re = ar / br;
                kd = ai / br;
              }
            } else if (br == 0.0) {
              if (ar == 0.0) {
                re = ai / bi;
                kd = 0.0;
              } else if (ai == 0.0) {
                re = 0.0;
                kd = -(ar / bi);
              } else {
                re = ai / bi;
                kd = -(ar / bi);
              }
            } else {
              brm = std::abs(br);
              kd = std::abs(bi);
              if (brm > kd) {
                r = bi / br;
                kd = br + r * bi;
                re = (ar + r * ai) / kd;
                kd = (ai - r * ar) / kd;
              } else if (kd == brm) {
                if (br > 0.0) {
                  r = 0.5;
                } else {
                  r = -0.5;
                }

                if (bi > 0.0) {
                  kd = 0.5;
                } else {
                  kd = -0.5;
                }

                re = (ar * r + ai * kd) / brm;
                kd = (ai * r - ar * kd) / brm;
              } else {
                r = br / bi;
                kd = bi + r * br;
                re = (r * ar + ai) / kd;
                kd = (r * ai - ar) / kd;
              }
            }

            TrFunEst[b_i].re = re;
            TrFunEst[b_i].im = kd;
          }

          coder::fftshift(TrFunEst);
          for (k = 0; k < 14001; k++) {
            bi = TrFunEst[k + 28001].im;
            r = TrFunEst[k + 28001].re;
            newPhase[k] = rt_atan2d_snf(bi, r);
            newGain[k] = rt_hypotd_snf(r, bi);
          }

          // com
          // com
          // com
          // com
          // com
          // end%com
          //  END SWEEP CODE
          kd = 2.0;
          for (i = 0; i < 14000; i++) {
            bi = newPhase[i + 1];
            r = bi - newPhase[i];
            if (r > std::floor((kd - 1.0) * 2.0 * 3.1415926535897931) - 0.7) {
              if (r > std::floor(kd * 2.0 * 3.1415926535897931) - 0.7) {
                kd++;
              }

              newPhase[i + 1] = bi - (kd - 1.0) * 2.0 * 3.1415926535897931;
            } else {
              newPhase[i + 1] = bi - (kd - 2.0) * 2.0 * 3.1415926535897931;
            }
          }

          for (i = 0; i < 14000; i++) {
            bi = newPhase[i + 1];
            r = bi - newPhase[i];
            if (r < -(std::floor((kd - 1.0) * 2.0 * 3.1415926535897931) - 0.7))
            {
              if (r < -(std::floor(kd * 2.0 * 3.1415926535897931) - 0.7)) {
                kd++;
              }

              newPhase[i + 1] = bi + (kd - 1.0) * 2.0 * 3.1415926535897931;
            } else {
              newPhase[i + 1] = bi + (kd - 2.0) * 2.0 * 3.1415926535897931;
            }
          }

          ft[0] = -140000.0;
          ft[57819] = 139995.15738498786;
          for (k = 0; k < 28908; k++) {
            kd = (static_cast<double>(k) + 1.0) * 4.8426150121065374;
            ft[k + 1] = kd + -140000.0;
            ft[57818 - k] = 139995.15738498786 - kd;
          }

          ft[28909] = -4.8426150121085811;
          ft[28910] = -2.9103830456733704E-11;

          //  plot(gain_sogl_freq(1:end-25),20*log10(gain_sogl(1:end-25))-10, 'lineWidth', 2) 
          //  hold on
          //  plot(gain_phase_impulse_cable_buf634.Gain{4},gain_phase_impulse_cable_buf634.Gain{8}-3.5) 
          //  ylabel('Gain, Db');
          //  xlabel('freq, kHz');
          //  legend('Imit + SRP Imit','Cabel')
          //  grid on

          //---- GAIN for predistortion ----//
          // 
          // change b_dv1 array_size from 513 to 1024
          uint16_t size = 4096;//qam_str.fft_order;
          int32_t Fd = 280000;
          double freq_coef = static_cast<double>(Fd) / static_cast<double>(size);
//          FILE* file;
//          fopen_s(&file,"test_gain.txt", "w");
          b_dv1[0] = (double) (- Fd / 2);
          for (b_i = 1; b_i < size; b_i++) {
              b_dv1[b_i] = b_dv1[b_i - 1] + freq_coef;// *static_cast<double>(b_i);//136.71875
          }
          for (uint16_t i = 0; i < size; i++) {
              abs_freq[i] = std::abs(b_dv1[i]);
          }

          coder::vmovfun(newGain, 14001, b_dv);
          coder::interp1(*(double (*)[14001])&F_tr[28001], b_dv, &b_dv1[size/2], gain_s, size/4+1);

          // 69725
          std::memcpy(&b_gain_s[0], &gain_s[0], (size/4+1) * sizeof(double));
          for (b_i = 0; b_i < size/4-1; b_i++) {
            b_gain_s[b_i + size/4+1] = gain_s[size/4];
          }

          b_gain_s[0] = b_gain_s[2];
          b_gain_s[1] = b_gain_s[2];
          std::memcpy(gain, &b_gain_s[0], (size/2) * sizeof(double));
          //coder::interp1SplineOrPCHIP(b_gain_s, &b_dv1[size / 2], abs_freq, gain, size);

//          for (int i = 0; i < 2048; i++)
//              fprintf(file, "%f,", gain[i]);
//          fclose(file);


          //---- PHASE for predistortion ----//

          for (k = 0; k < 57820; k++) {
            y[k] = std::abs(ft[k]);
          }

          /*for (b_i = 0; b_i < 513; b_i++) {
            b_dv1[b_i] = 136.71875 * static_cast<double>(b_i);
          }*/

          coder::vmovfun(newPhase, 14001, b_dv);
          coder::interp1(*(double (*)[14001])&F_tr[28001], b_dv, &b_dv1[size/2], gain_s, size / 4 + 1);

          // 70000
          std::memcpy(&b_gain_s[0], &gain_s[0], (size/4+1) * sizeof(double));
          for (b_i = 0; b_i < size/4-1; b_i++) {
            b_gain_s[b_i + size/4+1] = gain_s[size/4];
          }

          // 70000
          b_gain_s[0] = b_gain_s[2];
          b_gain_s[1] = b_gain_s[2];
//          FILE* file1;
//          fopen_s(&file1,"test_phase.txt", "w");
          std::memcpy(phase, &b_gain_s[0], (size/2) * sizeof(double));
          //coder::interp1SplineOrPCHIP(b_gain_s, &b_dv1[size / 2], abs_freq, phase, size);
//          for (int i = 0; i < 2048; i++)
//              fprintf(file1,"%f,",phase[i]);
//          fclose(file1);
          // end PHASE for predistortion
          for (k = 0; k < 57820; k++) {
            b_y[k] = std::abs(ft[k]);
            Vq[k] = 0.0;
          }

          coder::interp1SplineOrPCHIP_1(phase, b_y, Vq);
          for (k = 0; k < 57820; k++) {
            kd = Vq[k];
            re = kd * 0.0;
            channel2[k].re = re;
            channel2[k].im = kd;
            if (kd == 0.0) {
              channel2[k].re = std::exp(re);
              channel2[k].im = 0.0;
            } else {
              r = std::exp(re / 2.0);
              channel2[k].re = r * (r * std::cos(kd));
              channel2[k].im = r * (r * std::sin(kd));
            }

            Vq[k] = 0.0;
          }

          coder::interp1SplineOrPCHIP_1(gain, y, Vq);
          for (k = 0; k < 57820; k++) {
            bi = Vq[k];
            channel2[k].re *= bi;
            channel2[k].im *= bi;
            bi = ft[k];
            if (bi < 0.0) {
              r = -1.0;
            } else {
              r = (bi > 0.0);
            }

            ft[k] = r;
          }

          coder::b_fft(resamp_sweep, spec_sweep);
          coder::eml_fftshift(spec_sweep, 1);
          coder::eml_fftshift(spec_sweep, 2);
          for (b_i = 0; b_i < 57820; b_i++) {
            ar = spec_sweep[b_i].re;
            ai = spec_sweep[b_i].im;
            bi = channel2[b_i].im;
            r = ft[b_i];
            br = channel2[b_i].re + bi * 0.0 * r;
            bi *= r;
            if (bi == 0.0) {
              if (ai == 0.0) {
                re = ar / br;
                kd = 0.0;
              } else if (ar == 0.0) {
                re = 0.0;
                kd = ai / br;
              } else {
                re = ar / br;
                kd = ai / br;
              }
            } else if (br == 0.0) {
              if (ar == 0.0) {
                re = ai / bi;
                kd = 0.0;
              } else if (ai == 0.0) {
                re = 0.0;
                kd = -(ar / bi);
              } else {
                re = ai / bi;
                kd = -(ar / bi);
              }
            } else {
              brm = std::abs(br);
              kd = std::abs(bi);
              if (brm > kd) {
                r = bi / br;
                kd = br + r * bi;
                re = (ar + r * ai) / kd;
                kd = (ai - r * ar) / kd;
              } else if (kd == brm) {
                if (br > 0.0) {
                  r = 0.5;
                } else {
                  r = -0.5;
                }

                if (bi > 0.0) {
                  kd = 0.5;
                } else {
                  kd = -0.5;
                }

                re = (ar * r + ai * kd) / brm;
                kd = (ai * r - ar * kd) / brm;
              } else {
                r = br / bi;
                kd = bi + r * br;
                re = (r * ar + ai) / kd;
                kd = (r * ai - ar) / kd;
              }
            }

            spec_sweep[b_i].re = re;
            spec_sweep[b_i].im = kd;
          }

          coder::ifftshift(spec_sweep);
          coder::b_ifft(spec_sweep, channel2);
          for (b_i = 0; b_i < 57820; b_i++) {
            resamp_sweep[b_i] = channel2[b_i].re;
          }

          if (!rtIsNaN(resamp_sweep[0])) {
            flag_array_zero = 1;
          } else {
            flag_array_zero = 0;
            k = 2;
            exitg1 = false;
            while ((!exitg1) && (k < 57821)) {
              if (!rtIsNaN(resamp_sweep[k - 1])) {
                flag_array_zero = k;
                exitg1 = true;
              } else {
                k++;
              }
            }
          }

          if (flag_array_zero == 0) {
            kd = resamp_sweep[0];
          } else {
            kd = resamp_sweep[flag_array_zero - 1];
            b_i = flag_array_zero + 1;
            for (k = b_i; k < 57821; k++) {
              bi = resamp_sweep[k - 1];
              if (kd < bi) {
                kd = bi;
              }
            }
          }

          for (b_i = 0; b_i < 57820; b_i++) {
            ft[b_i] = resamp_sweep[b_i] / kd;
          }

          coder::xcorr(ft, *(double (*)[1820])&sweep_math[0], c, unusedU0);
          std::memset(&found_c_max[0], 0, 58820U * sizeof(double));
          b_i = static_cast<int>((1.0 - d) + 115639.0);
          for (i = 0; i < b_i; i++) {
            r = d + static_cast<double>(i);
            found_c_max[static_cast<int>(((r - 57820.0) + PreSPP * 2.0) + 1.0) -
              1] = c[static_cast<int>(r) - 1];
          }

          coder::internal::maximum(found_c_max, &kd, &iindx);

          // [~,Im] = max(c(length(SignalB)-PreSPP*2:end));
          //  [~,Im] = max(c(find(lags==-PreSPP*2):end));
          shift_pream = 2.0;
          i = 0;
          exitg1 = false;
          while ((!exitg1) && (i < 10)) {
            d = static_cast<double>(iindx) - shift_pream;
            bi = PreSPP * 2.0 - d;
            b_i = static_cast<int>(bi);
            if (0 <= b_i - 1) {
              std::memset(&resampSweepAfter23[0], 0, b_i * sizeof(double));
            }

            b_i = static_cast<int>(57820.0 - bi);
            for (flag_array_zero = 0; flag_array_zero < b_i; flag_array_zero++)
            {
              resampSweepAfter23[static_cast<int>(((static_cast<double>
                (flag_array_zero) + 1.0) + PreSPP * 2.0) - d) - 1] =
                resamp_sweep[flag_array_zero];
            }

            // resampSweepAfter23 = [zeros(1,PreSPP*2-(Im-shift_pream)) SignalB(1:end-(PreSPP*2-(Im-shift_pream)))]; 
            coder::fft(*(double (*)[56001])&resampSweepAfter23[1819], TrFunEst);
            for (b_i = 0; b_i < 56001; b_i++) {
              ar = TrFunEst[b_i].re;
              ai = TrFunEst[b_i].im;
              br = y_tmp[b_i].re;
              bi = y_tmp[b_i].im;
              if (bi == 0.0) {
                if (ai == 0.0) {
                  re = ar / br;
                  kd = 0.0;
                } else if (ar == 0.0) {
                  re = 0.0;
                  kd = ai / br;
                } else {
                  re = ar / br;
                  kd = ai / br;
                }
              } else if (br == 0.0) {
                if (ar == 0.0) {
                  re = ai / bi;
                  kd = 0.0;
                } else if (ai == 0.0) {
                  re = 0.0;
                  kd = -(ar / bi);
                } else {
                  re = ai / bi;
                  kd = -(ar / bi);
                }
              } else {
                brm = std::abs(br);
                kd = std::abs(bi);
                if (brm > kd) {
                  r = bi / br;
                  kd = br + r * bi;
                  re = (ar + r * ai) / kd;
                  kd = (ai - r * ar) / kd;
                } else if (kd == brm) {
                  if (br > 0.0) {
                    r = 0.5;
                  } else {
                    r = -0.5;
                  }

                  if (bi > 0.0) {
                    kd = 0.5;
                  } else {
                    kd = -0.5;
                  }

                  re = (ar * r + ai * kd) / brm;
                  kd = (ai * r - ar * kd) / brm;
                } else {
                  r = br / bi;
                  kd = bi + r * br;
                  re = (r * ar + ai) / kd;
                  kd = (r * ai - ar) / kd;
                }
              }

              TrFunEst[b_i].re = re;
              TrFunEst[b_i].im = kd;
            }

            coder::fftshift(TrFunEst);
            for (k = 0; k < 13581; k++) {
              c_y[k] = rt_atan2d_snf(TrFunEst[k + 28401].im, TrFunEst[k + 28401]
                .re);
            }

            r = c_y[0];
            for (k = 0; k < 13580; k++) {
              r += c_y[k + 1];
            }

            r /= 13581.0;

            // *ones(1,length(TrFunEst(firstFreq:lastFreq)));
            if (r >= 0.08) {
              shift_pream++;
              i++;
            } else if (r <= -0.08) {
              shift_pream--;
              i++;
            } else {
              exitg1 = true;
            }
          }

          double mod_freq_pos;
          mod_freq_pos = std::round(35000/(140000/2048)) + 1;

          *shift_for_qam_data = rt_roundd_snf(phase[static_cast<int>(mod_freq_pos)] / 6.2831853071795862 * 8.0);
        }
      }
    }
  }
}

//
// File trailer for HS_EWL_TR_FUN_EST.cpp
//
// [EOF]
//
