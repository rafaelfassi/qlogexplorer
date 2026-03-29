// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "Re2Regex.h"

class Re2RegexResult : public RegexResult
{
public:
    Re2RegexResult() {}

    Re2RegexResult(std::shared_ptr<re2::RE2> rx, std::string_view text) : m_rx(std::move(rx))
    {
        if (!m_rx)
            return;

        m_groups = m_rx->NumberOfCapturingGroups();
        m_submatches.resize(m_groups + 1); // +1 for the full match
        m_args.resize(m_groups);
        m_arg_ptrs.resize(m_groups);

        for (int i = 0; i < m_groups; ++i)
        {
            m_args[i] = &m_submatches[i + 1];
            m_arg_ptrs[i] = &m_args[i];
        }

        if (re2::RE2::PartialMatchN(text, *m_rx, m_arg_ptrs.data(), m_groups))
        {
            m_hasMatch = true;
        }
    }

    bool hasMatch() const override { return m_hasMatch; }

    std::string getCaptured(int groupNumb) const override
    {
        if (groupNumb < m_submatches.size())
        {
            return std::string(m_submatches[groupNumb]);
        }
        LOG_ERR("Invalid groupNumb {} size is {}", groupNumb, m_submatches.size());
        return {};
    }

    std::string getCaptured(std::string_view groupName) const override
    {
        const auto &names = m_rx->NamedCapturingGroups();
        if (const auto it = names.find(std::string(groupName)); it != names.end())
        {
            return getCaptured(it->second);
        }
        return {};
    }

    bool m_hasMatch = false;
    int m_groups = 0;
    std::shared_ptr<re2::RE2> m_rx;
    std::vector<re2::StringPiece> m_submatches; // +1 for the full match
    std::vector<re2::RE2::Arg> m_args;
    std::vector<re2::RE2::Arg *> m_arg_ptrs;
};

Re2Regex::Re2Regex(std::string_view pattern, RegexFlags opts)
{
    build(pattern, opts);
}

bool Re2Regex::build(std::string_view pattern, RegexFlags opts)
{
    re2::RE2::Options options;
    options.set_log_errors(false);
    options.set_case_sensitive(opts.has(RegexOption::CaseSensitive));
    options.set_never_capture(opts.has(RegexOption::DontCapture));

    m_rx = std::make_shared<re2::RE2>(pattern, options);
    if (!m_rx->ok() && (pattern.find("(?<") != std::string_view::npos))
    {
        // Versions of RE2 pre-dating 2023-09-01 release only support Python-style
        // syntax for named groups: "(?P<GroupName>)"
        LOG_WAR("Trying to fix named group to fix error: {}", m_rx->error());
        std::string fixedPattern(pattern);
        utl::replaceAll(fixedPattern, "(?<", "(?P<");
        m_rx = std::make_shared<re2::RE2>(fixedPattern, options);
    }
    return m_rx->ok();
}

bool Re2Regex::hasError()
{
    if (m_rx)
    {
        return !m_rx->ok();
    }
    return true;
}

std::string Re2Regex::getError() const
{
    if (m_rx)
    {
        return m_rx->error();
    }
    return "re2 not created";
}

bool Re2Regex::hasMatch(std::string_view text)
{
    if (m_rx)
    {
        return re2::RE2::PartialMatch(text, *m_rx);
    }

    LOG_ERR("hasMatch called without re2 instance");
    return false;
}

RegexResult::Uptr Re2Regex::match(std::string_view text)
{
    if (m_rx)
    {
        return std::make_unique<Re2RegexResult>(m_rx, text);
    }

    LOG_ERR("match called without re2 instance");
    return std::make_unique<Re2RegexResult>();
}

int Re2Regex::getCaptureCount() const
{
    if (m_rx)
    {
        return m_rx->NumberOfCapturingGroups();
    }
    return -1;
}

std::vector<std::string> Re2Regex::getNamedCaptureGroups()
{
    std::vector<std::string> res;
    if (!m_rx)
    {
        return res;
    }

    const auto &names = m_rx->CapturingGroupNames();
    res.reserve(names.size() + 1);
    res.emplace_back(); // full match group
    for (const auto &[idx, name] : names)
    {
        res.emplace_back(name);
    }
    return res;
}
