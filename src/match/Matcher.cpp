// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "Matcher.h"
#include "RegexMatcher.h"
#include "SubStringMatcher.h"
#include "RangeMatcher.h"

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

bool Matcher::match(const std::string &text) const
{
    return match(m_matchers, m_orOp, text);
}

bool Matcher::matchInRow(const tp::RowData &rowData) const
{
    return matchInRow(m_matchers, m_orOp, rowData);
}

void Matcher::makeMatcher(const tp::SearchParam &param, Matchers &matchers)
{
    switch (param.type)
    {
    case tp::SearchType::Regex:
        matchers.emplace_back(std::make_unique<RegexMatcher>(param));
        break;
    case tp::SearchType::SubString:
        matchers.emplace_back(std::make_unique<SubStringMatcher>(param));
        break;
    case tp::SearchType::Range:
        matchers.emplace_back(std::make_unique<RangeMatcher>(param));
        break;
    default:
        LOG_ERR("invalid SearchType {}", tp::toInt(param.type));
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

bool Matcher::match(const Matchers &matchers, bool orOp, const std::string &text)
{
    std::uint32_t cnt(0);

    for (const auto &matcher : matchers)
    {
        bool matched(false);
        if (matcher->match(text))
        {
            matched = true;
            break;
        }
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
