#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>
#include <QProcess>

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
    virtual int getState();

signals:
    void serviceStateChanged(ServiceMgr::ServiceState state);

protected:
    ServiceState m_serviceState;

private:
    MainWindow *m_mainwindow;

};

#endif // SERVICEMANAGER_H
