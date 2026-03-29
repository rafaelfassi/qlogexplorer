// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#ifndef NO_STD_PARALLEL_ALGORITHMS
#include <execution>
#define PARALLEL_SORT(it_first, it_last) std::sort(std::execution::par, it_first, it_last);
#else
#define PARALLEL_SORT(it_first, it_last) std::sort(it_first, it_last);
#endif

#define LOG_INF(...) utl::log(utl::getSrcFile(__FILE__), __LINE__, tp::LogLevel::Info, fmt::format(__VA_ARGS__))
#define LOG_WAR(...) utl::log(utl::getSrcFile(__FILE__), __LINE__, tp::LogLevel::Warning, fmt::format(__VA_ARGS__))
#define LOG_ERR(...) utl::log(utl::getSrcFile(__FILE__), __LINE__, tp::LogLevel::Error, fmt::format(__VA_ARGS__))

namespace utl
{

static constexpr const char *getSrcFile(const char *const path)
{
    return path + sizeof(APP_BASE_SRC_DIR);
}

static constexpr std::array<char, 256> makeUpperArray()
{
    std::array<char, 256> table{};
    for (int i = 0; i < 256; ++i)
    {
        table[i] = static_cast<char>(i);
    }
    for (int i = 'a'; i <= 'z'; ++i)
    {
        table[i] = (static_cast<char>(i) & ~0x20);
    }
    return table;
}
constexpr auto utab = utl::makeUpperArray();

inline bool compareCaseInsensitive(char l, char r)
{
    return (utab[static_cast<unsigned char>(l)] == utab[static_cast<unsigned char>(r)]);
}

void log(const char *file, const std::uint32_t line, tp::LogLevel level, const std::string &msg);

std::string toStr(const rapidjson::Value &json);

std::string toStr(const QString &str);

std::string toStr(const QColor &color);

inline QString toQStr(std::string_view str)
{
    return QString::fromUtf8(str.data(), str.size());
}

std::string join(const std::vector<std::string> &strList, const std::string &delim);

std::vector<std::string> split(const std::string &str, const std::string &delim);

std::string toUpper(const std::string &text);

QString elideLeft(const std::string &str, tp::UInt maxSize);

bool startsWith(std::string_view str, std::string_view prefix);

bool endsWith(std::string_view str, std::string_view suffix);

bool contains(std::string_view str, std::string_view sub, bool icase = false);

void replaceAll(std::string &str, const std::string &toSearch, const std::string &replaceWith);

QVariant toVariant(const tp::Column &column, const QString &text);

template <typename T> std::optional<T> GetValueOpt(const rapidjson::Value &jsonObj, const std::string &key)
{
    std::optional<T> val;

    if (const auto &it = jsonObj.FindMember(key); it != jsonObj.MemberEnd() && !it->value.IsNull())
    {
        if constexpr (std::is_enum_v<T> || std::is_base_of<tp::BaseFlags, T>::value)
        {
            const auto s = it->value.GetString();
            val = tp::fromStr<T>(s);
        }
        else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
        {
            val = it->value.GetString();
        }
        else if constexpr (std::is_same_v<T, QString>)
        {
            val = toQStr(it->value.GetString());
        }
        else if constexpr (std::is_integral<T>::value)
        {
            if constexpr (std::is_same<T, bool>::value)
            {
                val = it->value.GetBool();
            }
            else if constexpr (std::is_signed<T>::value)
            {
                val = it->value.GetInt64();
            }
            else
            {
                val = it->value.GetUint64();
            }
        }
        else if constexpr (std::is_floating_point<T>::value)
        {
            val = it->value.GetDouble();
        }
    }

    return val;
}

} // namespace utl
