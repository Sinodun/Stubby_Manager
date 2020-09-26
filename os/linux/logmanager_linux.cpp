/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "logmanager_linux.h"
#include "mainwindow.h"

#include <QDebug>
#include <QTextCursor>

LogMgrLinux::LogMgrLinux(MainWindow *parent) :
    QObject(parent)
{
}

LogMgrLinux::~LogMgrLinux()
{
}

void LogMgrLinux::start()
{
}

void LogMgrLinux::stop()
{
}

ILogMgr *ILogMgr::factory(MainWindow *parent)
{
    return new LogMgrLinux(parent);
}

