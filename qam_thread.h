#ifndef QAM_WORKTHREAD_H
#define QAM_WORKTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <ringbuffer.h>
#include <message_box.h>
#include "qam_common.h"
#include <median_window_filter.h>

using namespace QAM_Common;

class MessageBox;

class QamThread : public QThread
{
    Q_OBJECT

signals:
    void consolePutData(const QString &data, quint8 priority);
    void postTxDataToSerialPort(const uint8_t *p_data, int len);
    void consoleFrameErrorFile(const double *p_data, quint32 len, quint8 type);

public slots:

public:
    explicit QamThread(QObject *parent = nullptr);
    ~QamThread();

    void setFirstPassFlag();
    void srpModeSet(uint8_t mode);

private:
    void run() override;
    void QAM_Decoder();

    bool m_quit = false;

    static const uint32_t DATA_SIZE = (256*1024);

    double Length;

    QElapsedTimer peformance_timer;     // timer for QAM decoder performance measurements
    QElapsedTimer peformance_timer2;    // timer for RS decoder performance measurements

    const qint64 qam_rx_timeout_ms = 100;// rx timeout between QAM packets when decoding multi frame packet, ms

    enum {
        CORRECT = 0,    // warning_satus = 0 input array correct
        WARNING_1 = 1,  // warning_satus = 1 not enough sample in the end array
        WARNING_2 = 2,  // warning_satus = 2 all or more than 0.2 array equal 0
        WARNING_3 = 3,  // warning_satus = 3 start array sample equal 0 less than 0.2
        WARNING_4 = 4,  // warning_satus = 4 kostyl check failed
    } HsEwlReceive_WarningStatus;

    // QAM data related sizes & offsets
    uint32_t TxPacketRsCodesSize = 8;//8*2 for qam256 double frame//8 for qam64 and qpsk
    uint32_t TxPacketDataSize = (50 - TxPacketRsCodesSize);//(225 - TxPacketRsCodesSize) - for qam64

    bool m_QamDecoderFirstPassFlag = true;

    QMutex mutex;

    //uint8_t frame_decoded[TxPacketRsCodesSize + TxPacketDataSize];          // data decoded from single qam packet
    uint8_t frame_decoded[255*2];

    uint8_t n_error_frame = 0;

    bool m_ChangeSpeed = false;     // received request-command to change speed (LS\HS QAM64\HS QAM256)

    quint8 tmp_buffer[255];

    MedianWindowFilter *m_flt = nullptr;
};

#endif // QAM_WORKTHREAD_H
