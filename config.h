/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Config
{
    enum class NetworkProfile { trusted, untrusted, hostile };
    enum class NetworkProfileChoice { default_profile, trusted, untrusted, hostile };

    enum class UseNetworkProvidedServer { exclude, include, use_only };

    struct Server
    {
        Server(const std::string& name, const std::string& link, const std::vector<std::string>& addrs, const std::string& authName, const std::string& digest);
        Server();

        std::string name;
        std::string link;
        std::vector<std::string> addresses;
        std::string tlsAuthName;
        std::string pubKeyDigestType;
        std::string pubKeyDigestValue;

        std::unordered_set<NetworkProfile> hidden;
        std::unordered_set<NetworkProfile> inactive;

        bool operator==(const Server& server) const;
        void setServerDataEqual(const Config::Server& server);
        void setServerActiveEqualForProfile(const Config::Server& server, Config::NetworkProfile);
    };

    struct Profile
    {
        Profile (bool encryptAll, bool alwaysAuthenticate, bool validateData,
                 bool roundRobin, UseNetworkProvidedServer useNetworkProvidedServer);
        Profile();

        bool encryptAll;
        bool alwaysAuthenticate;
        bool validateData;
        bool roundRobin;
        UseNetworkProvidedServer useNetworkProvidedServer;

        bool operator==(const Profile& profile) const;
    };

    std::vector<Server> servers;
    std::unordered_map<NetworkProfile, Profile> profiles;
    NetworkProfile defaultNetworkProfile;

    typedef enum {
        WiFi = 0,
        Ethernet
    } InterfaceTypes;

    struct NetworkInformation
    {
        NetworkProfileChoice profile;
        InterfaceTypes interfaceType;
        bool interfaceActive;

        bool operator==(const NetworkInformation& netinfo) const;
    };

    std::map<std::string, NetworkInformation> networks;

    Config();
    void loadFromFile(const std::string& filePath);
    void saveToFile(const std::string& filePath) const;
    void reset();
    void copyProfile(const Config& cfg, Config::NetworkProfile networkProfile);

    bool operator==(const Config& cfg) const;
    bool operator!=(const Config& cfg) const;
    bool equalProfile(const Config& cfg, Config::NetworkProfile networkProfile) const;

    bool serverDataIsEqual(const Config::Server& server1, const Config::Server& server2) const;
    bool serverActiveIsEqualForProfile(const Config::Server& server1, const Config::Server& server2, Config::NetworkProfile) const;

    static std::string networkProfileDisplayName(NetworkProfile np);
    static std::string networkProfileChoiceDisplayName(NetworkProfileChoice npc);
    static std::string interfaceTypeDisplayName(InterfaceTypes it);

    static NetworkProfile           networkProfileFromChoice(NetworkProfileChoice npc, NetworkProfile default_profile);
    static NetworkProfileChoice     networkChoiceFromProfile(NetworkProfile np);

    static std::string              networkProfileYamlKey(NetworkProfile np);
    static NetworkProfile           networkProfileFromYamlKey(const std::string& key);
    static std::string              networkProfileChoiceYamlKey(NetworkProfileChoice npc);
    static NetworkProfileChoice     networkProfileChoiceFromYamlKey(const std::string& key);
};

#endif

/* Local Variables: */
/* mode: c++        */
/* End:             */
