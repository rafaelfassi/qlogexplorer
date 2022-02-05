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
    // Way more farter, but works only for ascii characters.
    // std::string res(text);
    // for (auto it = res.begin(); it != res.end(); ++it)
    //     if (*it >= 'a' && *it <= 'z')
    //         *it &= ~0x20;
    // return res;

    std::string res(text);
    std::transform(text.begin(), text.end(), res.begin(), [](int c) { return std::toupper(c); });
    return res;
}

QString elideLeft(const std::string &str, tp::UInt maxSize)
{
    QString res(str.c_str());
    if ((res.size() > maxSize) && (maxSize > 3))
    {
        res = res.mid(res.size() - maxSize + 3);
        res.prepend("...");
    }
    return res;
}

QVariant toVariant(const tp::Column& column, const QString& text)
{
    QVariant v;
    switch (column.type)
    {
    case tp::ColumnType::Int:
        v = text.toLongLong();
    case tp::ColumnType::UInt:
        return text.toULongLong();
    case tp::ColumnType::Time:
        return QDateTime::fromString(text, column.format.c_str());
    case tp::ColumnType::Float:
        return text.toDouble();
    default:
        return text;
    }
}

} // namespace utl