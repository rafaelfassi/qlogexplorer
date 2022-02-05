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
