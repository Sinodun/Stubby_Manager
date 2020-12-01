/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <fstream>
#include <stdexcept>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251 4275)
#endif

#include <yaml-cpp/yaml.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "config.h"

Config::Server::Server(const std::string& name, const std::string& link, const std::vector<std::string>& addrs, const std::string& authName, const std::string& digest)
    : name(name), link(link), addresses(addrs), tlsAuthName(authName),
      pubKeyDigestType("sha256"), pubKeyDigestValue(digest),
      hidden(), inactive()
{
}

Config::Server::Server()
    : name(), link(), addresses(), tlsAuthName(),
      pubKeyDigestType("sha256"), pubKeyDigestValue(),
      hidden(), inactive()
{
}

Config::Profile::Profile (bool encryptAll, bool alwaysAuthenticate,
                          bool validateData, bool roundRobin,
                          UseNetworkProvidedServer useNetworkProvidedServer)
    : encryptAll(encryptAll), alwaysAuthenticate(alwaysAuthenticate),
      validateData(validateData), roundRobin(roundRobin),
      useNetworkProvidedServer(useNetworkProvidedServer)
{
}

Config::Profile::Profile()
    : encryptAll(), alwaysAuthenticate(),
      validateData(), roundRobin(),
      useNetworkProvidedServer()
{
}

Config::Config()
    : servers(), profiles(),
      defaultNetworkProfile(NetworkProfile::untrusted), networks()
{
    reset();
}

void Config::reset()
{
    servers.clear();
    servers.push_back(Server("Stubby server (Sinodun)", "dnsprivacy.org", {"145.100.185.15"}, "dnsovertls.sinodun.com", "62lKu9HsDVbyiPenApnc4sfmSYTHOVfFgL3pyB+cBL4="));
    servers.push_back(Server("Stubby server (Sinodun1)", "dnsprivacy.org", {"145.100.185.16"}, "dnsovertls1.sinodun.com", "cE2ecALeE5B+urJhDrJlVFmf38cJLAvqekONvjvpqUA="));
    servers.push_back(Server("Stubby server (getdnsapi.net)", "dnsprivacy.org", {"185.49.141.37"}, "getdnsapi.net", "foxZRnIh9gZpWnl+zEiKa0EJ2rdCGroMWm02gaxSc9Q="));

    profiles.clear();
    profiles[NetworkProfile::trusted] = Profile({ false, false, false, false, UseNetworkProvidedServer::use_only });
    profiles[NetworkProfile::untrusted] = Profile({ true, true, false, false, UseNetworkProvidedServer::include });
    profiles[NetworkProfile::hostile] = Profile({ true, true, false, false, UseNetworkProvidedServer::exclude });

    defaultNetworkProfile = NetworkProfile::untrusted;
}

static Config::NetworkProfile networkProfileFromYaml(const std::string& key, const YAML::Mark& mark)
{
    try
    {
        return Config::networkProfileFromYamlKey(key);
    }
    catch (const std::invalid_argument& ie)
    {
        throw YAML::ParserException(mark, ie.what());
    }
}

static Config::NetworkProfileChoice networkProfileChoiceFromYaml(const std::string& key, const YAML::Mark& mark)
{
    try
    {
        return Config::networkProfileChoiceFromYamlKey(key);
    }
    catch (const std::invalid_argument& ie)
    {
        throw YAML::ParserException(mark, ie.what());
    }
}

static void yamlInputNetworkProfileSet(const YAML::Node& yml, const std::string& name, std::unordered_set<Config::NetworkProfile>& set)
{
    if ( !yml || yml.Type() != YAML::NodeType::Sequence )
        throw YAML::ParserException(yml.Mark(), "server must have " + name + " sequence");

    for ( const auto& nt : yml )
        set.insert(networkProfileFromYaml(nt.as<std::string>(), yml.Mark()));
}

