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
    m_mainwindow->statusMsg("Status: Stubby service - not implemented.");
    return 0;
}

int ServiceMgrWindows::startService()
{
    return 0;
}

int ServiceMgrWindows::stopService()
{
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
