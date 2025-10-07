#include "mod_transmitter_thread.h"
#include "usb_global.h"
#include <QElapsedTimer>

#include "crc16.h"
#include "crc8.h"
#include "srp_mod_protocol.h"
#include "../SRP_HS_USB_PROTOCOL/SRP_HS_USB_Protocol.h"
#include "ringbuffer.h"
#include "atomic_vars.h"
#include "agc_algorithm.h"

/* Extern global variables */
extern QWaitCondition modTransmitWakeUp;
extern QMutex m_mutex_mod;

extern float gain_data_float[2048];
extern float phase_data_float[2048];
extern float gain_resamp_data[128];
extern float phase_resamp_data[128];

extern uint16_t shift_for_qam_data_int;

extern RingBuffer *m_ring;

extern bool is_auto_config_work;
extern bool mod2_auto_cfg_start_answer;
extern bool mod2_auto_cfg_stop_answer;
extern bool mod2_set_rx_parameters_answer;

ModTransmitterThread::ModTransmitterThread(QObject *parent) :
    QThread(parent)
{
}

ModTransmitterThread::~ModTransmitterThread()
{
    m_mutex_mod.lock();
    m_quit = true;
    modTransmitWakeUp.wakeOne();
    m_mutex_mod.unlock();

    wait();
}

void ModTransmitterThread::setState(TState state)
{
    State = state;
    emit consolePutData(QString(":: Predistortion auto cfg :: State #%1\n").arg(State), 2);
    m_statusAutoCfgPredistortion = (quint8)state;
}

