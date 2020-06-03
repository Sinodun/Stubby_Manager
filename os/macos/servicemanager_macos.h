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
    void on_getStateofService_finished(int, QProcess::ExitStatus);
    void on_getStateofService_readyReadStdout();
    void on_startService_finished(int, QProcess::ExitStatus);
    void on_stopService_finished(int, QProcess::ExitStatus);
    void on_restartServiceStarter_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_restartServiceStopper_finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    RunHelperTaskMacos *m_getStateofService;
    RunHelperTaskMacos *m_startService;
    RunHelperTaskMacos *m_stopService;
    QString m_getStateofService_output;


private:
    int getStateofService();
    int startService();
    int stopService();
    int restartService();

};

#endif // SERVICEMGRMACOS_H
