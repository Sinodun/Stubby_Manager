/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <string>

#include <winsock2.h>
#include <windows.h>
#include <initguid.h>
#include <KnownFolders.h>
#include <ShlObj.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include "configmanager_windows.h"
#include "exceptions_windows.h"
#include "mainwindow.h"

ConfigMgrWindows::ConfigMgrWindows(MainWindow *parent)
    : ConfigMgr(parent)
{
}

ConfigMgrWindows::~ConfigMgrWindows()
{
}

static std::string GetKnownFolder(REFKNOWNFOLDERID rfid, const std::string& appdir)
{
    PWSTR path;
    HRESULT hr = SHGetKnownFolderPath(rfid, KF_FLAG_CREATE, NULL, &path);

    if ( SUCCEEDED(hr) )
    {
        QString qstr = QString::fromWCharArray(path);
        CoTaskMemFree(path);

        // Both below need app dir appended.
        QDir dir(QDir::cleanPath(qstr));
        return dir.filePath(QString::fromStdString(appdir)).toStdString();
    }

    //throw windows_system_error("Can't find known folder");
}

std::string ConfigMgrWindows::appDataDir()
{
    return GetKnownFolder(FOLDERID_ProgramData, APPDIRNAME);
}

std::string ConfigMgrWindows::appDir()
{
    return QCoreApplication::applicationDirPath().toStdString();
}

ConfigMgr *ConfigMgr::factory(MainWindow *parent)
{
    return new ConfigMgrWindows(parent);
}