void ModTransmitterThread::run()
{
    static uint32_t n_commands = 0;
    static uint32_t Timeout = 0;

    while(!m_quit)
    {
        m_mutex_mod.lock();
        modTransmitWakeUp.wait(&m_mutex_mod); // Wait condition unlocks mutex before 'wait', and will lock it again just after 'wait' is complete

        switch(State)
        {
            case IDLE:
            case ERROR_AGC_MODSTAT:
            case ERROR_SWEEP_TIMEOUT:
            case ERROR_AGC_SWEEP:
            case ERROR_FREQ_ESTIMATE_TIMEOUT:
            case ERROR_AGC_SIN35KHZ:
            case ERROR_MOD2_AUTO_CFG_START:
            case ERROR_MOD2_AUTO_CFG_STOP:
            case ERROR_MOD2_SEND_RX_PARAMETERS:
                m_mutex_mod.unlock();
                continue;

            case AUTOCFG_START:
                // Disable ring buffer for QAM decoder (disable QAM decoder)
                emit consolePutData(":: Predistortion auto cfg :: disable QAM decoder ring buffer'\n", 2);
                m_ring->SetActive(false);

                is_auto_config_work = true;

//                if(mod2_auto_cfg_start_answer == false)
//                {
//                    if(n_commands > n_MaxMod2AutoCfgCommands)
//                    {
//                        emit consolePutData(":: Predistortion auto cfg :: send cmd to mod2 'MOD_AUTO_CFG_START' error: too many 'MOD_AUTO_CFG_START' commands transmitted to MOD and MOD not answered\n", 2);
//                        setState(ERROR_MOD2_AUTO_CFG_START);
//                        n_commands = 0;
//                        calculatePredistortionTablesStop();
//                        break;
//                    }
//                    message.command = CMessageBox::MOD_AUTO_CFG_START;
//                    message.packet_adr = 0;
//                    message.data_len = 0;
//                    message.message_id = 0;//0;
//                    message.master_address = CMessageBox::MOD2_ADDR;
//                    message.own_address = CMessageBox::MASTER_ADDR;

//                    uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
//                    emit consolePutData(":: Predistortion auto cfg :: Send 'MOD_AUTO_CFG_START' command to MOD2\n", 2);
//                    postDataToStm32H7(message_box_buffer_mod, tx_len);
//                    n_commands++;

//                        // Start timeout before next command
//                    emit startAnswerTimeoutTimer(timeoutAnswer_ms);
//                    break;
//                }
//                else
//                {
                    mod2_auto_cfg_start_answer = false;
                    // Send 'AGC start' to STM32
                    emit consolePutData(":: Predistortion auto cfg :: send 'AGC start'\n", 2);
                    emit sendCommandToSTM32(USB_CMD_AGC_START, nullptr, 0);

                    // Delay
                    QThread::msleep(100);

                    setState(SIN35KHZ_MOD_COMMANDS_FOR_AGC);
                    n_commands = 0;
                    AgcStateGlobal = AGC_START;
//                }
                /* fallthrough */

            case SIN35KHZ_MOD_COMMANDS_FOR_AGC:
            case SIN35KHZ_MOD_COMMANDS_FOR_AGC_2:
                {
                    // Send 'SEND_SIN_35KHZ' command to MOD
                    uint8_t AGCState = AgcStateGlobal;

                    // Check for continuous AGC errors
                    if(AGCState == AGC_ERROR)
                    {
                        if(++n_continuous_errors > n_MaxContinuousAgcErrors)
                        {
                            n_continuous_errors = 0;
                            emit consolePutData(":: Predistortion auto cfg :: AGC failed with status 'AGC error' (too much continuous errors)\n", 2);
                            setState(ERROR_AGC_SIN35KHZ);
                            calculatePredistortionTablesStop();
                            break;
                        }
                    }
                    else
                        n_continuous_errors = 0;

                    // At least one 'SIN 35kHz' must be transmitted and AGCState = AGC_OK
                    if(AGCState != AGC_OK || !n_commands)
                    {
                        if(++n_commands > n_MaxSin35kHzCommands)
                        {
                            emit consolePutData(":: Predistortion auto cfg :: AGC for 'SIN 35kHz' error: too many 'SEND_SIN_35KHZ' commands transmitted to MOD and still no 'AGC OK' status from STM32\n", 2);
                            setState(ERROR_AGC_SIN35KHZ);
                            calculatePredistortionTablesStop();
                            break;
                        }

                        //message.command = CMessageBox::SEND_SIN_35KHZ;
                        //message.packet_adr = 0;
                        //message.data_len = 0;
                        //message.message_id = 3;//0;
                        //message.master_address = CMessageBox::MOD2_ADDR;
                        //message.own_address = CMessageBox::MASTER_ADDR;
                        message_box_buffer_mod[0] = CMessageBox::MOD2_ADDR;
                        message_box_buffer_mod[1] = CMessageBox::MASTER_ADDR;
                        message_box_buffer_mod[2] = CMessageBox::SEND_SIN_35KHZ;
                        //message_box_buffer_mod[3] = calc_crc8(message_box_buffer_mod, 3);

                        uint16_t tx_len = 3;//CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                        emit consolePutData(QString(":: Predistortion auto cfg :: Send 'SEND_SIN_35KHZ' command to MOD #%1 of max #%2\n").arg(n_commands).arg(n_MaxSin35kHzCommands), 2);
                        postDataToStm32H7(message_box_buffer_mod, tx_len);

                        // Start timeout before next command
                        emit startAnswerTimeoutTimer(timeoutAgcSin35kHzCommands_ms);
                        break;
                    }

                    emit consolePutData(":: Predistortion auto cfg :: AGC for 'SIN 35kHz' configured, state AGC_OK\n", 2);

                    if(State == SIN35KHZ_MOD_COMMANDS_FOR_AGC_2)
                    {
                        setState(START_TX_PREDISTORTION_TABLES_TO_MOD);
                        emit startAnswerTimeoutTimer(100);
                        break;
                    }

                    setState(ADC_START_FOR_SIN600);
                }
                /* fallthrough */

            case ADC_START_FOR_SIN600:
            {
                // Send 'ADC_START for SIN 600' to STM32
                emit consolePutData(":: Predistortion auto cfg :: send 'ADC_START for SIN 600'\n", 2);
                sin600_command = true;
                common_special_command = true;

                uint32_t adc_data_length = 31400*2;
                emit sendCommandToSTM32(USB_CMD_ADC_START, (uint8_t*)&adc_data_length, 4);

                // Next state

                setState(SIN600_MOD_COMMAND);

                // Delay
                QThread::msleep(100);
            }
                /* fallthrough */

            case SIN600_MOD_COMMAND:
            {
                emit consolePutData(QString(":: Predistortion auto cfg :: Send 'SEND_SIN_35KHZ_600' command to MOD\n"), 2);
                FreqEstState = TFreqEstState::FREQ_EST_START;

                //message.command = CMessageBox::SEND_SIN_35KHZ_600;
                //message.packet_adr = 0;
                //message.data_len = 0;
                //message.message_id = 0;
                //message.master_address = CMessageBox::MOD2_ADDR;
                //message.own_address = CMessageBox::MASTER_ADDR;

                message_box_buffer_mod[0] = CMessageBox::MOD2_ADDR;
                message_box_buffer_mod[1] = CMessageBox::MASTER_ADDR;
                message_box_buffer_mod[2] = CMessageBox::SEND_SIN_35KHZ_600;
                //message_box_buffer_mod[3] = calc_crc8(message_box_buffer_mod, 3);

                uint16_t tx_len = 3;//CMessageBox::message_header_to_array(&message, message_box_buffer_mod);

                //uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                postDataToStm32H7(message_box_buffer_mod, tx_len);

                // Wait for freq estimate
                QThread::msleep(100);

                // Next state
                State = FREQ_ESTIMATE_FUNC_WAIT;

                // Reset timeout
                Timeout = 0;

                // Start timeout to check 'frequency estimate' function completion
                emit startAnswerTimeoutTimer(500);
            }
                break;

            case FREQ_ESTIMATE_FUNC_WAIT:
                {
                    ++Timeout;

                    quint8 tmp = FreqEstState;
                    TFreqEstState s = TFreqEstState(tmp);
                    //emit consolePutData(":: Predistortion auto cfg :: send 'FREQ_ESTIMATE_FUNC_WAIT'\n", 2);
                    //QThread::msleep(10000);
                    if(s == FREQ_EST_COMPLETE)
                    {
                        // Complete, go to next state
                        setState(AGC_START_FOR_SWEEP);
                    }
                    else if(Timeout >= 75000/100)  // 30 sec
                    {
                        emit consolePutData(":: Predistortion auto cfg :: Error freq estimate timeout elapsed\n", 2);
                        setState(ERROR_FREQ_ESTIMATE_TIMEOUT);
                        calculatePredistortionTablesStop();
                        break;
                    }
                    else
                    {
                        // Start timeout to check 'frequency estimate' function completion
                        emit startAnswerTimeoutTimer(100);
                        break;
                    }
                }
                /* fallthrough */

            case AGC_START_FOR_SWEEP:
                // Send 'AGC start' to STM32
                emit consolePutData(":: Predistortion auto cfg :: send 'AGC start'\n", 2);
                emit sendCommandToSTM32(USB_CMD_AGC_START, nullptr, 0);

                // Delay
                QThread::msleep(100);

                setState(SWEEP_MOD_COMMANDS_FOR_AGC);
                n_commands = 0;
                AgcStateGlobal = AGC_START;
                /* fallthrough */

            case SWEEP_MOD_COMMANDS_FOR_AGC:
                {
                    // 2. Send 'SWEEP' command to MOD
                    uint8_t AGCState = AgcStateGlobal;

                    // At least one 'SWEEP' must be transmitted and AGCState = AGC_OK
                    if(AGCState != AGC_OK || !n_commands)
                    {
                        if(++n_commands > n_MaxSweepCommands)
                        {
                            emit consolePutData(":: Predistortion auto cfg :: AGC for 'SWEEP' error: too many 'SEND_SWEEP_SIGNAL' commands transmitted to MOD and still no 'AGC OK' status from STM32\n", 2);
                            setState(ERROR_AGC_SWEEP);
                            calculatePredistortionTablesStop();
                            break;
                        }

                        //message.command = CMessageBox::SEND_SWEEP_SIGNAL;
                        //message.packet_adr = 0;
                        //message.data_len = 0;
                        //message.message_id = 0;
                        //message.master_address = CMessageBox::MOD2_ADDR;
                        //message.own_address = CMessageBox::MASTER_ADDR;

                        message_box_buffer_mod[0] = CMessageBox::MOD2_ADDR;
                        message_box_buffer_mod[1] = CMessageBox::MASTER_ADDR;
                        message_box_buffer_mod[2] = CMessageBox::SEND_SWEEP_SIGNAL;
                        //message_box_buffer_mod[3] = calc_crc8(message_box_buffer_mod, 3);

                        uint16_t tx_len = 3;

                        //uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                        emit consolePutData(QString(":: Predistortion auto cfg :: Send 'SEND_SWEEP_SIGNAL' command to MOD #%1 of max #%2\n").arg(n_commands).arg(n_MaxSweepCommands), 2);
                        postDataToStm32H7(message_box_buffer_mod, tx_len);

                        // Start timeout before next command
                        emit startAnswerTimeoutTimer(timeoutAgcSweepCommands_ms);
                        break;
                    }

                    emit consolePutData(":: Predistortion auto cfg :: AGC for 'SWEEP' configured, state AGC_OK\n", 2);
                    setState(ADC_START_FOR_SWEEP);
                }
                /* fallthrough */

            case ADC_START_FOR_SWEEP:
                {
                    // 3. Send 'ADC_START for SWEEP' to STM32
                    emit consolePutData("Predistortion auto cfg: send 'ADC_START for 'SIN 35KHZ 600''\n", 2);
                    sweep_command = true;
                    common_special_command = true;

                    uint32_t adc_data_length = 440000;
                    emit sendCommandToSTM32(USB_CMD_ADC_START, (uint8_t*)&adc_data_length, 4);

                    // Next state
                    setState(SWEEP_MOD_COMMAND);

                    // Delay
                    QThread::msleep(100);
                }
                /* fallthrough */

            case SWEEP_MOD_COMMAND:
                {
                    SweepState = TSweepState::SWEEP_START;

                    //message.command = CMessageBox::SEND_SWEEP_SIGNAL;
                    //message.packet_adr = 0;
                    //message.data_len = 0;
                    //message.message_id = 0;
                    //message.master_address = CMessageBox::MOD2_ADDR;
                    //message.own_address = CMessageBox::MASTER_ADDR;

                    message_box_buffer_mod[0] = CMessageBox::MOD2_ADDR;
                    message_box_buffer_mod[1] = CMessageBox::MASTER_ADDR;
                    message_box_buffer_mod[2] = CMessageBox::SEND_SWEEP_SIGNAL;
                    //message_box_buffer_mod[3] = calc_crc8(message_box_buffer_mod, 3);

                    uint16_t tx_len = 3;

                    //uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                    emit consolePutData(QString(":: Predistortion auto cfg :: Send 'SEND_SWEEP_SIGNAL' command to MOD\n"), 2);
                    postDataToStm32H7(message_box_buffer_mod, tx_len);

                    // Wait for freq estimate
                    QThread::msleep(100);

                    // Next state
                    setState(SWEEP_FUNC_WAIT);

                    // Reset timeout
                    Timeout = 0;

                    // Start timeout to check 'sweep' function completion
                    emit startAnswerTimeoutTimer(500);
                }
                /* fallthrough */

            case SWEEP_FUNC_WAIT:
                {
                    ++Timeout;
                    uint8_t tmp = SweepState;
                    TSweepState s = TSweepState(tmp);

                    if(s == SWEEP_COMPLETE)
                    {
                        // Complete, go to next state
                        setState(AGC_START_FOR_SIN35KHZ);
                    }
                    else if(Timeout >= 30000/100)  // 30 sec
                    {
                        emit consolePutData(":: Predistortion auto cfg :: Error predistortion auto cfg: sweep func timeout elapsed\n", 2);
                        setState(ERROR_SWEEP_TIMEOUT);
                        calculatePredistortionTablesStop();
                        break;
                    }
                    else
                    {
                        // Start timeout to check 'sweep' function completion
                        emit startAnswerTimeoutTimer(100);
                        break;
                    }
                }
                /* fallthrough */

            case AGC_START_FOR_SIN35KHZ:
                // Send 'AGC start' to STM32
                emit consolePutData(":: Predistortion auto cfg :: send 'AGC start'\n", 2);
                emit sendCommandToSTM32(USB_CMD_AGC_START, nullptr, 0);

                // Delay
                QThread::msleep(100);

                setState(SIN35KHZ_MOD_COMMANDS_FOR_AGC_2);
                n_commands = 0;
                AgcStateGlobal = AGC_START;

                // Start timeout before sending commands to MOD
                emit startAnswerTimeoutTimer(100);
                break;

            case START_TX_PREDISTORTION_TABLES_TO_MOD:
                // Start tranmitting predistortion tables to MOD
                message.command = CMessageBox::SET_PHASE_TABLE;
                message.message_id = 0;
                message.master_address = CMessageBox::MOD2_ADDR;
                message.own_address = CMessageBox::MASTER_ADDR;
                message.data_len = 32;

                StatePredistTx = TX_START;

                retry = false;
                n_attempts = 0;
                n_attempts_high_level = 0;

                emit sendCommandToSTM32(USB_CMD_SET_GAIN_TABLES, (uint8_t*)&gain_data_float, sizeof(gain_data_float)/2);
                QThread::msleep(1500);
                emit sendCommandToSTM32(USB_CMD_SET_PHASE_TABLES, (uint8_t*)&phase_data_float, sizeof(phase_data_float)/2);
                QThread::msleep(1500);
                emit sendCommandToSTM32(USB_CMD_SET_PRED_SHIFT, (uint8_t*)&shift_for_qam_data_int, sizeof(shift_for_qam_data_int));
                QThread::msleep(1500);

                setState(TX_PREDISTORTION_TABLES_TO_MOD);
                /* fallthrough */

             case TX_PREDISTORTION_TABLES_TO_MOD:
                transmitPredistortionTables();
                break;

            case AGCCFG_START:
                // Disable ring buffer for QAM decoder (disable QAM decoder)
                emit consolePutData(":: Automatic Gain Control (AGC) configuration started'\n", 2);
                emit consolePutData(":: Disable QAM decoder ring buffer'\n", 2);
                m_ring->SetActive(false);
                /* fallthrough */

            case AGC_START_FOR_MOD_STAT:
                // Send 'AGC start' to STM32
                emit consolePutData(":: Predistortion auto cfg :: send 'AGC start'\n", 2);
                emit sendCommandToSTM32(USB_CMD_AGC_START, nullptr, 0);

                // Delay
                QThread::msleep(100);

                setState(STAT_MOD_COMMANDS_FOR_AGC);
                n_commands = 0;
                AgcStateGlobal = AGC_START;
                /* fallthrough */

             case STAT_MOD_COMMANDS_FOR_AGC:
                {
                    // Send 'GET_STATUS' command to MOD
                    uint8_t AGCState = AgcStateGlobal;

                    // At least one 'GET STATUS' must be transmitted and AGCState = AGC_OK
                    if(AGCState != AGC_OK || !n_commands)
                    {
                        if(++n_commands > n_MaxModStatusCommands)
                        {
                            emit consolePutData(":: Predistortion auto cfg :: AGC for 'MOD STATUS' error: too many 'GET STATUS' commands transmitted to MOD and still no 'AGC OK' status from STM32\n", 2);
                            setState(ERROR_AGC_MODSTAT);
                            calculatePredistortionTablesStop();
                            break;
                        }

                        //message.command = CMessageBox::STATUS;
                        //message.packet_adr = 0;
                        //message.data_len = 0;
                        //message.message_id = 0;
                        //message.master_address = CMessageBox::MOD2_ADDR;
                        //message.own_address = CMessageBox::MASTER_ADDR;
                        message_box_buffer_mod[0] = CMessageBox::MOD2_ADDR;
                        message_box_buffer_mod[1] = CMessageBox::MASTER_ADDR;
                        message_box_buffer_mod[2] = CMessageBox::STATUS;
                        //message_box_buffer_mod[3] = calc_crc8(message_box_buffer_mod, 3);

                        uint16_t tx_len = 3;

                        //uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                        emit consolePutData(QString(":: Predistortion auto cfg :: Send 'GET STATUS' command to MOD #%1 of max #%2\n").arg(n_commands).arg(n_MaxModStatusCommands), 2);
                        postDataToStm32H7(message_box_buffer_mod, tx_len);

                        // Start timeout before next command
                        emit startAnswerTimeoutTimer(timeoutModStatusCommands_ms);
                        break;
                    }

                    emit consolePutData(":: Predistortion auto cfg :: AGC for 'GET STATUS' configured, state AGC_OK\n", 2);

                    // Finishing
                    //setState(AUTOCFG_COMPLETE_SUCCESSFULLY);
                    n_commands = 0;
                    //setState(SEND_TO_MOD2_RX_PARAMETERS);
                    setState(AUTOCFG_COMPLETE_SUCCESSFULLY);
                    emit startAnswerTimeoutTimer(100);
                }
                break;

//             case SEND_TO_MOD2_RX_PARAMETERS:
//                    // Send 'AGC start' to STM32
//                    if(hs_data_received == false)
//                    {
//                        if(n_commands > n_MaxMod2AutoCfgCommands)
//                        {
//                            emit consolePutData(":: Predistortion auto cfg :: tx cmd to mod2 'SEND_TO_MOD2_RX_PARAMETERS' error: too many 'MOD_AUTO_CFG_START' commands transmitted to MOD and MOD not answered\n", 2);
//                            setState(ERROR_MOD2_SEND_RX_PARAMETERS);
//                            calculatePredistortionTablesStop();
//                            break;
//                        }

//                        // Send 'AGC start' to STM32
//                        emit consolePutData(":: Predistortion auto cfg :: send 'SET RX PARAMETERS' to MOD2\n", 2);
//                        emit sendCommandToSTM32(USB_CMD_SET_RX_PARAMETRS_FOR_MOD, nullptr, 0);

//                        n_commands++;

//                        // Start timeout before next command
//                        emit startAnswerTimeoutTimer(5000);//timeoutAnswer_ms);
//                        break;
//                    }
//                    else
//                    {
//                        hs_data_received = false;
//                        n_commands = 0;
//                        mod2_set_rx_parameters_answer = false;
//                        emit consolePutData(":: Predistortion auto cfg :: new rx parameters upload to MOD2\n", 2);
//                        setState(AUTOCFG_COMPLETE_SUCCESSFULLY);
//                        //break;
//                    }
                    /* fallthrough */
             //case STAT_SRP_COMMANDS_FOR_AGC:


             case AUTOCFG_COMPLETE_SUCCESSFULLY:

//                if(hs_data_received == false)
//                {
//                    if(n_commands > n_MaxMod2AutoCfgCommands)
//                    {
//                        emit consolePutData(":: Predistortion auto cfg :: send cmd to mod2 'MOD_AUTO_CFG_STOP' error: too many 'MOD_AUTO_CFG_STOP' commands transmitted to MOD and MOD not answered\n", 2);
//                        setState(ERROR_MOD2_AUTO_CFG_STOP);
//                        n_commands = 0;
//                        calculatePredistortionTablesStop();
//                        break;
//                    }
//                    //message.command = CMessageBox::MOD_AUTO_CFG_STOP;
//                    //message.packet_adr = 0;
//                    //message.data_len = 0;
//                    //message.message_id = 0;//0;
//                    //message.master_address = CMessageBox::MOD2_ADDR;
//                    //message.own_address = CMessageBox::MASTER_ADDR;
//                    message_box_buffer_mod[0] = CMessageBox::MOD2_ADDR;
//                    message_box_buffer_mod[1] = CMessageBox::MASTER_ADDR;
//                    message_box_buffer_mod[2] = CMessageBox::MOD_AUTO_CFG_STOP;

//                    uint16_t tx_len = 3;
//                    //uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
//                    emit consolePutData(":: Predistortion auto cfg :: Send 'MOD_AUTO_CFG_STOP' command to MOD2\n", 2);
//                    postDataToStm32H7(message_box_buffer_mod, tx_len);
//                    n_commands++;

//                        // Start timeout before next command
//                    emit startAnswerTimeoutTimer(timeoutAnswer_ms);
//                    break;
//                }
//                else
//                {
                    hs_data_received = false;
                    emit consolePutData(":: Predistortion auto cfg :: auto configuration complete, all operations completed successfully\n", 2);
                    calculatePredistortionTablesStop();
                    break;
//                }

            //====================================================================================================================

            case SPECIAL_USR_REQ_START:
                emit consolePutData(":: User asked to record 'Sweep' signal, starting...\n", 2);
                emit consolePutData(":: Automatic Gain Control (AGC) configuration started\n", 2);
                emit consolePutData(":: Disable QAM decoder ring buffer\n", 2);
                // Disable ring buffer for QAM decoder (disable QAM decoder)
                m_ring->SetActive(false);
                /* fallthrough */

            case SPECIAL_USR_REQ_AGC_START_FOR_SWEEP:
                // Send 'AGC start' to STM32
                emit consolePutData(":: Send 'AGC start'\n", 2);
                emit sendCommandToSTM32(USB_CMD_AGC_START, nullptr, 0);

                // Delay
                QThread::msleep(100);

                setState(SPECIAL_USR_REQ_SWEEP_MOD_COMMANDS_FOR_AGC);
                n_commands = 0;
                AgcStateGlobal = AGC_START;
                /* fallthrough */

            case SPECIAL_USR_REQ_SWEEP_MOD_COMMANDS_FOR_AGC:
            {
                // Send 'SWEEP' command to MOD
                uint8_t AGCState = AgcStateGlobal;

                // At least one 'SWEEP' must be transmitted and AGCState = AGC_OK
                if(AGCState != AGC_OK || !n_commands)
                {
                    if(++n_commands > n_MaxSweepCommands)
                    {
                        emit consolePutData(":: AGC for 'SWEEP' error: too many 'SEND_SWEEP_SIGNAL' commands transmitted to MOD and still no 'AGC OK' status from STM32\n", 2);
                        setState(ERROR_AGC_SWEEP);
                        calculatePredistortionTablesStop();
                        break;
                     }

                    message.command = CMessageBox::SEND_SWEEP_SIGNAL;
                    message.packet_adr = 0;
                    message.data_len = 0;
                    message.message_id = 0;
                    message.master_address = CMessageBox::MOD2_ADDR;
                    message.own_address = CMessageBox::MASTER_ADDR;

                    uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                    emit consolePutData(QString(":: Send 'SEND_SWEEP_SIGNAL' command to MOD #%1 of max #%2\n").arg(n_commands).arg(n_MaxSweepCommands), 2);
                    postDataToStm32H7(message_box_buffer_mod, tx_len);

                    // Start timeout before next command
                    emit startAnswerTimeoutTimer(timeoutAgcSweepCommands_ms);
                    break;
                }

                emit consolePutData(":: AGC for 'SWEEP' configured, state AGC_OK\n", 2);

                n_commands = 0;
                setState(SPECIAL_USR_REQ_ADC_START_FOR_SWEEP);
            }
                /* fallthrough */

            case SPECIAL_USR_REQ_ADC_START_FOR_SWEEP:
            {
                // Send 'ADC_START for SWEEP' to STM32
                emit consolePutData("Send 'ADC_START for 'sweep'\n", 2);
                sweep_record_to_file = sweep_command = true;
                common_special_command = true;

                uint32_t adc_data_length = 440000;
                emit sendCommandToSTM32(USB_CMD_ADC_START, (uint8_t*)&adc_data_length, 4);

                // Delay
                QThread::msleep(100);
            }
            /* fallthrough */

            case SPECIAL_USR_REQ_SWEEP_MOD_COMMAND:
            {
                if(++n_commands > 3)
                {
                    emit consolePutData("User requested 'Sweep' records has finished\n", 2);
                    setState(AUTOCFG_COMPLETE_SUCCESSFULLY);
                    calculatePredistortionTablesStop();
                    break;
                }

                message.command = CMessageBox::SEND_SWEEP_SIGNAL;
                message.packet_adr = 0;
                message.data_len = 0;
                message.message_id = 0;
                message.master_address = CMessageBox::MOD2_ADDR;
                message.own_address = CMessageBox::MASTER_ADDR;

                uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
                emit consolePutData(QString(":: Predistortion auto cfg :: Send 'SEND_SWEEP_SIGNAL' command to MOD\n"), 2);
                postDataToStm32H7(message_box_buffer_mod, tx_len);

                // Start timeout before next command
                emit startAnswerTimeoutTimer(timeoutAgcSweepCommands_ms);
                break;
            }
            /* fallthrough */

            //====================================================================================================================

            default:
                emit consolePutData(":: Predistortion auto cfg :: error wrong state\n", 2);
                break;
        }

        m_mutex_mod.unlock();
    }
}

