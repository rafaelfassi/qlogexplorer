#pragma once

class BaseMatcher
{
public:
    BaseMatcher(const tp::SearchParam &param) : m_param(param) {}
    virtual bool match(const std::string &text) = 0;
    bool isRegex() const { return (m_param.type == tp::SearchType::Regex); }
    bool matchCase() const { return (m_param.flags & tp::SearchFlags::MatchCase); }
    bool notOp() const { return (m_param.flags & tp::SearchFlags::NotOperator); }
    bool hasColumn() const { return m_param.column.has_value(); }
    tp::UInt getColumn() const { return m_param.column.value(); }

protected:
    const tp::SearchParam m_param;
};

using Matchers = std::vector<std::unique_ptr<BaseMatcher>>;
