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
#include "message_box.h"
#include "ringbuffer.h"
#include "atomic_vars.h"

#include <QApplication>
#include <QCommandLineParser>

/* Global variables */
RingBuffer *m_ring = nullptr;       // ring data buffer (ADC data) for QAM decoder
QWaitCondition ringNotEmpty;
QMutex m_mutex_ring_wait;

QWaitCondition sinFreqSweepBufNotEmpty;
QMutex m_mutex_sweep_thread;

QWaitCondition modTransmitWakeUp;
QMutex m_mutex_mod;

QMutex m_freqValMutex;

QElapsedTimer profiler_timer;       // debug timer for time measurements

bool is_auto_config_work = false;
bool mod2_auto_cfg_start_answer = false;
bool mod2_auto_cfg_stop_answer = false;
bool mod2_set_rx_parameters_answer = false;

int main(int argc, char *argv[])
{
    // Init SRP variables
    mod_status_reg = ready_status;

    // Init ring buffer for ADC data (HS EWL)
    m_ring = new RingBuffer();
    m_ring->Init();

    QApplication a(argc, argv);
    //QCoreApplication a(argc, argv);
    //a.setApplicationName("version-cmd");
    a.setApplicationVersion(VERSION_STRING);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.process(a);

    MainWindow w;
    w.setWindowTitle(QString("SBC application v.") + VERSION_STRING);
    w.show();

    return a.exec();
}
