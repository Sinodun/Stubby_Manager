#ifndef SERVICEMGRMACOS_H
#define SERVICEMGRMACOS_H

#include <QProcess>

#include "servicemanager.h"
#include "os/macos/runtask_macos.h"

class MainWindow;

class ServiceMgrMacos : public ServiceMgr
{
    Q_OBJECT

public:
    ServiceMgrMacos(MainWindow *parent);
    virtual ~ServiceMgrMacos();
    
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


private:
    int getStateofService();
    int startService();
    int stopService();
    int restartService();

};

#endif // SERVICEMGRMACOS_H
