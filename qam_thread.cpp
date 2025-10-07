#include "qam_thread.h"
#include "usb_global.h"
#include "qam_decoder/qam_decoder.h"
#include "qam_decoder/HS_EWL_DEMOD_QAM.h"
#include "qam_decoder/HS_EWL_DEMOD_QAM_terminate.h"
#include "qam_decoder/HS_EWL_DEMOD_QAM_types.h"
#include "qam_decoder/HS_EWL_FREQ_ACQ.h"
#include "qam_decoder/HS_EWL_FREQ_EST_FOR_SWEEP.h"
#include "qam_decoder/HS_EWL_TR_FUN_EST.h"
#include <QElapsedTimer>
#include "qam_decoder/signal_sweep.h"
#include "qam_decoder/signal.h"
#include "qam_decoder/rt_nonfinite.h"
#include "crc8.h"
#include "crc16.h"
#include "srp_mod_protocol.h"
#include "rs_decoder.h"
#include "qam_decoder/rtwtypes.h"
#include "atomic_vars.h"
#include "statistics.h"
#include <median_window_filter.h>

/* Extern global variables */
extern RingBuffer *m_ring;              // ring data buffer (ADC data) for QAM decoder
extern QWaitCondition ringNotEmpty;
extern QMutex m_mutex_ring_wait;
extern QElapsedTimer profiler_timer;
extern QMutex m_freqValMutex;

/* Private variables */
static double Signal[USB_MAX_DATA_SIZE];
static int16_t FrameErrorAdcBuffer[10][USB_MAX_DATA_SIZE];

//double Fs = 1832061;//280000;//ADC sample rate
//extern double f_opt;
double f0 = 35045;          //carrier freq
double f0_saved = 17523;//35045;    //carrier freq saved for blackbox usage
double sps = round(Fs/f0);  //sample per spreamble_lenymbol
double mode = 0;            //1-both stages enabled, 0-only sevond stage

qam qam_str;

//  output var for HS_EWL_FREQ_ACQ
double warning_status;
double data[27300];
//double index_data;
int32_T len_data;
double f_est_data;//estimated frequency
//int f_est_size;

//  output var for HS_EWL_DEMOD_QAM
//creal_T qam_symbols_data[255];
int qam_symbols_size;
double byte_data[469];
int byte_data_size;

double qam_symbols_real[525];
double qam_symbols_imag[525];
double start_inf_data;

#define DATA_DECODED_SIZE           (128*1024)
#define N_DATA_DECODED_BUFFERS      20

static uint8_t data_decoded[N_DATA_DECODED_BUFFERS][DATA_DECODED_SIZE];      // data decoded from all qam packet
static uint8_t n_data_buf = 0;

double ref_cos[] = {1, 9.927089e-01, 9.709418e-01, 9.350162e-01, 8.854560e-01, 8.229839e-01, 7.485107e-01, 6.631227e-01, 5.680647e-01, 4.647232e-01, 3.546049e-01, 2.393157e-01, 1.205367e-01, 6.123234e-17, -1.205367e-01, -2.393157e-01, -3.546049e-01, -4.647232e-01, -5.680647e-01, -6.631227e-01, -7.485107e-01, -8.229839e-01, -8.854560e-01, -9.350162e-01, -9.709418e-01, -9.927089e-01, -1, -9.927089e-01, -9.709418e-01, -9.350162e-01, -8.854560e-01, -8.229839e-01, -7.485107e-01, -6.631227e-01, -5.680647e-01, -4.647232e-01, -3.546049e-01, -2.393157e-01, -1.205367e-01, -1.836970e-16, 1.205367e-01, 2.393157e-01, 3.546049e-01, 4.647232e-01, 5.680647e-01, 6.631227e-01, 7.485107e-01, 8.229839e-01, 8.854560e-01, 9.350162e-01, 9.709418e-01, 9.927089e-01};
double ref_sin[] = {0, 1.205367e-01, 2.393157e-01, 3.546049e-01, 4.647232e-01, 5.680647e-01, 6.631227e-01, 7.485107e-01, 8.229839e-01, 8.854560e-01, 9.350162e-01, 9.709418e-01, 9.927089e-01, 1, 9.927089e-01, 9.709418e-01, 9.350162e-01, 8.854560e-01, 8.229839e-01, 7.485107e-01, 6.631227e-01, 5.680647e-01, 4.647232e-01, 3.546049e-01, 2.393157e-01, 1.205367e-01, 1.224647e-16, -1.205367e-01, -2.393157e-01, -3.546049e-01, -4.647232e-01, -5.680647e-01, -6.631227e-01, -7.485107e-01, -8.229839e-01, -8.854560e-01, -9.350162e-01, -9.709418e-01, -9.927089e-01, -1, -9.927089e-01, -9.709418e-01, -9.350162e-01, -8.854560e-01, -8.229839e-01, -7.485107e-01, -6.631227e-01, -5.680647e-01, -4.647232e-01, -3.546049e-01, -2.393157e-01, -1.205367e-01};

