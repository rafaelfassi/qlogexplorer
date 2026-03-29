// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "BaseMatcher.h"

class SubStringMatcher : public BaseMatcher
{
public:
    SubStringMatcher(const tp::SearchParam &param);
    bool match(std::string_view text) override;
    bool quickRawMatch(tp::FileType fileType, bool isBlock, std::string_view rawText) override;

private:
    void initRawMatch(tp::FileType fileType, bool isBlock);

    const std::string m_textToSearch;
    bool m_rawMatchnitiated = false;
    bool m_canUseRawMatch = true;
};
