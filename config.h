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
    enum class NetworkProfileChoice { default, trusted, untrusted, hostile };

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

    std::map<std::string, NetworkProfileChoice> networks;

    Config();
    void loadFromFile(const std::string& filePath);
    void saveToFile(const std::string& filePath) const;
    void reset();
    void copyProfile(const Config& cfg, Config::NetworkProfile networkProfile);

    bool operator==(const Config& cfg) const;
    bool operator!=(const Config& cfg) const;
    bool equalProfile(const Config& cfg, Config::NetworkProfile networkProfile) const;
    bool networksModifiedFrom(const std::map<std::string, NetworkProfileChoice>& from) const;

    static std::string networkProfileDisplayName(NetworkProfile np);
    static std::string networkProfileKey(NetworkProfile np);
    static NetworkProfile networkProfileFromKey(const std::string& key);
    static std::string networkProfileChoiceDisplayName(NetworkProfileChoice npc);
    static std::string networkProfileChoiceKey(NetworkProfileChoice npc);
    static NetworkProfileChoice networkProfileChoiceFromKey(const std::string& key);
    static NetworkProfile networkProfileFromChoice(NetworkProfileChoice npc, NetworkProfile default);
    static NetworkProfileChoice networkChoiceFromProfile(NetworkProfile np);
};

#endif

/* Local Variables: */
/* mode: c++        */
/* End:             */
