// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "match/SubStringMatcher.h"

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

bool SubStringMatcher::preMatchRawText(tp::FileType fileType, bool isBlock, std::string_view rawText)
{
    if (!m_rawMatcherInitiated)
    {
        initRawMatch(fileType, isBlock);
    }

    if (!m_canUseRawMatch)
    {
        return true;
    }

    if (m_rawRx)
        return m_rawRx->hasMatch(rawText);
    else
        return match(rawText);
}

void SubStringMatcher::initRawMatch(tp::FileType fileType, bool isBlock)
{
    m_rawMatcherInitiated = true;

    if (fileType == tp::FileType::Json && (m_param.pattern.find_first_of(" \"/\\") != std::string_view::npos))
    {
        // For raw json it's necessary to use regex to match the ways json can escape some characters.

        // Simplify the substring pattern to normalize and remove duplicated whitespaces
        std::string pattern(m_param.pattern);
        utl::simplify(pattern);

        std::string rawPattern;
        rawPattern.reserve(2 * pattern.size());

        for (auto ch : pattern)
        {
            switch (ch)
            {
                case ' ':
                case '\"':
                case '/':
                case '\\':
                    rawPattern += utl::getRxReplacementForRawJson(ch);
                    break;
                case '*':
                case '.':
                case '?':
                case '^':
                case '$':
                case '|':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                    rawPattern += "\\";
                    rawPattern += ch;
                    break;
                default:
                    rawPattern += ch;
                    break;
            }
        }

        if (rawPattern != m_param.pattern)
        {
            LOG_DBG("Raw pattern: '{}'", rawPattern);

            RegexFlags opts = RegexOption::DontCapture;
            if (matchCase())
            {
                opts.set(RegexOption::CaseSensitive);
            }

            m_rawRx = RegexBuilder::build(rawPattern, opts);
            if (m_rawRx->hasError())
            {
                LOG_ERR("Invalid raw regex pattern for raw substring: '{}', - {}", rawPattern, m_rawRx->getError());
                m_rawRx.reset();
                return;
            }
        }
    }

    m_canUseRawMatch = true;
}
