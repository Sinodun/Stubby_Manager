/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <fstream>
#include <stdexcept>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QString>

#include <ctemplate/template.h>

#include "configmanager.h"
#include "mainwindow.h"

ConfigMgr::ConfigMgr(MainWindow *parent)
    : QObject(parent), m_mainwindow(parent), restartRequired(false)
{
    qInfo("Creating config mgr");
}

ConfigMgr::~ConfigMgr()
{
}

const std::string ConfigMgr::APPDIRNAME = "Stubby";
const std::string ConfigMgr::NETPROFILENAME = "netprofile.yml";
const std::string ConfigMgr::DEFNETPROFILENAME = "netprofile_defaults.yml";
const std::string ConfigMgr::STUBBYTEMPLATENAME = "stubbytemplate.yml";

void ConfigMgr::init()
{
    QDir factoryDir(QString::fromStdString(appDataDir()));
    QFileInfo factoryConfigFile(factoryDir, QString::fromStdString(DEFNETPROFILENAME));

    if ( factoryConfigFile.isFile() && factoryConfigFile.isReadable() )
    {
        factoryConfig.loadFromFile(factoryConfigFile.filePath().toStdString());
        qInfo("Read default config file.");
        emit configChanged();
        return;
    }

    //TODO: This should probably be a user message
    qWarning("No readable default configuration file, using built-in defaults");
}

void ConfigMgr::load()
{
    displayedConfig = factoryConfig;

    QDir customDir(QString::fromStdString(appDataDir()));
    QFileInfo customConfigFile(customDir, QString::fromStdString(NETPROFILENAME));

    if ( customConfigFile.isFile() && customConfigFile.isReadable() )
    {
        savedConfig.loadFromFile(customConfigFile.filePath().toStdString());
        displayedConfig = savedConfig;
        qInfo("Read custom config file.");
        emit configChanged();
        return;
    }
    else
        save(false);

    qWarning("No readable custom configuration file");
}

bool ConfigMgr::unsavedChanges(bool profile, bool network) {
    // For simplicity, always do a restart if the network profiles have changed
    // (working out if only the running/updated newtworks have changed is hard)
    if (networksModifiedFromSaved() && network) {
        qInfo("Network config modified");
        return true;
    }
    // If the active profile is being modified the service should be restarted
    // Let the main window do the check on the service state
    if (profileModifiedFromSaved(m_mainwindow->getCurrentNetworkProfile())) {
        qInfo("Running config modifed");
        return true;
    }
    return false;
}

void ConfigMgr::setRestartRequired(bool profile, bool network) {
    if (!unsavedChanges(profile, network))
        return;
    qInfo("Restart required");
    restartRequired = true;
}

void ConfigMgr::restartDone() {
    restartRequired = false;
}

void ConfigMgr::save(bool restart)
{
    if (restart)
        setRestartRequired(true, true);
    savedConfig = displayedConfig;
    saveConfig(savedConfig);
}

void ConfigMgr::saveProfile(Config::NetworkProfile networkProfile)
{
    setRestartRequired(true, false);
    savedConfig.copyProfile(displayedConfig, networkProfile);
    saveConfig(savedConfig);
}

void ConfigMgr::saveNetworks()
{
    setRestartRequired(false, true);
    savedConfig.networks = displayedConfig.networks;
    saveConfig(savedConfig);
}

void ConfigMgr::saveConfig(const Config& config)
{
    QDir customDir(QString::fromStdString(appDataDir()));
    QFileInfo customConfig(customDir, QString::fromStdString(NETPROFILENAME));
    QDir::root().mkpath(customDir.absolutePath());
    config.saveToFile(customConfig.filePath().toStdString());
    emit configChanged();
}

