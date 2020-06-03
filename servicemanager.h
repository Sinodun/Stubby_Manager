#ifndef SERVICEMGR_H
#define SERVICEMGR_H

#include <QObject>


class MainWindow;

class ServiceMgr : public QObject
{
    Q_OBJECT

public:
    ServiceMgr(MainWindow *parent);
    virtual ~ServiceMgr();

    static ServiceMgr * factory(MainWindow *parent);

    typedef enum {
        Stopped,
        Starting,
        Running,
        Stopping,
        Unknown,
        Error
    } ServiceState;

    int getState();
    int start();
    int stop();
    int restart();

signals:
    void serviceStateChanged(ServiceMgr::ServiceState state);

protected:
    MainWindow *m_mainwindow;
    ServiceState m_serviceState;

private:
    virtual int getStateofService() = 0;
    virtual int startService() = 0;
    virtual int stopService() = 0;
    virtual int restartService() = 0;

};

#endif // SERVICEMGR_H
