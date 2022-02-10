// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "BaseMatcher.h"

class SubStringMatcher : public BaseMatcher
{
public:
    SubStringMatcher(const tp::SearchParam &param);
    bool match(const std::string &text) override;

private:
    const std::string m_textToSearch;
};
