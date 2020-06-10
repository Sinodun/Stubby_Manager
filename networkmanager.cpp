#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"


NetworkMgr::NetworkMgr(MainWindow *parent) :
    QObject(parent),
    m_mainwindow(parent),
    m_networkState(NetworkMgr::Unknown)

{
    qInfo("Creating network mgr");
}

NetworkMgr::~NetworkMgr()
{
}

int NetworkMgr::setLocalhost()
{
    // TODO: Should we check state here before trying to update?
    m_mainwindow->statusMsg("Setting DNS to localhost...");
    return setLocalhostDNS();
}

int NetworkMgr::unsetLocalhost()
{
    m_mainwindow->statusMsg("Setting DNS back to system settings...");
    return unsetLocalhostDNS();
}

int NetworkMgr::getState()
{
    return getStateDNS();
}
