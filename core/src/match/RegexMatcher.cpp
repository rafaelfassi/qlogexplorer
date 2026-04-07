// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "match/RegexMatcher.h"

RegexMatcher::RegexMatcher(const tp::SearchParam &param) : BaseMatcher(param)
{
    if (param.pattern.empty())
        return;

    m_rx = RegexBuilder::build(param.pattern, getOpts());
    if (m_rx->hasError())
    {
        Notifier::notifyError(QObject::tr("The regular expression is not valid:\n%1").arg(utl::toQStr(m_rx->getError())));
        m_rx.reset();
    }
}

RegexFlags RegexMatcher::getOpts()
{
    RegexFlags opts = RegexOption::DontCapture;
    if (matchCase())
    {
        opts.set(RegexOption::CaseSensitive);
    }
    return opts;
}

bool RegexMatcher::match(std::string_view text)
{
    if (m_rx)
        return m_rx->hasMatch(text);
    else
        return false;
}

bool RegexMatcher::preMatchRawText(tp::FileType fileType, bool isBlock, std::string_view rawText)
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
    else if (m_rx)
        return m_rx->hasMatch(rawText);
    else
        return false;
}

void RegexMatcher::initRawMatch(tp::FileType fileType, bool isBlock)
{
    m_rawMatcherInitiated = true;

    if (!m_rx || m_param.pattern.empty())
        return;

    std::string rawPattern(m_param.pattern);

    if (fileType == tp::FileType::Json)
    {
        // For raw json it's required to apply some tweaks in the regex pattern to match the ways json can
        // escape some characters.

        rawPattern = utl::reduceRxPatternForReplacement(rawPattern);
        LOG_DBG("Simplified pattern: '{}'", rawPattern);

        utl::replaceStrIf(
            rawPattern,
            "\\\\",
            utl::getRxReplacementForRawJson('\\'),
            [](std::string_view s, std::size_t p) -> bool { return (p == 0 || s[p - 1] != '\\'); }
        );

        utl::replaceStrIf(
            rawPattern,
            "/",
            utl::getRxReplacementForRawJson('/'),
            [](std::string_view s, std::size_t p) -> bool { return (p == 0 || s[p - 1] != '\\'); }
        );

        utl::replaceStrIf(
            rawPattern,
            "\"",
            utl::getRxReplacementForRawJson('\"'),
            [](std::string_view s, std::size_t p) -> bool { return (p == 0 || s[p - 1] != '\\'); }
        );

        utl::replaceStrIf(
            rawPattern,
            " ",
            utl::getRxReplacementForRawJson(' '),
            [](std::string_view s, std::size_t p) -> bool { return (p == 0 || s[p - 1] != '\\'); }
        );
    }

    // When parsing raw text the anchors need to be removed, because it runs on full block(many rows) and full
    // row(text is not split into columns)

    if (!utl::removeRegexAnchors(rawPattern))
    {
        LOG_INF("Could not remove regex anchors from: {}", rawPattern);
        return;
    }

    if (rawPattern != m_param.pattern)
    {
        LOG_DBG("Raw pattern: '{}'", rawPattern);
        m_rawRx = RegexBuilder::build(rawPattern, getOpts());
        if (m_rawRx->hasError())
        {
            LOG_ERR("Invalid raw regex: '{}' - {}", rawPattern, m_rawRx->getError());
            m_rawRx.reset();
            return;
        }
    }

    m_canUseRawMatch = true;
}
