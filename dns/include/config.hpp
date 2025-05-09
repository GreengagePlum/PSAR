#pragma once

#include <netinet/in.h>

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace microdns
{

class ConfigLoader
{
    std::string path;
    std::unordered_map<std::string, std::string> params;
    std::vector<struct in_addr> nameservers;

    const std::regex ipMatcher{R"(^nameserver (\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\s*$)"};

    void loadNameservers();

  public:
    ConfigLoader(const std::string &path);

    const std::string &getParam(const std::string &param) const;

    const decltype(nameservers) &getNameservers() const;

    size_t getCount() const;

    const std::string &getPath() const;
};

} // namespace microdns
