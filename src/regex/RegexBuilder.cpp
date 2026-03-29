// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "RegexBuilder.h"
#include "QtRegex.h"
#include "Re2Regex.h"

Regex::Uptr RegexBuilder::build(std::string_view pattern, RegexFlags opts)
{
    if (auto rx = std::make_unique<Re2Regex>(pattern, opts); !rx->hasError())
    {
        return rx;
    }
    else
    {
        LOG_WAR("Re2Regex error {} - falling back to QtRegex", rx->getError());
        return std::make_unique<QtRegex>(pattern, opts);
    }
}
