/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <exception>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <Poco/UnicodeConverter.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Path.h>

#include <winsock2.h>

#include <Iphlpapi.h>
#include <ipifcons.h>
#include <processenv.h>
#include <processthreadsapi.h>
#include <shellapi.h>
#include <synchapi.h>
#include <versionhelpers.h>
#include <wlanapi.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>

#include "exceptions_windows.h"
#include "mainwindow.h"
#include "networkmanager_windows.h"

#include "networkinterface_windows.hpp"

#if (_WIN32_WINNT < 0x0601) || (NTDDI_VERSION < 0x06010100)
#  error Windows 7 or later required.
#endif

const ULONG INITIAL_GAA_BUFFER_SIZE = 15*1024;
const ULONG MAX_GAA_TRIES = 3;


QueryTaskWindows::QueryTaskWindows(QObject *parent) :
    QProcess(parent)
{
    // getdns_query.exe is in the same directory as this executable.
    QFileInfo getdns_query = QFileInfo(QDir(QCoreApplication::applicationDirPath()), "getdns_query.exe");
    setProgram(getdns_query.filePath());
    QStringList arguments;
    arguments << "@127.0.0.1" << "getdnsapi.net";
    setArguments(arguments);
}

QueryTaskWindows::~QueryTaskWindows()
{
    if (state() == Running) {
        terminate();
        waitForFinished();
    }
}

void QueryTaskWindows::start()
{
    QProcess::start(ReadWrite);
}


static std::string pwchar_to_string(const PWCHAR pwchar)
{
    std::string res;
    Poco::UnicodeConverter::toUTF8(pwchar, res);
    return res;
}

static std::string GUID_to_string(const GUID& guid)
{
    std::ostringstream res;
    res << std::hex << std::uppercase << std::noshowbase
        << std::left << std::setfill('0')
        << '{' << std::setw(8) << guid.Data1
        << '-' << std::setw(4) << guid.Data2
        << '-' << guid.Data3 << std::setw(0)
        << '-' << std::setw(2) << +guid.Data4[0] << +guid.Data4[1]
        << '-' << std::setw(2);
    for ( int i = 2; i < sizeof(guid.Data4); ++i )
        res << +guid.Data4[i];
    res << std::setw(0) << '}';
    return res.str();
}

static void run_dns_process(std::string params = "")
{
    // We're calling a script StubbySetDns.ps1 that we declare must be in the
    // same directory as the current executable.
    QFileInfo script_path(QDir(QCoreApplication::applicationDirPath()), "StubbySetDns.ps1");

    if ( !script_path.isFile() )
        throw windows_system_error("StubbySetDns.ps1 not found");

    // Search system PATH for powershell.exe.
    std::vector<char> powershell_path;
    LPSTR fname_part;
    DWORD ret_val;

    do
    {
        powershell_path.reserve(powershell_path.capacity() + MAX_PATH);
        ret_val = SearchPathA(NULL, "powershell.exe", NULL,
                              static_cast<DWORD>(powershell_path.capacity()),
                              powershell_path.data(),
                              &fname_part);
        if ( ret_val == 0 )
            throw windows_system_error("SearchPathA()");
    } while ( ret_val >= powershell_path.capacity() );

    std::string powershell(powershell_path.data());

    SHELLEXECUTEINFOA info{0};
    std::string cmd = "-NoLogo -NonInteractive -WindowStyle Hidden -ExecutionPolicy Unrestricted -File \"" + script_path.filePath().toStdString() + "\" " + params;

    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.hwnd = NULL;
    info.lpVerb = "runas";
    info.lpFile = powershell.c_str();
    info.lpParameters = cmd.c_str();
    info.lpDirectory = NULL;
    info.nShow = SW_HIDE;

    if ( !ShellExecuteExA(&info) )
        throw windows_system_error("ShellExecuteExA()");

    if ( WaitForSingleObject(info.hProcess, INFINITE) == WAIT_FAILED )
        throw windows_system_error("WaitForSingleObject()");

    DWORD exit_code;

    if ( !GetExitCodeProcess(info.hProcess, &exit_code) )
        throw windows_system_error("GetExitCodeProcess()");

    if ( exit_code != 0 )
        throw std::runtime_error(QString("%1 %2 exit code %3").arg(QString(powershell.c_str()), QString(cmd.c_str()), QString("%1").arg(exit_code)).toStdString());
}

