#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"
#include "servicemanager_macos.h"


ServiceMgrMacos::ServiceMgrMacos(MainWindow *parent) :
    ServiceMgr(parent),
    m_getStateofService(0),
    m_startService(0),
    m_stopService(0),
    m_getStateofService_output("")

{
    qInfo("Creating Macos service mgr");

    m_getStateofService = new RunHelperTaskMacos("list", QString(), QString(), this, parent);
    connect(m_getStateofService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_getStateofService_finished(int, QProcess::ExitStatus)));
    connect(m_getStateofService, SIGNAL(readyReadStandardOutput()), this, SLOT(on_getStateofService_readyReadStdout()));

    m_startService = new RunHelperTaskMacos("start", RunHelperTaskMacos::RIGHT_DAEMON_RUN, QString(), this, parent);
    m_stopService = new RunHelperTaskMacos("stop", RunHelperTaskMacos::RIGHT_DAEMON_RUN, QString(), this, parent);
}

ServiceMgrMacos::~ServiceMgrMacos()
{
}

int ServiceMgrMacos::getStateofService() {
    qInfo("gettting macos state");
    m_getStateofService_output = "";
    m_getStateofService->start();
    return 0;
}

int ServiceMgrMacos::startService()
{
    connect(m_startService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_startService_finished(int, QProcess::ExitStatus)));
    m_startService->start();
    return 0;
}

int ServiceMgrMacos::stopService()
{
    connect(m_stopService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_stopService_finished(int, QProcess::ExitStatus)));
    m_stopService->start();
    return 0;
}

int ServiceMgrMacos::restartService()
{
    m_serviceState = Stopping;
    connect(m_stopService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartServiceStopper_finished(int, QProcess::ExitStatus)));
    m_stopService->start();
    return 0;
}

/* private slots to handle QProcess signals */

void ServiceMgrMacos::on_getStateofService_finished(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        /* search for "PID" = integer in the text */
        QRegularExpression pidRegEx("\"PID\" = \\d+;");
        QRegularExpressionMatch match;
        qInfo("%s", m_getStateofService_output.toLatin1().data());
        match = pidRegEx.match(m_getStateofService_output);
        if (match.hasMatch()) {
            m_serviceState = Running;
            emit serviceStateChanged(m_serviceState);
            m_mainwindow->statusMsg("Status: Stubby service running.");
        } else {
            m_serviceState = Stopped;
            emit serviceStateChanged(m_serviceState);
            m_mainwindow->statusMsg("Status: Stubby service stopped.");
        }
    } else {
        qInfo("Checker process failed");
        m_serviceState = Unknown;
        emit serviceStateChanged(m_serviceState);
        m_mainwindow->statusMsg("Status: Problem with Stubby service - state unknown.");
    }
}

void ServiceMgrMacos::on_getStateofService_readyReadStdout()
{
    m_getStateofService_output = m_getStateofService_output + QString::fromLatin1(m_getStateofService->readAllStandardOutput().data());
}

void ServiceMgrMacos::on_startService_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_startService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_startService_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgrMacos::on_restartServiceStarter_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_startService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartServiceStarter_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgrMacos::on_stopService_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_stopService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_stopService_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgrMacos::on_restartServiceStopper_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    m_serviceState = Starting;
    disconnect(m_stopService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartServiceStopper_finished(int, QProcess::ExitStatus)));
    connect(m_startService, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartServiceStarter_finished(int, QProcess::ExitStatus)));
    m_startService->start();
}

ServiceMgr *ServiceMgr::factory(MainWindow *parent)
{
    return new ServiceMgrMacos(parent);
}
