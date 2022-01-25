#pragma once

namespace tp
{

using SInt = ssize_t;
using UInt = size_t;
using SIntList = std::deque<tp::SInt>;
using SharedSIntList = std::shared_ptr<SIntList>;

enum class LogLevel
{
    Info,
    Warning,
    Error
};
void toStr(const LogLevel &type, std::string &str);
void fromStr(const std::string &str, LogLevel &type);

enum class FileType
{
    None,
    Text,
    Json
};
void toStr(const FileType &type, std::string &str);
void fromStr(const std::string &str, FileType &type);

enum class SearchType
{
    Regex,
    SubString
};
void toStr(const SearchType &type, std::string &str);
void fromStr(const std::string &str, SearchType &type);

enum SearchFlags
{
    NoSearchFlags = 0x00,
    MatchCase = 0x01,
    NotOperator = 0x02
};
void toStr(const SearchFlags &, const int &flags, std::string &str);
void fromStr(const SearchFlags &, const std::string &str, int &flags);

enum class ColumnType
{
    Str,
    Int,
    UInt,
    Time,
    Bool,
    Float
};
void toStr(const ColumnType &type, std::string &str);
void fromStr(const std::string &str, ColumnType &type);

struct Column
{
    int32_t idx = -1;
    int32_t pos = -1;
    std::string key;
    std::string name;
    std::string format;
    ColumnType type = ColumnType::Str;
    int32_t width = -1;
};
using Columns = std::vector<Column>;
using ColumnsRef = std::vector<std::reference_wrapper<Column>>;

struct HighlighterParam
{
    std::optional<int32_t> column;
    SearchType type;
    std::string pattern;
    SearchFlags flags;
    QColor textColor;
    QColor bgColor;
};
using HighlighterParams = std::vector<HighlighterParam>;

template <typename T> std::string toStr(const T &type)
{
    std::string str;
    toStr(type, str);
    return str;
}

template <typename T> T fromStr(const std::string& str)
{
    T type;;
    fromStr(str, type);
    return type;
}

} // namespace tp