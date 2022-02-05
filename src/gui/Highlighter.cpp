// Copyright (C) 2022 Rafael Fassi Lobao
// This file is part of qlogviewer project licensed under GPL-3.0

#include "pch.h"
#include "Highlighter.h"
#include "Matcher.h"

Highlighter::Highlighter(const tp::HighlighterParam& param) : m_param(param)
{
    m_matcher.setParam(m_param.searchParam);
}

bool Highlighter::matchInRow(const tp::RowData &rowData) const
{
    return m_matcher.matchInRow(rowData);
}