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
#include "networkmanager_macos.h"


NetworkMgrMacos::NetworkMgrMacos(MainWindow *parent) :
    NetworkMgr(parent),
    m_setLocalhost(0),
    m_unsetLocalhost(0),
    m_getNetworkState(0),
    m_testQuery(0)
{
    m_setLocalhost = new RunHelperTaskMacos("dns_stubby", RunHelperTaskMacos::RIGHT_DNS_LOCAL, QString(), this, parent);
    connect(m_setLocalhost, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_setLocalhost_finished(int,QProcess::ExitStatus)));

    m_unsetLocalhost = new RunHelperTaskMacos("dns_default", RunHelperTaskMacos::RIGHT_DNS_LOCAL, QString(), this, parent);
    connect(m_unsetLocalhost, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_unsetLocalhost_finished(int, QProcess::ExitStatus)));

    m_getNetworkState = new RunHelperTaskMacos("dns_list", QString(), QString(), this, parent);
    connect(m_getNetworkState, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_getNetworkState_finished(int, QProcess::ExitStatus)));
    connect(m_getNetworkState, SIGNAL(readyReadStandardOutput()), this, SLOT(on_getNetworkState_readyReadStdout()));

    m_getActiveNetworks = new RunHelperTaskMacos("dns_get_active_networks", QString(), QString(), this, parent);
    m_getSSID = new RunHelperTaskMacos("dns_get_wifi_ssid", QString(), QString(), this, parent);

    m_testQuery = new RunTaskMacos("dig @127.0.0.1 getdnsapi.net", this);
    connect(m_testQuery, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_testQuery_finished(int, QProcess::ExitStatus)));
}

NetworkMgrMacos::~NetworkMgrMacos()
{
}

int NetworkMgrMacos::setLocalhostDNS()
{
    m_setLocalhost->start();
    return 0;
}

int NetworkMgrMacos::unsetLocalhostDNS()
{
    m_unsetLocalhost->start();
    return 0;
}

int NetworkMgrMacos::getStateDNS(bool reportNoChange)
{
    m_getNetworkState_output = "";
    m_getNetworkState->start();
    return 0;
}

int NetworkMgrMacos::testQuery() {
    m_testQuery->start();
    return 0;
}

void NetworkMgrMacos::on_testQuery_finished(int, QProcess::ExitStatus)
{
    QByteArray stdoutData;
    stdoutData = m_testQuery->readAllStandardOutput();
    if (stdoutData.isEmpty()) {
        emit testQueryResult(false);
        return;
    }
    if (stdoutData.contains("NOERROR")) {
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "OK";   
        emit testQueryResult(true);
    }
    else {
        emit testQueryResult(false);
    }
}

void NetworkMgrMacos::on_setLocalhost_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug("SetLocalhost exit code is %d", exitCode);
    getState(false);
}

void NetworkMgrMacos::on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug("UnsetLocalhost exit code is %d", exitCode);
    getState(false);
}

void NetworkMgrMacos::on_getNetworkState_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("getState exit code is %d", exitCode);
    /*
     * $ sudo ./stubby-setdns.sh -l
     * ** Current DNS settings **
     * Apple USB Ethernet Adapter:    127.0.0.1 ::1
     * Wi-Fi:                         127.0.0.1 ::1
     * Bluetooth PAN:                 127.0.0.1 ::1
     * $ sudo ./stubby-setdns.sh
     * ** Current DNS settings ** -l
     * Apple USB Ethernet Adapter:    There aren't any DNS Servers set on Apple USB Ethernet Adapter.
     * Wi-Fi:                         There aren't any DNS Servers set on Wi-Fi.
     * Bluetooth PAN:                 There aren't any DNS Servers set on Bluetooth PAN.
     * $
     */

     if (exitStatus == QProcess::NormalExit) {
        QStringList slist = m_getNetworkState_output.split("\n");
        qDebug("Output from stdout is\n%s", m_getNetworkState_output.toLatin1().data());
        int numberOfConnectors = slist.length() - 2;
        bool isLocalhost[numberOfConnectors];
        bool isNotLocalhost[numberOfConnectors];

        QString localPID("127.0.0.1 ::1");
        int i = 1;
        while (i <= numberOfConnectors) {
            if (slist[i].contains(&localPID)) {
                isLocalhost[i-1] = true;
                isNotLocalhost[i-1] = false;
            } else {
                isLocalhost[i-1] = false;
                isNotLocalhost[i-1] = true;
            }
            i++;
        }

        /* if all are set to Localhost then stubby DNS is place */
        i = 0;
        while ((i < numberOfConnectors) && isLocalhost[i]) { i++; }
        if (i==numberOfConnectors) {
            m_networkState = Localhost;
            m_mainwindow->statusMsg("Status: DNS settings using localhost.");
            emit networkStateChanged(Localhost);
            qDebug("All connections are using Localhost");
            return;
        }

        /* if all none are Localhost then something else is being used */
        i = 0;
        while ((i < numberOfConnectors) && isNotLocalhost[i]) { i++; }
        if (i==numberOfConnectors) {
            m_mainwindow->statusMsg("Status: DNS settings using default system settings.");
            m_networkState = NotLocalhost;
            emit networkStateChanged(NotLocalhost);
            qDebug("No connections are using Localhost");
            return;
        }

        /* otherwhise, the true state is unknown.*/
     }
     m_networkState = Unknown;
     m_mainwindow->statusMsg("Error - DNS server use of localhost is not clear");
     emit networkStateChanged(Unknown);
     qDebug("Error - DNS server use of localhost is not clear");
}

void NetworkMgrMacos::on_getNetworkState_readyReadStdout()

{
    m_getNetworkState_output = m_getNetworkState_output + QString::fromLatin1(m_getNetworkState->readAllStandardOutput().data());
}

std::vector<std::string> NetworkMgrMacos::getNetworks() {
    std::vector<std::string> res;
    
    m_getActiveNetworks->start();
    if (!m_getActiveNetworks->waitForStarted())
        res.push_back("Process didn't start");
    // TODO: This is blocking.... change interface so it uses a callback
    // TODO: deal with multiple networks
    m_getActiveNetworks->waitForReadyRead();
    QString result = QString::fromLatin1(m_getActiveNetworks->readAll().data());
    if (result == "Wi-Fi\n") {
        m_getSSID->start();
        if (!m_getSSID->waitForStarted())
            res.push_back("Process didn't start");
        // TODO: This is blocking.... change interface so it uses a callback
        m_getSSID->waitForReadyRead();
        QString ssid = QString::fromLatin1(m_getSSID->readAll().data());
        ssid.chop(1);
        if (ssid.isEmpty())
            ssid = "Not connected";
        res.push_back("Wi-Fi (" + ssid.toStdString() + ")");
    }
    return res;
}

NetworkMgr *NetworkMgr::factory(MainWindow *parent) {
    return new NetworkMgrMacos(parent);
}
