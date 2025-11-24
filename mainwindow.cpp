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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "../SRP_HS_USB_PROTOCOL/SRP_HS_USB_Protocol.h"

#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <QFileDialog>
#include "crc16.h"
#include "atomic_vars.h"
#include "synchro_watcher.h"
#include "indigo_base_protocol.h"
#include "statistics.h"
#include <blackbox.h>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

// Extern variables
extern QElapsedTimer profiler_timer;
extern RingBuffer *m_ring;
extern qint64 elapsed_all_saved;
extern IndigoBaseProtocolBuilderV1 g_frame_builder;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_console(new Console),
    m_serial(new QSerialPort(this)),
    m_message_box(new CMessageBox),
    m_gpio_shutdown(new GpioTracker(26, 0, this)),
    m_gpio_output(new GpioTracker(20, 1, this))
{
    m_ui->setupUi(this);

    m_console->setEnabled(false);
    m_console->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_console);
    m_ui->tab_1->setLayout(layout);

    m_ui->statusBar->addWidget(m_status);

    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readDataSerialPort);
    connect(m_message_box, &CMessageBox::postData, this, &MainWindow::transmitDataSerialPort);
    connect(m_message_box, &CMessageBox::postDataToStm32H7, this, &MainWindow::postTxDataSTM);
    connect(m_message_box, &CMessageBox::commandCalculatePredistortionTablesStart, this, &MainWindow::commandCalculatePredistortionTablesStart);
    connect(m_message_box, &CMessageBox::commandCalculatePredistortionTablesContinue, this, &MainWindow::commandCalculatePredistortionTablesContinue);
    connect(m_message_box, &CMessageBox::commandAgcStart, this, &MainWindow::commandAgcStart);
    connect(m_message_box, &CMessageBox::srpModeSet, this, &MainWindow::srpModeSet);

    connect(m_gpio_shutdown, &GpioTracker::consolePutData, this, &MainWindow::consolePutData);
    connect(m_gpio_output, &GpioTracker::consolePutData, this, &MainWindow::consolePutData);
    connect(&m_usb_thread, &UsbWorkThread::postTxDataToSerialPort, this, &MainWindow::transmitDataSerialPort, Qt::ConnectionType::QueuedConnection);
    connect(&m_usb_thread, &UsbWorkThread::postWaitToSerialPort, this, &MainWindow::transmitWaitToSerialPort, Qt::ConnectionType::QueuedConnection);
    connect(&m_usb_thread, &UsbWorkThread::consolePutData, this, &MainWindow::consolePutData, Qt::ConnectionType::QueuedConnection);
    connect(&m_usb_thread, &UsbWorkThread::usbInitTimeoutStart, this, &MainWindow::usbInitTimeoutStart, Qt::ConnectionType::QueuedConnection);
    connect(&m_usb_thread, &UsbWorkThread::consoleAdcFile, this, &MainWindow::consoleAdcData, Qt::ConnectionType::QueuedConnection);
    connect(&m_usb_thread, &UsbWorkThread::hsDataReceived, this, &MainWindow::usbHsDataReceived, Qt::ConnectionType::QueuedConnection);
    connect(&m_usb_thread, &UsbWorkThread::srpModeSet, this, &MainWindow::srpModeSet, Qt::ConnectionType::QueuedConnection);
    connect(&m_qam_thread, &QamThread::consolePutData, this, &MainWindow::consolePutData, Qt::ConnectionType::QueuedConnection);
    connect(&m_qam_thread, &QamThread::consoleFrameErrorFile, this, &MainWindow::consoleDataAdcSpecial, Qt::ConnectionType::QueuedConnection);
    connect(&m_qam_thread, &QamThread::postTxDataToSerialPort, this, &MainWindow::transmitDataSerialPort, Qt::ConnectionType::QueuedConnection);
    connect(&m_freq_sweep_thread, &SinFreqSweepThread::consolePutData, this, &MainWindow::consolePutData, Qt::ConnectionType::QueuedConnection);
    connect(&m_freq_sweep_thread, &SinFreqSweepThread::consolePutAdcDataSpecial, this, &MainWindow::consoleDataAdcSpecial, Qt::ConnectionType::QueuedConnection);

    connect(&m_mod_tx_thread, &ModTransmitterThread::consolePutData, this, &MainWindow::consolePutData, Qt::ConnectionType::QueuedConnection);
    connect(&m_mod_tx_thread, &ModTransmitterThread::postDataToStm32H7, this, &MainWindow::postTxDataSTM, Qt::ConnectionType::QueuedConnection);
    connect(&m_mod_tx_thread, &ModTransmitterThread::sendCommandToSTM32, this, &MainWindow::sendCommandToSTM32, Qt::ConnectionType::QueuedConnection);
    connect(&m_mod_tx_thread, &ModTransmitterThread::qamDecoderReset, this, &MainWindow::qamDecoderReset, Qt::ConnectionType::QueuedConnection);

    m_console->putData(QString("SBC Application v.") + VERSION_STRING + "\n", 2);
    m_console->putData("Opening serial port...\n", 2);

    // Clear serial port tx & rx buffer
    TtyUserRxBuffer.clear();

    // Timers
    // Reconnect to serial port in case of error
    timerSpReconnect = new QTimer();
    timerSpReconnect->setSingleShot(true);
    connect(timerSpReconnect, SIGNAL(timeout()), this, SLOT(timeoutSerialPortReconnect()));

    // Usb initialization timer
    timerUsbInit = new QTimer();
    timerUsbInit->setSingleShot(true);
    connect(timerUsbInit, SIGNAL(timeout()), this, SLOT(timeoutUsbInitCallback()));

    // Gpio tracking tmer
    timerGpioTracking = new QTimer();
    connect(timerGpioTracking, SIGNAL(timeout()), this, SLOT(timeoutGpioCallback()));

    // Indigo base protocol initialization
    g_frame_builder.Initialize();
    g_frame_builder.SetNotify(frame_builded_cb, this);

    openSerialPort();

    // LibUsb initialization, init usb device
    /* Get libusb version */
    const struct libusb_version *version;
    version = libusb_get_version();
    m_console->putData(QString("libusb version: %1.%2.%3.%4\n")
                       .arg(version->major).arg(version->minor)
                       .arg(version->micro).arg(version->nano), 2);

    // Start timer for continuous usb initialization attempts
    timerUsbInit->start(m_usb_thread.timeoutUsbInit_ms);

    // Init & read BlackBox from file
    m_blackbox.Init();
    m_blackbox.Read();

    // Start QAM decoder thread
    m_qam_thread.start();

    // Start QAM frequency estimate for sweep thread
    m_freq_sweep_thread.start();

    // Start MOD commands transmitter thread
    m_mod_tx_thread.start();

    // Timeout timer for MOD answers
    timerModAnswerTimeout = new QTimer(this);
    timerModAnswerTimeout->setSingleShot(true);
    timerModAnswerTimeout->connect(&m_mod_tx_thread, SIGNAL(startAnswerTimeoutTimer(int)), SLOT(start(int)), Qt::ConnectionType::QueuedConnection);
    timerModAnswerTimeout->connect(&m_mod_tx_thread, SIGNAL(stopAnswerTimeoutTimer()), SLOT(stop()), Qt::ConnectionType::QueuedConnection);

    m_mod_tx_thread.connect(timerModAnswerTimeout, SIGNAL(timeout()), SLOT(timeoutAnswer()));

    // Init & start GPIO monitoring
    if(m_gpio_shutdown->Init() && m_gpio_output->Init())
    {
        m_console->putData("GPIO initialization successfull\n", 2);

        // Start input (m_gpio_shutdown) monitoring
        timerGpioTracking->start(timeoutGpioTrackingPeriod_ms);
    }
    else
        m_console->putData("Error: GPIO initialization failed\n", 2);

    // Set gpio output (m_gpio_output) to 1
    m_gpio_output->writeValue(1);
}

