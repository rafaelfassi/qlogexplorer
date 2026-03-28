// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "Matcher.h"
#include "RegexMatcher.h"
#include "SubStringMatcher.h"
#include "RangeMatcher.h"

static bool hasRegexPattern(const tp::SearchParam &param)
{
    return (param.pattern.find_first_of(".*+?^$|()[]{}\\") != std::string::npos);
}

void Matcher::setParam(const tp::SearchParam &param)
{
    m_matchers.clear();
    makeMatcher(param, m_matchers);
}

void Matcher::setParams(const tp::SearchParams &params, bool orOp)
{
    m_matchers.clear();
    makeMatchers(params, m_matchers);
    m_orOp = orOp;
}

bool Matcher::match(std::string_view text) const
{
    return match(m_matchers, m_orOp, text);
}

bool Matcher::matchInRow(const tp::RowData &rowData) const
{
    return matchInRow(m_matchers, m_orOp, rowData);
}

bool Matcher::quickRawMatch(tp::FileType fileType, bool isBlock, std::string_view rawText) const
{
    return quickRawMatch(m_matchers, m_orOp, fileType, isBlock, rawText);
}

void Matcher::makeMatcher(const tp::SearchParam &param, Matchers &matchers)
{
    switch (param.type)
    {
        case tp::SearchType::Regex:
            if (hasRegexPattern(param))
                matchers.emplace_back(std::make_unique<RegexMatcher>(param));
            else
                matchers.emplace_back(std::make_unique<SubStringMatcher>(param));
            break;
        case tp::SearchType::SubString:
            matchers.emplace_back(std::make_unique<SubStringMatcher>(param));
            break;
        case tp::SearchType::Range:
            matchers.emplace_back(std::make_unique<RangeMatcher>(param));
            break;
        default:
            LOG_ERR("invalid SearchType {}", tp::toSInt(param.type));
    };
}

void Matcher::makeMatchers(const tp::SearchParams &params, Matchers &matchers)
{
    matchers.reserve(params.size());
    for (const auto &param : params)
    {
        makeMatcher(param, matchers);
    }
}

bool Matcher::match(const Matchers &matchers, bool orOp, std::string_view text)
{
    std::uint32_t cnt(0);

    for (const auto &matcher : matchers)
    {
        bool matched = matcher->match(text);
        if (matcher->notOp())
            matched = !matched;

        if (matched)
        {
            if ((++cnt == matchers.size()) || orOp)
                return true;
        }
    }

    return false;
}

bool Matcher::matchInRow(const Matchers &matchers, bool orOp, const tp::RowData &rowData)
{
    std::uint32_t cnt(0);

    for (const auto &matcher : matchers)
    {
        if (matcher->hasColumn())
        {
            if (matcher->getColumn() < rowData.size())
            {
                bool matched = matcher->match(rowData[matcher->getColumn()]);
                if (matcher->notOp())
                    matched = !matched;
                if (matched)
                {
                    if ((++cnt == matchers.size()) || orOp)
                        return true;
                }
            }
            else
            {
                LOG_ERR("Matcher column {} is bigger than row columns {}", matcher->getColumn(), rowData.size() - 1);
            }
        }
        else
        {
            bool matched(false);
            for (const auto &columnData : rowData)
            {
                if (matcher->match(columnData))
                {
                    matched = true;
                    break;
                }
            }
            if (matcher->notOp())
                matched = !matched;
            if (matched)
            {
                if ((++cnt == matchers.size()) || orOp)
                    return true;
            }
        }
    }

    return false;
}

bool Matcher::quickRawMatch(
    const Matchers &matchers,
    bool orOp,
    tp::FileType fileType,
    bool isBlock,
    std::string_view rawText)
{
    std::uint32_t cnt(0);

    if (orOp)
    {
        for (const auto &matcher : matchers)
        {
            // Cannot use NOT operator in a block of data, so it's always considered a match.
            // Therefore, when using OR operator there is no need to call the matchers.
            if (matcher->notOp() && isBlock)
                return true;
        }
    }

    for (const auto &matcher : matchers)
    {
        // The NOT operator can only be considered for row data without defined column
        const bool isNot = matcher->notOp();
        const bool ignore = (isNot && (isBlock || matcher->hasColumn()));
        bool matched = ignore ? true : matcher->quickRawMatch(fileType, isBlock, rawText);
        if (isNot && !ignore)
            matched = !matched;
        if (matched)
        {
            if ((++cnt == matchers.size()) || orOp)
                return true;
        }
    }

    return false;
}
