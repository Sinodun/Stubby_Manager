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

#include "configmanager_linux.h"
#include "mainwindow.h"

// TDOD: Set this module to look for default stuff in /etc/StubbyGUI
static const char STUBBY_GUI_CONF_DIR[] = "/tmp";

ConfigMgrLinux::ConfigMgrLinux(MainWindow *parent)
    : ConfigMgr(parent)
{
}

ConfigMgrLinux::~ConfigMgrLinux()
{
}


std::string ConfigMgrLinux::defaultConfigDir()
{
    return STUBBY_GUI_CONF_DIR;
}

std::string ConfigMgrLinux::factoryConfigDir()
{
    return STUBBY_GUI_CONF_DIR;
}

ConfigMgr *ConfigMgr::factory(MainWindow *parent)
{
    return new ConfigMgrLinux(parent);
}
