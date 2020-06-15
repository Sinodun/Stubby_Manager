#include <QDebug>

#include <windows.h>

#include "mainwindow.h"
#include "servicemanager_windows.h"

const char SVCNAME[] = "Stubby";

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
    ServiceMgr(parent)

{
    qInfo("Creating Windows service mgr");
}

ServiceMgrWindows::~ServiceMgrWindows()
{
}

int ServiceMgrWindows::getStateofService() {
    qInfo("getting windows state");

    m_serviceState = Error;
    const char* state = "error";

    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        STANDARD_RIGHTS_READ);   // Just read

    if ( schSCManager ) {
        schService = OpenService(
            schSCManager,              // SCM database
            SVCNAME,                   // name of service
            SERVICE_QUERY_STATUS);     // intention

        if ( schService ) {
            SERVICE_STATUS st;

            if ( QueryServiceStatus(
                     schService,                // service
                     &st                        // result
                     ) != 0 ) {
                switch (st.dwCurrentState)
                {
                case SERVICE_RUNNING:
                    m_serviceState = Running;
                    state = "running";
                    break;

                case SERVICE_START_PENDING:
                    m_serviceState = Starting;
                    state = "starting";
                    break;

                case SERVICE_STOP_PENDING:
                    m_serviceState = Stopping;
                    state = "stopping";
                    break;

                case SERVICE_STOPPED:
                    m_serviceState = Stopped;
                    state = "stopped";
                    break;

                default:
                    m_serviceState = Unknown;
                    state = "unknown";
                    qWarning("Unexpected status: %d", st.dwCurrentState);
                    break;
                }
            } else {
                winlasterr("Query service failed");
            }

            CloseServiceHandle(schService);
        } else {
            winlasterr("Open service failed");
        }

        CloseServiceHandle(schSCManager);
    }
    else
        winlasterr("Open service manager failed");

    emit serviceStateChanged(m_serviceState);
    m_mainwindow->statusMsg(QString("Status: Stubby service state is %1").arg(state));
    return (m_serviceState == Error);
}

int ServiceMgrWindows::startService()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    int res = 1;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        STANDARD_RIGHTS_WRITE);  // Just read

    if ( schSCManager ) {
        schService = OpenService(
            schSCManager,              // SCM database
            SVCNAME,                   // name of service
            SERVICE_START);            // intention

        if ( schService ) {
            if ( StartService(schService, 0, NULL) )
                res = 0;
            else
                winlasterr("Start service failed");

            CloseServiceHandle(schService);
        } else {
            winlasterr("Open service failed");
        }

        CloseServiceHandle(schSCManager);
    }
    else
        winlasterr("Open service manager failed");

    getStateofService();
    return res;
}

int ServiceMgrWindows::stopService()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    int res = 1;

    schSCManager = OpenSCManager(
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        STANDARD_RIGHTS_WRITE);  // Just read

    if ( schSCManager ) {
        schService = OpenService(
            schSCManager,              // SCM database
            SVCNAME,                   // name of service
            SERVICE_STOP);             // intention

        if ( schService ) {
            SERVICE_STATUS st;

            if ( ControlService(schService, SERVICE_CONTROL_STOP, &st) )
                res = 0;
            else
                winlasterr("Start service failed");

            CloseServiceHandle(schService);
        } else {
            winlasterr("Open service failed");
        }

        CloseServiceHandle(schSCManager);
    }
    else
        winlasterr("Open service manager failed");

    getStateofService();
    return res;
}

int ServiceMgrWindows::restartService()
{
    stopService();
    return startService();
}


ServiceMgr *ServiceMgr::factory(MainWindow *parent)
{
    return new ServiceMgrWindows(parent);
}
