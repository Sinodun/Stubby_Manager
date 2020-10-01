/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKMGR_H
#define NETWORKMGR_H

#include <string>
#include <vector>

#include <QObject>

class MainWindow;


class NetworkMgr : public QObject
{
    Q_OBJECT

public:
    NetworkMgr(MainWindow *parent);
    virtual ~NetworkMgr();

    static NetworkMgr * factory(MainWindow *parent);

    typedef enum {
        NotLocalhost = 0,
        Localhost,
        Unknown
    } NetworkState;

    int setLocalhost();
    int unsetLocalhost();
    int getState(bool reportNoChange);
    virtual int testQuery() = 0;
    std::vector<std::string> getRunningNetworks();
    std::vector<std::string> getNetworkResolvers(const std::string& network);

signals:
    void networkConfigChanged();
    void networkStateChanged(NetworkMgr::NetworkState state);
    void testQueryResult(bool result);

protected:
    virtual int setLocalhostDNS() = 0;
    virtual int unsetLocalhostDNS() = 0;
    virtual int getStateDNS(bool reportNoChange) = 0;
    virtual std::vector<std::string> getNetworks() = 0;
    virtual std::vector<std::string> getResolvers(const std::string& network) = 0;

    MainWindow *m_mainwindow;
    NetworkState m_networkState;

};

#endif // NETWORKMGR_H
