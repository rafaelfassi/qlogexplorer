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

    if (m_rawRx)
        return m_rawRx->hasMatch(rawText);
    else
        return match(rawText);
}

void SubStringMatcher::initRawMatch(tp::FileType fileType, bool isBlock)
{
    m_rawMatchnitiated = true;

    if (fileType == tp::FileType::Json && (m_param.pattern.find_first_of(" \"/\\") != std::string_view::npos))
    {
        // For raw json it's necessary to use regex to match the ways json can escape some characters.

        std::string rawPattern;
        rawPattern.reserve(2 * m_param.pattern.size());

        for (auto ch : m_param.pattern)
        {
            switch (ch)
            {
                case ' ':
                    rawPattern += utl::getRxReplacementForRawJson(ch);
                    rawPattern += '+';
                    break;
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
            // Must be case insensitive due to the hex for the unicode.
            m_rawRx = RegexBuilder::build(rawPattern, RegexOption::DontCapture);
            if (m_rawRx->hasError())
            {
                LOG_ERR("Invalid raw regex for raw substring: {}", m_rawRx->getError());
                m_rawRx.reset();
                m_canUseRawMatch = false;
            }
        }
    }
}
