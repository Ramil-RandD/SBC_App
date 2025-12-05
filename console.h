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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QScopedPointer>
#include <QTextStream>
#include <QDateTime>
#include "qam_decoder/tmwtypes.h"

class Console : public QPlainTextEdit
{
    Q_OBJECT

signals:
    //void getData(const QByteArray &data);

public:
    explicit Console(QWidget *parent = nullptr);

    void putData(const QString &data, uint8_t priority);
    void putDataAdc(const quint8 *p_data, quint32 size);
    void putDataAdcSpecial(const qint16 *p_data, quint32 len, uint8_t type);
    void putDataAdcSpecial_test(const double *p_data, quint32 len, uint8_t type);
    void putDataEqualData(const creal_T *p_data,const creal_T *eql_data, quint32 len, uint8_t flag);
    void putDataFilterCoeff(const creal_T *filt_coef_p, quint32 len, uint8_t flag);
    void Close();
    void fileFlush();
    void fileOpen();
    void openLogsFolder();
    void setSweepRecordDirectory(QString dir);

protected:

private:
    void mousePressEvent(QMouseEvent *e) override
    {
        Q_UNUSED(e);
    }
    void mouseDoubleClickEvent(QMouseEvent *e) override
    {
        Q_UNUSED(e);
    }

    QScopedPointer<QFile> m_logFile;        // Smart pointer to log file
    QScopedPointer<QFile> m_adcFile[20];    // Smart pointer to received adc data file
    QScopedPointer<QFile> m_frameErrorFile; // Smart pointer to received adc data file (special for frame errors)
    QScopedPointer<QFile> m_equalizedFilterDebug; // Smart pointer to received adc data file (special for frame errors)
    QScopedPointer<QFile> m_FilterCoeffDebug;
    QScopedPointer<QFile> m_sweepFile;      // Smart pointer to 'sweep' signal adc data file
    QScopedPointer<QFile> m_sin600File;     // Smart pointer to 'sin 600 periods' signal adc data file

    QTextStream out;
    QTextStream outAdc[20];
    QTextStream outFrameErrorAdc;
    QTextStream outEqalizedFilter;
    QTextStream outFilterCoeff;
    QTextStream outSweep;
    QTextStream outSin600;
    QTextStream outSweepRecords;

    uint8_t n_file = 0;                     // ADC data file switcher
    uint32_t n_frame = 0;                   // Number of frame for 'frame error adc data file'

    const int n_MaxLogFiles = 20;           // Maximum log files in the 'Logs' folder

    QString m_SweepSaveDirectory = "SBC_Logs/Sweep_records";
};

#endif // CONSOLE_H
