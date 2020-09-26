/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LOGMANAGERLINUX_H
#define LOGMANAGERLINUX_H

#include <QObject>

#include "logmanager.h"

class LogMgrLinux : public QObject, public ILogMgr
{
    Q_OBJECT

    Q_INTERFACES(ILogMgr)

public:
    LogMgrLinux(MainWindow *parent = 0);
    ~LogMgrLinux();

    void start();
    void stop();

signals:
    void alert(bool on);
};

#endif // LOGMANAGERLINUX_H
