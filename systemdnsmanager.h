#ifndef SYSTEMDNSMGR_H
#define SYSTEMDNSMGR_H

#include <QObject>

class MainWindow;


class SystemDNSMgr : public QObject
{
    Q_OBJECT

public:
    SystemDNSMgr(MainWindow *parent);
    virtual ~SystemDNSMgr();

    static SystemDNSMgr * factory(MainWindow *parent);

    typedef enum {
        NotLocalhost = 0,
        Localhost,
        Unknown
    } SystemDNSState;

    int setLocalhost();
    int unsetLocalhost();
    int getState();

signals:
    void systemDNSStateChanged(SystemDNSMgr::SystemDNSState state);

protected:
    virtual int setLocalhostDNS() = 0;
    virtual int unsetLocalhostDNS() = 0;
    virtual int getStateDNS() = 0;

    MainWindow *m_mainwindow;
    SystemDNSState m_systemDNSState;

};

#endif // SYSTEMDNSMGR_H
