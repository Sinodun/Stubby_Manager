#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"
#include "systemdnsmanager_macos.h"


SystemDNSMgr::SystemDNSMgr(MainWindow *parent) :
    QObject(parent),
    m_systemDNSState(SystemDNSMgr::Unknown),
    m_mainwindow(parent),
    m_setLocalhost(0),
    m_unsetLocalhost(0),
    m_getSystemDNSState(0),
    m_getSystemDNSState_output("")
{
    m_setLocalhost = new RunHelperTaskMacos("dns_stubby", RunHelperTaskMacos::RIGHT_DNS_LOCAL, QString(), this, parent);
    connect(m_setLocalhost, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_setLocalhost_finished(int,QProcess::ExitStatus)));

    m_unsetLocalhost = new RunHelperTaskMacos("dns_default", RunHelperTaskMacos::RIGHT_DNS_LOCAL, QString(), this, parent);
    connect(m_unsetLocalhost, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_unsetLocalhost_finished(int, QProcess::ExitStatus)));

    m_getSystemDNSState = new RunHelperTaskMacos("dns_list", QString(), QString(), this, parent);
    connect(m_getSystemDNSState, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_getSystemDNSState_finished(int, QProcess::ExitStatus)));
    connect(m_getSystemDNSState, SIGNAL(readyReadStandardOutput()), this, SLOT(on_getSystemDNSState_readyReadStdout()));
}

SystemDNSMgr::~SystemDNSMgr()
{
}

int SystemDNSMgr::setLocalhost()
{
    m_setLocalhost->start();
    return 0;
}

int SystemDNSMgr::unsetLocalhost()
{
    m_unsetLocalhost->start();
    return 0;
}

int SystemDNSMgr::getState()
{
    m_getSystemDNSState_output = "";
    m_getSystemDNSState->start();
    return 0;
}

void SystemDNSMgr::on_setLocalhost_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug("Exit code is %d", exitCode);
    getState();
}

void SystemDNSMgr::on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug("Exit code is %d", exitCode);
    getState();
}

void SystemDNSMgr::on_getSystemDNSState_finished(int exitCode, QProcess::ExitStatus exitStatus)
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
            return;
        }

        /* if all none are Localhost then something else is being used */
        i = 0;
        while ((i < numberOfConnectors) && isNotLocalhost[i]) { i++; }
        if (i==numberOfConnectors) {
            m_systemDNSState = NotLocalhost;
            emit systemDNSStateChanged(NotLocalhost);
            qDebug("No connections are using Localhost");
             return;
        }

        /* otherwhise, the true state is unknown.*/
     }
     m_systemDNSState = Unknown;
     emit systemDNSStateChanged(Unknown);
     qDebug("Error - dns server use of Localhost is not clear");
}

void SystemDNSMgr::on_getSystemDNSState_readyReadStdout()

{
    m_getSystemDNSState_output = m_getSystemDNSState_output + QString::fromLatin1(m_getSystemDNSState->readAllStandardOutput().data());
}
