/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CONFIGMGRLINUX_H
#define CONFIGMGRLINUX_H

#include <QObject>

#include "configmanager.h"

class ConfigMgrLinux : public ConfigMgr
{
    Q_OBJECT

public:
    ConfigMgrLinux(MainWindow *parent);
    virtual ~ConfigMgrLinux();

protected:
    virtual std::string appDataDir();
};

#endif
