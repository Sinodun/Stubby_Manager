/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdexcept>

#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>

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
    //qInfo("gettting state");
    return getStateofService();
}

int ServiceMgr::start(ConfigMgr& configMgr, Config::NetworkProfile networkProfile)
{
    m_mainwindow->statusMsg("Starting Stubby service...");
    try {
        std::string stubby_yml = configMgr.generateStubbyConfig(networkProfile);
        if (stubby_yml.empty()) {
            m_mainwindow->statusMsg("Configuration for active profile was invalid. Please check there is at least one server active.");
            return 1;
        }
        m_mainwindow->statusMsg("Configuration generated.");
        m_serviceState = Starting;
        return startService(QString::fromStdString(stubby_yml));
    }
    catch(const std::runtime_error& err)
    {
        qCritical("Error: %s", err.what());
        return 1;
    }
}

int ServiceMgr::stop()
{
    m_mainwindow->statusMsg("Stopping Stubby service...");
    m_serviceState = Stopping;
    return stopService();
}

int ServiceMgr::restart()
{
    m_mainwindow->statusMsg("Re-starting Stubby service...");
    m_serviceState = Restarting;
    return restartService();
}
