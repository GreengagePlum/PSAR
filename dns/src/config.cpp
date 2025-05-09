#include <arpa/inet.h>
#include <netinet/in.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <utility>

#include "config.hpp"

namespace microdns
{

ConfigLoader::ConfigLoader(const std::string &path) : path(path)
{
    std::ifstream ifs(path);
    for (std::string s; std::getline(ifs, s);)
    {
        auto pos = s.find('=');

        // Move on if malformed line (no equal sign found)
        if (pos == s.npos)
            continue;

        std::string key = s.substr(0, pos);
        std::string value = s.substr(pos + 1);

        // Remove quotes on both ends if present
        if (value[0] == '"' && value[value.size() - 1] == '"')
        {
            value.erase(value.begin());
            value.erase(std::prev(value.end()));
        }

        params.emplace(std::make_pair(std::move(key), std::move(value)));
    }

    loadNameservers();
}

void ConfigLoader::loadNameservers()
{
    auto &resolv = getParam("BACKUP_FILE_RESOLV");

    std::ifstream ifs(resolv);
    for (std::string s; std::getline(ifs, s);)
    {
        std::smatch ipMatch;
        // Move on if malformed line (no nameserver line with IP found)
        if (!std::regex_match(s, ipMatch, ipMatcher))
            continue;

        struct in_addr ip;
        if (inet_pton(AF_INET, ipMatch[1].str().c_str(), &ip) == 0)
        {
            std::cerr << "ConfigLoader invalid IPv4 address read from resolv.conf, ignoring..." << std::endl;
            continue;
        }

        nameservers.emplace_back(std::move(ip));
    }
}

const decltype(ConfigLoader::nameservers) &ConfigLoader::getNameservers() const
{
    return nameservers;
}

// may throw std::out_of_range
// https://en.cppreference.com/w/cpp/container/unordered_map/at
const std::string &ConfigLoader::getParam(const std::string &param) const
{
    return params.at(param);
}

size_t ConfigLoader::getCount() const
{
    return params.size();
}

const std::string &ConfigLoader::getPath() const
{
    return path;
}

} // namespace microdns
