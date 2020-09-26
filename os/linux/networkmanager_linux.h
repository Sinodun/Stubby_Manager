/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKMGRLINUX_H
#define NETWORKMGRLINUX_H

#include <QObject>
#include <QProcess>

#include "networkmanager.h"

class NetworkMgrLinux : public NetworkMgr
{
    Q_OBJECT

public:
    NetworkMgrLinux(MainWindow *parent = 0);
    virtual ~NetworkMgrLinux();

protected:
    int setLocalhostDNS();
    int unsetLocalhostDNS();
    int getStateDNS();
    int testQuery();
    std::vector<std::string> getNetworks();
};

#endif // NETWORKMGRLINUX_H