// Debug variables
qint64 elapsed_all = 0;
qint64 elapsed_all_saved = 0;

// Reed Solomon decoder variables
/* generator polynomial */
unsigned char gs[nn-K1+1];

QamThread::QamThread(QObject *parent) :
    QThread(parent),
    m_flt(new MedianWindowFilter(10, this))
{
}

QamThread::~QamThread()
{
    m_mutex_ring_wait.lock();
    m_quit = true;
    ringNotEmpty.wakeOne();
    m_mutex_ring_wait.unlock();
    wait();
}

void QamThread::run()
{
    bool res = false;

    // RS decoder init
    generate_gf();
    gen_poly(T1, gs);
    //gen_poly(Ts2, gs2);

    // QAM init
    //qam64_init(&qam_str);
    qam256_double_frame_init(&qam_str);
    TxPacketRsCodesSize = 8*2;
    TxPacketDataSize = 469 - TxPacketRsCodesSize;

    // Reset filter for 'f0' carrier frequency
    m_flt->Reset();

    while(!m_quit)
    {
        m_mutex_ring_wait.lock();
        if(!m_ring->DataAvailable())
            ringNotEmpty.wait(&m_mutex_ring_wait);    // Wait condition unlocks mutex before 'wait', and will lock it again just after 'wait' is complete
        m_mutex_ring_wait.unlock();

        m_qamDecodedDataAvailable = true;             // Some data available for QAM decoder

        res = m_ring->GetDouble(Signal, &Length);

        if(!res)
        {
            emit consolePutData("Error ring buffer get() returned false\n", 2);
            continue;
        }

        QAM_Decoder();

        if(m_ChangeSpeed)   // Change speed if needed
        {
            m_ChangeSpeed = false;
            switch(m_SrpMode)
            {
                case HS_210_MODE:
                    // QAM64
                    emit consolePutData(QString("Changing speed to HS_210_MODE\n"), 2);
                    qam64_init(&qam_str);
                    TxPacketRsCodesSize = 8;
                    TxPacketDataSize = 225 - TxPacketRsCodesSize;
                    setFirstPassFlag();
                    break;

                case HS_280_MODE:
                    // QAM256 Double Frame
                    emit consolePutData(QString("Changing speed to HS_280_MODE\n"), 2);
                    qam256_double_frame_init(&qam_str);
                    TxPacketRsCodesSize = 8*2;
                    TxPacketDataSize = 469 - TxPacketRsCodesSize;
                    setFirstPassFlag();
                    break;

                case HS_QPSK_MODE:
                    // QPSK
                    emit consolePutData(QString("Changing speed to HS_QPSK_MODE\n"), 2);
                    qpsk_init(&qam_str);
                    TxPacketRsCodesSize = 8;
                    TxPacketDataSize = 50 - TxPacketRsCodesSize;
                    setFirstPassFlag();
                    break;

                default:
                    break;
            }
        }
    }
}

void QamThread::setFirstPassFlag()
{
    mutex.lock();
    emit consolePutData(QString("Filter f0: reset filter\n"), 1);
    m_QamDecoderFirstPassFlag = true;
    mutex.unlock();
}

void QamThread::srpModeSet(uint8_t mode)
{
    TSrpMode m = TSrpMode(mode);

    switch(m)
    {
        case HS_210_MODE:
        case HS_280_MODE:
        case HS_QPSK_MODE:
            if(m_SrpMode != m)
            {
                emit consolePutData(QString("Received request to change speed, it will be changed later\n"), 2);
                m_SrpMode = m;
                m_ChangeSpeed = true;
            }
            break;

        default:
            break;
    }
}