class WlanApi
{
public:
    WlanApi();
    virtual ~WlanApi();

    std::map<std::string, std::string> connected_SSIDs() const;

    bool wifi_present() const
    {
        return wifi_present_;
    }

private:
    HANDLE handle_;
    bool wifi_present_;
};

WlanApi::WlanApi()
    : handle_(0), wifi_present_(false)
{
    DWORD negotiated_api;
    DWORD ret_val = WlanOpenHandle(2, nullptr, &negotiated_api, &handle_);

    wifi_present_ = ( ret_val == ERROR_SUCCESS );
}

WlanApi::~WlanApi()
{
    if ( wifi_present_ )
        WlanCloseHandle(handle_, nullptr);
}

std::map<std::string, std::string> WlanApi::connected_SSIDs() const
{
    std::map<std::string, std::string> res;

    if ( !wifi_present_ )
        return res;

    PWLAN_INTERFACE_INFO_LIST info_list = nullptr;
    DWORD ret_val = WlanEnumInterfaces(handle_, nullptr, &info_list);

    if ( ret_val != ERROR_SUCCESS )
        throw windows_system_error("WlanEnumInterfaces()");

    std::unique_ptr<WLAN_INTERFACE_INFO_LIST,
                    std::function<void(WLAN_INTERFACE_INFO_LIST*)>>
        ilist(info_list,
              [](WLAN_INTERFACE_INFO_LIST* ptr)
              {
                  WlanFreeMemory(ptr);
              });

    for ( DWORD i = 0; i < ilist->dwNumberOfItems; ++i )
    {
        PWLAN_INTERFACE_INFO info = &ilist->InterfaceInfo[i];

        if ( info->isState == wlan_interface_state_connected )
        {
            PWLAN_AVAILABLE_NETWORK_LIST network_list = nullptr;
            DWORD ret_val = WlanGetAvailableNetworkList(handle_, &info->InterfaceGuid, 0, NULL, &network_list);

            if ( ret_val != ERROR_SUCCESS )
                throw windows_system_error(ret_val, "WlanGetAvailableNetworkList()");

            for ( int n = 0; n < network_list->dwNumberOfItems; ++i )
            {
                PWLAN_AVAILABLE_NETWORK net = &network_list->Network[n];

                if ( net->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED )
                {
                    ULONG ssid_len = net->dot11Ssid.uSSIDLength;
                    const char* ssid = reinterpret_cast<const char*>(net->dot11Ssid.ucSSID);

                    res.emplace(std::make_pair(GUID_to_string(info->InterfaceGuid),
                                               std::string(ssid, ssid_len)));
                    break;
                }
            }

            WlanFreeMemory(network_list);
        }
    }

    return res;
}

std::vector<std::unique_ptr<NetworkInterfaceWindows>> NetworkMgrWindows::getInterfaces()
{
    std::vector<std::unique_ptr<NetworkInterfaceWindows>> res;

    for ( auto iface : interfaces )
        res.emplace_back(std::make_unique<NetworkInterfaceWindows>(iface));
    return res;
}


NetworkMgrWindows::NetworkMgrWindows(MainWindow *parent) :
    NetworkMgr(parent), m_networkConfig()
{
    m_testQuery = new QueryTaskWindows(this);
    connect(m_testQuery, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_testQuery_finished(int, QProcess::ExitStatus)));

    connect(&m_networkConfig, &QNetworkConfigurationManager::configurationAdded,
            this, &NetworkMgrWindows::on_networkInferfaces_changed);
    connect(&m_networkConfig, &QNetworkConfigurationManager::configurationChanged,
            this, &NetworkMgrWindows::on_networkInferfaces_changed);
    connect(&m_networkConfig, &QNetworkConfigurationManager::configurationRemoved,
            this, &NetworkMgrWindows::on_networkInferfaces_changed);
}

NetworkMgrWindows::~NetworkMgrWindows()
{

}

int NetworkMgrWindows::setLocalhostDNS()
{
    try {
        run_dns_process("-Loopback");
    } catch (const std::runtime_error& e) {
        m_mainwindow->statusMsg(QString("*Error* when trying to update system DNS settings: %1").arg(QString(e.what())));
    }
    getStateDNS(true);
    return 0;
}

