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

#include <QDebug>

#include "mainwindow.h"
#include "systemdnsmanager_windows.h"

#include "networkinterface_windows.hpp"

#if (_WIN32_WINNT < 0x0601) || (NTDDI_VERSION < 0x06010100)
#  error Windows 7 or later required.
#endif

const ULONG INITIAL_GAA_BUFFER_SIZE = 15*1024;
const ULONG MAX_GAA_TRIES = 3;

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
    std::vector<char> fpath;
    DWORD ret_val;

    do
    {
        fpath.reserve(fpath.capacity() + MAX_PATH);
        ret_val = GetModuleFileNameA(NULL, fpath.data(), static_cast<DWORD>(fpath.capacity()));
        if ( ret_val == 0 )
            throw std::system_error(GetLastError(), std::system_category(), "GetModuleFileNameA()");
    } while ( ret_val == fpath.capacity() );

    Poco::Path script_path(fpath.data());

    script_path.setFileName("StubbySetDns");
    script_path.setExtension("ps1");

    if ( !script_path.isFile() )
        throw std::system_error(GetLastError(), std::system_category(), "StubbySetDns.ps1 not found");

    // Search system PATH for powershell.exe.
    std::vector<char> powershell_path;
    LPSTR fname_part;

    do
    {
        powershell_path.reserve(powershell_path.capacity() + MAX_PATH);
        ret_val = SearchPathA(NULL, "powershell.exe", NULL,
                              static_cast<DWORD>(powershell_path.capacity()),
                              powershell_path.data(),
                              &fname_part);
        if ( ret_val == 0 )
            throw std::system_error(GetLastError(), std::system_category(), "SearchPathA()");
    } while ( ret_val >= powershell_path.capacity() );

    std::string powershell(powershell_path.data());

    SHELLEXECUTEINFOA info{0};
    std::string cmd = "-NoLogo -NonInteractive -WindowStyle Hidden -ExecutionPolicy Unrestricted -File " + script_path.toString() + " " + params;

    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.hwnd = NULL;
    info.lpVerb = "runas";
    info.lpFile = powershell.c_str();
    info.lpParameters = cmd.c_str();
    info.lpDirectory = NULL;
    info.nShow = SW_HIDE;

    if ( !ShellExecuteExA(&info) )
        throw std::system_error(GetLastError(), std::system_category(), "ShellExecuteExA()");

    if ( WaitForSingleObject(info.hProcess, INFINITE) == WAIT_FAILED )
        throw std::system_error(GetLastError(), std::system_category(), "WaitForSingleObject()");

    DWORD exit_code;

    if ( !GetExitCodeProcess(info.hProcess, &exit_code) )
        throw std::system_error(GetLastError(), std::system_category(), "GetExitCodeProcess()");

    if ( exit_code != 0 )
        throw std::system_error(exit_code, std::system_category(), "StubbySetDns.ps1 exit code");
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
        throw std::system_error(ret_val, std::system_category(), "WlanEnumInterfaces()");

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
                throw std::system_error(ret_val, std::system_category(), "WlanGetAvailableNetworkList()");

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

std::vector<std::unique_ptr<WindowsNetworkInterface>> SystemDNSMgrWindows::getInterfaces()
{
    std::vector<std::unique_ptr<WindowsNetworkInterface>> res;

    for ( auto iface : interfaces )
        res.emplace_back(std::make_unique<WindowsNetworkInterface>(iface));
    return res;
}


SystemDNSMgrWindows::SystemDNSMgrWindows(MainWindow *parent) :
    SystemDNSMgr(parent)
{
}

SystemDNSMgrWindows::~SystemDNSMgrWindows()
{

}

int SystemDNSMgrWindows::setLocalhostDNS()
{
    try {
        run_dns_process("-Loopback");
    } catch (...) {
        m_mainwindow->statusMsg("*Error* when trying to update system DNS settings: ");
    }
    getStateDNS();
    return 0;
}

int SystemDNSMgrWindows::unsetLocalhostDNS()
{
    run_dns_process("");
    getStateDNS();
    return 0;
}

int SystemDNSMgrWindows::getStateDNS()
{
    reload();
    if (isResolverLoopback()) {
      m_systemDNSState = Localhost;
      emit systemDNSStateChanged(Localhost);
      m_mainwindow->statusMsg("Status: DNS settings using localhost.");
    } else {
        m_systemDNSState = NotLocalhost;
        emit systemDNSStateChanged(NotLocalhost);
        m_mainwindow->statusMsg("Status: DNS settings NOT using localhost.");
    }
    return 0;
}

void SystemDNSMgrWindows::reload()
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
        throw std::system_error(ret_val, std::system_category(), "GetAdaptersAddress()");

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

        qDebug("Interface %30s running state is %d, loopback state is %d, ssid is %s, adaptor type is %d", name.c_str(), running, resolver_loopback, ssid.c_str(), adapter->IfType);
        interfaces.emplace_back(
                name,
                adapter_name,
                description,
                dns_suffix,
                ssid,
                resolver_loopback,
                running,
                adapter->IfType);
                //adapter->OperStatus);*/
    }
}

bool SystemDNSMgrWindows::isResolverLoopback() const
{
   if ( interfaces.empty() ) {
        qDebug("No interfaces found");
        return false;
   }

    return std::all_of(interfaces.begin(), interfaces.end(),
                       [](const auto& iface)
                       {
                           return iface.is_resolver_loopback();
                       });
}

bool SystemDNSMgrWindows::isRunning() const
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


SystemDNSMgr *SystemDNSMgr::factory(MainWindow *parent) {
    return new SystemDNSMgrWindows(parent);
}
