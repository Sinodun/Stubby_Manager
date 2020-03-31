#include <QDebug>

#include "mainwindow.h"
#include "servicemanager.h"


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
    emit serviceStateChanged(Error);
    return ServiceState::Error;
}
