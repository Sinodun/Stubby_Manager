/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"
#include "servicemanager_linux.h"


ServiceMgrLinux::ServiceMgrLinux(MainWindow *parent) :
    ServiceMgr(parent)
{
    qInfo("Creating Linux service mgr");
}

ServiceMgrLinux::~ServiceMgrLinux()
{
}

int ServiceMgrLinux::getStateofService()
{
    qInfo("getting service state");
    return 1;
}

int ServiceMgrLinux::startService(QString configfile, int loglevel)
{
    qInfo("start service");

    return 1;
}

int ServiceMgrLinux::stopService()
{
    qInfo("stop service");

    return 1;
}

int ServiceMgrLinux::restartService()
{
    stopService();
    return startService();
}

ServiceMgr *ServiceMgr::factory(MainWindow *parent)
{
    return new ServiceMgrLinux(parent);
}
