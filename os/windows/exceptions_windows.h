/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

/* ; -*-C++-*- */
#ifndef EXCEPTIONS_WINDOWS_H
#define EXCEPTIONS_WINDOWS_H

#include <system_error>

#include <winsock2.h>
#include <windows.h>

class windows_system_error : public std::system_error
{
public:
    windows_system_error(DWORD err, const std::string& what)
        : std::system_error(err, std::system_category(), what)
    {
    }

    windows_system_error(DWORD err, const char* what)
        : std::system_error(err, std::system_category(), what)
    {
    }

    windows_system_error(const std::string& what)
        : windows_system_error(GetLastError(), what)
    {
    }

    windows_system_error(const char* what)
        : windows_system_error(GetLastError(), what)
    {
    }
};

#endif
