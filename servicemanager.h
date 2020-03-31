#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>

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
    int getState();

signals:
    void serviceStateChanged(ServiceMgr::ServiceState state);

private:
    MainWindow *m_mainwindow;
    ServiceState m_serviceState;

};

#endif // SERVICEMANAGER_H
