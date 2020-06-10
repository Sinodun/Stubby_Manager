#ifndef NETWORKMGR_H
#define NETWORKMGR_H

#include <QObject>

class MainWindow;


class NetworkMgr : public QObject
{
    Q_OBJECT

public:
    NetworkMgr(MainWindow *parent);
    virtual ~NetworkMgr();

    static NetworkMgr * factory(MainWindow *parent);

    typedef enum {
        NotLocalhost = 0,
        Localhost,
        Unknown
    } NetworkState;

    int setLocalhost();
    int unsetLocalhost();
    int getState();

signals:
    void networkStateChanged(NetworkMgr::NetworkState state);

protected:
    virtual int setLocalhostDNS() = 0;
    virtual int unsetLocalhostDNS() = 0;
    virtual int getStateDNS() = 0;

    MainWindow *m_mainwindow;
    NetworkState m_networkState;

};

#endif // NETWORKMGR_H
