/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SERVICEMGRWINDOWS_H
#define SERVICEMGRWINDOWS_H

#include <QFileInfo>
#include <QProcess>

#include <windows.h>

#include "servicemanager.h"

class MainWindow;


class ServiceMgrWindows : public ServiceMgr
{
    Q_OBJECT

public:
    ServiceMgrWindows(MainWindow *parent);
    virtual ~ServiceMgrWindows();

    void notifyCallback();

private:
    int getStateofService();
    int startService(QString configfile = "", int loglevel = 6);
    int stopService();
    int restartService();

    QString logMessageSource(PEVENTLOGRECORD pevt);
    QString formatLogMessage(PEVENTLOGRECORD pevt);
    void updateState(DWORD state);

    SC_HANDLE m_schSCManager;
    SC_HANDLE m_schService;
    SERVICE_NOTIFY m_serviceNotify;
    QFileInfo m_ConfigFile;
};

#endif // SERVICEMGRWINDOWS_H
