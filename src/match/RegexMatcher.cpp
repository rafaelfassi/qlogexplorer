// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "RegexMatcher.h"
#include <QMessageBox>

RegexMatcher::RegexMatcher(const tp::SearchParam &param) : BaseMatcher(param)
{
    if (param.pattern.empty())
        return;

    m_rx = RegexBuilder::build(param.pattern, getOpts());
    if (m_rx->hasError())
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("The regular expression is not valid:\n%1").arg(utl::toQStr(m_rx->getError())));
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

bool RegexMatcher::quickRawMatch(tp::FileType fileType, bool isBlock, std::string_view rawText)
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
    else if (m_rx)
        return m_rx->hasMatch(rawText);
    else
        return false;
}

void RegexMatcher::initRawMatch(tp::FileType fileType, bool isBlock)
{
    m_rawMatchnitiated = true;

    if (!m_rx || m_param.pattern.empty())
        return;

    std::string_view pattern(m_param.pattern);

    if (fileType == tp::FileType::Json)
    {
        // For raw json it's not safe to apply raw filter when the pattern has characters that json can escape.
        // E.g.: The char (") can be found in the raw json as (\") or (\u0022)
        // Trying to modify the regex pattern to handle escape cases would be fragile and only increases the
        // complexity.
        if (pattern.find_first_of("\"\n\t\r") != std::string_view::npos || utl::contains(pattern, "\\\\"))
        {
            m_canUseRawMatch = false;
            return;
        }
    }

    // When parsing raw text the anchors need to be removed, because it runs on full block(many rows) and full
    // row(text is not split into columns)
    const bool hasAnchor = ((pattern.front() == '^') || ((pattern.back() == '$') && !utl::endsWith(pattern, "\\$")));
    if (hasAnchor)
    {
        if (pattern.front() == '^')
            pattern.remove_prefix(1);
        if (pattern.back() == '$')
            pattern.remove_suffix(1);
        m_rawRx = RegexBuilder::build(pattern, getOpts());
        if (m_rawRx->hasError())
        {
            LOG_ERR("Invalid raw regex: {}", m_rawRx->getError());
            m_rawRx.reset();
            m_canUseRawMatch = false;
        }
    }
}
