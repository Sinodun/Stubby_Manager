/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SERVICEMGRLINUX_H
#define SERVICEMGRLINUX_H

#include <QProcess>

#include "servicemanager.h"

class MainWindow;


class ServiceMgrLinux : public ServiceMgr
{
    Q_OBJECT

public:
    ServiceMgrLinux(MainWindow *parent);
    virtual ~ServiceMgrLinux();

private:
    int getStateofService();
    int startService(QString configfile = "", int loglevel = 6);
    int stopService();
    int restartService();
};

#endif // SERVICEMGRLINUX_H
