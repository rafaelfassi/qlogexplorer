// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma once

#include "BaseMatcher.h"

class RangeMatcher : public BaseMatcher
{
public:
    RangeMatcher(const tp::SearchParam &param);
    bool match(std::string_view text) override;
    bool preMatchRawText(tp::FileType fileType, bool isBlock, std::string_view rawText) override;

private:
    QVariant m_from;
    QVariant m_to;
};
