#ifndef SYSTEMDNSMGR_H
#define SYSTEMDNSMGR_H

#include <QObject>

#include "runtask_macos.h"

class SystemDNSMgr : public QObject
{
    Q_OBJECT

public:
    SystemDNSMgr(MainWindow *parent = 0);
    virtual ~SystemDNSMgr();

    typedef enum {
        NotLocalhost = 0,
        Localhost,
        Unknown
    } SystemDNSState;

    int setLocalhost();
    int unsetLocalhost();
    int getState();

public:
signals:
    void systemDNSStateChanged(SystemDNSMgr::SystemDNSState state);

private slots:
    void on_setLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_getSystemDNSState_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_getSystemDNSState_readyReadStdout();

private:
    SystemDNSState m_systemDNSState;
    MainWindow *m_mainwindow;
    RunHelperTaskMacos *m_setLocalhost;
    RunHelperTaskMacos *m_unsetLocalhost;
    RunHelperTaskMacos *m_getSystemDNSState;
    QString m_getSystemDNSState_output;
};

#endif // SYSTEMDNSMGR_H
