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


NetworkMgr::NetworkMgr(MainWindow *parent) :
    QObject(parent),
    m_mainwindow(parent),
    m_networkState(NetworkMgr::Unknown)

{
    qInfo("Creating network mgr");
}

NetworkMgr::~NetworkMgr()
{
}

int NetworkMgr::setLocalhost()
{
    m_mainwindow->statusMsg("Setting DNS to localhost...");
    return setLocalhostDNS();
}

int NetworkMgr::unsetLocalhost()
{
    m_mainwindow->statusMsg("Setting DNS back to system settings...");
    return unsetLocalhostDNS();
}

int NetworkMgr::getState(bool reportNoChange)
{
    return getStateDNS(reportNoChange);
}

std::vector<std::string> NetworkMgr::getRunningNetworks() {
    return getNetworks();
}

std::vector<std::string> NetworkMgr::getNetworkResolvers(const std::string& network) {
    return getResolvers(network);
}
