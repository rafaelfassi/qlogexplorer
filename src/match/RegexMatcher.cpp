// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "pch.h"
#include "RegexMatcher.h"
#include <QMessageBox>

RegexMatcher::RegexMatcher(const tp::SearchParam &param) : BaseMatcher(param)
{
    if (param.pattern.empty())
        return;

    const auto &pattern = param.pattern;
    const auto opts = getOpts();
    m_rx = RegexBuilder::build(pattern, opts);
    if (m_rx->hasError())
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("The regular expression is not valid:\n%1").arg(utl::toQStr(m_rx->getError())));
        m_rx.reset();
        return;
    }

    // When parsing raw text the anchors need to be removed, because it runs on full block(many rows) and full
    // row(text is not split into columns)
    const bool hasAnchor = ((pattern.front() == '^') || ((pattern.back() == '$') && !utl::endsWith(pattern, "\\$")));
    if (hasAnchor)
    {
        std::string_view rawPattern(pattern);
        if (rawPattern.front() == '^')
            rawPattern.remove_prefix(1);
        if (rawPattern.back() == '$')
            rawPattern.remove_suffix(1);
        m_rawRx = RegexBuilder::build(rawPattern, opts);
        if (m_rawRx->hasError())
        {
            LOG_ERR("Invalid raw regex: {}", m_rawRx->getError());
            m_rawRx.reset();
        }
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
    if (m_rawRx)
        return m_rawRx->hasMatch(rawText);
    else if (m_rx)
        return m_rx->hasMatch(rawText);
    else
        return false;
}
