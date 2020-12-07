/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>

#include "config.h"

class MainWindow;


class ServiceMgr : public QObject
{
    Q_OBJECT

public:
    ServiceMgr(MainWindow *parent);
    virtual ~ServiceMgr();

    static ServiceMgr * factory(MainWindow *parent);

    typedef enum {
        Stopped,
        Starting,
        Restarting,
        Running,
        Stopping,
        Unknown,
        Error
    } ServiceState;

    int getState();
    int start(ConfigMgr& config);
    int stop();
    int restart();

signals:
    void serviceStateChanged(ServiceMgr::ServiceState state);

protected:
    MainWindow *m_mainwindow;
    ServiceState m_serviceState;

private:
    virtual int getStateofService() = 0;
    virtual int startService(QString configfile = "", int loglevel = 6) = 0;
    virtual int stopService() = 0;
    virtual int restartService() = 0;

};

#endif // SERVICEMGR_H
