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

#include "console.h"

#include <QScrollBar>
#include <QDesktopServices>
#include <QFileInfo>
#include <QDir>
#include "atomic_vars.h"

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    document()->setMaximumBlockCount(100);

    QFont font("Ubuntu", 10, QFont::Normal);
    document()->setDefaultFont(font);

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::GlobalColor::white);
    p.setColor(QPalette::Text, Qt::GlobalColor::blue);
    setPalette(p);

    // Check & create folder 'SBC_Logs'
    if(!QDir("SBC_Logs").exists())
        QDir().mkdir("SBC_Logs");

    // Delete old log files
    QStringList nameFilters;
    nameFilters.append("*.txt");

    QStringList file_list = QDir("SBC_Logs").entryList(nameFilters, QDir::Filter::NoFilter, QDir::SortFlag::Time);
    if(file_list.size())
    {
        while(file_list.count() > n_MaxLogFiles)
        {
            QFile::remove("SBC_Logs/" + file_list.last());
            file_list.removeLast();
        }
    }

    // Set the logging file
    m_logFile.reset(new QFile("SBC_Logs/SBC_log_"+ QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
    // Open the file logging
    m_logFile.data()->open(QFile::Append | QFile::Text);

    // Open stream file writes
    out.setDevice(m_logFile.data());

#ifdef QT_DEBUG
    // Set the adc data file
    for(uint8_t i = 0; i < 20; ++i)
    {
        m_adcFile[i].reset(new QFile("SBC_Logs/Adc_data" + QString::number(i) + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
        // Open the file logging
        m_adcFile[i].data()->open(QFile::Append | QFile::Text);

        // Open stream file writes
        outAdc[i].setDevice(m_adcFile[i].data());
    }
#endif
    // Set the adc data file for frame errors
    m_frameErrorFile.reset(new QFile("SBC_Logs/Frame_errors_adc_data_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
    m_equalizedFilterDebug.reset(new QFile("SBC_Logs/equalized_and_raw_qam_symbols_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
    m_FilterCoeffDebug.reset(new QFile("SBC_Logs/equalized_filter_coeff_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
    m_sweepFile.reset(new QFile("SBC_Logs/Sweep_adc_data_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
    m_sin600File.reset(new QFile("SBC_Logs/Sin600_adc_data_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
    // Open the file logging
    m_frameErrorFile.data()->open(QFile::Append | QFile::Text);
    m_equalizedFilterDebug.data()->open(QFile::Append | QFile::Text);
    m_FilterCoeffDebug.data()->open(QFile::Append | QFile::Text);
    m_sweepFile.data()->open(QFile::Append | QFile::Text);
    m_sin600File.data()->open(QFile::Append | QFile::Text);

    // Open stream file writes
    outFrameErrorAdc.setDevice(m_frameErrorFile.data());
    outEqalizedFilter.setDevice(m_equalizedFilterDebug.data());
    outFilterCoeff.setDevice(m_FilterCoeffDebug.data());
    outSweep.setDevice(m_sweepFile.data());
    outSin600.setDevice(m_sin600File.data());
}

void Console::putData(const QString &data, uint8_t priority)
{
    if(!m_logFile->isOpen())
        return;

#ifdef QT_DEBUG
    ++priority;
#endif

    if((priority > 1) || (priority == 1 && m_ShowAdvancedLogs))
    {
        insertPlainText(data);

        QScrollBar *bar = verticalScrollBar();
        bar->setValue(bar->maximum());
    }

    // Log to file (any 'priority')

    // Write the date of recording
    out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ");

    // Write to the output category of the message and the message itself
    out << data;
}

void Console::putDataAdc(const quint8 *p_data, quint32 size)
{
    if(!m_adcFile[n_file]->isOpen())
        return;

    QString data;
    uint16_t value;

    for(uint64_t i = 0; i < size; i+=2)
    {
        value = p_data[i];
        value |= ((uint16_t)p_data[i+1]) << 8;

        data.append(QString("%1\n").arg((int16_t)value));
    }

    // Choose adc data file to write to
    m_adcFile[n_file]->resize(0);   // Clear file
    outAdc[n_file] << data;
    outAdc[n_file].flush();    // Clear the buffered data

    if(++n_file >= 20)
        n_file = 0;
}

void Console::putDataEqualData(const creal_T *p_data, const creal_T *eql_data, quint32 len, uint8_t flag)
{
    QScopedPointer<QFile> *f;
    QTextStream *out = nullptr;
    QString data;
    bool flush_data_after_write = true;

    f = &m_equalizedFilterDebug;

    if(!(*f)->isOpen())
        return;

    if((*f)->size() > 100*1024*1024)    // 100 Mb
        return;
    out = &outEqalizedFilter;
    flush_data_after_write = false;     // do not flush data for frame errors

    if(flag)
        data.append(QString("// =================================== Frame #%1, with RS CODE ===================================\n").arg(n_frame++));
    else
        data.append(QString("// =================================== Frame #%1, with CRC ===================================\n").arg(n_frame++));

    for(uint64_t i = 0; i < len; i++)
        data.append(QString("%1 %2 %3 %4\n").arg(p_data[i].re).arg(p_data[i].im).arg(eql_data[i].re).arg(eql_data[i].im));

    if(out != nullptr)
    {
        *out << data;
        if(flush_data_after_write)
        {
            (*out).flush();       // Clear the buffered data
        }
    }
}

void Console::putDataFilterCoeff(const creal_T *filt_coef_p, quint32 len, uint8_t flag)
{
    QScopedPointer<QFile> *f;
    QTextStream *out = nullptr;
    QString data;
    bool flush_data_after_write = true;

    f = &m_FilterCoeffDebug;

    if(!(*f)->isOpen())
        return;

    if((*f)->size() > 100*1024*1024)    // 100 Mb
        return;
    out = &outFilterCoeff;
    flush_data_after_write = false;     // do not flush data for frame errors

    if(flag)
        data.append(QString("// =================================== Frame #%1, with RS CODE ===================================\n").arg(n_frame++));
    else
        data.append(QString("// =================================== Frame #%1, with CRC ===================================\n").arg(n_frame++));

    for(uint64_t i = 0; i < len; i++)
        data.append(QString("%1 %2\n").arg(filt_coef_p[i].re).arg(filt_coef_p[i].im));

    if(out != nullptr)
    {
        *out << data;
        if(flush_data_after_write)
        {
            (*out).flush();       // Clear the buffered data
        }
    }
}

void Console::putDataAdcSpecial(const qint16 *p_data, quint32 len, uint8_t type)
{
    QScopedPointer<QFile> *f;
    QTextStream *out = nullptr;
    QString data;
    bool flush_data_after_write = true;

    switch(type)
    {
        default:
        case 0:
            f = &m_frameErrorFile;

            if(!(*f)->isOpen())
                return;

            if((*f)->size() > 100*1024*1024)    // 100 Mb
                return;
            out = &outFrameErrorAdc;
            flush_data_after_write = false;     // do not flush data for frame errors

            data.append(QString("%1 // =================================== Frame #%2, len %3 ===================================\n").arg((int16_t)p_data[0]).arg(n_frame++).arg(len));
            break;

        case 1:
            f = &m_sin600File;

            if(!(*f)->isOpen())
                return;

            if((*f)->size() > 10*1024*1024)     // 10 Mb
                return;
            out = &outSin600;

            data.append(QString("// =================================== SIN 600, len %3 ===================================\n").arg(len));
            break;

        case 2:
            f = &m_sweepFile;

            if(!(*f)->isOpen())
                return;

            if((*f)->size() > 10*1024*1024)     // 10 Mb
                return;
            out = &outSweep;

            data.append(QString("// =================================== Sweep, len %3 ===================================\n").arg(len));
            break;

        case 3:
        {
            // Check & create folder for 'sweep' records
            bool res = false;

            if(!QDir(m_SweepSaveDirectory).exists())
            {
                res = QDir().mkdir(m_SweepSaveDirectory);

                if(!res)
                {
                    putData("Error can't create directory for 'sweep' records '" + m_SweepSaveDirectory + "'\n", 2);
                    break;
                }
            }

            if(!QDir(m_SweepSaveDirectory).isReadable())
            {
                putData("Error directory for 'sweep' records is not readable '" + m_SweepSaveDirectory + "'\n", 2);
                break;
            }

            QScopedPointer<QFile> m_sweepRecordFile;      // Smart pointer to file
            m_sweepRecordFile.reset(new QFile(m_SweepSaveDirectory + "/sweep_record_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
            res = m_sweepRecordFile.data()->open(QFile::Append | QFile::Text);

            if(!res)
            {
                putData("Error can't write file for 'sweep' records. Wrong directory or root is needed to access '" + m_SweepSaveDirectory + "'\n", 2);
                m_sweepRecordFile->close();
                break;
            }

            outSweepRecords.setDevice(m_sweepRecordFile.data());
            outSweepRecords << QString("// =================================== Sweep, len %3 ===================================\n").arg(len);

            for(uint64_t i = 1; i < len; ++i)
                outSweepRecords << QString("%1\n").arg((int16_t)p_data[i]);

            outSweepRecords.flush();

            m_sweepRecordFile->close();
            return;
        }
    }

    for(uint64_t i = 1; i < len; ++i)
        data.append(QString("%1\n").arg((int16_t)p_data[i]));

    if(out != nullptr)
    {
        *out << data;
        if(flush_data_after_write)
        {
            (*out).flush();       // Clear the buffered data
        }
    }
}

void Console::putDataAdcSpecial_test(const double *p_data, quint32 len, uint8_t type)
{
    QScopedPointer<QFile> *f;
    QTextStream *out = nullptr;
    QString data;
    bool flush_data_after_write = true;

    switch(type)
    {
        default:
        case 0:
            f = &m_frameErrorFile;

            if(!(*f)->isOpen())
                return;

            if((*f)->size() > 100*1024*1024)    // 100 Mb
                return;
            out = &outFrameErrorAdc;
            flush_data_after_write = false;     // do not flush data for frame errors

            data.append(QString("%1 // =================================== Frame #%2, len %3 ===================================\n").arg((int16_t)p_data[0]).arg(n_frame++).arg(len));
            break;

        case 1:
            f = &m_sin600File;

            if(!(*f)->isOpen())
                return;

            if((*f)->size() > 10*1024*1024)     // 10 Mb
                return;
            out = &outSin600;

            data.append(QString("// =================================== SIN 600, len %3 ===================================\n").arg(len));
            break;

        case 2:
            f = &m_sweepFile;

            if(!(*f)->isOpen())
                return;

            if((*f)->size() > 10*1024*1024)     // 10 Mb
                return;
            out = &outSweep;

            data.append(QString("// =================================== Sweep, len %3 ===================================\n").arg(len));
            break;

        case 3:
        {
            // Check & create folder for 'sweep' records
            bool res = false;

            if(!QDir(m_SweepSaveDirectory).exists())
            {
                res = QDir().mkdir(m_SweepSaveDirectory);

                if(!res)
                {
                    putData("Error can't create directory for 'sweep' records '" + m_SweepSaveDirectory + "'\n", 2);
                    break;
                }
            }

            if(!QDir(m_SweepSaveDirectory).isReadable())
            {
                putData("Error directory for 'sweep' records is not readable '" + m_SweepSaveDirectory + "'\n", 2);
                break;
            }

            QScopedPointer<QFile> m_sweepRecordFile;      // Smart pointer to file
            m_sweepRecordFile.reset(new QFile(m_SweepSaveDirectory + "/sweep_record_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss.zzz") + ".txt"));
            res = m_sweepRecordFile.data()->open(QFile::Append | QFile::Text);

            if(!res)
            {
                putData("Error can't write file for 'sweep' records. Wrong directory or root is needed to access '" + m_SweepSaveDirectory + "'\n", 2);
                m_sweepRecordFile->close();
                break;
            }

            outSweepRecords.setDevice(m_sweepRecordFile.data());
            outSweepRecords << QString("// =================================== Sweep, len %3 ===================================\n").arg(len);

            for(uint64_t i = 1; i < len; ++i)
                outSweepRecords << QString("%1\n").arg((int16_t)p_data[i]);

            outSweepRecords.flush();

            m_sweepRecordFile->close();
            return;
        }
    }

    for(uint64_t i = 1; i < len; ++i)
        data.append(QString("%1\n").arg(p_data[i]));

    if(out != nullptr)
    {
        *out << data;
        if(flush_data_after_write)
        {
            (*out).flush();       // Clear the buffered data
        }
    }
}

void Console::fileFlush()
{
    putData("Force log file flush by user\n", 2);
    out.flush();    // Clear the buffered data
}

void Console::fileOpen()
{
    // Find file path
    QFileInfo info;
    info.setFile(m_logFile->fileName());
    info.makeAbsolute();

    putData(QString("Opening log file: ") + info.filePath() + QString("\n"), 2);

    // Flush before open
    fileFlush();

    // Open file with external program
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath()));
}

void Console::openLogsFolder()
{
    // Flush log file
    fileFlush();

    // Open logs folder
    const QString& path = "SBC_Logs";
    QFileInfo info(path);

#if defined(Q_OS_WIN)
    QStringList args;
    if (!info.isDir())
        args << "/select,";
    args << QDir::toNativeSeparators(path);
    if (QProcess::startDetached("explorer", args))
        return;
#elif defined(Q_OS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + path + "\"";
    args << "-e";
    args << "end tell";
    args << "-e";
    args << "return";
    if (!QProcess::execute("/usr/bin/osascript", args))
        return;
#endif
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.isDir()? path : info.path()));
}

void Console::Close()
{
    out.flush();                    // Clear the buffered data
    m_logFile->close();             // Close file

#ifdef QT_DEBUG
    for(uint8_t i = 0; i < 20; ++i)
    {
        outAdc[i].flush();          // Clear the buffered data
        m_adcFile[i]->close();      // Close file
    }
#endif

    outFrameErrorAdc.flush();       // Clear the buffered data
    m_frameErrorFile->close();      // Close file

    outSweep.flush();
    m_sweepFile->close();

    outSin600.flush();
    m_sin600File->close();
}

void Console::setSweepRecordDirectory(QString dir)
{
    m_SweepSaveDirectory = dir;
}
