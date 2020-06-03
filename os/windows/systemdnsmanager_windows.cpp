#include <QDebug>

#include "mainwindow.h"
#include "systemdnsmanager_windows.h"


SystemDNSMgrWindows::SystemDNSMgrWindows(MainWindow *parent) :
    SystemDNSMgr(parent)
{
}

SystemDNSMgrMacos::~SystemDNSMgrMacos()
{
}

int SystemDNSMgrWindows::setLocalhostDNS()
{
    return 0;
}

int SystemDNSMgrWindows::unsetLocalhostDNS()
{
    return 0;
}

int SystemDNSMgrWindows::getStateDNS()
{
    qInfo("gettting windows service state");
    m_mainwindow->statusMsg("Status: Stubby DNS state - not implemented.");
    return 0;
}


SystemDNSMgr *SystemDNSMgr::factory(MainWindow *parent) {
    return new SystemDNSMgrWindows(parent);
}