void ModTransmitterThread::ModStartTransmitPhaseGain()
{
    m_mutex_mod.lock();

    if(State != IDLE
       && State != AUTOCFG_COMPLETE_SUCCESSFULLY
       && State != ERROR_AGC_MODSTAT
       && State != ERROR_SWEEP_TIMEOUT
       && State != ERROR_AGC_SWEEP
       && State != ERROR_FREQ_ESTIMATE_TIMEOUT
       && State != ERROR_AGC_SIN35KHZ)
    {
        emit consolePutData("Error: unable to start transmitting phase & gain tables, state != IDLE (already transmitting?)\n", 2);
        m_mutex_mod.unlock();
        return;
    }

    setState(START_TX_PREDISTORTION_TABLES_TO_MOD);

    m_mutex_mod.unlock();
    modTransmitWakeUp.wakeOne();
}

void ModTransmitterThread::transmitPredistortionTables()
{
    if(retry)
    {
        retry = false;
        if(++n_attempts == n_MaxAttempts)
        {
            n_attempts = 0;

            if(++n_attempts_high_level > n_MaxAttemptsHighLevel)
            {
                emit consolePutData(":: Predistortion auto cfg :: failed to transmit predistorion tables to MOD\n", 2);
                setState(ERROR_PREDISTORTION_TABLES_TX_FAILED);
                calculatePredistortionTablesStop();
                return;
            }

            emit consolePutData(QString(":: Predistortion auto cfg :: error attempts exceeded %1, start again from first command\n").arg(n_MaxAttempts), 2);
            emit consolePutData(":: Predistortion auto cfg :: Starting transmit 'phase table' \n", 2);
            StatePredistTx = TX_PHASE_TABLE;
            n_channel = 0;
        }
    }
    else
    {
        n_attempts = 0;

        switch(StatePredistTx)
        {
            case TX_START:
                emit consolePutData(QString(":: Predistortion auto cfg :: Starting transmit predistortion tables, attempt %1\n").arg(n_attempts_high_level), 2);
                emit consolePutData(":: Predistortion auto cfg :: Starting transmit 'phase table' \n", 2);

                StatePredistTx = TX_PHASE_TABLE;
                n_channel = 0;

//                setState(AGC_START_FOR_MOD_STAT);
//                emit consolePutData(":: Predistortion auto cfg :: Mod predistortion tables and 'shift + crc' transmission completed\n", 2);

//                // Wait some time before AGC, so MOD could apply new predistortion tables
//                emit startAnswerTimeoutTimer(500);
//                return;
                break;

            case TX_PHASE_TABLE:
                //n_channel = n_elements/16 - 1;
                if(++n_channel == n_elements/8)
                {
                    emit consolePutData(":: Predistortion auto cfg :: Finished transmitting 'phase table', transmit 'gain table' next\n", 2);
                    StatePredistTx = TX_GAIN_TABLE;
                    n_channel = 0;
                }
                break;

            case TX_GAIN_TABLE:
                //n_channel = n_elements/16 - 1;
                if(++n_channel == n_elements/8)
                {
                    emit consolePutData(":: Predistortion auto cfg :: Finished transmitting 'gain table', transmit 'shift + crc' next\n", 2);
                    StatePredistTx = TX_SHIFT_CRC;
                }
                //emit startAnswerTimeoutTimer(5000);
                break;

            case TX_SHIFT_CRC:
                emit consolePutData(":: Predistortion auto cfg :: Mod predistortion tables and 'shift + crc' transmission completed\n", 2);
                StatePredistTx = TX_FINISHED;

                emit consolePutData(":: Predistortion auto cfg :: send 'AGC stop'\n", 2);
                emit sendCommandToSTM32(USB_CMD_AGC_STOP, nullptr, 0);

                // Start AGC for final configuration
                setState(AGC_START_FOR_MOD_STAT);

                // Wait some time before AGC, so MOD could apply new predistortion tables
                emit startAnswerTimeoutTimer(500);
                return;

             default:
                emit consolePutData(":: Predistortion auto cfg :: error wrong state\n", 2);
                break;
        }
    }

    // Send next command
    switch(StatePredistTx)
    {
        case TX_PHASE_TABLE:
        case TX_GAIN_TABLE:
        {
            message.command = (StatePredistTx == TX_PHASE_TABLE) ? CMessageBox::SET_PHASE_TABLE : CMessageBox::SET_GAIN_TABLE;
            message.packet_adr = n_channel;

            uint8_t *p_f;
            uint32_t j = 0;

            for(uint32_t i = 0; i < 8; ++i)
            {
                if(StatePredistTx == TX_PHASE_TABLE)
                    p_f = reinterpret_cast<uint8_t*>(&phase_resamp_data[n_channel*8 + i]);
                else //if(State == TRANSMIT_GAIN)
                    p_f = reinterpret_cast<uint8_t*>(&gain_resamp_data[n_channel*8 + i]);

                message_box_buffer_mod[11 + j++] = p_f[0];
                message_box_buffer_mod[11 + j++] = p_f[1];
                message_box_buffer_mod[11 + j++] = p_f[2];
                message_box_buffer_mod[11 + j++] = p_f[3];
            }
            break;
        }

        case TX_SHIFT_CRC:
        {
            message.command = CMessageBox::SET_SHIFT_QAM_DATA;
            message.packet_adr = 0;

            message_box_buffer_mod[11] = uint8_t(shift_for_qam_data_int & 0xFF);
            message_box_buffer_mod[12] = uint8_t((shift_for_qam_data_int >> 8) & 0xFF);

            // Calculate crc16 for phase & gain tables and shift
            calc_crc16((uint8_t*)&phase_resamp_data, sizeof(phase_resamp_data));
            calc_crc16_continue((uint8_t*)&gain_resamp_data, sizeof(gain_resamp_data));
            uint16_t crc16 = calc_crc16_continue((uint8_t*)&shift_for_qam_data_int, sizeof(shift_for_qam_data_int));

            message_box_buffer_mod[13] = uint8_t(crc16 & 0xFF);
            message_box_buffer_mod[14] = uint8_t((crc16 >> 8) & 0xFF);
        }
            break;

        default:
            emit consolePutData(":: Predistortion auto cfg :: error wrong state\n", 2);
            break;
    }

    hs_data_received = false;

    uint16_t tx_len = CMessageBox::message_header_to_array(&message, message_box_buffer_mod);
    postDataToStm32H7(message_box_buffer_mod, tx_len);

    // Start answer timeout
    emit startAnswerTimeoutTimer(timeoutAnswer_ms);
}

