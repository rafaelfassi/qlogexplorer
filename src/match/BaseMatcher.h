// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

class BaseMatcher
{
public:
    BaseMatcher(const tp::SearchParam &param) : m_param(param) {}
    virtual ~BaseMatcher() {}
    virtual bool match(std::string_view text) = 0;
    virtual bool quickRawMatch(tp::FileType fileType, bool isBlock, std::string_view rawText) = 0;
    bool isRegex() const { return (m_param.type == tp::SearchType::Regex); }
    bool matchCase() const { return m_param.flags.has(tp::SearchFlag::MatchCase); }
    bool notOp() const { return m_param.flags.has(tp::SearchFlag::NotOperator); }
    bool hasColumn() const { return m_param.column.has_value(); }
    tp::UInt getColumn() const { return m_param.column.value().idx; }

protected:
    const tp::SearchParam m_param;
};

using Matchers = std::vector<std::unique_ptr<BaseMatcher>>;
