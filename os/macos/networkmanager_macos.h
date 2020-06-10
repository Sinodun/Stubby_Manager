#ifndef NETWORKMGRMACOS_H
#define NETWORKMGRMACOS_H

#include <QObject>
#include <QProcess>

#include "networkmanager.h"
#include "os/macos/runtask_macos.h"


class NetworkMgrMacos : public NetworkMgr
{
    Q_OBJECT

public:
    NetworkMgrMacos(MainWindow *parent = 0);
    virtual ~NetworkMgrMacos();

protected:
    int setLocalhostDNS();
    int unsetLocalhostDNS();
    int getStateDNS();

private slots:
    void on_setLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_unsetLocalhost_finished(int exitCode, QProcess::ExitStatus);
    void on_getNetworkState_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_getNetworkState_readyReadStdout();

private:
    RunHelperTaskMacos *m_setLocalhost;
    RunHelperTaskMacos *m_unsetLocalhost;
    RunHelperTaskMacos *m_getNetworkState;
    QString m_getNetworkState_output;
};

#endif // NETWORKMGRMACOS_H
