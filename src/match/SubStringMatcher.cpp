// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "SubStringMatcher.h"

static constexpr std::array<char, 256> makeUpperArray()
{
    std::array<char, 256> table{};
    for (int i = 0; i < 256; ++i)
    {
        table[i] = static_cast<char>(i);
    }
    for (int i = 'a'; i <= 'z'; ++i)
    {
        table[i] = (static_cast<char>(i) & ~0x20);
    }
    return table;
}

static inline bool compareCharWithUpperChar(char c, char u)
{
    static constexpr auto utab = makeUpperArray();
    return (u == utab[static_cast<unsigned char>(c)]);
}

SubStringMatcher::SubStringMatcher(const tp::SearchParam &param)
    : BaseMatcher(param),
      m_textToSearch(matchCase() ? m_param.pattern : utl::toUpper(m_param.pattern))
{
}

bool SubStringMatcher::match(const std::string &text)
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
