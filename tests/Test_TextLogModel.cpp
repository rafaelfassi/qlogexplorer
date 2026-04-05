// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "TestUtils.h"
#include "model/TextLogModel.h"

// The purpose of this test is to check if all data in a text log file
// is properly loaded without any missing entry or corrupted data.
// Can take a tittle longer because it must create data that occupies
// many chunks.
// This test is also used to check how the parsed data relates to the
// raw data in the text file.
TEST(Test_TextLogModel, DataReliability_001)
{
    const std::size_t rowCount = g_chunkSize / 4;
    const std::string logFileName = "Test_TextLogModel-DataReliability_001.log";
    const std::string templateFileName = "Test_TextLogModel-DataReliability_001_tmpl.json";
    std::size_t logEntriesCount = 0;

    std::vector<std::string> lvls{"FATAL", "ERROR", "WARNING", "INFO", "DEBUG"};
    std::vector<std::pair<std::string, std::vector<std::string>>> msgs{
        {"The new law will take effect next week.", {"The new law will take effect next week."}},
        {"The medicine had an immediate effect on my headache. ",
         {"The medicine had an immediate effect on my headache. "}},
        {"He was too late\nconsequently, he missed the meeting.",
         {"He was too late", "consequently, he missed the meeting."}},
        {"\"quotes\" with many spaces .", {"\"quotes\" with many spaces ."}}};

    std::ofstream outFile(logFileName, std::ios::trunc);
    ASSERT_TRUE(outFile.is_open());
    for (std::size_t i = 0; i < rowCount; ++i)
    {
        const size_t li = (i % lvls.size());
        const size_t mi = (i % msgs.size());
        logEntriesCount += msgs[mi].second.size();
        outFile << FORMAT(R"_([{}] ({}): {})_", lvls[li], i, msgs[mi].first) << std::endl;
    }
    outFile.close();

    auto fileConf = FileConf::make(templateFileName);
    fileConf->setFileName(logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);
    ASSERT_EQ(model.rowCount(), logEntriesCount);
    // Ensures the log file occupies at least 3 chunks
    EXPECT_GE(model.chunksCount(), 3);

    tp::RowData rowData;
    std::size_t ri = 0; // index for the main log entries
    // i = the index for all rows in the model (including the NoMaching ones)
    for (std::size_t i = 0; i < logEntriesCount; ++i, ++ri)
    {
        const size_t li = (ri % lvls.size());
        const size_t mi = (ri % msgs.size());

        ASSERT_EQ(model.getRow(i, rowData), i);
        ASSERT_EQ(rowData.size(), 3);

        ASSERT_EQ(rowData.at(0), lvls[li]);
        ASSERT_EQ(std::stoi(rowData.at(1)), ri);

        std::size_t msgLine = 0;
        for (const auto &msg : msgs[mi].second)
        {
            if (++msgLine > 1)
            {
                ++i;
                ASSERT_EQ(model.getRow(i, rowData), i);
                ASSERT_EQ(rowData.size(), 3);
                ASSERT_EQ(rowData.at(0), "");
                ASSERT_EQ(rowData.at(1), "");
            }
            ASSERT_EQ(rowData.at(2), msg);
        }
    }
}

// The purpose of this test is to ensure that a row bigger than the default
// chunk size is properly handled.
TEST(Test_TextLogModel, BigRow_001)
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
        outFile << rowText << std::endl;
    }
    outFile.close();

    auto fileConf = FileConf::make(tp::FileType::Text);
    fileConf->setFileName(logFileName);
    TextLogModel model(fileConf);

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

TEST(Test_TextLogModel, SearchSubString_001)
{
    auto fileConf = FileConf::make("text_0001_tmpl.json");
    fileConf->setFileName("text_0001.log");
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = R"_(New symbol 'ZDSP')_";
    param.type = tp::SearchType::SubString;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), R"_(New symbol 'ZDSP' found in upstream)_");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}

TEST(Test_TextLogModel, SearchRegex_001)
{
    auto fileConf = FileConf::make("text_0001_tmpl.json");
    fileConf->setFileName("text_0001.log");
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = R"_(symbol\s+'\w{4}')_";
    param.type = tp::SearchType::Regex;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), R"_(The symbol 'XNUW' was not found in upstream)_");

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), R"_(New symbol 'ZDSP' found in upstream)_");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}
