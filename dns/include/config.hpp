#pragma once

#include <string>
#include <unordered_map>

namespace microdns
{

class ConfigLoader
{
    std::string path;
    std::unordered_map<std::string, std::string> params;

  public:
    ConfigLoader(const std::string &path);

    const std::string &getParam(const std::string &param) const;

    size_t getCount() const;

    const std::string &getPath() const;
};

} // namespace microdns
