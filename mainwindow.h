/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <message_box.h>
#include <usb_workthread.h>
#include <qam_thread.h>
#include <sin_freq_sweep_thread.h>
#include <mod_transmitter_thread.h>
#include <gpio_tracker.h>
#include <blackbox.h>

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class MessageBox;
class GpioTracker;
class BlackBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    bool openSerialPort();
    void closeSerialPort();
    void logFileFlush();
    void logFileOpen();
    void openLogsFolder();

    void sendHsCommandAdcStart();       // command to STM32
    void sendHsCommandAdcStart2();      // command to STM32
    void sendHsCommandAdcStart3();      // command to STM32
    void sendHsCommandAgcStart();       // command to STM32
    void recordSweep();                 // command to STM32
    void readDataSerialPort();
    void transmitDataSerialPort(const uint8_t *p_data, int length);
    void transmitWaitToSerialPort();
    void postTxDataSTM(const uint8_t *p_data, const int length);
    void on_actionShow_advanced_logs_in_console_triggered(bool checked);

    // Commands from Indigo Suite
    void commandCalculatePredistortionTablesStart();
    void commandCalculatePredistortionTablesContinue();
    void commandAgcStart();
    void srpModeSet(uint8_t mode);

    void handleError(QSerialPort::SerialPortError error);
    void consolePutData(const QString &data, quint8 priority);
    void consoleAdcData(const quint8 *p_data, quint32 size);
    void consoleDataAdcSpecial(const qint16 *p_data, quint32 len, quint8 type);
    void consoleDataAdcSpecial_test(const double *p_data, quint32 len, quint8 type);
    void consoleDataEqualizFilter(const creal_T *p_data, const creal_T *eql_data, quint32 len, quint8 flag);
    void consoleFilterCoeff(const creal_T *p_data, quint32 len, quint8 flag);
    void timeoutSerialPortReconnect();
    void timeoutUsbInitCallback();
    void timeoutGpioCallback();
    void usbHsDataReceived();
    void sendCommandToSTM32(quint8 command, const quint8 *p_data, quint32 data_size);
    void qamDecoderReset();

    void usbInitTimeoutStart(int timeout_ms);

    void on_actionReset_statistics_in_status_bar_triggered();

private:
    void initActionsConnections();

private:
    void showStatusMessage(const QString &message);
    void parseDataSerialPort();
    void serialPortRxCleanup();
    int shutdownProcedure();
    void secondElapsedCallback();

    static bool frame_builded_cb(const void *param, const uint8_t *buffer, uint32_t size);

    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;

    Console *m_console = nullptr;
    QSerialPort *m_serial = nullptr;
    CMessageBox *m_message_box = nullptr;
    UsbWorkThread m_usb_thread;
    QamThread m_qam_thread;
    SinFreqSweepThread m_freq_sweep_thread;
    ModTransmitterThread m_mod_tx_thread;
    GpioTracker *m_gpio_shutdown = nullptr;
    GpioTracker *m_gpio_output = nullptr;
    BlackBox m_blackbox;

    QByteArray TtyUserRxBuffer;

    quint64 TtyUserRxBuffer_MaxSize = 4096;
    quint64 TtyUserRxBuffer_len = 0;

    QTimer *timerSpReconnect;
    QTimer *timerUsbPoll;
    QTimer *timerUsbInit;
    QTimer *timerModAnswerTimeout;
    QTimer *timerGpioTracking;

    const int timeoutSerialPortReconnect_ms = 1000;     // timeout between serial port reconnection attempts (in case of error)
    const int timeoutGpioTrackingPeriod_ms = 1000;      // period for checking GPIO state

    uint8_t ShutdownTime = 0;

#ifdef QUICK_ANSWERS_ENABLED
    uint8_t quick_answer[2] = {0x00, 0x1d};
    bool transmit_quick_answer = false;
#endif
};

#endif // MAINWINDOW_H
