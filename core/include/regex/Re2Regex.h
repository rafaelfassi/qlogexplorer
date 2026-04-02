// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "regex/Regex.h"
#include "re2/re2.h"

class Re2Regex : public Regex
{
public:
    Re2Regex(std::string_view text, RegexFlags opts);
    bool build(std::string_view text, RegexFlags opts) override;
    bool hasError() override;
    std::string getError() const override;
    bool hasMatch(std::string_view text) override;
    RegexResult::Uptr match(std::string_view text) override;
    int getCaptureCount() const override;
    std::vector<std::string> getNamedCaptureGroups() override;

private:
    std::shared_ptr<re2::RE2> m_rx;
};
