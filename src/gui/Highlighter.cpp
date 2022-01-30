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