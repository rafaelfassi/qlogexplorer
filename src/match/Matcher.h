#pragma once

#include "BaseMatcher.h"

class Matcher
{
public:
    Matcher() = default;

    void setParam(const tp::SearchParam &param);
    void setParams(const tp::SearchParams &params, bool orOp);
    bool match(const std::string &text) const;
    bool matchInRow(const tp::RowData &rowData) const;

    static void makeMatcher(const tp::SearchParam &param, Matchers &matchers);
    static void makeMatchers(const tp::SearchParams &params, Matchers &matchers);
    static bool match(const Matchers &matchers, bool orOp, const std::string &text);
    static bool matchInRow(const Matchers &matchers, bool orOp, const tp::RowData &rowData);

private:

    Matchers m_matchers;
    bool m_orOp = false;
};
