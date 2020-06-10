#ifndef NETWORKINTERFACEWINDOWS_H
#define NETWORKINTERFACEWINDOWS_H

//#include <ifdef.h>
#include <windows.h>

#include "networkinterface.hpp"

class NetworkInterfaceWindows : public NetworkInterface
{
public:
    NetworkInterfaceWindows(const std::string& name,
                            const std::string& adapter_name,
                            const std::string& descripton,
                            const std::string& dns_suffix,
                            const std::string& ssid,
                            bool is_resolver_loopback,
                            bool is_running,
                            DWORD if_type);
                            //IF_OPER_STATUS oper_status);
    ~NetworkInterfaceWindows() = default;

    std::string name() const override;

    bool is_resolver_loopback() const override;
    bool is_running() const override;
   // bool is_up() const override;
    bool is_wireless() const override;

    virtual std::string ssid() const override;

private:
    std::string name_;
    std::string adapter_name_;
    std::string description_;
    std::string dns_suffix_;
    std::string ssid_;
    bool is_resolver_loopback_;
    bool is_running_;
    DWORD if_type_;
    //IF_OPER_STATUS oper_status_;
};

#endif
