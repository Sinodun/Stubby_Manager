/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cctype>
#include <ctime>
#include <sstream>
#include <vector>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include <windows.h>

#include "mainwindow.h"
#include "servicemanager_windows.h"

const char SVCNAME[] = "Stubby";

static void CALLBACK notify_callback_trampoline(void* param)
{
    SERVICE_NOTIFY* sn = static_cast<SERVICE_NOTIFY*>(param);
    ServiceMgrWindows* smw = static_cast<ServiceMgrWindows*>(sn->pContext);
    smw->notifyCallback();
}

static void winerr(const char* operation, DWORD err)
{
    char msg[512];

    if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       err,
                       0,
                       msg,
                       sizeof(msg),
                       NULL) == 0 )
        qCritical("Error: %s: errno=%d\n", operation, err);
    else
        qCritical("Error: %s: %s\n", operation, msg);
}

static void winlasterr(const char* operation)
{
    winerr(operation, GetLastError());
}

ServiceMgrWindows::ServiceMgrWindows(MainWindow *parent) :
    ServiceMgr(parent), m_schSCManager(NULL), m_schService(NULL),
    m_ConfigFile(QDir(QCoreApplication::applicationDirPath()), "stubbyservice.yml")
{
    qInfo("Creating Windows service mgr");
    m_schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        STANDARD_RIGHTS_WRITE);  // need to change state
    if ( !m_schSCManager )
        winlasterr("Open service manager");
    else
    {
        m_schService = OpenService(
            m_schSCManager,              // SCM database
            SVCNAME,                   // name of service
            SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS); // intention
        if ( m_schService )
        {
            DWORD err;

            m_serviceNotify.dwVersion = SERVICE_NOTIFY_STATUS_CHANGE;
            m_serviceNotify.pfnNotifyCallback = reinterpret_cast<PFN_SC_NOTIFY_CALLBACK>(notify_callback_trampoline);
            m_serviceNotify.pContext = this;

            err = NotifyServiceStatusChange(
                m_schService,
                SERVICE_NOTIFY_START_PENDING | SERVICE_NOTIFY_RUNNING |
                SERVICE_NOTIFY_STOP_PENDING | SERVICE_NOTIFY_STOPPED,
                &m_serviceNotify);
            if ( err != ERROR_SUCCESS )
                winerr("Notify status change", err);
        }
        else
            winlasterr("Open service");
    }
}

ServiceMgrWindows::~ServiceMgrWindows()
{
    if ( m_schService )
        CloseServiceHandle(m_schService);
    if ( m_schSCManager )
        CloseServiceHandle(m_schSCManager);
}

void ServiceMgrWindows::notifyCallback()
{
    //qDebug("Notify callback status %d", m_serviceNotify.dwNotificationStatus);
    if ( m_serviceNotify.dwNotificationStatus == ERROR_SUCCESS )
        updateState(m_serviceNotify.ServiceStatus.dwCurrentState);

    DWORD err = NotifyServiceStatusChange(
        m_schService,
        SERVICE_NOTIFY_START_PENDING | SERVICE_NOTIFY_RUNNING |
        SERVICE_NOTIFY_STOP_PENDING | SERVICE_NOTIFY_STOPPED,
        &m_serviceNotify);
    if ( err != ERROR_SUCCESS )
        winerr("Notify status change", err);
}

void ServiceMgrWindows::updateState(DWORD state)
{
    const char* state_msg;
    ServiceMgr::ServiceState oldState = m_serviceState;

    switch(state)
    {
    case SERVICE_RUNNING:
        m_serviceState = Running;
        state_msg = "running";
        break;

    case SERVICE_START_PENDING:
        m_serviceState = Starting;
        state_msg = "starting";
        break;

    case SERVICE_STOP_PENDING:
        m_serviceState = Stopping;
        state_msg = "stopping";
        break;

    case SERVICE_STOPPED:
        m_serviceState = Stopped;
        state_msg = "stopped";
        break;

    default:
        m_serviceState = Unknown;
        state_msg = "unknown";
        qWarning("Unexpected status: %d", state);
        break;
    }

    if (oldState != m_serviceState
    //    || (m_mainwindow->getUpdateState() == MainWindow::UpdateState::Init
        || m_mainwindow->getUpdateState() == MainWindow::UpdateState::Probe) {
        m_mainwindow->statusMsg(QString("Status: Stubby service state is %1").arg(state_msg));
        emit serviceStateChanged(m_serviceState);
    }
}

int ServiceMgrWindows::getStateofService()
{
    //qInfo("getting service state");

    if ( m_schService ) {
        SERVICE_STATUS st;

        if ( QueryServiceStatus(
                 m_schService,              // service
                 &st                        // result
                 ) != 0 ) {
            updateState(st.dwCurrentState);
            return 0;
        }
    }

    winlasterr("Query service failed");
    return 1;
}

int ServiceMgrWindows::startService(QString configfile, int loglevel)
{
    qInfo("start service");

    TCHAR loglevelstr[2];
    loglevelstr[0] = '0' + loglevel;
    loglevelstr[1] = '\0';

    // QString to const char * must be done in two steps or else QByteArray is
    // destoyed before const data accessed.
    QByteArray conf_ba     = configfile.toUtf8();
    QByteArray def_conf_ba = m_ConfigFile.filePath().toUtf8();
    LPCTSTR args[2] = {
        loglevelstr,
        configfile.isEmpty() ? def_conf_ba.constData() : conf_ba.constData()
    };
    int nargs = 2;

    if ( m_schService && StartService(m_schService, nargs, args) )
        return 0;

    winlasterr("Start service failed");
    return 1;
}

int ServiceMgrWindows::stopService()
{
    qInfo("stop service");

    SERVICE_STATUS st;

    if ( m_schService && ControlService(m_schService, SERVICE_CONTROL_STOP, &st) )
        return 0;

    winlasterr("Stop service failed");
    return 1;
}

int ServiceMgrWindows::restartService()
{
    // Service start is triggered later from main windown
    qInfo("restart service");
    return stopService();
}

ServiceMgr *ServiceMgr::factory(MainWindow *parent)
{
    return new ServiceMgrWindows(parent);
}
