#include <QDebug>

#include "mainwindow.h"
#include "servicemanager_windows.h"


ServiceMgrWindows::ServiceMgrWindows(MainWindow *parent) :
    ServiceMgr(parent)

{
    qInfo("Creating Windows service mgr");
}

ServiceMgrWindows::~ServiceMgrWindows()
{
}

int ServiceMgrWindows::getStateofService() {
    qInfo("gettting windows state");
    emit serviceStateChanged(m_serviceState);
    m_mainwindow->statusMsg("Status: FAKE Stubby service state is Unknown");
    return 0;
}

int ServiceMgrWindows::startService()
{
    m_serviceState = Running;
    emit serviceStateChanged(m_serviceState);
    m_mainwindow->statusMsg("Status: FAKE Stubby service started.");
    return 0;
}

int ServiceMgrWindows::stopService()
{
    m_serviceState = Stopped;
    emit serviceStateChanged(m_serviceState);
    m_mainwindow->statusMsg("Status: FAKE Stubby service stopped.");
    return 0;
}

int ServiceMgrWindows::restartService()
{
    return 0;
}


ServiceMgr *ServiceMgr::factory(MainWindow *parent)
{
    return new ServiceMgrWindows(parent);
}
