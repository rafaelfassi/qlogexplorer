#pragma once

namespace utl
{

std::string toStr(const rapidjson::Value &json);

std::string join(const std::vector<std::string> &strList, const std::string &delim);

std::vector<std::string> split(const std::string &str, const std::string &delim);

} // namespace utl
