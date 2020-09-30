/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include <string>

#include <QObject>

#include "config.h"
#include "networkmanager.h"

class MainWindow;

class ConfigMgr : public QObject
{
    Q_OBJECT

public:
    ConfigMgr(MainWindow *parent);
    virtual ~ConfigMgr();

    static const std::string APPDIRNAME;
    static const std::string NETPROFILENAME;
    static const std::string DEFNETPROFILENAME;
    static const std::string STUBBYTEMPLATENAME;

    static ConfigMgr * factory(MainWindow *parent);

    Config displayedConfig;

    void init();
    void load();
    void save(bool restart);
    void saveProfile(Config::NetworkProfile networkProfile);
    void saveNetworks();

    std::string generateStubbyConfig(Config::NetworkProfile networkProfile);

    bool profileModifiedFromFactory(Config::NetworkProfile networkProfile);
    bool profileModifiedFromSaved(Config::NetworkProfile networkProfile);
    bool networksModifiedFromSaved();
    bool modifiedFromFactoryDefaults();
    bool modifiedFromSavedConfig();

    void profileRestoreSaved(Config::NetworkProfile networkProfile);
    void profileRestoreFactory(Config::NetworkProfile networkProfile);
    void networksRestoreSaved();
    void restoreSaved();
    void restoreFactory();

    void addNetwork(const std::string& name, NetworkMgr::InterfaceTypes type, bool active);
    void resetNetworksActiveState();
    Config::NetworkProfileChoice getDisplayedNetworkProfile(const std::string& name);

    bool unsavedChanges(bool profile, bool network);
    bool getRestartRequired() const {return restartRequired;};
    void restartDone();

signals:
    void configChanged();

protected:
    bool profileModifiedFrom(const Config& cfg, Config::NetworkProfile networkProfile);
    bool networksModifiedFrom(const Config& cfg);
    bool modifiedFrom(const Config& cfg);
    void profileRestoreFrom(const Config& cfg, Config::NetworkProfile networkProfile);
    void networksRestoreFrom(const Config& cfg);
    void restoreFrom(const Config& cfg);
    void saveConfig(const Config& cfg);
    void setRestartRequired(bool profile, bool network);

    MainWindow *m_mainwindow;
    Config factoryConfig;
    Config savedConfig;
    bool restartRequired;

    virtual std::string appDataDir() = 0;
    virtual std::string appDir();
};


#endif
