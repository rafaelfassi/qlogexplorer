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
    bool operator==(const Flags<EnumT> &other) const { return (m_bits == other.m_bits); }

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
    Column(SInt _idx) : idx(_idx), pos(_idx) {}
    SInt idx;
    SInt pos;
    std::string key;
    std::string name;
    std::string format;
    ColumnType type = ColumnType::Str;
    SInt width = -1;
};
inline bool areSimilar(const Column &lhs, const Column &rhs)
{
    return (lhs.idx == rhs.idx) && (lhs.type == rhs.type);
}
inline bool areSimilar(const std::optional<Column> &lhs, const std::optional<Column> &rhs)
{
    if (lhs.has_value() == rhs.has_value())
    {
        return lhs.has_value() ? areSimilar(lhs.value(), rhs.value()) : true;
    }
    return false;
}
inline bool operator==(const Column &lhs, const Column &rhs)
{
    return (lhs.idx == rhs.idx) && (lhs.pos == rhs.pos) && (lhs.key == rhs.key) && (lhs.name == rhs.name) &&
           (lhs.format == rhs.format) && (lhs.type == rhs.type) && (lhs.width == rhs.width);
}
using Columns = std::vector<Column>;
using ColumnsRef = std::vector<std::reference_wrapper<Column>>;

struct SearchParam
{
    SearchType type = SearchType::SubString;
    SearchFlags flags;
    std::string pattern;
    std::optional<Column> column;
};
inline bool areSimilar(const SearchParam &lhs, const SearchParam &rhs)
{
    return (lhs.type == rhs.type) && (lhs.flags == rhs.flags) && (lhs.pattern == rhs.pattern) &&
           areSimilar(lhs.column, rhs.column);
}
inline bool operator==(const SearchParam &lhs, const SearchParam &rhs)
{
    return (lhs.type == rhs.type) && (lhs.flags == rhs.flags) && (lhs.pattern == rhs.pattern) &&
           (lhs.column == rhs.column);
}
inline bool operator!=(const SearchParam &lhs, const SearchParam &rhs)
{
    return !(lhs == rhs);
}
using SearchParams = std::vector<SearchParam>;

struct SectionColor
{
    SectionColor() = default;
    SectionColor(const QColor &_fg, const QColor &_bg) : fg(_fg), bg(_bg) {}
    QColor fg;
    QColor bg;
};
inline bool operator==(const SectionColor &lhs, const SectionColor &rhs)
{
    return (lhs.fg == rhs.fg) && (lhs.bg == rhs.bg);
}

struct TextCan
{
    TextCan() = default;
    TextCan(const QString &_text) : text(_text) {}
    TextCan(const QRect &_rect, const QString &_text = {}) : rect(_rect), text(_text) {}
    QRect rect;
    QString text;
};

struct TextSelection
{
    TextSelection() = default;
    TextSelection(const TextCan &_can, const SectionColor &_color) : can(_can), color(_color) {}
    TextCan can;
    SectionColor color;
};

struct HighlighterParam
{
    SearchParam searchParam;
    SectionColor color;
};
inline bool operator==(const HighlighterParam &lhs, const HighlighterParam &rhs)
{
    return (lhs.searchParam == rhs.searchParam) && (lhs.color == rhs.color);
}
using HighlighterParams = std::vector<HighlighterParam>;

struct FilterParam
{
    SearchParam searchParam;
    std::string name;
};
inline bool operator==(const FilterParam &lhs, const FilterParam &rhs)
{
    return (lhs.searchParam == rhs.searchParam) && (lhs.name == rhs.name);
}
using FilterParams = std::vector<FilterParam>;

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

template <typename T> SInt toSInt(const T &type)
{
    return static_cast<SInt>(type);
}

template <typename T> int toInt(const T &type)
{
    return static_cast<int>(type);
}

template <typename T> T fromInt(SInt typeInt)
{
    const auto &typeStr(toStr<T>(static_cast<T>(typeInt)));
    return fromStr<T>(typeStr);
}

} // namespace tp
