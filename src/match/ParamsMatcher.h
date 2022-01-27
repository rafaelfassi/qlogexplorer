#pragma once

#include "BaseMatcher.h"

class ParamsMatcher
{
public:
    ParamsMatcher() = default;
    ParamsMatcher(const tp::SearchParams &params, bool orOp);

    void setParams(const tp::SearchParams &params, bool orOp);
    bool match(const std::string &text);
    bool matchInRow(const tp::RowData &rowData);

    static Matchers makeMatchers(const tp::SearchParams &params);
    static bool match(const Matchers &matchers, bool orOp, const std::string &text);
    static bool matchInRow(const Matchers &matchers, bool orOp, const tp::RowData &rowData);

private:
    Matchers m_matchers;
    bool m_orOp = false;
};
