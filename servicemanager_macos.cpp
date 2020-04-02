#include <QDebug>
#include <QRegularExpression>

#include "mainwindow.h"
#include "runtask_macos.h"


ServiceMgr::ServiceMgr(MainWindow *parent) :
    QObject(parent),
    m_mainwindow(parent),
    m_serviceState(ServiceMgr::Unknown),
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

ServiceMgr::~ServiceMgr()
{
}

int ServiceMgr::getState() {
    qInfo("gettting macos state");
    m_checkerProcess_output = "";
    m_checkerProcess->start();
    return 0;
}

int ServiceMgr::start()
{
    if (m_serviceState == Unknown || m_serviceState == Stopped) {
        connect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_starterProcess_finished(int, QProcess::ExitStatus)));
        m_serviceState = Starting;
        m_starterProcess->start();
    }
    return 0;
}

int ServiceMgr::stop()
{
    if (m_serviceState == Unknown || m_serviceState == Running) {
        connect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_stopperProcess_finished(int, QProcess::ExitStatus)));
        m_serviceState = Stopping;
        m_stopperProcess->start();
    }
    return 0;
}

int ServiceMgr::restart()
{
    if (m_serviceState == Unknown || m_serviceState == Running) {
        m_serviceState = Stopping;
        connect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStopperProcess_finished(int, QProcess::ExitStatus)));
        m_stopperProcess->start();
    }
    return 0;
}

/* private slots to handle QProcess signals */

void ServiceMgr::on_checkerProcess_finished(int, QProcess::ExitStatus exitStatus)
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
        } else {
            m_serviceState = Stopped;
            emit serviceStateChanged(m_serviceState);
        }
    } else {
        qInfo("Checker process failed");
        m_serviceState = Unknown;
        emit serviceStateChanged(m_serviceState);
    }
}

void ServiceMgr::on_checkerProcess_readyReadStdout()
{
    m_checkerProcess_output = m_checkerProcess_output + QString::fromLatin1(m_checkerProcess->readAllStandardOutput().data());
}

void ServiceMgr::on_starterProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_starterProcess_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgr::on_restartStarterProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStarterProcess_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgr::on_stopperProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_stopperProcess_finished(int, QProcess::ExitStatus)));
    getState();
}

void ServiceMgr::on_restartStopperProcess_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug("Exit status %d, launchstl exit code %d", exitStatus, exitCode);
    disconnect(m_stopperProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStopperProcess_finished(int, QProcess::ExitStatus)));
    connect(m_starterProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_restartStarterProcess_finished(int, QProcess::ExitStatus)));
    m_starterProcess->start();
}
