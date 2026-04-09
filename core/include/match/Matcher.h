// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "BaseMatcher.h"

class Matcher
{
public:
    Matcher() = default;

    void setParam(const tp::SearchParam &param);
    void setParams(const tp::SearchParams &params, bool orOp);
    bool match(std::string_view text) const;
    bool matchInRow(const tp::RowData &rowData) const;
    bool preMatchRawText(tp::FileType fileType, bool isBlock, std::string_view rawText) const;

    static void makeMatcher(const tp::SearchParam &param, Matchers &matchers);
    static void makeMatchers(const tp::SearchParams &params, Matchers &matchers);
    static bool match(const Matchers &matchers, bool orOp, std::string_view text);
    static bool matchInRow(const Matchers &matchers, bool orOp, const tp::RowData &rowData);
    static bool preMatchRawText(
        const Matchers &matchers,
        bool orOp,
        tp::FileType fileType,
        bool isBlock,
        std::string_view rawText);

private:
    Matchers m_matchers;
    bool m_orOp = false;
};
