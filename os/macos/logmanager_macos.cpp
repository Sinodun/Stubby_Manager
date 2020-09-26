/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "logmanager_macos.h"
#include "mainwindow.h"

#include <QDebug>
#include <QTextCursor>

LogMgrMacOS::LogMgrMacOS(MainWindow *parent) :
    QObject(parent),
    m_mainWindow(parent),
    m_logger(0),
    m_isRunning(false)
{
    m_logger = new RunTaskMacos("tail -f /var/log/stubby.log", this);
    connect(m_logger, SIGNAL(started()), this, SLOT(on_started()));
    connect(m_logger, SIGNAL(finished(int)), this, SLOT(on_finished(int)));
    connect(m_logger, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyRead()));
}

LogMgrMacOS::~LogMgrMacOS()
{
}

void LogMgrMacOS::start()
{
    m_logger->start();
}

void LogMgrMacOS::stop()
{
    m_logger->terminate();
}

void LogMgrMacOS::on_started()
{
    m_isRunning = true;
}

void LogMgrMacOS::on_finished(int /*exitCode*/)
{
    m_isRunning = false;
}

void LogMgrMacOS::on_readyRead()
{
    QByteArray stdoutData;
    stdoutData = m_logger->readAllStandardOutput();

    //static bool isAlert = false;
    static QByteArray incompleteLine;
    incompleteLine.append(stdoutData);
    QList<QByteArray> lines = incompleteLine.split('\n');
    int n = lines.count();

    /*for (int i = 0; i < n-1; i++) {;
        // now parse this line of data - for now just look for indication of failure or success
        if (lines[i].contains("*FAILURE* no valid transports or upstreams available!")) {
            if (!isAlert) emit alert(true);
            isAlert = true;
        }
        // TODO: This won't mean a successfull connection, just a retry.
        // There's no text that works for Strict and Oppo at the moment.....
        if (lines[i].contains("Conn opened: ")) {
            if (isAlert) emit alert(false);
            isAlert = false;
        }
    }*/
    incompleteLine = lines[n-1]; //save the remnant so it can be processed next time

    m_mainWindow->logMsg(QString::fromLatin1(stdoutData.data()));

}

ILogMgr *ILogMgr::factory(MainWindow *parent)
{
    return new LogMgrMacOS(parent);
}

