// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

class RegexResult
{
public:
    using Uptr = std::unique_ptr<RegexResult>;

    virtual bool hasMatch() const = 0;
    virtual std::string getCaptured(int groupNumb) const = 0;
    virtual std::string getCaptured(std::string_view groupName) const = 0;
};

enum class RegexOption
{
    CaseSensitive,
    DontCapture
};
using RegexFlags = tp::Flags<RegexOption>;

class Regex
{
public:
    using Uptr = std::unique_ptr<Regex>;

    virtual bool build(std::string_view text, RegexFlags opts) = 0;
    virtual bool hasError() = 0;
    virtual std::string getError() const = 0;
    virtual bool hasMatch(std::string_view text) = 0;
    virtual RegexResult::Uptr match(std::string_view text) = 0;
    virtual int getCaptureCount() const = 0;
    virtual std::vector<std::string> getNamedCaptureGroups() = 0;
};
