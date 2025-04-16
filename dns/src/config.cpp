#include "config.hpp"
#include <fstream>
#include <iterator>
#include <utility>

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