void Config::loadFromFile(const std::string& path)
{
    YAML::Node ymlcfg = YAML::LoadFile(path);

    servers.clear();
    profiles.clear();
    networks.clear();

    YAML::Node ymlservers = ymlcfg["servers"];
    if ( !ymlservers )
        throw YAML::ParserException(ymlcfg.Mark(), "Missing 'servers' list");
    if ( ymlservers.Type() != YAML::NodeType::Sequence )
        throw YAML::ParserException(ymlcfg.Mark(), "'servers' must be a sequence");
    for ( const auto& s : ymlservers )
    {
        if ( s.Type() != YAML::NodeType::Map )
            throw YAML::ParserException(ymlservers.Mark(), "'servers' entry must be a map");
        Server server;
        server.name = s["name"].as<std::string>();
        server.link = s["link"].as<std::string>();
        server.tlsAuthName = s["tls_auth_name"].as<std::string>();

        YAML::Node ymladdrs = s["addresses"];
        if ( !ymladdrs || !ymladdrs.size() )
            throw YAML::ParserException(ymladdrs.Mark(), "server must have at least 1 address");
        for ( const auto& a : ymladdrs )
            server.addresses.push_back(a.as<std::string>());

        YAML::Node ymlpinset = s["tls_pubkey_pinset"];
        if ( !ymlpinset || ymlpinset.Type() != YAML::NodeType::Map )
            throw YAML::ParserException(ymladdrs.Mark(), "server must have tls_pubkey_pinset map");
        server.pubKeyDigestType = ymlpinset["digest"].as<std::string>();
        server.pubKeyDigestValue = ymlpinset["value"].as<std::string>();

        yamlInputNetworkProfileSet(s["hidden"], "hidden", server.hidden);
        yamlInputNetworkProfileSet(s["inactive"], "inactive", server.inactive);

        servers.push_back(server);
    }

    YAML::Node ymlprofiles = ymlcfg["profiles"];
    if ( !ymlprofiles )
        throw YAML::ParserException(ymlcfg.Mark(), "Missing 'profiles' map");
    if ( ymlprofiles.Type() != YAML::NodeType::Map )
        throw YAML::ParserException(ymlcfg.Mark(), "'profiles' must be a map");
    for ( const auto& n : ymlprofiles )
    {
        NetworkProfile nt = networkProfileFromYaml(n.first.as<std::string>(), ymlcfg.Mark());
        Profile prof;
        prof.encryptAll = n.second["encrypt_all"].as<bool>();
        prof.alwaysAuthenticate = n.second["always_authenticate"].as<bool>();
        prof.validateData = n.second["validate_data"].as<bool>();
        prof.roundRobin = n.second["round_robin"].as<bool>();

        std::string unps = n.second["user_network_provided_servers"].as<std::string>();
        if ( unps == "exclude" )
            prof.useNetworkProvidedServer = UseNetworkProvidedServer::exclude;
        else if ( unps == "include" )
            prof.useNetworkProvidedServer = UseNetworkProvidedServer::include;
        else if ( unps == "use-only" )
            prof.useNetworkProvidedServer = UseNetworkProvidedServer::use_only;
        else
            throw YAML::ParserException(n.Mark(), "bad value " + unps);

        profiles[nt] = prof;
    }

    YAML::Node ymldefnetprofile = ymlcfg["default_network_profile"];
    if ( ymldefnetprofile )
        defaultNetworkProfile = networkProfileFromYaml(ymldefnetprofile.as<std::string>(), ymlcfg.Mark());

    YAML::Node ymlnetworks = ymlcfg["networks"];
    if ( !ymlnetworks )
        throw YAML::ParserException(ymlcfg.Mark(), "Missing 'networks' map");
    if ( ymlnetworks.Type() != YAML::NodeType::Map )
        throw YAML::ParserException(ymlcfg.Mark(), "'networks' must be a map");
    for ( const auto& n : ymlnetworks )
    {
        std::string name = n.first.as<std::string>();
        NetworkProfileChoice profile = networkProfileChoiceFromYaml(n.second.as<std::string>(), ymlnetworks.Mark());
        networks[name].profile = profile;
    }
}

static void yamlOutputNetworkProfileSet(YAML::Emitter& out, const std::unordered_set<Config::NetworkProfile>& set)
{
    out << YAML::BeginSeq;
    for ( const auto& np : set )
        out << Config::networkProfileYamlKey(np);
    out << YAML::EndSeq;
}

