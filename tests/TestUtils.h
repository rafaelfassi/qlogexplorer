// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#pragma onve

#include "gtest/gtest.h"
#include "InFileStream.h"

namespace tutl
{

InFileStream::Ptr openTestFile(const std::string &fileName);
void copyResourcesToCurrentDir();

}
