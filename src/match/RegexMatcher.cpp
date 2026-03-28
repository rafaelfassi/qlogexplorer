// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "RegexMatcher.h"

RegexMatcher::RegexMatcher(const tp::SearchParam &param) : BaseMatcher(param), m_rx(param.pattern.c_str(), getOpts())
{
    if (param.pattern.empty())
        return;

    // When parsing raw text the anchors need to be removed
    if ((param.pattern.front() == '^') || (param.pattern.back() == '$'))
    {
        QString rawPattern(param.pattern.c_str());
        if (rawPattern.startsWith('^'))
            rawPattern.remove(0, 1);
        if (rawPattern.endsWith('$') && !rawPattern.endsWith("\\$"))
            rawPattern.chop(1);
        m_rawRx = QRegularExpression(rawPattern, getOpts());
    }
}

QRegularExpression::PatternOptions RegexMatcher::getOpts()
{
    QRegularExpression::PatternOptions opts = QRegularExpression::DontCaptureOption;
    if (!matchCase())
    {
        opts |= QRegularExpression::CaseInsensitiveOption;
    }
    return opts;
}

bool RegexMatcher::match(std::string_view text)
{
    return m_rx.match(QString::fromUtf8(text.data(), text.size())).hasMatch();
}

bool RegexMatcher::quickRawMatch(tp::FileType fileType, bool isBlock, std::string_view rawText)
{
    if (m_rawRx.has_value())
        return m_rawRx->match(QString::fromUtf8(rawText.data(), rawText.size())).hasMatch();
    else
        return match(rawText);
}
