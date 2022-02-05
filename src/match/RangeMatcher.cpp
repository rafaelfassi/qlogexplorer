// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "RangeMatcher.h"

RangeMatcher::RangeMatcher(const tp::SearchParam &param) : BaseMatcher(param)
{
    static const QRegularExpression rxRangeSppliter("^\\s*(.*?)\\s*->\\s*(.*?)\\s*$");
    if (!param.column.has_value())
    {
        LOG_ERR("No column specified for range matcher");
        return;
    }
    else if (param.pattern.empty())
    {
        LOG_ERR("Empty pattern was provided to range matcher");
        return;
    }

    auto m = rxRangeSppliter.match(param.pattern.c_str());
    if (m.hasMatch())
    {
        const auto from = m.captured(1);
        const auto to = m.captured(2);

        if (!from.isEmpty())
        {
            m_from = utl::toVariant(param.column.value(), from);
        }

        if (!to.isEmpty())
        {
            m_to = utl::toVariant(param.column.value(), to);
        }
    }
    else
    {
        m_from = utl::toVariant(param.column.value(), param.pattern.c_str());
    }
}

bool RangeMatcher::match(const std::string &text)
{
    if (text.empty())
        return false;

    const bool validFrom(m_from.isValid() && !m_from.isNull());
    const bool validTo(m_to.isValid() && !m_to.isNull());
    bool res(validFrom || validTo);

    if (!res)
        return false;

    const auto val(utl::toVariant(m_param.column.value(), text.c_str()));
    if (val.isValid() && !val.isNull())
    {
        if (validFrom)
            res &= (val >= m_from);

        if (validTo)
            res &= (val <= m_to);
    }

    return res;
}