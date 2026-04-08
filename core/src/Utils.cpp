// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "Utils.h"

namespace utl
{

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

    void replaceAll(std::string &str, const std::string &toSearch, const std::string &replaceWith)
    {
        size_t pos = str.find(toSearch);
        while (pos != std::string::npos)
        {
            str.replace(pos, toSearch.length(), replaceWith);
            pos = str.find(toSearch, pos + replaceWith.length());
        }
    }

    void replaceStrIf(
        std::string &str,
        const std::string &toSearch,
        std::string_view replacement,
        std::function<bool(std::string_view, std::size_t)> pred
    )
    {
        std::size_t pos = str.find(toSearch);
        while (pos != std::string::npos)
        {
            if (pred(str, pos))
            {
                str.replace(pos, toSearch.length(), replacement);
                pos = str.find(toSearch, pos + replacement.length());
            }
            else
            {
                pos = str.find(toSearch, pos + 1);
            }
        }
    }

    void simplify(std::string &str)
    {
        if ((str.find("  ") == std::string::npos) && (str.find_first_of("\t\n\r") == std::string::npos))
            return;

        std::string sStr;
        sStr.reserve(str.size());
        bool inSpace = false;
        for (auto ch : str)
        {
            switch (ch)
            {
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if (inSpace)
                        break;
                    sStr += ' ';
                    inSpace = true;
                    break;
                default:
                    sStr += ch;
                    if (inSpace)
                        inSpace = false;
                    break;
            }
        }

        str = std::move(sStr);
    }

    void trim(std::string &str)
    {
        if (!str.empty() && (str.front() == ' ' || str.back() == ' '))
        {

            std::string_view sv{str};
            while (!sv.empty() && sv.front() == ' ')
            {
                sv.remove_prefix(1);
            }

            while (!sv.empty() && sv.back() == ' ')
            {
                sv.remove_suffix(1);
            }

            str = std::string(sv);
        }
    }

    std::string reduceRxPatternForReplacement(std::string_view pattern)
    {
        std::string reducedRx;
        reducedRx.reserve(pattern.size());
        bool inSpace = false;
        for (auto i = 0; i < pattern.size(); ++i)
        {
            switch (pattern[i])
            {
                case ' ':
                    if (inSpace)
                        break;
                    reducedRx += ' ';
                    inSpace = true;
                    break;
                case '\\':
                    if (i + 1 < pattern.size())
                    {
                        if (char nc = pattern[i + 1]; nc == 's' || nc == 'n' || nc == 'r' || nc == 't')
                        {
                            ++i;
                            if ((i + 1 < pattern.size()) && (pattern[i + 1] == '+' || pattern[i + 1] == '*'))
                            {
                                ++i;
                            }
                            if (inSpace)
                                break;
                            reducedRx += ' ';
                            inSpace = true;
                            break;
                        }
                        else if (nc == '/')
                        {
                            reducedRx += nc;
                            ++i;
                            if (inSpace)
                                inSpace = false;
                            break;
                        }
                        else if (nc == '\\')
                        {
                            reducedRx += "\\\\";
                            ++i;
                            if (inSpace)
                                inSpace = false;
                            break;
                        }
                    }
                    [[fallthrough]];
                default:
                    reducedRx += pattern[i];
                    if (inSpace)
                        inSpace = false;
                    break;
            }
        }

        return reducedRx;
    }

    std::string_view getRxReplacementForRawJson(char c)
    {
        /*
            (")   = (\") or (U+0022)
            (\)   = (\\) or (U+005C)
            (/)   = (\/) or (U+002F)
            (\n)  = (\n) or (U+000A)
            (\r)  = (\r) or (U+000D)
            (\t)  = (\t) or (U+0009)
        */

        switch (c)
        {
            case '/':
                return R"_(((\/)|(\\u002[Ff])))_";
            case '\\':
                return R"_(((\\\\)|(\\u005[Cc])))_";
            case '\"':
                return R"_(((\\")|(\\u0022)))_";
            case ' ':
                return R"_(((\s)|(\\t)|(\\n)|(\\r)|(\\u000[9ADad]))+)_";
            default:
                return "";
        }
    }

    bool removeRegexAnchors(std::string &pattern)
    {
        if (!utl::contains(pattern, '^')
            && !utl::contains(pattern, '$')
            && !utl::contains(pattern, "\\A")
            && !utl::contains(pattern, "\\z"))
            return true;

        std::string_view sv{pattern};
        if (sv.front() == '^')
            sv.remove_prefix(1);
        else if (utl::startsWith(sv, "\\A"))
            sv.remove_prefix(2);

        if (sv.back() == '$' && !utl::endsWith(sv, "\\$"))
            sv.remove_suffix(1);
        else if (utl::endsWith(sv, "\\z") && !utl::endsWith(sv, "\\\\z"))
            sv.remove_suffix(2);

        bool inBlock = false;
        bool inBkSlash = false;
        for (auto ch : sv)
        {
            switch (ch)
            {
                case '\\':
                    inBkSlash = !inBkSlash;
                    break;
                case '[':
                    if (!inBkSlash)
                        inBlock = true;
                    inBkSlash = false;
                    break;
                case ']':
                    if (!inBkSlash && inBlock)
                        inBlock = false;
                    inBkSlash = false;
                    break;
                case '$':
                case '^':
                    if (!inBlock && !inBkSlash)
                        return false;
                    inBkSlash = false;
                    break;
                case 'A':
                case 'z':
                    if (inBkSlash)
                        return false;
                    break;
                default:
                    inBkSlash = false;
                    break;
            }
        }

        if (sv != pattern)
        {
            pattern = std::string(sv);
        }

        return true;
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
