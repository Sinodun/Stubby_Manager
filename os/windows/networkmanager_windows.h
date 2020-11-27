/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKMGRWINDOWS_H
#define NETWORKMGRWINDOWS_H

#include <QObject>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QProcess>

#include "networkinterface_windows.hpp"
#include "networkmanager.h"


class QueryTaskWindows : public QProcess
{
    Q_OBJECT

public:
    QueryTaskWindows(QObject *parent = 0);
    virtual ~QueryTaskWindows();
    void start();
};

class NetworkMgrWindows : public NetworkMgr
{
    Q_OBJECT

public:
    NetworkMgrWindows(MainWindow *parent = 0);
    virtual ~NetworkMgrWindows();

protected:
    int setLocalhostDNS() override;
    int unsetLocalhostDNS() override;
    int getStateDNS(bool reportChange) override;
    int testQuery() override;
    std::map<std::string, interfaceInfo> getNetworks() override;

private slots:
    void on_testQuery_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_networkInferfaces_changed(const QNetworkConfiguration& cfg);

private:
    std::vector<std::unique_ptr<NetworkInterfaceWindows>> getInterfaces();
    std::vector<NetworkInterfaceWindows> interfaces;

    bool isRunning() const;
    bool isResolverLoopback() const;
    void reload();

    QProcess *m_testQuery;
    QNetworkConfigurationManager m_networkConfig;
};

#endif // NETWORKMGRWINDOWS_H