void Config::saveToFile(const std::string& path) const
{
    std::ofstream fout(path);
    fout.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    YAML::Emitter out(fout);

    out << YAML::BeginMap << YAML::Key << "servers" << YAML::BeginSeq;
    for ( const auto& s : servers )
    {
        out << YAML::BeginMap;
        out << YAML::Key << "name" << YAML::Value << s.name;
        out << YAML::Key << "link" << YAML::Value << s.link;
        out << YAML::Key << "addresses" << YAML::Value << s.addresses;
        out << YAML::Key << "tls_auth_name" << YAML::Value << s.tlsAuthName;
        out << YAML::Key << "tls_pubkey_pinset" << YAML::Value
            << YAML::BeginMap
            << YAML::Key << "digest" << YAML::Value << s.pubKeyDigestType
            << YAML::Key << "value" << YAML::Value << s.pubKeyDigestValue
            << YAML::EndMap;
        out << YAML::Key << "hidden";
        yamlOutputNetworkProfileSet(out, s.hidden);
        out << YAML::Key << "inactive";
        yamlOutputNetworkProfileSet(out, s.inactive);
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "profiles" << YAML::Value << YAML::BeginMap;
    for (const auto& nt : profiles)
    {
        out << YAML::Key << networkProfileYamlKey(nt.first) << YAML::Value
            << YAML::BeginMap
            << YAML::Key << "encrypt_all" << YAML::Value << nt.second.encryptAll
            << YAML::Key << "always_authenticate" << YAML::Value << nt.second.alwaysAuthenticate
            << YAML::Key << "validate_data" << YAML::Value << nt.second.validateData
            << YAML::Key << "round_robin" << YAML::Value << nt.second.roundRobin
            << YAML::Key << "user_network_provided_servers" << YAML::Value;
        switch(nt.second.useNetworkProvidedServer)
        {
        case UseNetworkProvidedServer::exclude:
            out << "exclude";
            break;

        case UseNetworkProvidedServer::include:
            out << "include";
            break;

        case UseNetworkProvidedServer::use_only:
            out << "use-only";
            break;
        }
        out << YAML::EndMap;
    }
    out << YAML::EndMap;

    out << YAML::Key << "default_network_profile" << YAML::Value << networkProfileYamlKey(defaultNetworkProfile);

    out << YAML::Key << "networks" << YAML::Value << YAML::BeginMap;
    for (const auto& nt : networks)
        out << YAML::Key << nt.first << YAML::Value << networkProfileChoiceYamlKey(nt.second.profile);
    out << YAML::EndMap;

    fout.close();
}

bool Config::Server::operator==(const Config::Server& server) const
{
    return
        name == server.name &&
        link == server.link &&
        addresses == server.addresses &&
        tlsAuthName == server.tlsAuthName &&
        pubKeyDigestType == server.pubKeyDigestType &&
        pubKeyDigestValue == server.pubKeyDigestValue &&
        hidden == server.hidden &&
        inactive == server.inactive;
}

bool Config::Profile::operator==(const Config::Profile& server) const
{
    return
        encryptAll == server.encryptAll &&
        alwaysAuthenticate == server.alwaysAuthenticate &&
        validateData == server.validateData &&
        roundRobin == server.roundRobin &&
        useNetworkProvidedServer == server.useNetworkProvidedServer;
}

bool Config::NetworkInformation::operator==(const Config::NetworkInformation& netinfo) const
{
    return
        profile == netinfo.profile &&
        interfaceType == netinfo.interfaceType &&
        interfaceActive == netinfo.interfaceActive;
}

bool Config::operator==(const Config& cfg) const
{
    return
        defaultNetworkProfile == cfg.defaultNetworkProfile &&
        profiles == cfg.profiles &&
        servers == cfg.servers &&
        !networksModifiedFrom(cfg.networks);
}

bool Config::operator!=(const Config& cfg) const
{
    return !(*this == cfg);
}

void Config::copyProfile(const Config& cfg, Config::NetworkProfile networkProfile)
{
    defaultNetworkProfile = cfg.defaultNetworkProfile;
    profiles[networkProfile] = cfg.profiles.at(networkProfile);

    for ( auto& s : servers )
    {
        s.hidden.erase(networkProfile);
        s.inactive.erase(networkProfile);

        for ( const auto& s2 : cfg.servers )
            if ( s.name == s2.name )
            {
                if ( s2.hidden.find(networkProfile) != s2.hidden.end() )
                    s.hidden.insert(networkProfile);
                if ( s2.inactive.find(networkProfile) != s2.inactive.end() )
                    s.inactive.insert(networkProfile);
                break;
            }
    }
}

bool Config::equalProfile(const Config& cfg, Config::NetworkProfile networkProfile) const
{
    // TODO: strictly need to check if either of the default new profiles match the one we are interested in...
    if ( defaultNetworkProfile != cfg.defaultNetworkProfile ||
         !(profiles.at(networkProfile)== cfg.profiles.at(networkProfile)))
        return false;

    for ( auto& s : servers )
    {
        bool found = false;

        for ( const auto& s2 : cfg.servers )
            if ( s.name == s2.name )
            {
                if ( s.hidden.count(networkProfile) != s2.hidden.count(networkProfile) ||
                     s.inactive.count(networkProfile) != s2.inactive.count(networkProfile) )
                     return false;
                found = true;
                break;
            }

        if ( !found )
            return false;
    }

    return true;
}

bool Config::networksModifiedFrom(const std::map<std::string, NetworkInformation>& from) const
{
    // Networks can just appear, and when they do they get a profile of
    // 'default'. Two configs where one has no record for the network and
    // the other has a record at 'default' are therefore the same.
    // Calculate this by making copies of the network maps
    // containing only non-default entries, and comparing those.
    std::map<std::string, NetworkInformation> nodef_nets;
    std::map<std::string, NetworkInformation> from_nodef_nets;

    std::copy_if(networks.begin(), networks.end(),
                 std::inserter(nodef_nets, nodef_nets.end()),
                 [](auto const& p) {
                     return p.second.profile != NetworkProfileChoice::default_profile;
                 });
    std::copy_if(from.begin(), from.end(),
                 std::inserter(from_nodef_nets, from_nodef_nets.end()),
                 [](auto const& p) {
                     return p.second.profile != NetworkProfileChoice::default_profile;
                 });
    return nodef_nets != from_nodef_nets;
}

std::string Config::networkProfileDisplayName(Config::NetworkProfile np)
{
    switch(np)
    {
    case Config::NetworkProfile::trusted:
        return "Trusted";

    case Config::NetworkProfile::untrusted:
        return "Untrusted";

    case Config::NetworkProfile::hostile:
        return "Hostile";
    }
    assert("Unknown network type");
    return "Unknown";
}

std::string Config::networkProfileYamlKey(Config::NetworkProfile np)
{
    switch(np)
    {
    case Config::NetworkProfile::trusted:
        return "trusted";

    case Config::NetworkProfile::untrusted:
        return "untrusted";

    case Config::NetworkProfile::hostile:
        return "hostile";
    }
    assert("Unknown network type");
    return "unknown";
}

Config::NetworkProfile Config::networkProfileFromYamlKey(const std::string& key)
{
    if ( key == "trusted" )
        return Config::NetworkProfile::trusted;
    else if ( key == "untrusted" )
        return Config::NetworkProfile::untrusted;
    else if ( key == "hostile" )
        return Config::NetworkProfile::hostile;
    else
        throw std::invalid_argument(key + " is not a network profile");
}

std::string Config::interfaceTypeDisplayName(Config::InterfaceTypes it)
{
    switch(it)
    {
    case Config::InterfaceTypes::WiFi:
        return "WiFi";

    case Config::InterfaceTypes::Ethernet:
        return "Ethernet";
    }
    assert("Unknown interface type");
    return "unknown";
}

std::string Config::networkProfileChoiceDisplayName(Config::NetworkProfileChoice npc)
{
    if ( npc == Config::NetworkProfileChoice::default_profile )
        return "Default";

    return networkProfileDisplayName(networkProfileFromChoice(npc, Config::NetworkProfile::untrusted));
}

std::string Config::networkProfileChoiceYamlKey(Config::NetworkProfileChoice npc)
{
    if ( npc == Config::NetworkProfileChoice::default_profile )
        return "default";

    return networkProfileYamlKey(networkProfileFromChoice(npc, Config::NetworkProfile::untrusted));
}

Config::NetworkProfileChoice Config::networkProfileChoiceFromYamlKey(const std::string& key)
{
    if ( key == "default" )
        return Config::NetworkProfileChoice::default_profile;
    else return networkChoiceFromProfile(networkProfileFromYamlKey(key));
}

Config::NetworkProfile Config::networkProfileFromChoice(Config::NetworkProfileChoice npc, Config::NetworkProfile default_profile)
{
    switch(npc)
    {
    case Config::NetworkProfileChoice::default_profile:
        return default_profile;

    case Config::NetworkProfileChoice::trusted:
        return Config::NetworkProfile::trusted;

    case Config::NetworkProfileChoice::untrusted:
        return Config::NetworkProfile::untrusted;

    case Config::NetworkProfileChoice::hostile:
        return Config::NetworkProfile::hostile;
    }
    assert("Unknown network type");
    return Config::NetworkProfile::untrusted;
}

Config::NetworkProfileChoice Config::networkChoiceFromProfile(Config::NetworkProfile np)
{
    switch(np)
    {
    case Config::NetworkProfile::trusted:
        return Config::NetworkProfileChoice::trusted;

    case Config::NetworkProfile::untrusted:
        return Config::NetworkProfileChoice::untrusted;

    case Config::NetworkProfile::hostile:
        return Config::NetworkProfileChoice::hostile;
    }
    assert("Unknown network type");
    return Config::NetworkProfileChoice::untrusted;
}
