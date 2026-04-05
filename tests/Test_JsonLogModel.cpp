// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "TestUtils.h"
#include "model/JsonLogModel.h"

// The purpose of this test is to check if all data in a json log file
// is properly loaded without any missing entry or corrupted data.
// Can take a tittle longer because it must create data that occupies
// many chunks.
// This test is also used to check how the parsed data relates to the
// raw data in the json file.
TEST(Test_JsonLogModel, DataReliability_001)
{
    const std::size_t rowCount = g_chunkSize / 4;
    const std::string logFileName = "Test_JsonLogModel-DataReliability_001.json";

    std::vector<std::string> lvls{"FATAL", "ERROR", "WARNING", "INFO", "DEBUG"};
    std::vector<std::pair<std::string, std::string>> msgs{
        std::make_pair(R"_(Some\tlog\u0009with\ttabs)_", R"_(Some log with tabs)_"),
        std::make_pair(R"_(This\nlog\u000Ahas\r\nnewline\n)_", R"_(This log has newline )_"),
        std::make_pair(R"_(\ntest backslash\u005Creturn\r.)_", R"_( test backslash\return .)_"),
        std::make_pair(R"_(\"quotes\" with many spaces \t\r\n.)_", R"_("quotes" with many spaces .)_")};

    std::ofstream outFile(logFileName, std::ios::trunc);
    ASSERT_TRUE(outFile.is_open());
    for (std::size_t i = 0; i < rowCount; ++i)
    {
        const size_t li = (i % lvls.size());
        const size_t mi = (i % msgs.size());
        outFile << FORMAT(R"_({{"Level":"{}","Number": {},"Message":"{}"}})_", lvls[li], i, msgs[mi].first)
                << std::endl;
    }
    outFile.close();

    auto fileConf = FileConf::make(tp::FileType::Json);
    fileConf->setFileName(logFileName);
    JsonLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);
    ASSERT_EQ(model.rowCount(), rowCount);
    // Ensures the log file occupies at least 3 chunks
    EXPECT_GE(model.chunksCount(), 3);

    tp::RowData rowData;
    for (std::size_t i = 0; i < rowCount; ++i)
    {
        const size_t li = (i % lvls.size());
        const size_t mi = (i % msgs.size());

        ASSERT_EQ(model.getRow(i, rowData), i);
        ASSERT_EQ(rowData.size(), 3);
        ASSERT_EQ(rowData.at(0), lvls[li]);
        ASSERT_EQ(std::stoi(rowData.at(1)), i);
        ASSERT_EQ(rowData.at(2), msgs[mi].second);
    }
}

// The purpose of this test is to ensure that a row bigger than the default
// chunk size is properly handled.
TEST(Test_JsonLogModel, BigRow_001)
{
    const std::size_t rowCount = 3;
    const std::size_t rowsize = 3 * g_chunkSize;
    const std::string logFileName = "Test_TextLogModel-BigRow_001.log";

    const std::string rowSeed = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789.";
    std::string rowText;
    rowText.reserve(rowsize + rowSeed.size());
    while (rowText.size() < rowsize)
    {
        rowText.append(rowSeed);
    }

    std::ofstream outFile(logFileName, std::ios::trunc);
    ASSERT_TRUE(outFile.is_open());
    for (std::size_t i = 0; i < rowCount; ++i)
    {
        outFile << FORMAT(R"_({{"Message":"{}"}})_", rowText) << std::endl;
    }
    outFile.close();

    auto fileConf = FileConf::make(tp::FileType::Json);
    fileConf->setFileName(logFileName);
    JsonLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);
    ASSERT_EQ(model.rowCount(), rowCount);

    tp::RowData rowData;
    for (std::size_t r = 0; r < rowCount; ++r)
    {
        ASSERT_EQ(model.getRow(r, rowData), r);
        ASSERT_EQ(rowData.size(), 1);
        ASSERT_EQ(rowData.at(0), rowText);
    }
}

TEST(Test_JsonLogModel, SearchSubString_001)
{
    auto fileConf = FileConf::make(tp::FileType::Json);
    fileConf->setFileName("json_0001.json");
    JsonLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = R"_(Has backslash \ return double whitespace and slash/finished)_";
    param.type = tp::SearchType::SubString;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has backslash \ return double whitespace and slash/finished)_");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}

TEST(Test_JsonLogModel, SearchRegex_001)
{
    auto fileConf = FileConf::make(tp::FileType::Json);
    fileConf->setFileName("json_0001.json");
    JsonLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = R"_(Has backslash \\ return double whitespace and slash/finished)_";
    param.type = tp::SearchType::Regex;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has backslash \ return double whitespace and slash/finished)_");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}