void QamThread::QAM_Decoder()
{
    QString log_str1 = "";  // combined logs with priority = 1
    QString log_str2 = "";  // combined logs with priority = 2

    int rs_decode_flag[2] = { -1, -1 };
    qint64 rs_elapsed = 0;

    // ERROR status
    int HS_EWL_FREQ_ACQ_error_status;
    int HS_EWL_DEMOD_QAM_error_status;

    bool crc_error = true;
    bool last_frame_received = false;
    double *signal = (double*)&Signal;
    double resamp_qpsk_signal[29300] = {0};
    int32_T resamp_qpsk_len = (int32_T)Length;
    double len = Length;
    double sample_rate = Fs;

    peformance_timer.start();

    if(qam_str.order == 4)
    {
        resamp_qpsk_len = lagrange_reamp(signal, &resamp_qpsk_len, resamp_qpsk_signal, Fs, Fs*2, 1);
        signal = resamp_qpsk_signal;
        len = (double)resamp_qpsk_len;
        sample_rate = Fs/2;
        f0 = 17510;
        sps = 52;
    }

    HS_EWL_FREQ_ACQ_error_status = HS_EWL_FREQ_ACQ(signal, len, sample_rate, f0, sps, mode, preamble_len,
        &qam_str, data, &len_data, (double*)&f_est_data, &warning_status);
    if(f_est_data == 17510)
    {
        f_est_data = 17511;
    }
    if(HS_EWL_FREQ_ACQ_error_status == 0)
    {
        HS_EWL_DEMOD_QAM_error_status = HS_EWL_DEMOD_QAM(data, len_data, f_est_data, sample_rate, &qam_str, qam_symbols_real,
                    qam_symbols_imag, byte_data, &start_inf_data);

        switch(HS_EWL_DEMOD_QAM_error_status)
        {
            case 1: // input data LEN <= 0
                log_str2.append("HS_EWL_DEMOD_QAM_error_status: input data LEN <= 0\n");
                break;
            case 2: // incorrect input sample freq for lagrange_resamp func
                log_str2.append("HS_EWL_DEMOD_QAM_error_status: incorrect input sample freq for lagrange_resamp func\n");
                break;
            default:
            case 0: // OK
                break;
        }

        // Data parsing
        // Convert decoded data from 'double' to 'uint8', copy to data_decoded[] buffer

        for(uint16_t i = 0; i < TxPacketRsCodesSize + TxPacketDataSize; ++i)
            frame_decoded[i] = (uint8_t)byte_data[i];

        // Parse 'frame' tail
        frame_tail_nlast_t *tail = (frame_tail_nlast_t*)&frame_decoded[TxPacketRsCodesSize + TxPacketDataSize - sizeof(frame_tail_nlast_t)];

        // Check 'frame' CRC
        uint8_t crc8 = calc_crc8(frame_decoded + TxPacketRsCodesSize, TxPacketDataSize - 1);


        if(crc8 != tail->crc8)
        {
            crc_error = true;

            peformance_timer2.start();

            // Zero padding before decode
            memset(frame_decoded + TxPacketRsCodesSize + TxPacketDataSize, 0x00, sizeof(frame_decoded) - TxPacketRsCodesSize - TxPacketDataSize);

            // RS Codes length + Data length <= 255 ?
            if(TxPacketRsCodesSize + TxPacketDataSize <= 255)
            {
                // Single QAM frame mode
                rs_decode_flag[0] = decode_rs1(frame_decoded);

                if(rs_decode_flag[0] < 2)
                {
                    // Check crc again
                    crc8 = calc_crc8(frame_decoded + TxPacketRsCodesSize, TxPacketDataSize - 1);
                }
            }
            else
            {
                // Double QAM frame mode
                // 8 rs codes, 8 rs codes, 255-8 data first frame, 255-8 data second frame

                // First QAM frame
                // Copy RS codes to 'tmp_buffer'
                for(quint16 i = 0; i < TxPacketRsCodesSize/2; ++i)
                    tmp_buffer[i] = frame_decoded[i];

                // Copy data to 'tmp_buffer'
                quint16 p_data = TxPacketRsCodesSize;
                for(quint16 i = TxPacketRsCodesSize/2; i < 255; ++i)
                    tmp_buffer[i] = frame_decoded[p_data++];

                rs_decode_flag[0] = decode_rs1(tmp_buffer);

                if(rs_decode_flag[0] < 2)
                {
                    // Copy corrected rs codes & data back
                    memcpy(frame_decoded, tmp_buffer, TxPacketRsCodesSize/2);
                    memcpy(frame_decoded + TxPacketRsCodesSize, tmp_buffer + TxPacketRsCodesSize/2, 255-TxPacketRsCodesSize/2);
                }

                // Second QAM frame
                // Copy RS codes to 'tmp_buffer'
                for(quint16 i = 0; i < TxPacketRsCodesSize/2; ++i)
                    tmp_buffer[i] = frame_decoded[TxPacketRsCodesSize/2 + i];

                // Copy data to 'tmp_buffer'
                p_data = TxPacketRsCodesSize + (255 - TxPacketRsCodesSize/2);
                for(quint16 i = TxPacketRsCodesSize/2; i < 255; ++i)
                    tmp_buffer[i] = frame_decoded[p_data++];

                rs_decode_flag[1] = decode_rs1(tmp_buffer);

                if(rs_decode_flag[1] < 2)
                {
                    // Copy corrected rs codes & data back
                    memcpy(frame_decoded + TxPacketRsCodesSize/2, tmp_buffer, TxPacketRsCodesSize/2);
                    memcpy(frame_decoded + TxPacketRsCodesSize + (255 - TxPacketRsCodesSize/2), tmp_buffer + TxPacketRsCodesSize/2, 255-TxPacketRsCodesSize/2);
                }

                if(rs_decode_flag[0] < 2 || rs_decode_flag[1] < 2)
                {
                    // Check crc again
                    crc8 = calc_crc8(frame_decoded + TxPacketRsCodesSize, TxPacketDataSize - 1);
                }
            }

            rs_elapsed = peformance_timer2.elapsed();
        }

        if(crc8 == tail->crc8)
        {
            crc_statistics_good_crc_received();

            crc_error = false;

            const uint32_t data_size_not_last_frame = TxPacketDataSize - sizeof(frame_tail_nlast_t);    // valuable data in 'not last' frame
            const uint32_t data_size_last_frame = TxPacketDataSize - sizeof(frame_tail_last_t);         // valuable data in 'last' frame

            // Not last frame
            if(tail->frame_flag == ENUM_FRAME_NOT_LAST)
            {
                if(DATA_DECODED_SIZE < tail->frame_id * data_size_not_last_frame + data_size_not_last_frame)
                {
                    log_str2.append(QString("Error wrong frame number %1 or too too long continuous data receive\n").arg(tail->frame_id));
                }
                else
                {
                    // Copy decoded frame to data_decoded buffer
                    for(uint16_t i = 0; i < data_size_not_last_frame; ++i)
                        data_decoded[n_data_buf][tail->frame_id * data_size_not_last_frame + i] = (uint8_t)frame_decoded[TxPacketRsCodesSize + i];
                }
            }
            // Last frame
            else // tail->frame_flag == ENUM_FRAME_LAST
            {
                last_frame_received = true;
                frame_tail_last_t *tail_last = (frame_tail_last_t*)&frame_decoded[TxPacketRsCodesSize + TxPacketDataSize - sizeof(frame_tail_last_t)];

                if(DATA_DECODED_SIZE < tail->frame_id * data_size_not_last_frame + data_size_last_frame)
                {
                    log_str2.append(QString("Error wrong frame number %1 or too too long continuous data receive\n").arg(tail->frame_id));
                }
                else
                {
                    // Check data length
                    if(tail_last->len < MOD_SRP_MIN_VALID_LENGTH_NEW)
                    {
                        // Error length is too short
                        log_str2.append(QString("HS data parsing error, packet data length is too short %1\n").arg(tail_last->len));
                    }
                    else if(tail_last->len > (tail->frame_id * data_size_not_last_frame + TxPacketDataSize - sizeof(frame_tail_last_t)))
                    {
                        // Error length is too long
                        log_str2.append(QString("HS data parsing error, packet data length is too long %1\n").arg(tail_last->len));
                    }
                    else
                    {
                        // Length is ok
                        // Copy decoded frame to data_decoded buffer (just copy everything, do not calculate data length in last frame)
                        for(uint16_t i = 0; i < data_size_last_frame; ++i)
                            data_decoded[n_data_buf][tail->frame_id * data_size_not_last_frame + i] = (uint8_t)frame_decoded[TxPacketRsCodesSize + i];

                        // Transmit decoded data to PC
                        emit postTxDataToSerialPort((uint8_t*)&data_decoded[n_data_buf][MASTER_ADDR_SIZE], tail_last->len - MASTER_ADDR_SIZE);
                        m_qamDecodedDataAvailable = false;  // No data for QAM decoder
                        if(++n_data_buf == N_DATA_DECODED_BUFFERS)
                            n_data_buf = 0;

                        log_str2.append(QString(QString("Last frame #%1, data length %2\n").arg(tail_last->tail.frame_id).arg(tail_last->len)));
                    }
                }
            }
        }

        if(crc8 != tail->crc8)
        {
            crc_statistics_bad_crc_received();
            log_str2.append("CRC error, rs decoder failed to correct data\n");
        }
    }
    
    else
    {
        HS_EWL_DEMOD_QAM_error_status = -1; // HS_EWL_DEMOD_QAM not use
    }

    // RS decoder debug information
    for(uint8_t i = 0; i < 2; ++i)
    {
        switch(rs_decode_flag[i])
        {
            case 0:
                /* no non-zero syndromes => no errors: output received codeword */
                log_str2.append(QString("RS decoder finished in %1 ms: no errors\n").arg(rs_elapsed));
                break;

            case 1:
                log_str2.append(QString("RS decoder finished in %1 ms: errors corrected\n").arg(rs_elapsed));
                break;

            case 2:
            case 3:
                log_str2.append(QString("RS decoder finished in %1 ms: unable to correct\n").arg(rs_elapsed));
                break;

            default:
                break;
        }
    }

    if(rs_decode_flag[0] == -1 && rs_decode_flag[1] == -1)
        rs_statistics_add_no_correction();
    else if(rs_decode_flag[0] < 2 || rs_decode_flag[1] < 2)
        rs_statistics_add_correction();

    // Debug information
    int r = int(warning_status);

    switch(r)
    {
        case CORRECT:
            // warning_status = 0 input array correct
            break;
        case WARNING_1:
            // warning_status = 1 not enough sample in the end array
            log_str2.append("warning_status = 1: not enough sample in the end array\n");
            break;
        case WARNING_2:
            // warning_status = 2 all or more than 0.2 array equal 0
            log_str2.append("warning_status = 2: all or more than 0.2 array equal 0\n");
            break;
        case WARNING_3:
            // warning_status = 3 start array sample equal 0 less than 0
            log_str2.append("warning_status = 3: start array sample equal 0 less than 0\n");
            break;
        case WARNING_4:
            // warning_status = 4 f_est == 0
            log_str2.append("warning_status = 4: error probably wrong f_est\n");
            break;
    };

    // Save 'error-frames' to file
    if(crc_error || warning_status != CORRECT)
    {
        for(uint32_t i = 0; i < Length; ++i)
        {
            double val = Signal[i];
            FrameErrorAdcBuffer[n_error_frame][i] = (int16_t)val;
        }

        emit consoleFrameErrorFile(FrameErrorAdcBuffer[n_error_frame], Length, 0);

        if(++n_error_frame == 10)
            n_error_frame = 0;
    }

    // Debug
    qint64 elapsed = peformance_timer.elapsed();
    elapsed_all += elapsed;

    log_str1.append(QString("QAM decoder finished, len %1, freq %2 Hz, elapsed time = %3 ms\n").arg(Length).arg(f_est_data).arg(elapsed));

    if(last_frame_received)
    {
        log_str1.append(QString("Elapsed all qam = %1\n").arg(elapsed_all));
        elapsed_all_saved = elapsed_all;
        elapsed_all = 0;
    }

    ++m_QamFramesCounter;

//    HS_EWL_DEMOD_QAM_terminate();

    // If frame received without any errors
    if(!crc_error && warning_status == CORRECT && rs_decode_flag[0] == -1 && rs_decode_flag[1] == -1)
    {
        // Filter f0 carrier frequency
        mutex.lock();
        if(m_QamDecoderFirstPassFlag)
        {
            m_QamDecoderFirstPassFlag = false;
            m_flt->Reset();
        }
        mutex.unlock();

        m_flt->AddElement(f_est_data);
        if(m_flt->IsFilled())
        {
            f0 = m_flt->Filter();
            m_freqValMutex.lock();
            f0_saved = f0;
            m_freqValMutex.unlock();
            log_str1.append(QString("Filter f0: median filtered = %1\n").arg(f0));
        }
        else
        {
            log_str1.append("Filter f0: filter is not filled yet, skipping\n");
        }
    }

    // Show logs to console and save to log file (depending on priority)
    if(!log_str2.isEmpty()) emit consolePutData(log_str2, 2);
    if(!log_str1.isEmpty()) emit consolePutData(log_str1, 1);
    //if(!log_str0.isEmpty()) emit consolePutData(log_str0, 0);
}