void ModTransmitterThread::ModAnswerDataReceived()
{
    m_mutex_mod.lock();
    if(State != IDLE)
        hs_data_received = true;
    m_mutex_mod.unlock();
}

void ModTransmitterThread::timeoutAnswer()
{
//    emit consolePutData("Mod answer timeout\n", 1);

    m_mutex_mod.lock();

    if(State == IDLE)// || State == AGC_START_FOR_MOD_STAT)
    {
        m_mutex_mod.unlock();
        return;
    }

    else if(State == TX_PREDISTORTION_TABLES_TO_MOD)
    {
        if(!hs_data_received)
        {
            emit consolePutData(":: Predistortion auto cfg :: retry previous transfer\n", 2);
            retry = true;
        }
        else
        {
            // Some HS data (probably broken) received
            // Assume it is an answer from MOD
            //emit consolePutData(":: Predistortion auto cfg :: some HS data was received, assume it was a good answer from MOD\n", 2);
            emit consolePutData(QString(":: Predistortion auto cfg :: some HS data was received, assume it was a good answer from MOD %1\n").arg(n_channel), 2);
            hs_data_received = false;
        }
    }
    m_mutex_mod.unlock();

    // Get back to thread run()
    modTransmitWakeUp.wakeOne();
}

// Public
void ModTransmitterThread::calculatePredistortionTablesStart()
{
    m_AutoConfigurationMode = true;

    emit consolePutData("==================================================\n"
                        "Starting predistortion auto configuration sequence\n"
                        "==================================================\n", 2);
    m_mutex_mod.lock();
    setState(AUTOCFG_START);
    StatePredistTx = TX_IDLE;
    m_mutex_mod.unlock();
    modTransmitWakeUp.wakeOne();
}

