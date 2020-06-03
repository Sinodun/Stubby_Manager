#ifndef SYSTEMDNSMGRWINDOWS_H
#define SYSTEMDNSMGRWINDOWS_H

#include <QObject>
#include <QProcess>

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

};

#endif // SYSTEMDNSMGRWINDOWS_H
