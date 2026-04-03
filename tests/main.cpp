// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "TestUtils.h"

class GlobalTestingEnv : public ::testing::Environment
{
public:
    void SetUp() override
    {
        tutl::copyResourcesToCurrentDir();
    }
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // GoogleTest takes ownership of the pointer
    ::testing::AddGlobalTestEnvironment(new GlobalTestingEnv);
    return RUN_ALL_TESTS();
}
