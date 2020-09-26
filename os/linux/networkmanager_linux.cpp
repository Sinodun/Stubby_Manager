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
#include "networkmanager_linux.h"


NetworkMgrLinux::NetworkMgrLinux(MainWindow *parent) :
    NetworkMgr(parent)
{
}

NetworkMgrLinux::~NetworkMgrLinux()
{
}

int NetworkMgrLinux::setLocalhostDNS()
{
    return 1;
}

int NetworkMgrLinux::unsetLocalhostDNS()
{
    return 1;
}

int NetworkMgrLinux::getStateDNS()
{
    return 1;
}

int NetworkMgrLinux::testQuery()
{
    return 1;
}

std::vector<std::string> NetworkMgrLinux::getNetworks()
{
    std::vector<std::string> res;
    res.push_back("eth0");
    return res;
}

NetworkMgr *NetworkMgr::factory(MainWindow *parent) {
    return new NetworkMgrLinux(parent);
}
