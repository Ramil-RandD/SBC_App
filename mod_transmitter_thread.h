#ifndef MOD_TRANSMITTER_THREAD_H
#define MOD_TRANSMITTER_THREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QTimer>
#include <message_box.h>

class ModTransmitterThread : public QThread
{
    Q_OBJECT

signals:
    void consolePutData(const QString &data, quint8 priority);
    void postDataToStm32H7(const uint8_t *p_data, const int length);
    void startAnswerTimeoutTimer(int timeout_ms);
    void stopAnswerTimeoutTimer();
    void sendCommandToSTM32(quint8 command, const quint8 *p_data, quint32 data_size);
    void qamDecoderReset();

public slots:
    void timeoutAnswer();

public:
    explicit ModTransmitterThread(QObject *parent = nullptr);
    ~ModTransmitterThread();

    void ModStartTransmitPhaseGain();

    void ModAnswerDataReceived();
    void calculatePredistortionTablesStart();
    void calculatePredistortionTablesContinue();
    void separateAgcStart();
    void separateRecordSweepStart();

    bool m_AutoConfigurationMode = false;                   // Set true if auto configuration is active (in progress)

private:
    void run() override;
    void transmitPredistortionTables();
    void calculatePredistortionTablesStop();
    void calculatePredistortionTablesStop_AGC_STOP();

    bool m_quit = false;

    message_header message;
    uint8_t message_box_buffer_mod[1024];
    uint16_t n_channel = 0;

    uint32_t n_elements = 128;//2048;

    typedef enum
    {
        IDLE = 0,                                   // Auto configuration is not active or finished, idle state
        AUTOCFG_START = 1,                          // Auto configuration started
        SIN35KHZ_MOD_COMMANDS_FOR_AGC = 2,          // AGC (automatic gain configuration) for 'sinus 35 kHz' signal is in progress
        ADC_START_FOR_SIN600 = 3,                   // Configuring ADC to record 'sinus 35 kHz 600 periods' signal
        SIN600_MOD_COMMAND = 4,                     // Requesting MOD to send 'sinus 34 kHz 600 periods' signal
        FREQ_ESTIMATE_FUNC_WAIT = 5,                // Frequency estimate is in progress, please wait...
        AGC_START_FOR_SWEEP = 6,                    // Starting AGC (automatic gain configuration) for 'sweep' signal
        SWEEP_MOD_COMMANDS_FOR_AGC = 7,             // AGC (automatic gain configuration) for 'sweep' signal is in progress
        ADC_START_FOR_SWEEP = 8,                    // Configuring ADC to record 'sweep' signal
        SWEEP_MOD_COMMAND = 9,                      // Requesting MOD to send 'sweep' signal
        SWEEP_FUNC_WAIT = 10,                       // 'Sweep' is in progress, please wait...
        AGC_START_FOR_SIN35KHZ = 11,                // tarting AGC (automatic gain configuration) for 'sinus 35 kHz' signal
        SIN35KHZ_MOD_COMMANDS_FOR_AGC_2 = 12,       // AGC (automatic gain configuration) for 'sinus 35 kHz' signal is in progress
        START_TX_PREDISTORTION_TABLES_TO_MOD = 13,  // Preparing to transmit 'predistortion tables' to MOD
        TX_PREDISTORTION_TABLES_TO_MOD = 14,        // Transmitting 'predistortion tables' to MOD, please wait...
        AGC_START_FOR_MOD_STAT = 15,                // 'Predistortion tables' calculation complete. Starting AGC (automatic gain configuration) for 'MOD get status' signal
        STAT_MOD_COMMANDS_FOR_AGC = 16,             // AGC (automatic gain configuration) for 'MOD get status' signal is in progress
        SEND_TO_MOD2_RX_PARAMETERS = 17,

        AGCCFG_START = 100,                         // AGC auto configuration (separately from 'Auto configuration') started

        SPECIAL_USR_REQ_START = 101,                // Start special user requested commands sequence: configure AGC for Sweep, then save Sweep to file and exit
        SPECIAL_USR_REQ_AGC_START_FOR_SWEEP = 102,  // Special user requested commands sequence: AGC for Sweep
        SPECIAL_USR_REQ_SWEEP_MOD_COMMANDS_FOR_AGC = 103,   // Special user requested commands sequence: requesting MOD to send 'sweep' signals
        SPECIAL_USR_REQ_ADC_START_FOR_SWEEP = 104,
        SPECIAL_USR_REQ_SWEEP_MOD_COMMAND = 105,

        ERROR_MOD2_SEND_RX_PARAMETERS = 245,
        ERROR_MOD2_AUTO_CFG_STOP = 246,
        ERROR_MOD2_AUTO_CFG_START = 247,
        ERROR_ANSWER_TIMEOUT = 248,                 // Reserved for MAXIM/Indigo Suite DLL: no answer from SRP2
        ERROR_PREDISTORTION_TABLES_TX_FAILED = 249,     // Errors todo
        ERROR_AGC_MODSTAT = 250,
        ERROR_SWEEP_TIMEOUT = 251,
        ERROR_AGC_SWEEP = 252,
        ERROR_FREQ_ESTIMATE_TIMEOUT = 253,
        ERROR_AGC_SIN35KHZ = 254,

        AUTOCFG_COMPLETE_SUCCESSFULLY = 255         // Auto configuration complete successfully, ready for work

    } TState;

    TState State = IDLE;

    void setState(TState state);

    typedef enum
    {
        TX_IDLE = 0,
        TX_START = 1,
        TX_PHASE_TABLE = 2,
        TX_GAIN_TABLE = 3,
        TX_SHIFT_CRC = 4,
        TX_FINISHED = 5
    } TPredistTxState;

    TPredistTxState StatePredistTx = TX_IDLE;

    bool retry = false;
    uint32_t n_attempts = 0;
    uint32_t n_attempts_high_level = 0;
    uint32_t n_continuous_errors = 0;                       // number of continuous AGC errors

    const uint32_t n_MaxAttempts = 15;
    const uint32_t n_MaxAttemptsHighLevel = 3;
    const uint32_t n_MaxContinuousAgcErrors = 10;           // max number of continuous AGC errors

    const uint32_t timeoutAnswer_ms = 1100;

    const uint32_t timeoutAgcSin35kHzCommands_ms = 500;     // timeout between SIN 35 kHz transfers for AGC
    const uint32_t timeoutAgcSweepCommands_ms = 600;        // timeout between Sweep transfers for AGC
    const uint32_t timeoutModStatusCommands_ms = 500;       // timeout between MOD GET STATUS transfers for AGC

    const uint32_t n_MaxMod2AutoCfgCommands = 10;           // max number of command that transfers to MOD2 during auto_configuration
    const uint32_t n_MaxSin35kHzCommands = 200;             // max number of SIN 35 kHz transfers for AGC
    const uint32_t n_MaxSweepCommands = 500;                // max number of Sweep transfers for AGC
    const uint32_t n_MaxModStatusCommands = 200;            // max number of MOD STATUS transfers for AGC

    bool hs_data_received = false;                          // If true, indicates that at least some 'HS' data was received from usb STM32 H7
};

#endif // MOD_TRANSMITTER_THREAD_H
