/*#include <algorithm>
#include <exception>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>

#include <Poco/UnicodeConverter.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Path.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <ipifcons.h>
#include <processenv.h>
#include <processthreadsapi.h>
#include <shellapi.h>
#include <synchapi.h>
#include <versionhelpers.h>
#include <wlanapi.h>*/

#include <QDebug>

#include "mainwindow.h"
#include "systemdnsmanager_windows.h"


SystemDNSMgrWindows::SystemDNSMgrWindows(MainWindow *parent) :
    SystemDNSMgr(parent)
{
    //IP_ADAPTER_UNICAST_ADDRESS addr;
    //Poco::Net::IPAddress(addr.Address);
}

SystemDNSMgrWindows::~SystemDNSMgrWindows()
{
    //HANDLE handle_;
    //handle_ = nullptr;
    //WlanCloseHandle(handle_, nullptr);
}

int SystemDNSMgrWindows::setLocalhostDNS()
{
    return 0;
}

int SystemDNSMgrWindows::unsetLocalhostDNS()
{
    return 0;
}

int SystemDNSMgrWindows::getStateDNS()
{
    qInfo("gettting windows service state");
    m_mainwindow->statusMsg("Status: Stubby DNS state - not implemented.");
    return 0;
}


SystemDNSMgr *SystemDNSMgr::factory(MainWindow *parent) {
    return new SystemDNSMgrWindows(parent);
}
