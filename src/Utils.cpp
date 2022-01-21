#include "pch.h"
#include "Utils.h"
#include <sstream>

namespace utl
{

std::string toStr(const rapidjson::Value &json)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
}

std::string join(const std::vector<std::string> &strList, const std::string &delim)
{
    std::string res;
    for (auto it = strList.begin(); it != strList.end(); ++it)
    {
        if (it != strList.begin())
            res.append(delim);
        res.append(*it);
    }
    return res;
}

std::vector<std::string> split(const std::string &str, const std::string &delim)
{
    std::vector<std::string> res;
    auto start = 0U;
    auto end = str.find(delim);
    while (end != std::string::npos)
    {
        res.emplace_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    res.emplace_back(str.substr(start));
    return res;
}

}