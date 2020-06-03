#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"


SystemDNSMgr::SystemDNSMgr(MainWindow *parent) :
    QObject(parent),
    m_mainwindow(parent),
    m_systemDNSState(SystemDNSMgr::Unknown)

{
    qInfo("Creating service mgr");
}

SystemDNSMgr::~SystemDNSMgr()
{
}

int SystemDNSMgr::setLocalhost()
{
    // TODO: Should we check state here before trying to update?
    m_mainwindow->statusMsg("Setting DNS to localhost...");
    return setLocalhostDNS();
}

int SystemDNSMgr::unsetLocalhost()
{
    m_mainwindow->statusMsg("Setting DNS back to system settings...");
    return unsetLocalhostDNS();
}

int SystemDNSMgr::getState()
{
    return getStateDNS();
}
