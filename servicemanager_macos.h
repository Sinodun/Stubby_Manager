#ifndef SERVICEMANAGER_MACOS_H
#define SERVICEMANAGER_MACOS_H

#include <QObject>

#include "servicemanager.h"
#include "runtask_macos.h"

class ServiceMgrMacos : public ServiceMgr
{
    Q_OBJECT

public:
    ServiceMgrMacos(MainWindow *parent);
    virtual ~ServiceMgrMacos();

//    int start();
//    int stop();
//    int restart();
    int getState();

private slots:
    void on_checkerProcess_finished(int, QProcess::ExitStatus);
    void on_checkerProcess_readyReadStderr();
    void on_checkerProcess_readyReadStdout();
    void on_checkerProcess_errorOccurred(QProcess::ProcessError);

private:
    RunHelperTaskMacos *m_checkerProcess;
    QString m_checkerProcess_output;

};

#endif // SERVICEMANAGER_MACOS_H
