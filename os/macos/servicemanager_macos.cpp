#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"
#include "servicemanager_macos.h"
#include "runtask_macos.h"


ServiceMgrMacos::ServiceMgrMacos(MainWindow *parent) :
    ServiceMgr(parent),
    m_checkerProcess(0),
    m_starterProcess(0),
    m_stopperProcess(0),
    m_checkerProcess_output("")

{
    qInfo("Creating Macos service mgr");

    m_checkerProcess = new RunHelperTaskMacos("list", QString(), QString(), this, parent);
    connect(m_checkerProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_checkerProcess_finished(int, QProcess::ExitStatus)));
    connect(m_checkerProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(on_checkerProcess_readyReadStdout()));

    m_starterProcess = new RunHelperTaskMacos("start", RunHelperTaskMacos::RIGHT_DAEMON_RUN, QString(), this, parent);
    m_stopperProcess = new RunHelperTaskMacos("stop", RunHelperTaskMacos::RIGHT_DAEMON_RUN, QString(), this, parent);
}

ServiceMgrMacos::~ServiceMgrMacos()
{
}

int ServiceMgrMacos::getStateofService() {
    qInfo("gettting macos state");
    m_checkerProcess_output = "";
    m_checkerProcess->start();
    return 0;
}

int ServiceMgrMacos::startService()
{
    connect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_starterProcess_finished(int, QProcess::ExitStatus)));
    m_serviceState = Starting;
    m_starterProcess->start();
    return 0;
}

int ServiceMgrMacos::stopService()
{
    connect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_stopperProcess_finished(int, QProcess::ExitStatus)));
    m_serviceState = Stopping;
    m_stopperProcess->start();
    return 0;
}

int ServiceMgrMacos::restartService()
{
    m_serviceState = Stopping;
    connect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStopperProcess_finished(int, QProcess::ExitStatus)));
    m_stopperProcess->start();
    return 0;
}

/* private slots to handle QProcess signals */

void ServiceMgrMacos::on_checkerProcess_finished(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        /* search for "PID" = integer in the text */
        QRegularExpression pidRegEx("\"PID\" = \\d+;");
        QRegularExpressionMatch match;
        qInfo("%s", m_checkerProcess_output.toLatin1().data());
        match = pidRegEx.match(m_checkerProcess_output);
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

void ServiceMgrMacos::on_checkerProcess_readyReadStdout()
{
    m_checkerProcess_output = m_checkerProcess_output + QString::fromLatin1(m_checkerProcess->readAllStandardOutput().data());
}

void ServiceMgrMacos::on_starterProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_starterProcess_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgrMacos::on_restartStarterProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStarterProcess_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgrMacos::on_stopperProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_stopperProcess_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgrMacos::on_restartStopperProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStopperProcess_finished(int, QProcess::ExitStatus)));
    connect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStarterProcess_finished(int, QProcess::ExitStatus)));
    m_starterProcess->start();
}

ServiceMgr *ServiceMgr::factory(MainWindow *parent)
{
    return new ServiceMgrMacos(parent);
}
