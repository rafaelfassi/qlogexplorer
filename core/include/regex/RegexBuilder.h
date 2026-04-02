// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "Regex.h"

class RegexBuilder
{
public:
    static Regex::Uptr build(std::string_view pattern, RegexFlags opts = {});
};
