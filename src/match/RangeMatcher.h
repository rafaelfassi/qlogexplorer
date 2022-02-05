// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#pragma once

#include "BaseMatcher.h"

class RangeMatcher : public BaseMatcher
{
public:
    RangeMatcher(const tp::SearchParam &param);
    bool match(const std::string &text) override;

private:
    QVariant m_from;
    QVariant m_to;
};
