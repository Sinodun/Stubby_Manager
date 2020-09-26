/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RUNTASK_MACOS_H
#define RUNTASK_MACOS_H

#include <Security/Authorization.h>

#include <QObject>
#include <QProcess>

class MainWindow;


class RunTaskMacos : public QProcess
{
    Q_OBJECT

public:
    RunTaskMacos(const QString command, QObject *parent = 0);
    virtual ~RunTaskMacos();
    void start();

private:
    QString m_command;
};

class RunHelperTaskMacos : public QProcess
{
    Q_OBJECT

public:
    RunHelperTaskMacos(const QString command, QString need_right = QString(), const QString config = QString(), QObject *parent = 0, MainWindow *mainwindow = 0);
    virtual ~RunHelperTaskMacos();
    int execute();
    void start();

    static const char *RIGHT_DAEMON_RUN;
    static const char *RIGHT_DNS_LOCAL;

private:
    QString makeCommandLine();
    QString makeExternalAuth();

    QString m_command;
    QString m_need_right;
    QString m_config;
    MainWindow *m_mainwindow;
    AuthorizationRef m_auth_ref;
};

#endif // RUNTASK_MACOS_H
