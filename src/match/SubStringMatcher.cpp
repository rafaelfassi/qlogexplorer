// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SubStringMatcher.h"

SubStringMatcher::SubStringMatcher(const tp::SearchParam &param) : BaseMatcher(param)
{
}

bool SubStringMatcher::match(std::string_view text)
{
    if (matchCase())
    {
        return utl::contains(text, m_param.pattern);
    }
    else
    {
        return utl::containsICase(text, m_param.pattern);
    }
}

bool SubStringMatcher::quickRawMatch(tp::FileType fileType, bool isBlock, std::string_view rawText)
{
    if (!m_rawMatchnitiated)
    {
        initRawMatch(fileType, isBlock);
    }

    if (!m_canUseRawMatch)
    {
        return true;
    }

    return match(rawText);
}

void SubStringMatcher::initRawMatch(tp::FileType fileType, bool isBlock)
{
    m_rawMatchnitiated = true;
    if (fileType == tp::FileType::Json)
    {
        // See reason is explained in initRawMatch() of RegexMatcher
        if (m_param.pattern.find_first_of("\"\n\t\r\\") != std::string_view::npos)
        {
            m_canUseRawMatch = false;
            return;
        }
    }
}
