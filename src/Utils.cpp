// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "Utils.h"
#include <iostream>

namespace utl
{

void log(const char *file, const std::uint32_t line, tp::LogLevel level, const std::string &msg)
{
    std::cout << "[" << tp::toStr<tp::LogLevel>(level) << "] " << file << ":" << line << ": " << msg << std::endl;
}

std::string toStr(const rapidjson::Value &json)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
}

std::string toStr(const QString &str)
{
    return str.toStdString();
}

std::string toStr(const QColor &color)
{
    return toStr(color.name());
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

std::string toUpper(const std::string &text)
{
    std::string res(text);
    for (auto it = res.begin(); it != res.end(); ++it)
        if (*it >= 'a' && *it <= 'z')
            *it &= ~0x20;
    return res;
}

QString elideLeft(const std::string &str, tp::UInt maxSize)
{
    QString res(utl::toQStr(str));
    if ((res.size() > maxSize) && (maxSize > 3))
    {
        res = res.mid(res.size() - maxSize + 3);
        res.prepend("...");
    }
    return res;
}

bool startsWith(std::string_view str, std::string_view prefix)
{
    return (str.size() >= prefix.size()) && (str.compare(0, prefix.size(), prefix) == 0);
}

bool endsWith(std::string_view str, std::string_view suffix)
{
    if (str.length() >= suffix.length())
    {
        return (0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix));
    }
    return false;
}

bool contains(std::string_view str, std::string_view sub, bool icase)
{
    if (!icase)
    {
        return (str.find(sub) != std::string::npos);
    }
    else
    {
        return std::search(str.begin(), str.end(), sub.begin(), sub.end(), compareCaseInsensitive) != str.end();
    }
}

void replaceAll(std::string &str, const std::string &toSearch, const std::string &replaceWith)
{
    size_t pos = str.find(toSearch);
    while (pos != std::string::npos)
    {
        str.replace(pos, toSearch.length(), replaceWith);
        pos = str.find(toSearch, pos + replaceWith.length());
    }
}

QVariant toVariant(const tp::Column &column, const QString &text)
{
    switch (column.type)
    {
        case tp::ColumnType::Int:
            return text.toLongLong();
        case tp::ColumnType::UInt:
            return text.toULongLong();
        case tp::ColumnType::Time:
        {
            if (column.format == "SECONDS")
                return QDateTime::fromSecsSinceEpoch(text.toLongLong());
            else if (column.format == "MILLISECONDS")
                return QDateTime::fromMSecsSinceEpoch(text.toLongLong());
            else
                return QDateTime::fromString(text, utl::toQStr(column.format));
        }
        case tp::ColumnType::Float:
            return text.toDouble();
        default:
            return text;
    }
}

} // namespace utl
