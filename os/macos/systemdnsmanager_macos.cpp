#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"
#include "systemdnsmanager_macos.h"


SystemDNSMgrMacos::SystemDNSMgrMacos(MainWindow *parent) :
    SystemDNSMgr(parent),
    m_setLocalhost(0),
    m_unsetLocalhost(0),
    m_getSystemDNSState(0)
{
    m_setLocalhost = new RunHelperTaskMacos("dns_stubby", RunHelperTaskMacos::RIGHT_DNS_LOCAL, QString(), this, parent);
    connect(m_setLocalhost, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_setLocalhost_finished(int,QProcess::ExitStatus)));

    m_unsetLocalhost = new RunHelperTaskMacos("dns_default", RunHelperTaskMacos::RIGHT_DNS_LOCAL, QString(), this, parent);
    connect(m_unsetLocalhost, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_unsetLocalhost_finished(int, QProcess::ExitStatus)));

    m_getSystemDNSState = new RunHelperTaskMacos("dns_list", QString(), QString(), this, parent);
    connect(m_getSystemDNSState, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_getSystemDNSState_finished(int, QProcess::ExitStatus)));
    connect(m_getSystemDNSState, SIGNAL(readyReadStandardOutput()), this, SLOT(on_getSystemDNSState_readyReadStdout()));
}

SystemDNSMgrMacos::~SystemDNSMgrMacos()
{
}

int SystemDNSMgrMacos::setLocalhostDNS()
{
    m_setLocalhost->start();
    m_mainwindow->statusMsg("Setting DNS to localhost...");
    return 0;
}

int SystemDNSMgrMacos::unsetLocalhostDNS()
{
    m_unsetLocalhost->start();
    m_mainwindow->statusMsg("Setting DNS back to default system settings...");
    return 0;
}

int SystemDNSMgrMacos::getStateDNS()
{
    m_getSystemDNSState_output = "";
    m_getSystemDNSState->start();
    return 0;
}

void SystemDNSMgrMacos::on_setLocalhost_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug("Exit code is %d", exitCode);
    getState();
}

void SystemDNSMgrMacos::on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug("Exit code is %d", exitCode);
    getState();
}

void SystemDNSMgrMacos::on_getSystemDNSState_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit code is %d", exitCode);
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
        QStringList slist = m_getSystemDNSState_output.split("\n");
        qDebug("Output from stdout is\n%s", m_getSystemDNSState_output.toLatin1().data());
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
            m_systemDNSState = Localhost;
            emit systemDNSStateChanged(Localhost);
            qDebug("All connections are using Localhost");
            m_mainwindow->statusMsg("Status: DNS settings using localhost.");
            return;
        }

        /* if all none are Localhost then something else is being used */
        i = 0;
        while ((i < numberOfConnectors) && isNotLocalhost[i]) { i++; }
        if (i==numberOfConnectors) {
            m_systemDNSState = NotLocalhost;
            emit systemDNSStateChanged(NotLocalhost);
            qDebug("No connections are using Localhost");
            m_mainwindow->statusMsg("Status: DNS settings using default system settings.");
            return;
        }

        /* otherwhise, the true state is unknown.*/
     }
     m_systemDNSState = Unknown;
     emit systemDNSStateChanged(Unknown);
     qDebug("Error - DNS server use of localhost is not clear");
     m_mainwindow->statusMsg("Error - DNS server use of localhost is not clear");
}

void SystemDNSMgrMacos::on_getSystemDNSState_readyReadStdout()

{
    m_getSystemDNSState_output = m_getSystemDNSState_output + QString::fromLatin1(m_getSystemDNSState->readAllStandardOutput().data());
}

SystemDNSMgr *SystemDNSMgr::factory(MainWindow *parent) {
    return new SystemDNSMgrMacos(parent);
}