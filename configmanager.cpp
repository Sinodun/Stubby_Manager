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


bool check_config(std::string file) {
    QProcess check;
    // getdns_query.exe is in the same directory as this executable.
    QFileInfo getdns_query = QFileInfo(QDir(QCoreApplication::applicationDirPath()), "stubby.exe");
    QStringList arguments;
    arguments << "-i" << "-C" << file.c_str();
    check.setProgram(getdns_query.filePath());
    check.setArguments(arguments);
    check.start();
    if (!check.waitForStarted())
        return false;
    if (!check.waitForFinished())
        return false;
    QByteArray stdoutData;
    int exitcode = check.exitCode();
    qInfo("check_config result is %d", exitcode);
    //qDebug() << __FILE__ << ":" << __FUNCTION__ << stdoutData;
    return exitcode;
}

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
        try {
            factoryConfig.loadFromFile(factoryConfigFile.filePath().toStdString());
            qInfo("Read default config file.");
        } catch (...) {
            m_mainwindow->statusMsg("ERROR: Unable to load default configuration file, using built-in defaults");
            qWarning("No readable default configuration file, using built-in defaults");
            factoryConfig.reset();
        }
        emit configChanged(false);
        return;
    }
    m_mainwindow->statusMsg("WARNING: No default configuration file available, using built-in defaults");
    qWarning("No readable default configuration file, using built-in defaults");
    factoryConfig.reset();
    return;
}

void ConfigMgr::load()
{
    displayedConfig = factoryConfig;

    QDir customDir(QString::fromStdString(appDataDir()));
    QFileInfo customConfigFile(customDir, QString::fromStdString(NETPROFILENAME));

    if ( customConfigFile.isFile() && customConfigFile.isReadable() )
    {
        try {
            savedConfig.loadFromFile(customConfigFile.filePath().toStdString());
            displayedConfig = savedConfig;
            qInfo("Read custom config file.");
        } catch (...) {
            m_mainwindow->statusMsg("ERROR: Unable to load user configuration file, using default conifguration instead.");
            m_mainwindow->systrayMsg("ERROR: Unable to load user configuration file, using default conifguration instead.");
            // TODO: we should think about renaming the broken config file for later inspection, we need to overwrite
            // it so a saved config isgenreatedand the GUI doesn't break on an empty one....
        }
    }
    saveAll(false);
    emit configChanged(false);
    qWarning("No readable custom configuration file");
}

std::string ConfigMgr::getCurrentProfileString() const {
    std::string profile_string = Config::networkProfileDisplayName(m_current_profile);
    if (m_current_profile == savedConfig.defaultNetworkProfile)
        profile_string.append(" (Default)");
    return profile_string;
}

std::string ConfigMgr::getCurrentNetworksString() const {
    return m_current_networks_string;
}

bool ConfigMgr::profilesValid(Config::NetworkProfile check_profile, bool check_all) {
    std::unordered_map<Config::NetworkProfile, Config::Profile> profiles = displayedConfig.profiles;
    for (const auto& profile : profiles) {
        Config::NetworkProfile prof_type = profile.first;
        if (!check_all && prof_type != check_profile)
            continue;
        int server_count=0;
        int server_auth=0;
        int server_cleartext=0;
        for ( const auto& s : displayedConfig.servers )
        {
            if ( s.hidden.find(prof_type) != s.hidden.end() ||
                 s.inactive.find(prof_type) != s.inactive.end() )
                continue;

            server_count++;
            for ( const auto& a : s.addresses )
            {
                if (!s.tlsAuthName.empty() || !s.pubKeyDigestValue.empty())
                    server_auth++;
                if (a.compare("1.1.1.1") == 0 || a.compare("9.9.9.9") == 0 ||
                    a.compare("8.8.8.8") == 0 || a.compare("176.103.130.130") == 0 || a.compare("89.233.43.71") == 0) {
                    server_cleartext++;
                }
            }
        }
        // 1) must be at least one server
        if (server_count == 0) {
            QString message = "ERROR: The ";
            message.append(Config::networkProfileDisplayName(prof_type).c_str());
            message.append(" profile cannot be saved: there must be at least one server in a profile.");
            m_mainwindow->systrayMsg(message);
            return false;
        }
        // 2) If authenticate then must be at least one server with auth creds
        if (profile.second.encryptAll && profile.second.alwaysAuthenticate && server_auth == 0) {
            QString message = "ERROR: The ";
            message.append(Config::networkProfileDisplayName(prof_type).c_str());
            message.append(" profile cannot be saved: there must be at least one server with either an authentication name or a pinset in a profile that requires authentication.");
            m_mainwindow->systrayMsg(message);
            return false;
        }
        // 3) Warn if not encryption and none of the shipped servers that do cleartext are included
        if (!profile.second.encryptAll && server_cleartext == 0 ){
            //&&
             //profile.second.useNetworkProvidedServer == Config::UseNetworkProvidedServer::exclude) {
            QString message = "WARNING: The ";
            message.append(Config::networkProfileDisplayName(prof_type).c_str());
            message.append(" profile is set not to encrypt traffic, but none of the shipped servers known to support cleartext are included.");
            m_mainwindow->systrayMsg(message);
            return false;
        }
    }
    return true;
}