MainWindow::~MainWindow()
{
    timerUsbInit->stop();
    timerSpReconnect->stop();
    timerGpioTracking->stop();

    closeSerialPort();
    m_usb_thread.USB_Deinit();

    m_console->Close();
    delete m_ui;
}

void MainWindow::consolePutData(const QString &data, quint8 priority)
{
    if(m_console == nullptr)
        return;

    m_console->putData(data, priority);
}

void MainWindow::consoleAdcData(const quint8 *p_data, quint32 size)
{
    if(m_console == nullptr)
        return;

    m_console->putDataAdc(p_data, size);
}

void MainWindow::consoleDataAdcSpecial(const qint16 *p_data, quint32 len, quint8 type)
{
    if(m_console == nullptr)
        return;

    m_console->putDataAdcSpecial(p_data, len, type);
}
void MainWindow::consoleDataAdcSpecial_test(const double *p_data, quint32 len, quint8 type)
{
    if(m_console == nullptr)
        return;

    m_console->putDataAdcSpecial_test(p_data, len, type);
}

bool MainWindow::openSerialPort()
{
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };

    Settings settings;
    settings.name = "ttyGS0";
    settings.baudRate = 10000000;//460800;
    settings.stringBaudRate = "460800";
    settings.dataBits = QSerialPort::Data8;
    settings.flowControl = QSerialPort::NoFlowControl;
    ///settings.localEchoEnabled = true;
    settings.localEchoEnabled = false;
    settings.parity = QSerialPort::NoParity;
    settings.stopBits = QSerialPort::OneStop;
    settings.stringDataBits = "8";
    settings.stringFlowControl= "None";
    settings.stringParity = "None";
    settings.stringStopBits = "1";

    const Settings p = settings;
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);

    if (m_serial->open(QIODevice::ReadWrite))
    {
        m_console->setEnabled(true);
        m_console->putData(tr("Connected to %1 : %2, %3, %4, %5, %6\n")
                           .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                           .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl), 2);
        return true;
    }
    else
    {
        QByteArray str_b;
        str_b.append(tr("Error "));
        str_b.append(m_serial->errorString());
        str_b.append("\n");
        m_console->putData(str_b, 2);
        return false;
    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    //showStatusMessage(tr("Disconnected"));
    m_console->putData("Serial port disconnected\n", 2);
}