std::string ConfigMgr::generateStubbyConfig(Config::NetworkProfile networkProfile)
{    

    // Look for template file in factory dir and current dir.
    QDir tpldir(QString::fromStdString(appDataDir()));
    QFileInfo tpl(tpldir, QString::fromStdString(STUBBYTEMPLATENAME));

    if ( !tpl.isFile() || !tpl.isReadable() )
    {
        tpl = QFileInfo(QDir(QCoreApplication::applicationDirPath()), QString::fromStdString(STUBBYTEMPLATENAME));
        if ( !tpl.isFile() || !tpl.isReadable() )
            throw std::runtime_error("Stubby template file not found");
    }

    QDir tmpdir(QString::fromStdString(appDir()));
    if ( !QDir::root().mkpath(tmpdir.absolutePath()) )
        throw std::runtime_error("Can't make application temporary directory");
    QFileInfo stubby_yml(tmpdir, "stubbyservice.yml");

    ctemplate::TemplateDictionary dict("STUBBY_CONFIG");
    if ( savedConfig.profiles[networkProfile].encryptAll )
        dict.SetValue("TRANSPORT_LIST", "[GETDNS_TRANSPORT_TLS]");
    else
        dict.SetValue("TRANSPORT_LIST", "[GETDNS_TRANSPORT_UDP, GETDNS_TRANSPORT_TCP, GETDNS_TRANSPORT_TLS]");
    if ( savedConfig.profiles[networkProfile].alwaysAuthenticate )
        dict.SetValue("AUTHENTICATION", "GETDNS_AUTHENTICATION_REQUIRED");
    else
        dict.SetValue("AUTHENTICATION", "GETDNS_AUTHENTICATION_NONE");
    if ( savedConfig.profiles[networkProfile].validateData )
        dict.SetValue("DNSSEC", "GETDNS_EXTENSION_TRUE");
    else
        dict.SetValue("DNSSEC", "GETDNS_EXTENSION_FALSE");
    if ( savedConfig.profiles[networkProfile].roundRobin )
        dict.SetValue("ROUND_ROBIN", "1");
    else
        dict.SetValue("ROUND_ROBIN", "0");

    int server_count=0;
    for ( const auto& s : savedConfig.servers )
    {
        if ( s.hidden.find(networkProfile) != s.hidden.end() ||
             s.inactive.find(networkProfile) != s.inactive.end() )
            continue;

        server_count++;
        for ( const auto& a : s.addresses )
        {
            ctemplate::TemplateDictionary* subdict = dict.AddSectionDictionary("SERVER");
            subdict->SetValue("SERVER_ADDRESS", a);
            if (!s.tlsAuthName.empty()) {
                ctemplate::TemplateDictionary* sub_dict1 = subdict->AddSectionDictionary("AUTHNAME");
                sub_dict1->SetValue("SERVER_AUTH_NAME", s.tlsAuthName);
            }
            if (!s.pubKeyDigestValue.empty()) {
                ctemplate::TemplateDictionary* sub_dict2 = subdict->AddSectionDictionary("PINSET");
                sub_dict2->SetValue("SERVER_DIGEST_TYPE", s.pubKeyDigestType);
                sub_dict2->SetValue("SERVER_DIGEST_VALUE", s.pubKeyDigestValue);
            }
        }
    }    

    //Basic check that the config is valid with 1 server.
    // TODO: Proper crosscheck of validity at save time
    if (server_count == 0)
        return "";

    std::string expansion;
    if ( !ctemplate::ExpandTemplate(tpl.filePath().toStdString(), ctemplate::STRIP_BLANK_LINES, &dict, &expansion) )
        throw std::runtime_error("Template expansion failed");

    std::string res = stubby_yml.filePath().toStdString();
    std::ofstream fout(res);
    fout.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    fout << expansion;
    fout.close();
    return res;
}

std::string ConfigMgr::appDir()
{
    return QDir(QDir::tempPath()).filePath(QString::fromStdString(APPDIRNAME)).toStdString();
}

bool ConfigMgr::profileModifiedFromFactory(Config::NetworkProfile networkProfile)
{
    return profileModifiedFrom(factoryConfig, networkProfile);
}

bool ConfigMgr::profileModifiedFromSaved(Config::NetworkProfile networkProfile)
{
    return profileModifiedFrom(savedConfig, networkProfile);
}

bool ConfigMgr::networksModifiedFromSaved()
{
    return networksModifiedFrom(savedConfig);
}

bool ConfigMgr::modifiedFromFactoryDefaults()
{
    return modifiedFrom(factoryConfig);
}

bool ConfigMgr::modifiedFromSavedConfig()
{
    return modifiedFrom(savedConfig);
}

void ConfigMgr::profileRestoreSaved(Config::NetworkProfile networkProfile)
{
    profileRestoreFrom(savedConfig, networkProfile);
}

void ConfigMgr::profileRestoreFactory(Config::NetworkProfile networkProfile)
{
    profileRestoreFrom(factoryConfig, networkProfile);
}

void ConfigMgr::networksRestoreSaved()
{
    networksRestoreFrom(savedConfig);
}

void ConfigMgr::restoreSaved()
{
    restoreFrom(savedConfig);
}

void ConfigMgr::restoreFactory()
{
    restoreFrom(factoryConfig);
}

bool ConfigMgr::profileModifiedFrom(const Config& cfg, Config::NetworkProfile networkProfile)
{
    return !displayedConfig.equalProfile(cfg, networkProfile);
}

bool ConfigMgr::networksModifiedFrom(const Config& cfg)
{
    return cfg.networks != displayedConfig.networks;
}

bool ConfigMgr::modifiedFrom(const Config& cfg)
{
    return cfg != displayedConfig;
}

void ConfigMgr::profileRestoreFrom(const Config& cfg, Config::NetworkProfile networkProfile)
{
    displayedConfig.copyProfile(cfg, networkProfile);
    emit configChanged();
}

void ConfigMgr::networksRestoreFrom(const Config& cfg)
{
    displayedConfig.networks = cfg.networks;
    emit configChanged();
}

void ConfigMgr::restoreFrom(const Config& cfg)
{
    displayedConfig = cfg;
    emit configChanged();
}

void ConfigMgr::addNetwork(const std::string& name)
{
    // For now, since the user must have a default use this code to catch any corner case
    if ( displayedConfig.networks.find(name) == displayedConfig.networks.end() )
        displayedConfig.networks[name].profile =
            displayedConfig.defaultNewNetworkProfileSet
            ? displayedConfig.defaultNewNetworkProfile
            : Config::NetworkProfile::untrusted;
}

Config::NetworkProfile ConfigMgr::getDisplayedNetworkProfile(const std::string& name)
{
    addNetwork(name);
    return displayedConfig.networks[name].profile;
}
