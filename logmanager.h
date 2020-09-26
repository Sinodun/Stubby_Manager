/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

class MainWindow;

namespace Ui {
class ILogMgr;
}

class ILogMgr
{
public:
   virtual ~ILogMgr() = default;

   static ILogMgr * factory(MainWindow *parent);

   virtual void start() = 0;
   virtual void stop() = 0;

signals:
    virtual void alert(bool on) = 0;
};

#define LogMgr_iid "com.sinodun.stubby.LogMgr"
Q_DECLARE_INTERFACE(ILogMgr, LogMgr_iid)

#endif // LOGMANAGER_H