void MainWindow::timeoutUsbInitCallback()
{
    m_usb_thread.USB_Init();
}

void MainWindow::timeoutGpioCallback()
{
    // 1 second passed
    if(m_gpio_shutdown->readValue())
    {
        // Shutdown pin is set
        ++ShutdownTime;
        m_console->putData(QString("Shutdown pin is set for %1 sec\n").arg(ShutdownTime), 2);
    }
    else
    {
        // Shutdown pin is reset
        ShutdownTime = 0;
    }

    if(ShutdownTime > 3)
    {
        m_console->putData(QString("Shutdown in progress...\n"), 2);

        timerUsbInit->stop();
        timerSpReconnect->stop();
        timerGpioTracking->stop();

        m_console->Close();

        shutdownProcedure();

        // In case shutdown failed, close application
        MainWindow::close();
    }

    secondElapsedCallback();
}

void MainWindow::secondElapsedCallback()
{
    // Show transfer statistics in status bar
    static uint16_t prev_QamFramesCounter = 65535;
    //++m_QamFramesCounter; //test delete todo
    if(m_QamFramesCounter != prev_QamFramesCounter)
    {
        prev_QamFramesCounter = m_QamFramesCounter;

        static uint8_t cnt = 5;
        if(++cnt == 6)
            cnt = 0;

        QString squares = "";

        switch(cnt)
        {
            case 0: squares.append("⬜ ⬜ ⬜"); break;
            case 1: squares.append("⬛ ⬜ ⬜"); break;
            case 2: squares.append("⬛ ⬛ ⬜"); break;
            case 3: squares.append("⬛ ⬛ ⬛"); break;
            case 4: squares.append("⬜ ⬛ ⬛"); break;
            case 5: squares.append("⬜ ⬜ ⬛"); break;
        }

        showStatusMessage(squares + QString("  crc errors %1, rs successful corrections %2").arg(m_CrcErrorsCounter).arg(m_ReedSolomonCorrectionsCounter));
    }

    // Periodically save estimated frequency to file
    static uint16_t time_sec = 0;
    if(++time_sec == 60*10)    // 10 minutes
    {
        time_sec = 0;
        m_blackbox.Save();
    }
}

