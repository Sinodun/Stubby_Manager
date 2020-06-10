#ifndef NETWORKMGRWINDOWS_H
#define NETWORKMGRWINDOWS_H

#include <QObject>
#include <QProcess>

//#include "networkinterface.hpp"
#include "networkinterface_windows.hpp"
#include "networkmanager.h"


class NetworkMgrWindows : public NetworkMgr
{
    Q_OBJECT

public:
    NetworkMgrWindows(MainWindow *parent = 0);
    virtual ~NetworkMgrWindows();

protected:
    int setLocalhostDNS();
    int unsetLocalhostDNS();
    int getStateDNS();

private:
    std::vector<std::unique_ptr<NetworkInterfaceWindows>> getInterfaces();
    std::vector<NetworkInterfaceWindows> interfaces;

    bool isRunning() const;
    bool isResolverLoopback() const;
    void reload();

};

#endif // NETWORKMGRWINDOWS_H