int NetworkMgrWindows::unsetLocalhostDNS()
{
    try {
        run_dns_process("");
    } catch (const std::runtime_error& e) {
        m_mainwindow->statusMsg(QString("*Error* when trying to update system DNS settings: %1").arg(QString(e.what())));
    }
    getStateDNS(true);
    return 0;
}

int NetworkMgrWindows::getStateDNS(bool reportNoChange)
{
    int oldNetworkState = m_networkState;
    std::map<std::string, NetworkMgr::interfaceInfo> previous_networks = getNetworks();
    reload();
    std::map<std::string, NetworkMgr::interfaceInfo> running_networks = getNetworks();

    bool networksChanged = false;
    if (previous_networks.size() != running_networks.size())
        networksChanged = true;
    std::map<std::string, NetworkMgr::interfaceInfo>::const_iterator i,j;
    for(i = previous_networks.begin(), j = running_networks.begin(); i != previous_networks.end(); ++i, ++j) {
            if ( i->first.compare(j->first) != 0 ||
                 i->second.interfaceType != j->second.interfaceType ||
                 i->second.interfaceActive != j->second.interfaceActive) {
                   networksChanged = true;
                   break;
                }
     }
     if (networksChanged) {
         qInfo("Networks have changed");
         m_mainwindow->refreshNetworks(running_networks);
     }

    if (isResolverLoopback()) {
      m_networkState = Localhost;
      if (oldNetworkState != m_networkState || reportNoChange == true) {
        m_mainwindow->statusMsg("Status: DNS settings using localhost.");
      }
      emit DNSStateChanged(Localhost);
    } else {
        m_networkState = NotLocalhost;
        if (oldNetworkState != m_networkState  || reportNoChange == true)
            m_mainwindow->statusMsg("Status: DNS settings NOT using localhost.");
        emit DNSStateChanged(NotLocalhost);
    }
    return 0;
}

void NetworkMgrWindows::reload()
{
    if ( !IsWindows7OrGreater() )
        throw std::runtime_error("Windows 7 or later required.");

    WlanApi wlan_api;
    auto connected_ssids = wlan_api.connected_SSIDs();

    ULONG gaa_flags = (GAA_FLAG_SKIP_ANYCAST |
                       GAA_FLAG_SKIP_MULTICAST);
    std::unique_ptr<IP_ADAPTER_ADDRESSES[]> addresses;
    std::size_t gaa_bufsiz = INITIAL_GAA_BUFFER_SIZE / sizeof(IP_ADAPTER_ADDRESSES);
    ULONG iterations = 0;
    DWORD ret_val;

    do {
        ULONG out_buf_len = static_cast<ULONG>(gaa_bufsiz * sizeof(IP_ADAPTER_ADDRESSES));
        addresses = std::make_unique<IP_ADAPTER_ADDRESSES[]>(gaa_bufsiz);
        ret_val = GetAdaptersAddresses(AF_UNSPEC, gaa_flags, nullptr,
                                       addresses.get(), &out_buf_len);
        gaa_bufsiz = (out_buf_len / sizeof(IP_ADAPTER_ADDRESSES)) + 1;
    } while ( ret_val == ERROR_BUFFER_OVERFLOW && ++iterations <= MAX_GAA_TRIES );

    if ( ret_val != NO_ERROR )
        throw windows_system_error(ret_val, "GetAdaptersAddress()");

    interfaces.clear();

    for ( PIP_ADAPTER_ADDRESSES adapter = addresses.get();
          adapter;
          adapter = adapter->Next )
    {
        if ( adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
             adapter->IfType == IF_TYPE_OTHER)
            continue;

        std::string name(pwchar_to_string(adapter->FriendlyName));
        std::string adapter_name(adapter->AdapterName);
        std::string description(pwchar_to_string(adapter->Description));
        std::string dns_suffix(pwchar_to_string(adapter->DnsSuffix));

        // Need to ignore the Wi-Fi-Direct interfaces, but can't find a better way to identify them...
        if (!description.find("Microsoft Wi-Fi Direct Virtual Adapter", 0))
            continue;

        bool resolver_loopback = true;
        bool running = false;

        // We consider the interface up if it is reported as
        // up AND there is a non-link-local address allocated.
        if ( adapter->OperStatus == IfOperStatusUp )
        {
            for ( auto addr = adapter->FirstUnicastAddress;
                  addr;
                  addr = addr->Next )
            {
                auto uni_addr = Poco::Net::IPAddress(addr->Address);
                if ( !uni_addr.isLinkLocal() )
                {
                    running = true;
                    break;
                }
            }
        }

        for ( auto resolver_addr = adapter->FirstDnsServerAddress;
              resolver_addr;
              resolver_addr = resolver_addr->Next )
        {
            auto dns_addr = Poco::Net::IPAddress(resolver_addr->Address);
            if ( !dns_addr.isLoopback() )
            {
                resolver_loopback = false;
                break;
            }
        }

        std::string ssid;

        if ( wlan_api.wifi_present() && adapter->IfType == IF_TYPE_IEEE80211 )
        {
            // adapter_name seems to have interface GUID in it for Wifi.
            auto search = connected_ssids.find(adapter_name);
            if ( search != connected_ssids.end() )
                ssid = search->second;
        }

        qDebug("Interface %30s running state is %d, localhost state is %d, ssid is %s, adaptor type is %d, op status %d", name.c_str(), running, resolver_loopback, ssid.c_str(), adapter->IfType, adapter->OperStatus);
        interfaces.emplace_back(
                name,
                adapter_name,
                description,
                dns_suffix,
                ssid,
                resolver_loopback,
                running,
                adapter->IfType,
                adapter->OperStatus);
    }
}

