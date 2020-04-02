#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>
#include <QProcess>

#include "runtask_macos.h"

class MainWindow;

class ServiceMgr : public QObject
{
    Q_OBJECT

public:
    ServiceMgr(MainWindow *parent);
    virtual ~ServiceMgr();

    typedef enum {
        Stopped,
        Starting,
        Running,
        Stopping,
        Unknown,
        Error
    } ServiceState;

    int getState();
    int start();
    int stop();
    int restart();

signals:
    void serviceStateChanged(ServiceMgr::ServiceState state);

private:
    MainWindow *m_mainwindow;
    ServiceState m_serviceState;
    
private slots:
    void on_checkerProcess_finished(int, QProcess::ExitStatus);
    void on_checkerProcess_readyReadStdout();
    void on_starterProcess_finished(int, QProcess::ExitStatus);
    void on_stopperProcess_finished(int, QProcess::ExitStatus);
    void on_restartStarterProcess_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_restartStopperProcess_finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    RunHelperTaskMacos *m_checkerProcess;
    RunHelperTaskMacos *m_starterProcess;
    RunHelperTaskMacos *m_stopperProcess;
    QString m_checkerProcess_output;

};

#endif // SERVICEMGR_H