bool ConfigMgr::saveAll(bool restart)
{
    if (!profilesValid(Config::NetworkProfile::untrusted, true))
        return false;
    tempConfig = savedConfig;
    savedConfig = displayedConfig;
    saveConfig(savedConfig);
    return true;
}

bool ConfigMgr::saveProfile(Config::NetworkProfile networkProfile)
{
    if (!profilesValid(networkProfile, false))
        return false;
    tempConfig = savedConfig;
    savedConfig.copyProfile(displayedConfig, networkProfile);
    saveConfig(savedConfig);
    return true;
}

void ConfigMgr::saveNetworks()
{
    tempConfig = savedConfig;
    savedConfig.networks = displayedConfig.networks;
    savedConfig.defaultNetworkProfile = displayedConfig.defaultNetworkProfile;
    saveConfig(savedConfig);
}

void ConfigMgr::saveUpdatedNetworks()
{
    tempConfig = savedConfig;
    if (saveConfig(savedConfig))
        m_mainwindow->alertOnNetworksUpdatedRestart();
}

bool ConfigMgr::saveConfig(const Config& config)
{

    Config::NetworkProfile temp_current_profile = m_current_profile;
    m_current_networks_string="";
    m_current_profile=Config::NetworkProfile::trusted;
    for ( const auto& net : config.networks ) {
        auto net_name = net.first;
        auto net_active = net.second.interfaceActive;
        Config::NetworkProfile profile = Config::networkProfileFromChoice(net.second.profile, config.defaultNetworkProfile);
        // Only disply the active networks
        if (net_active) {
            if ( !m_current_networks_string.empty())
                m_current_networks_string.append("\n");
            m_current_networks_string.append(net_name);
            qInfo("Saving: network %s has profile %d", net_name.c_str(), profile);
            if ( profile > m_current_profile )
                m_current_profile = profile;
        }
    }

    QDir customDir(QString::fromStdString(appDataDir()));
    QFileInfo customConfig(customDir, QString::fromStdString(NETPROFILENAME));
    QDir::root().mkpath(customDir.absolutePath());
    config.saveToFile(customConfig.filePath().toStdString());

    // is restart needed?
    int restart = false;
    if (temp_current_profile != m_current_profile) {
        qInfo("Restart required if running - active profile changed from %s to %s",
               Config::networkProfileDisplayName(temp_current_profile).c_str(),
               Config::networkProfileDisplayName(m_current_profile).c_str());
        restart = true;
    }
    else if (!savedConfig.equalProfile(tempConfig, m_current_profile))
        restart = true;

    emit configChanged(restart);
    return restart;
}

