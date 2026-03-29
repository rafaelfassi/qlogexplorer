// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SubStringMatcher.h"

static inline bool compareCharWithUpperChar(char c, char u)
{
    return (u == utl::utab[static_cast<unsigned char>(c)]);
}

SubStringMatcher::SubStringMatcher(const tp::SearchParam &param)
    : BaseMatcher(param),
      m_textToSearch(matchCase() ? m_param.pattern : utl::toUpper(m_param.pattern))
{
}

bool SubStringMatcher::match(std::string_view text)
{
    if (matchCase())
    {
        return (text.find(m_textToSearch) != std::string::npos);
    }
    else
    {
        const auto it = std::search(
            text.begin(),
            text.end(),
            m_textToSearch.begin(),
            m_textToSearch.end(),
            compareCharWithUpperChar);
        return it != text.end();
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
