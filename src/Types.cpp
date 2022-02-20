// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

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
    LOG_ERR("Type {} not found", toInt(type));
    str = "NONE";
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
    LOG_ERR("Type '{}' not found", str);
    type = static_cast<T>(0);
}

template <typename T>
void flagsToStr(const std::vector<std::pair<T, std::string>> &flagsMap, const Flags<T> &flags, std::string &str)
{
    str.clear();
    std::vector<std::string> strFlags;
    for (const auto &flag : flagsMap)
    {
        if (flags.has(flag.first))
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
void flagsFromStr(const std::vector<std::pair<T, std::string>> &flagsMap, const std::string &str, Flags<T> &flags)
{
    flags.reset();
    const std::vector<std::string> &strFlags = utl::split(str, "|");
    for (const auto &strFlag : strFlags)
    {
        const auto &it =
            std::find_if(flagsMap.begin(), flagsMap.end(), [&strFlag](const auto &p) { return (p.second == strFlag); });
        if (it != flagsMap.end())
        {
            flags.set(it->first);
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
    {SearchType::SubString, "SUB_STR"},
    {SearchType::Range, "RANGE"}};
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

static const std::vector<std::pair<SearchFlag, std::string>> g_searchFlagsMap = {
    {SearchFlag::MatchCase, "MATCH_CASE"},
    {SearchFlag::NotOperator, "NOT"}};

void toStr(const SearchFlags &flags, std::string &str)
{
    flagsToStr<SearchFlag>(g_searchFlagsMap, flags, str);
}

void fromStr(const std::string &str, SearchFlags &flags)
{
    flagsFromStr<SearchFlag>(g_searchFlagsMap, str, flags);
}

} // namespace tp