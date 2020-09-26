/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

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
    virtual bool is_up() const = 0;
    virtual bool is_wireless() const = 0;

    virtual std::string ssid() const = 0;
};

#endif
