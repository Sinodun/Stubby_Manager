#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

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

//    int start();
//    int stop();
//    int restart();
    virtual int getState();

signals:
    void serviceStateChanged(ServiceMgr::ServiceState state);

private:
    MainWindow *m_mainwindow;
    ServiceState m_serviceState;
    
private slots:
    void on_checkerProcess_finished(int, QProcess::ExitStatus);
    void on_checkerProcess_readyReadStderr();
    void on_checkerProcess_readyReadStdout();
    void on_checkerProcess_errorOccurred(QProcess::ProcessError);

private:
    RunHelperTaskMacos *m_checkerProcess;
    QString m_checkerProcess_output;

};

#endif // SERVICEMANAGER_H