void ModTransmitterThread::calculatePredistortionTablesContinue()
{
    m_AutoConfigurationMode = true;
//    if(m_AutoConfigurationMode)
//    {
//        emit consolePutData("Error: unable to start user requested AGC configuration, because autoconfiguration is in progress\n", 2);
//        return;
//    }

//    emit consolePutData("==================================================\n"
//                        "Starting predistortion auto configuration sequence\n"
//                        "==================================================\n", 2);

    emit consolePutData("Continue predistortion auto configuration sequence\n", 2);
    m_mutex_mod.lock();
    //setState(AUTOCFG_START);
    StatePredistTx = TX_IDLE;
    m_mutex_mod.unlock();
    modTransmitWakeUp.wakeOne();
}

void ModTransmitterThread::separateAgcStart()
{
    if(m_AutoConfigurationMode)
    {
        emit consolePutData("Error: unable to start user requested AGC configuration, because autoconfiguration is in progress\n", 2);
        return;
    }

    m_AutoConfigurationMode = true;
    emit consolePutData("==================================================\n"
                        "Starting AGC configuration sequence\n"
                        "==================================================\n", 2);
    m_mutex_mod.lock();
    setState(AGCCFG_START);
    StatePredistTx = TX_IDLE;
    m_mutex_mod.unlock();
    modTransmitWakeUp.wakeOne();
}

