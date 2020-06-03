#ifndef SYSTEMDNSMGRMACOS_H
#define SYSTEMDNSMGRMACOS_H

#include <QObject>
#include <QProcess>

#include "mainwindow.h"
#include "systemdnsmanager.h"
#include "os/macos/runtask_macos.h"

class SystemDNSMgrMacos : public SystemDNSMgr
{
    Q_OBJECT

public:
    SystemDNSMgrMacos(MainWindow *parent = 0);
    virtual ~SystemDNSMgrMacos();

protected:
    int setLocalhostDNS();
    int unsetLocalhostDNS();
    int getStateDNS();

private slots:
    void on_setLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_getSystemDNSState_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_getSystemDNSState_readyReadStdout();

private:
    RunHelperTaskMacos *m_setLocalhost;
    RunHelperTaskMacos *m_unsetLocalhost;
    RunHelperTaskMacos *m_getSystemDNSState;
    QString m_getSystemDNSState_output;
};

#endif // SYSTEMDNSMGRMACOS_H
