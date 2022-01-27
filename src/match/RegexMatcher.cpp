#include "pch.h"
#include "RegexMatcher.h"

RegexMatcher::RegexMatcher(const tp::SearchParam &param) : BaseMatcher(param), m_rx(param.pattern.c_str(), getOpts())
{
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

bool RegexMatcher::match(const std::string &text)
{
    return m_rx.match(text.c_str()).hasMatch();
}
