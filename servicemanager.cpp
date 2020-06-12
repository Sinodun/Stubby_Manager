#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"


ServiceMgr::ServiceMgr(MainWindow *parent) :
    QObject(parent),
    m_mainwindow(parent),
    m_serviceState(ServiceMgr::Unknown)
{
    qInfo("Creating service mgr");

}

ServiceMgr::~ServiceMgr()
{
}

int ServiceMgr::getState() {
    qInfo("gettting state");
    return getStateofService();
}

int ServiceMgr::start()
{
    if (m_serviceState == Unknown || m_serviceState == Stopped) {
        m_mainwindow->statusMsg("Starting Stubby service...");
        m_serviceState = Starting;
        return startService();
    }
    return 0;
}

int ServiceMgr::stop()
{
    if (m_serviceState == Unknown || m_serviceState == Running) {
        m_mainwindow->statusMsg("Stopping Stubby service...");
           m_serviceState = Stopping;
        return stopService();
    }
    return 0;
}

int ServiceMgr::restart()
{
    //if (m_serviceState == Unknown || m_serviceState == Running) {
        m_mainwindow->statusMsg("Re-starting Stubby service...");
        return restartService();
    //}
    return 0;
}

