// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "regex/QtRegex.h"

class QtRegexResult : public RegexResult
{
public:
    QtRegexResult(QRegularExpressionMatch &&macher) : m_macher(std::move(macher)) {}

    bool hasMatch() const override { return m_macher.hasMatch(); }

    std::string getCaptured(int groupNumb) const override { return m_macher.captured(groupNumb).toStdString(); }

    std::string getCaptured(std::string_view groupName) const override
    {
        return m_macher.captured(utl::toQStr(groupName)).toStdString();
    }

    QRegularExpressionMatch m_macher;
};

QtRegex::QtRegex(std::string_view text, RegexFlags opts)
{
    build(text, opts);
}

bool QtRegex::build(std::string_view text, RegexFlags opts)
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!opts.has(RegexOption::CaseSensitive))
    {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    if (opts.has(RegexOption::DontCapture))
    {
        options |= QRegularExpression::DontCaptureOption;
    }

    m_rx.setPatternOptions(options);
    m_rx.setPattern(utl::toQStr(text));
    if (m_rx.isValid())
    {
        m_rx.optimize();
        return true;
    }
    return false;
}

bool QtRegex::hasError()
{
    return !m_rx.isValid();
}

std::string QtRegex::getError() const
{
    return m_rx.errorString().toStdString();
}

bool QtRegex::hasMatch(std::string_view text)
{
    return m_rx.match(utl::toQStr(text)).hasMatch();
}

RegexResult::Uptr QtRegex::match(std::string_view text)
{
    return std::make_unique<QtRegexResult>(m_rx.match(utl::toQStr(text)));
}

int QtRegex::getCaptureCount() const
{
    return m_rx.captureCount();
}

std::vector<std::string> QtRegex::getNamedCaptureGroups()
{
    std::vector<std::string> res;
    const auto groups = m_rx.namedCaptureGroups();
    res.reserve(groups.size());
    for (const auto &group : groups)
    {
        res.push_back(group.toStdString());
    }
    return res;
}
