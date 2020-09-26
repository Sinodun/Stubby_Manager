/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKMGRMACOS_H
#define NETWORKMGRMACOS_H

#include <QObject>
#include <QProcess>

#include "networkmanager.h"
#include "os/macos/runtask_macos.h"


class NetworkMgrMacos : public NetworkMgr
{
    Q_OBJECT

public:
    NetworkMgrMacos(MainWindow *parent = 0);
    virtual ~NetworkMgrMacos();

protected:
    int setLocalhostDNS();
    int unsetLocalhostDNS();
    int getStateDNS(bool reportNoChange);
    int testQuery();
    std::vector<std::string> getNetworks();

private slots:
    void on_setLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_getNetworkState_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_testQuery_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_getNetworkState_readyReadStdout();

private:
    RunHelperTaskMacos *m_setLocalhost;
    RunHelperTaskMacos *m_unsetLocalhost;
    RunHelperTaskMacos *m_getNetworkState;
    RunHelperTaskMacos *m_getActiveNetworks;
    RunHelperTaskMacos *m_getSSID;
    RunTaskMacos *m_testQuery;
    QString m_getNetworkState_output;
};

#endif // NETWORKMGRMACOS_H
