#ifndef SYSTEMDNSMGRWINDOWS_H
#define SYSTEMDNSMGRWINDOWS_H

#include <QObject>
#include <QProcess>

//#include "networkinterface.hpp"
#include "networkinterface_windows.hpp"
#include "systemdnsmanager.h"


class SystemDNSMgrWindows : public SystemDNSMgr
{
    Q_OBJECT

public:
    SystemDNSMgrWindows(MainWindow *parent = 0);
    virtual ~SystemDNSMgrWindows();

protected:
    int setLocalhostDNS();
    int unsetLocalhostDNS();
    int getStateDNS();

private:
    std::vector<std::unique_ptr<WindowsNetworkInterface>> getInterfaces();
    std::vector<WindowsNetworkInterface> interfaces;

    bool isRunning() const;
    bool isResolverLoopback() const;
    void reload();

};

#endif // SYSTEMDNSMGRWINDOWS_H
