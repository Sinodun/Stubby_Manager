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
    void saveAll(bool restart);
    void saveProfile(Config::NetworkProfile networkProfile);
    void saveNetworks();
    void saveUpdatedNetworks();

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

    void updateNetworks(std::map<std::string, NetworkMgr::interfaceInfo> running_networks);
    std::string getCurrentProfileString() const;
    std::string getCurrentNetworksString() const;

signals:
    void configChanged(bool restart);

protected:
    bool profileModifiedFrom(const Config& cfg, Config::NetworkProfile networkProfile);
    bool networksModifiedFrom(const Config& cfg);
    bool modifiedFrom(const Config& cfg);
    void profileRestoreFrom(const Config& cfg, Config::NetworkProfile networkProfile);
    void networksRestoreFrom(const Config& cfg);
    void restoreFrom(const Config& cfg);
    bool saveConfig(const Config& cfg);
    Config::NetworkProfile addNetwork(const std::string& name, NetworkMgr::InterfaceTypes type, bool active);

    MainWindow *m_mainwindow;
    Config factoryConfig;
    Config savedConfig;
    Config tempConfig; // used to determine if restart needed
    bool restartRequired;
    Config::NetworkProfile m_current_profile;
    std::string m_current_networks_string;

    virtual std::string appDataDir() = 0;
    virtual std::string appDir();
};


#endif
