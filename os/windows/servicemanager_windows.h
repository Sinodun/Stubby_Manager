#ifndef SERVICEMGRWINDOWS_H
#define SERVICEMGRWINDOWS_H

#include <QProcess>

#include "servicemanager.h"

class MainWindow;


class ServiceMgrWindows : public ServiceMgr
{
    Q_OBJECT

public:
    ServiceMgrWindows(MainWindow *parent);
    virtual ~ServiceMgrWindows();

private:
    int getStateofService();
    int startService();
    int stopService();
    int restartService();

};

#endif // SERVICEMGRWINDOWS_H
