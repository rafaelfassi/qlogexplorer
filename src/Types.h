// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

namespace tp
{

using SInt = std::intptr_t;
using UInt = std::uintptr_t;
using SIntList = std::deque<tp::SInt>;
using RowData = std::vector<std::string>;
using SharedSIntList = std::shared_ptr<SIntList>;

class BaseFlags
{
};
template <typename EnumT> class Flags : public BaseFlags
{
    static_assert(std::is_enum_v<EnumT>, "Flags can only be specialized for enum types");
    using UnderlyingTp = std::make_unsigned_t<std::underlying_type_t<EnumT>>;

public:
    Flags() = default;
    Flags(EnumT e) { set(e); }
    void set(EnumT e, bool value = true) { m_bits.set(underlying(e), value); }
    void reset() noexcept { m_bits.reset(); }
    bool none() const noexcept { return m_bits.none(); }
    bool has(EnumT e) const { return m_bits.test(underlying(e)); }

private:
    static constexpr UnderlyingTp underlying(EnumT e) { return static_cast<UnderlyingTp>(e); }
    std::bitset<sizeof(UnderlyingTp) * CHAR_BIT> m_bits;
};

enum class LogLevel
{
    None,
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
    None,
    Regex,
    SubString,
    Range
};
void toStr(const SearchType &type, std::string &str);
void fromStr(const std::string &str, SearchType &type);

enum class SearchFlag
{
    MatchCase,
    NotOperator
};
using SearchFlags = Flags<SearchFlag>;
void toStr(const SearchFlags &flags, std::string &str);
void fromStr(const std::string &str, SearchFlags &flags);

enum class ColumnType
{
    None,
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
    Column() = default;
    Column(SInt _idx) : idx(_idx), pos(_idx) {}
    SInt idx = -1;
    SInt pos = -1;
    std::string key;
    std::string name;
    std::string format;
    ColumnType type = ColumnType::Str;
    SInt width = -1;
};
using Columns = std::vector<Column>;
using ColumnsRef = std::vector<std::reference_wrapper<Column>>;

struct SearchParam
{
    SearchType type = SearchType::None;
    SearchFlags flags;
    std::string pattern;
    std::optional<Column> column;
};
using SearchParams = std::vector<SearchParam>;

struct HighlighterParam
{
    SearchParam searchParam;
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

template <typename T> T fromStr(const std::string &str)
{
    T type;
    fromStr(str, type);
    return type;
}

template <typename T> SInt toInt(const T &type)
{
    return static_cast<SInt>(type);
}

} // namespace tp