void ModTransmitterThread::separateRecordSweepStart()
{
    if(m_AutoConfigurationMode)
    {
        emit consolePutData("Error: unable to start user requested 'sweep' signal record sequence, because autoconfiguration is in progress\n", 2);
        return;
    }

    m_AutoConfigurationMode = true;
    emit consolePutData("==================================================\n"
                        "Starting 'sweep' signal record sequence\n"
                        "==================================================\n", 2);
    m_mutex_mod.lock();
    setState(SPECIAL_USR_REQ_START);
    StatePredistTx = TX_IDLE;
    m_mutex_mod.unlock();
    modTransmitWakeUp.wakeOne();
}

// Private
void ModTransmitterThread::calculatePredistortionTablesStop()
{
    // Check AGC state, stop AGC if needed
    uint8_t n_retrys = 2;
    uint8_t StateAGC;

    while(1)
    {
        StateAGC = AgcStateGlobal;
        if(StateAGC == AGC_OK || StateAGC == AGC_STOP)
            break;

        if(!n_retrys)
        {
            emit consolePutData(":: Predistortion auto cfg :: error 'AGC stop' failed\n", 2);
            break;
        }

        emit consolePutData(":: Predistortion auto cfg :: send 'AGC stop'\n", 2);
        emit sendCommandToSTM32(USB_CMD_AGC_STOP, nullptr, 0);
        QThread::msleep(1000);
        emit sendCommandToSTM32(USB_CMD_GET_STATUS, nullptr, 0);
        QThread::msleep(1000);

        --n_retrys;
    }

    StatePredistTx = TX_IDLE;
    emit consolePutData(":: Predistortion auto cfg :: enable QAM decoder ring buffer\n", 2);

    // Unlock ring buffer for QAM decoder (enable QAM decoder)
    m_ring->SetActive(true);

    // QAM decoder set first pass flag
    emit qamDecoderReset();

    is_auto_config_work = false;
    m_AutoConfigurationMode = false;
}
