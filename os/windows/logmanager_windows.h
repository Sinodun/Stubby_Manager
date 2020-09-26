/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LOGMANAGERWINDOWS_H
#define LOGMANAGERWINDOWS_H

#include <QObject>
#include <QString>
#include <QTimer>

#include <windows.h>

#include "logmanager.h"



class LogMgrWindows : public QObject, public ILogMgr
{
    Q_OBJECT

    Q_INTERFACES(ILogMgr)

public:
    LogMgrWindows(MainWindow *parent = 0);
    ~LogMgrWindows();

    void start();
    void stop();

signals:
    void alert(bool on);

protected:
    MainWindow *m_mainWindow;

private:

    QString logMessageSource(PEVENTLOGRECORD pevt);
    QString formatLogMessage(PEVENTLOGRECORD pevt);
    QString readServiceLog();
    HANDLE m_log;
    HMODULE m_eventResources;

    void logTimerExpired();
    QTimer *m_logTimer;

};

#endif // LOGMANAGERWINDOWS_H
