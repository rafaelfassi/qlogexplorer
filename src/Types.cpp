#include "pch.h"
#include "Types.h"

namespace tp
{

template <typename T>
void enumToStr(const std::vector<std::pair<T, std::string>> &typeMap, const T &type, std::string &str)
{
    const auto &it = std::find_if(typeMap.begin(), typeMap.end(), [&type](const auto &p) { return (p.first == type); });
    if (it != typeMap.end())
    {
        str = it->second;
        return;
    }
    LOG_ERR("ColumnType {} not found", static_cast<int>(type));
}

template <typename T>
void enumFromStr(const std::vector<std::pair<T, std::string>> &typeMap, const std::string &str, T &type)
{
    const auto &it = std::find_if(typeMap.begin(), typeMap.end(), [&str](const auto &p) { return (p.second == str); });
    if (it != typeMap.end())
    {
        type = it->first;
        return;
    }
    LOG_ERR("ColumnType '{}' not found", str);
}

template <typename T>
void flagsToStr(const std::vector<std::pair<T, std::string>> &flagsMap, const int &flags, std::string &str)
{
    std::vector<std::string> strFlags;
    for (const auto &flag : flagsMap)
    {
        if (flags & flag.first)
        {
            strFlags.emplace_back(flag.second);
        }
    }
    if (!strFlags.empty())
    {
        str = utl::join(strFlags, "|");
    }
}

template <typename T>
void flagsFromStr(const std::vector<std::pair<T, std::string>> &flagsMap, const std::string &str, int &flags)
{
    flags = 0;
    const std::vector<std::string> &strFlags = utl::split(str, "|");
    for (const auto &strFlag : strFlags)
    {
        const auto &it =
            std::find_if(flagsMap.begin(), flagsMap.end(), [&strFlag](const auto &p) { return (p.second == strFlag); });
        if (it != flagsMap.end())
        {
            flags |= it->first;
        }
    }
}

static const std::vector<std::pair<LogLevel, std::string>> g_logLevelMap = {
    {LogLevel::Info, "INFO"},
    {LogLevel::Warning, "WARN"},
    {LogLevel::Error, "ERROR"}};
void toStr(const LogLevel &type, std::string &str)
{
    enumToStr<LogLevel>(g_logLevelMap, type, str);
}
void fromStr(const std::string &str, LogLevel &type)
{
    enumFromStr<LogLevel>(g_logLevelMap, str, type);
}

static const std::vector<std::pair<FileType, std::string>> g_fileTypeMap = {
    {FileType::Text, "TEXT"},
    {FileType::Json, "JSON"}};
void toStr(const FileType &type, std::string &str)
{
    enumToStr<FileType>(g_fileTypeMap, type, str);
}
void fromStr(const std::string &str, FileType &type)
{
    enumFromStr<FileType>(g_fileTypeMap, str, type);
}

static const std::vector<std::pair<SearchType, std::string>> g_searchTypeMap = {
    {SearchType::Regex, "REGEX"},
    {SearchType::SubString, "SUB_STR"}};
void toStr(const SearchType &type, std::string &str)
{
    enumToStr<SearchType>(g_searchTypeMap, type, str);
}
void fromStr(const std::string &str, SearchType &type)
{
    enumFromStr<SearchType>(g_searchTypeMap, str, type);
}

static const std::vector<std::pair<ColumnType, std::string>> g_columnTypeMap = {
    {ColumnType::Str, "STR"},
    {ColumnType::Int, "INT"},
    {ColumnType::UInt, "UINT"},
    {ColumnType::Time, "TIME"},
    {ColumnType::Bool, "BOOL"},
    {ColumnType::Float, "FLOAT"}};
void toStr(const ColumnType &type, std::string &str)
{
    enumToStr<ColumnType>(g_columnTypeMap, type, str);
}
void fromStr(const std::string &str, ColumnType &type)
{
    enumFromStr<ColumnType>(g_columnTypeMap, str, type);
}

static const std::vector<std::pair<SearchFlags, std::string>> g_searchFlagsMap = {
    {SearchFlags::NoSearchFlags, "NONE"},
    {SearchFlags::MatchCase, "MATCH_CASE"},
    {SearchFlags::NotOperator, "NOT"}};

void toStr(const SearchFlags &, const int &flags, std::string &str)
{
    flagsToStr<SearchFlags>(g_searchFlagsMap, flags, str);
}

void fromStr(const SearchFlags &, const std::string &str, int &flags)
{
    flagsFromStr<SearchFlags>(g_searchFlagsMap, str, flags);
}

} // namespace tp