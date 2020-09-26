/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CONFIGMGRWINDOWS_H
#define CONFIGMGRWINDOWS_H

#include <QObject>

#include "configmanager.h"

class ConfigMgrWindows : public ConfigMgr
{
    Q_OBJECT

public:
    ConfigMgrWindows(MainWindow *parent);
    virtual ~ConfigMgrWindows();

protected:
    virtual std::string appDataDir();
    virtual std::string appDir();
};

#endif
