// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "SubStringMatcher.h"

SubStringMatcher::SubStringMatcher(const tp::SearchParam &param)
    : BaseMatcher(param),
      m_textToSearch(matchCase() ? m_param.pattern : utl::toUpper(m_param.pattern))
{
}

bool SubStringMatcher::match(const std::string &text)
{
    if (matchCase())
        return (text.find(m_textToSearch) != std::string::npos);
    else
        return (utl::toUpper(text).find(m_textToSearch) != std::string::npos);
}