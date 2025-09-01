#ifndef FREQ_SWEEP_THREAD_H
#define FREQ_SWEEP_THREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QElapsedTimer>

class SinFreqSweepThread : public QThread
{
    Q_OBJECT

signals:
    void consolePutData(const QString &data, quint8 priority);
    void consolePutAdcDataSpecial(const qint16 *p_data, quint32 len, quint8 type);

public slots:

public:
    explicit SinFreqSweepThread(QObject *parent = nullptr);
    ~SinFreqSweepThread();

private:
    void run() override;
    void FreqEstimateForSweep();
    void Sweep();
    bool ConvertToDouble(uint8_t *p_data_in, uint32_t length_in, double *p_data_out, double *p_length);
    void lagrange_resamp_for_phase_gain(const float *input_buf, float input_buf_len, float p, float q, float *output_buf);

    bool m_quit = false;

    double LengthSin = 0.f;
    double LengthSweep = 0.f;

    QElapsedTimer peformance_timer;     // timer for QAM decoder performance measurements
};

#endif // FREQ_SWEEP_THREAD_H