bool NetworkMgrWindows::isResolverLoopback() const
{
   if ( interfaces.empty() ) {
        qDebug("No interfaces found");
        return false;
   }

    return std::all_of(interfaces.begin(), interfaces.end(),
                       [](const auto& iface)
                       {
                          return (iface.is_running() ? iface.is_resolver_loopback() : true);
                       });
}

bool NetworkMgrWindows::isRunning() const
{
    if ( interfaces.empty() )
        return false;

    return std::any_of(interfaces.begin(), interfaces.end(),
                       [](const auto& iface)
                       {
                           return iface.is_running();
                       });
    return true;
}

std::map<std::string, NetworkMgr::interfaceInfo> NetworkMgrWindows::getNetworks()
{
    std::map<std::string, NetworkMgr::interfaceInfo> res;
    std::string network;
    std::string activeNetwork;

    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkList\\Profiles",
                       QSettings::NativeFormat);
    QStringList groups = settings.childGroups();
    // Best guess is that we have only one active network....
    // TODO: This name is the registry entry and is not updated by changing the network name using
    // the local security policy GUI... however I cannot find how to access that name so this is
    // the next best option
    if (groups.size() == 1 ) {
        QString path = groups.first();
        path.append("/ProfileName");
        QVariant activeNetworkName = settings.value(path);
        if (!activeNetworkName.isNull()) {
            activeNetwork.append(" (");
            activeNetwork.append(activeNetworkName.toString().toUtf8().constData());
            activeNetwork.append(")");
        }
    }


    if ( interfaces.empty() )
        return res;

    for (const auto& i: interfaces) {
        NetworkMgr::interfaceInfo info;
            network = i.name();
            info.interfaceActive=i.is_running();
            if (i.is_wireless()) {
                info.interfaceType=NetworkMgr::InterfaceTypes::WiFi;
                if (!i.ssid().empty()) {
                    network = i.ssid();
                }
            } else if ( i.is_ethernet() ) {
                    info.interfaceType=NetworkMgr::InterfaceTypes::Ethernet;
                    network.append(activeNetwork);
            }
            res.emplace(network, info);
    }
    return res;
}

int NetworkMgrWindows::testQuery() {
    m_testQuery->start();
    return 0;
}

void NetworkMgrWindows::on_testQuery_finished(int, QProcess::ExitStatus)
{
    QByteArray stdoutData;
    stdoutData = m_testQuery->readAllStandardOutput();
    if (stdoutData.isEmpty()) {
        qDebug("Test query returned no data");
        emit testQueryResult(false);
        return;
    }
    if (stdoutData.contains("NOERROR")) {
        qDebug() << __FILE__ << ":" << __FUNCTION__ << "OK";
        emit testQueryResult(true);
    }
    else {
        qDebug() << __FILE__ << ":" << __FUNCTION__ << stdoutData;
        emit testQueryResult(false);
    }
}

void NetworkMgrWindows::on_networkInferfaces_changed(const QNetworkConfiguration&)
{
    //qDebug("Network configuration changed");
    getStateDNS(false);
}

NetworkMgr *NetworkMgr::factory(MainWindow *parent) {
    return new NetworkMgrWindows(parent);
}