int MainWindow::shutdownProcedure()
{
    QDBusInterface logind{"org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus()};
    const auto message = logind.callWithArgumentList(QDBus::Block, "CanPowerOff", {});
    QDBusPendingReply<QString> canPowerOff = message;
    Q_ASSERT(canPowerOff.isFinished());

    if(canPowerOff.isError())
    {
        const auto error = canPowerOff.error();
        qWarning().noquote()
                << QDBusInterface::tr("Asynchronous call finished with error: %1 (%2)")
                   .arg(error.name(), error.message());
        return EXIT_FAILURE;
    }

    if (canPowerOff.value() == "yes")
    {
        QDBusPendingReply<> powerOff = logind.callWithArgumentList(QDBus::Block, "PowerOff", {true, });
        Q_ASSERT(powerOff.isFinished());
        if(powerOff.isError())
        {
            const auto error = powerOff.error();
            qWarning().noquote()
                    << QDBusInterface::tr("Asynchronous call finished with error: %1 (%2)")
                       .arg(error.name(), error.message());
            return EXIT_FAILURE;
        }
    }
    else
    {
        qCritical().noquote()
                << QCoreApplication::translate("poweroff", "Can't power off: CanPowerOff() result is %1")
                   .arg(canPowerOff.value());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void MainWindow::usbInitTimeoutStart(int timeout_ms)
{
    timerUsbInit->start(timeout_ms);
}

void MainWindow::timeoutSerialPortReconnect()
{
    m_console->putData("SP Trying to reconnect to a serial port...\n", 2);
    if(!openSerialPort())
    {
        // Start timer again
        timerSpReconnect->start(timeoutSerialPortReconnect_ms);
    }
}

void MainWindow::serialPortRxCleanup()
{
    TtyUserRxBuffer_len = 0;
    TtyUserRxBuffer.clear();
}

void MainWindow::readDataSerialPort()
{
    // Serial port data received (from PC)
    profiler_timer.start();

    qint64 length_rx = m_serial->bytesAvailable();

    do {
        while(length_rx)
        {
            if(TtyUserRxBuffer_len)
                m_console->putData("SP Received " + QString::number(TtyUserRxBuffer_len) + " + " + QString::number(length_rx) + " bytes\n", 0);
            else
            {
                m_console->putData("SP Received " + QString::number(length_rx) + " bytes\n", 0);
            }

            TtyUserRxBuffer.append(m_serial->readAll());
            TtyUserRxBuffer_len += length_rx;
            length_rx = m_serial->bytesAvailable();

            // Protection from infinite loop
            if(TtyUserRxBuffer_len > 65536) // 64 KBytes of data in single packet
            {
                m_console->putData("Error: serial port buffer overflow detected, drop data\n", 2);
                serialPortRxCleanup();
                return;
            }
        }

        // Continue receive
        QThread::msleep(1);
        length_rx = m_serial->bytesAvailable();

    } while(length_rx);

    parseDataSerialPort();
}

void MainWindow::parseDataSerialPort()
{
//    profiler_timer.start();
    uint8_t addr = TtyUserRxBuffer.at(0);

#ifdef QUICK_ANSWERS_ENABLED
    // If package for listed tools, response with 'quick answer'
    if (addr == m_message_box->PT_ADDR ||
        addr == m_message_box->SILS_ADDR ||
        addr == m_message_box->SFBS_ADDR ||
        addr == m_message_box->NAV_ADDR ||
        addr == m_message_box->ReCap_ADDR ||
        addr == m_message_box->XYC_ADDR ||
        addr == m_message_box->HEX_ADDR ||
        addr == m_message_box->Ind1h_ADDR)
    {
        quick_answer[0] = addr;
        transmit_quick_answer = true;
    }
#endif
    // If package for SRP2
    if (addr == m_message_box->SRP2_ADDR)
    {
        // Check length
        if(TtyUserRxBuffer_len >= TtyUserRxBuffer_MaxSize)
            TtyUserRxBuffer_len = TtyUserRxBuffer_MaxSize;

        // len < minimum packet size
        if(TtyUserRxBuffer_len < m_message_box->TX_HEADER_LENGTH + m_message_box->DATA_ADDRESS_LENGTH + m_message_box->CRC_LENGTH)
        {
            m_console->putData("SP Broken packet from PC to SRP: length is too short\n", 2);
            serialPortRxCleanup();
            return;
        }

        // Check crc
        uint16_t crc16 = uint16_t(TtyUserRxBuffer.at(TtyUserRxBuffer_len - 2) & 0xFF) | (uint16_t(TtyUserRxBuffer.at(TtyUserRxBuffer_len - 1)) << 8);

        const uint8_t *p_data = (const uint8_t*)TtyUserRxBuffer.data();
        if(calc_crc16(p_data + 1, TtyUserRxBuffer_len-1-m_message_box->CRC_LENGTH) != crc16)
        {
            m_console->putData("SP Broken packet from PC to SRP: crc error\n", 2);
            serialPortRxCleanup();
            return;
        }

        // Crc ok, continue to message box
        if(m_message_box->message_box_srp((uint8_t*)TtyUserRxBuffer.data(), uint16_t(TtyUserRxBuffer_len), m_message_box->MASTER_ADDR, m_message_box->SRP2_ADDR))
        {
        }
        else
        {
            // data is being transmitted to STM32H7, usb_receiver_drop flag will be reset after the transfer
        }

        serialPortRxCleanup();
        return;
    }

    // Data for other tools

    // Check if synchronization 'suspended time' is in progress
    if(syncIsSuspendedTimeInProgress())
    {
        uint16_t suspended_time_left = syncGetTime();
        if(suspended_time_left)
        {
            m_console->putData(QString("Sync: suspended time is in progress: answer with 'CmdWaitAgainSend' susp.time left %1\n").arg(suspended_time_left), 1);

            // Answer with 'wait' (indigo base protocol answer)
            command_sync_wait_creator(suspended_time_left, EnumCmdWaitAgainSend);

            // Clean serial port rx buffer
            serialPortRxCleanup();
            return;
        }
        else
        {
            // Uncomment if we get rid of 'USB_CMD_SYNCHRO_STOP'
            //syncSetSuspendedTimeInProgress(false, 0);
        }
    }

    // Check length
    if(TtyUserRxBuffer_len > TtyUserRxBuffer_MaxSize)
    {
        TtyUserRxBuffer_len = TtyUserRxBuffer_MaxSize;
        m_console->putData("SP Warning: received length is too big\n", 2);
    }

    // Transmit to STM32H7
    postTxDataSTM((uint8_t*)TtyUserRxBuffer.data(), int(TtyUserRxBuffer_len));
    serialPortRxCleanup();

#ifdef QUICK_ANSWERS_ENABLED
    if(transmit_quick_answer)
    {
        transmit_quick_answer = false;
        transmitDataSerialPort(quick_answer, sizeof(quick_answer));
    }
#endif
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    switch(error)
    {
        case QSerialPort::NoError:
            break;

        default:
            m_console->putData("Serial port error: " + m_serial->errorString() + "\n", 2);
            closeSerialPort();

            // Start timer to periodically try to reconnect
            timerSpReconnect->start(timeoutSerialPortReconnect_ms);
            break;
    }
}

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionLogsFlush, &QAction::triggered, this, &MainWindow::logFileFlush);
    connect(m_ui->actionLogsOpenFile, &QAction::triggered, this, &MainWindow::logFileOpen);
    connect(m_ui->actionOpen_logs_folder, &QAction::triggered, this, &MainWindow::openLogsFolder);
    connect(m_ui->actionADC_START, &QAction::triggered, this, &MainWindow::sendHsCommandAdcStart);
    connect(m_ui->actionSend_AGC_Start, &QAction::triggered, this, &MainWindow::sendHsCommandAgcStart);
    connect(m_ui->actionADC_START_for_SIN_600, &QAction::triggered, this, &MainWindow::sendHsCommandAdcStart2);
    connect(m_ui->actionADC_START_for_Sweep, &QAction::triggered, this, &MainWindow::sendHsCommandAdcStart3);
    connect(m_ui->actionRecord_Sweep_to_file, &QAction::triggered, this, &MainWindow::recordSweep);
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

// Serial port to PC
// Asynchronous transmit
void MainWindow::transmitDataSerialPort(const uint8_t *p_data, int length)
{
    m_serial->write((const char*)p_data, length);
    m_serial->flush();  // Sending the buffered data immediately to the serial port

    // Logs
#ifdef QT_DEBUG
    m_console->putData("SP Transmit " + QString::number(length) + " bytes\n", 0);
    m_console->putData(QString("SP Tx data:"), 0);
    uint8_t len_cut = length > 254 ? 254 : uint8_t(length);
    QString d;
    for(uint8_t i = 0; i < len_cut; ++i)
        d.append(QString("%1 ").arg(p_data[i], 2, 16, QLatin1Char('0')));
    d.append("\n");
    m_console->putData(d, 0);
#endif

    qint64 pt_elapsed = profiler_timer.elapsed();
    m_console->putData(QString("Profiler timer elapsed %1 # transmit to sp end, without qam %2\n").arg(pt_elapsed).arg(pt_elapsed - elapsed_all_saved), 0);
}

// USB to STM32H7 board
void MainWindow::postTxDataSTM(const uint8_t *p_data, const int length)
{
    int len = length;

    if(p_data == nullptr)
    {
        m_console->putData("Error postTxDataToSTM32H7 reported p_data = NULL\n", 2);
        return;
    }

    USBheader_t header;

    if(len + sizeof(header) > int(sizeof(m_usb_thread.UserTxBuffer)))
    {
        m_console->putData("Error postTxDataToSTM32H7 reported length is too long\n", 2);
        len = sizeof(m_usb_thread.UserTxBuffer);
    }

    if(len > int(sizeof(USB_Protocol::data)))
    {
        m_console->putData("Error postTxDataToSTM32H7 reported length is too long\n", 2);
        len = sizeof(USB_Protocol::data);
    }

    // Cancel ongoing transfer (if any)
    m_usb_thread.USB_StopTransmit();

    m_usb_thread.UserTxBuffer_len = len + sizeof(header);

    header.packet_length = m_usb_thread.UserTxBuffer_len;
    header.type = SRP_LS_DATA;
    header.cmd = 0;  // Don't care

    memcpy(m_usb_thread.UserTxBuffer, (uint8_t*)&header, sizeof(header));
    memcpy(m_usb_thread.UserTxBuffer + sizeof(header), p_data, len);

    m_console->putData("Transmit to H7 " + QString::number(m_usb_thread.UserTxBuffer_len) + " bytes\n", 0);
    m_usb_thread.start_transmit = true;
}

void MainWindow::transmitWaitToSerialPort()
{
    uint16_t suspended_time_left = syncGetTime();
    if(suspended_time_left)
    {
        command_sync_wait_creator(suspended_time_left, EnumCmdWaitRead);
    }
    m_console->putData(QString("Sync: suspended time is in progress and qam data available, send 'CmdWaitRead' susp.time left %1 ms\n").arg(suspended_time_left), 1);
}

// Actions
void MainWindow::logFileFlush()
{
    m_console->fileFlush();
    QMessageBox::information(this, tr("Info"), tr("Log file has been flushed!"));
}

void MainWindow::logFileOpen()
{
    m_console->fileOpen();
}

void MainWindow::openLogsFolder()
{
    m_console->openLogsFolder();
}

void MainWindow::sendCommandToSTM32(quint8 command, const quint8 *p_data, quint32 data_size)
{
    if(data_size && p_data == nullptr)
    {
        m_console->putData("Error sendCommandToSTM32sendCommandToSTM32 wrong parameters\n", 2);
        return;
    }

    if(command == USB_CMD_AGC_START)
        m_usb_thread.agc_is_active = true;

    m_usb_thread.sendHsCommand(command, data_size, p_data);
}

void MainWindow::usbHsDataReceived()
{
    m_mod_tx_thread.ModAnswerDataReceived();
}

// Debug HS command from GUI
void MainWindow::sendHsCommandAdcStart()
{
    uint32_t adc_data_length = 440000;
    m_console->putData(QString("Send HS command USB_CMD_ADC_START, adc_data_length %1\n").arg(adc_data_length), 2);
    m_usb_thread.sendHsCommand(USB_CMD_ADC_START, 4, (uint8_t*)&adc_data_length);
}

void MainWindow::sendHsCommandAdcStart2()
{
    sin600_command = true;
    common_special_command = true;

    uint32_t adc_data_length = 31400;
    m_console->putData(QString("Send HS command USB_CMD_ADC_START, adc_data_length %1\n").arg(adc_data_length), 2);
    m_usb_thread.sendHsCommand(USB_CMD_ADC_START, 4, (uint8_t*)&adc_data_length);
}

void MainWindow::sendHsCommandAdcStart3()
{
    sweep_command = true;
    common_special_command = true;

    uint32_t adc_data_length = 440000;
    m_console->putData(QString("Send HS command USB_CMD_ADC_START, adc_data_length %1\n").arg(adc_data_length), 2);
    m_usb_thread.sendHsCommand(USB_CMD_ADC_START, 4, (uint8_t*)&adc_data_length);
}

void MainWindow::sendHsCommandAgcStart()
{
    m_console->putData(QString("Send HS command USB_CMD_AGC_START\n"), 2);
    sendCommandToSTM32(USB_CMD_AGC_START, nullptr, 0);
}

void MainWindow::recordSweep()
{
    // Show 'select directory' modal dialog
    QString dir = QFileDialog::getExistingDirectory(this, "Select directory for 'sweep' signals", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(dir == NULL)
    {
        m_console->putData(QString("Sweep record canceled\n"), 2);
        return;
    }

    // Set directory
    m_console->setSweepRecordDirectory(dir);

    m_mod_tx_thread.separateRecordSweepStart();
}

void MainWindow::commandCalculatePredistortionTablesStart()
{
    m_mod_tx_thread.calculatePredistortionTablesStart();
}

void MainWindow::commandCalculatePredistortionTablesContinue()
{
    m_mod_tx_thread.calculatePredistortionTablesContinue();
}

void MainWindow::commandAgcStart()
{
    m_mod_tx_thread.separateAgcStart();
}

void MainWindow::srpModeSet(uint8_t mode)
{
    m_qam_thread.srpModeSet(mode);
}

void MainWindow::qamDecoderReset()
{
    m_qam_thread.setFirstPassFlag();
}

// Indigo Base Protocol callback
bool MainWindow::frame_builded_cb(const void *param, const uint8_t *buffer, uint32_t size)
{
    MainWindow *self = (MainWindow*)param;

    if(!self)
        return false;

    self->transmitDataSerialPort(buffer, size);

    return true;
}

void MainWindow::on_actionShow_advanced_logs_in_console_triggered(bool checked)
{
    m_ShowAdvancedLogs = checked;
}

void MainWindow::on_actionReset_statistics_in_status_bar_triggered()
{
    statisics_reset();
}
