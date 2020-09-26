/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LOGMANAGERMACOS_H
#define LOGMANAGERMACOS_H

#include <QObject>

#include "logmanager.h"
#include "runtask_macos.h"


class LogMgrMacOS : public QObject, public ILogMgr
{
    Q_OBJECT

    Q_INTERFACES(ILogMgr)

public:
    LogMgrMacOS(MainWindow *parent = 0);
    ~LogMgrMacOS();

    void start();
    void stop();

signals:
    void alert(bool on);

protected:
    MainWindow *m_mainWindow;

private slots:
    void on_started();
    void on_finished(int exitCode);
    void on_readyRead();

private:
    RunTaskMacos *m_logger;
    bool m_isRunning;
};

#endif // LOGMANAGERMACOS_H
