#include "pch.h"
#include "ParamsMatcher.h"
#include "RegexMatcher.h"
#include "SubStringMatcher.h"

ParamsMatcher::ParamsMatcher(const tp::SearchParamLst &params, bool orOp)
    : m_matchers(makeMatchers(params)),
      m_orOp(orOp)
{
}

void ParamsMatcher::setParams(const tp::SearchParamLst &params, bool orOp)
{
    m_matchers = makeMatchers(params);
    m_orOp = orOp;
}

bool ParamsMatcher::match(const std::string &text)
{
    return match(m_matchers, m_orOp, text);
}

bool ParamsMatcher::matchInRow(const tp::RowData &rowData)
{
    return matchInRow(m_matchers, m_orOp, rowData);
}

Matchers ParamsMatcher::makeMatchers(const tp::SearchParamLst &params)
{
    Matchers matchers;
    matchers.reserve(params.size());

    for (const auto &param : params)
    {
        if (param.type == tp::SearchType::Regex)
        {
            matchers.emplace_back(std::make_unique<RegexMatcher>(param));
        }
        else
        {
            matchers.emplace_back(std::make_unique<SubStringMatcher>(param));
        }
    }
    return matchers;
}

bool ParamsMatcher::match(const Matchers &matchers, bool orOp, const std::string &text)
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

bool ParamsMatcher::matchInRow(const Matchers &matchers, bool orOp, const tp::RowData &rowData)
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
