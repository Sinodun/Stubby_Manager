#include <winsock2.h>

#include "networkinterface_windows.hpp"

NetworkInterfaceWindows::NetworkInterfaceWindows(
    const std::string& name,
    const std::string& adapter_name,
    const std::string& description,
    const std::string& dns_suffix,
    const std::string& ssid,
    bool is_resolver_loopback,
    bool is_running,
    DWORD if_type)
    //IF_OPER_STATUS oper_status)
    : name_(name), adapter_name_(adapter_name),
      description_(description), dns_suffix_(dns_suffix), ssid_(ssid),
      is_resolver_loopback_(is_resolver_loopback),
      is_running_(is_running),
      if_type_(if_type)
       //, oper_status_(oper_status)
{
}

std::string NetworkInterfaceWindows::name() const
{
    return name_;
}

bool NetworkInterfaceWindows::is_resolver_loopback() const
{
    return is_resolver_loopback_;
}

bool NetworkInterfaceWindows::is_running() const
{
    return is_running_;
}

/*bool NetworkInterfaceWindows::is_up() const
{
    return oper_status_ & IfOperStatusUp;
}*/

bool NetworkInterfaceWindows::is_wireless() const
{
    //return if_type_ == IF_TYPE_IEEE80211;
    return false;
}

std::string NetworkInterfaceWindows::ssid() const
{
    return ssid_;
}