std::string ConfigMgr::generateStubbyConfig()
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
    QFileInfo stubby_yml(tmpdir, "stubbyservice_tmp.yml");
    QFileInfo stubby_yml_real(tmpdir, "stubbyservice.yml");

    ctemplate::TemplateDictionary dict("STUBBY_CONFIG");
    if ( savedConfig.profiles[m_current_profile].encryptAll )
        dict.SetValue("TRANSPORT_LIST", "[GETDNS_TRANSPORT_TLS]");
    else
        dict.SetValue("TRANSPORT_LIST", "[GETDNS_TRANSPORT_TLS, GETDNS_TRANSPORT_UDP, GETDNS_TRANSPORT_TCP]");
    if ( savedConfig.profiles[m_current_profile].alwaysAuthenticate )
        dict.SetValue("AUTHENTICATION", "GETDNS_AUTHENTICATION_REQUIRED");
    else
        dict.SetValue("AUTHENTICATION", "GETDNS_AUTHENTICATION_NONE");
    if ( savedConfig.profiles[m_current_profile].validateData )
        dict.SetValue("DNSSEC", "GETDNS_EXTENSION_TRUE");
    else
        dict.SetValue("DNSSEC", "GETDNS_EXTENSION_FALSE");
    if ( savedConfig.profiles[m_current_profile].roundRobin )
        dict.SetValue("ROUND_ROBIN", "1");
    else
        dict.SetValue("ROUND_ROBIN", "0");

    int server_count=0;
    for ( const auto& s : savedConfig.servers )
    {
        if ( s.hidden.find(m_current_profile) != s.hidden.end() ||
             s.inactive.find(m_current_profile) != s.inactive.end() )
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

    // now check the file is valid
    if (check_config(res) != 0) {
        return "";
    }

    // copy to real location, leave tmp file for potential debugging
    std::ifstream  src(res, std::ios::binary);
    std::ofstream  dst(stubby_yml_real.filePath().toStdString(),   std::ios::binary);
    dst << src.rdbuf();
    return stubby_yml_real.filePath().toStdString();
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
    return
        cfg.networks != displayedConfig.networks ||
        cfg.defaultNetworkProfile != displayedConfig.defaultNetworkProfile;
}

bool ConfigMgr::modifiedFrom(const Config& cfg)
{
    return cfg != displayedConfig;
}

void ConfigMgr::profileRestoreFrom(const Config& cfg, Config::NetworkProfile networkProfile)
{
    displayedConfig.copyProfile(cfg, networkProfile);
    emit configChanged(false);
}

void ConfigMgr::networksRestoreFrom(const Config& cfg)
{
    displayedConfig.networks = cfg.networks;
    displayedConfig.defaultNetworkProfile = cfg.defaultNetworkProfile;
    emit configChanged(false);
}

void ConfigMgr::restoreFrom(const Config& cfg)
{
    displayedConfig = cfg;
    emit configChanged(false);
}

Config::NetworkProfile ConfigMgr::addNetwork(const std::string& name, NetworkMgr::InterfaceTypes type, bool active)
{
    // For now, since the user must have a default use this code to catch any corner case
    if ( displayedConfig.networks.find(name) == displayedConfig.networks.end() ) {
        displayedConfig.networks[name].profile = Config::NetworkProfileChoice::default_profile;
        displayedConfig.networks[name].interfaceType=Config::InterfaceTypes(type);
    }
    // always update the active status in case it has changed
    displayedConfig.networks[name].interfaceActive=active;

    if ( savedConfig.networks.find(name) == savedConfig.networks.end() ) {
        savedConfig.networks[name].profile = Config::NetworkProfileChoice::default_profile;
        savedConfig.networks[name].interfaceType=Config::InterfaceTypes(type);
        m_mainwindow->alertOnNewNetwork(name, savedConfig.defaultNetworkProfile);
    }
    savedConfig.networks[name].interfaceActive=active;
    return Config::networkProfileFromChoice(savedConfig.networks[name].profile, savedConfig.defaultNetworkProfile);
}

void ConfigMgr::updateNetworks(std::map<std::string, NetworkMgr::interfaceInfo> running_networks) {

    // Set all networks to inactive, to ensure only active ones are set below
    for ( auto& a : savedConfig.networks) {
        a.second.interfaceActive = false;
    }
    for ( auto& a : displayedConfig.networks) {
        a.second.interfaceActive = false;
    }

    for ( const auto& net : running_networks ) {
        // Ignore the wifi when it is not connected as it has no ssid
        if (net.first.compare("Wi-Fi") == 0) {
            continue;
        }
        // This actually also updates the active status of the existing network.....
        Config::NetworkProfile profile = addNetwork(net.first, net.second.interfaceType, net.second.interfaceActive);
    }
    saveUpdatedNetworks();
}

