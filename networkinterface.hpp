#ifndef NETWORKINTERFACE_HPP
#define NETWORKINTERFACE_HPP

#include <string>

class NetworkInterface
{
public:
    virtual ~NetworkInterface() = default;

    virtual std::string name() const = 0;

    virtual bool is_resolver_loopback() const = 0;
    virtual bool is_running() const = 0;
   // virtual bool is_up() const = 0;
    virtual bool is_wireless() const = 0;

    virtual std::string ssid() const = 0;
};

#endif
