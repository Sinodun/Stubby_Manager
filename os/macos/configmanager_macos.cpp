/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <string>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include "configmanager_macos.h"
#include "mainwindow.h"

static const char STUBBY_GUI_CONF_DIR[] = "/Applications/StubbyManager.app/Contents/MacOS/StubbyGUI";

ConfigMgrMacOS::ConfigMgrMacOS(MainWindow *parent)
    : ConfigMgr(parent)
{
}

ConfigMgrMacOS::~ConfigMgrMacOS()
{
}

std::string ConfigMgrMacOS::appDataDir()
{
    return STUBBY_GUI_CONF_DIR;
}

ConfigMgr *ConfigMgr::factory(MainWindow *parent)
{
    return new ConfigMgrMacOS(parent);
}
