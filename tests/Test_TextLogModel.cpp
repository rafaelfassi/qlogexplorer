// Copyright (C) 2026 Rafael Fassi Lobao
// This file is part of qlogexplorer project licensed under GPL-3.0

#include "TestUtils.h"
#include "model/TextLogModel.h"

namespace biglog
{

static const std::string logFileName{"TextLogMode_BigLog.log"};
static const std::string templateFileName{"TextLogMode_BigLog_tmpl.json"};
static const std::size_t logEntries = g_chunkSize / 2;
static const std::vector<std::string> lvls{"FATAL", "ERROR", "WARNING", "INFO", "DEBUG"};
static const std::vector<std::pair<std::string, std::vector<std::string>>> msgs{
    {"The new law will take effect next week.", {"The new law will take effect next week."}},
    {"The medicine had an immediate effect on my headache. ",
     {"The medicine had an immediate effect on my headache. "}},
    {"He was too late\nconsequently, he missed the meeting.",
     {"He was too late", "consequently, he missed the meeting."}},
    {"\"quotes\" with many spaces .", {"\"quotes\" with many spaces ."}}};

// Creates log data for testing that occupies many chunks.
// Retuns:
//  - On success: The expected number of rows that also includes the no-matching ones.
//  - On error: std::string::npos.
static std::size_t createLog()
{
    static std::size_t rowCount = 0;

    if (rowCount > 0)
    {
        return rowCount;
    }

    std::ofstream outFile(biglog::logFileName, std::ios::trunc);
    if (!outFile.is_open())
    {
        return std::string::npos;
    }

    for (std::size_t i = 0; i < biglog::logEntries; ++i)
    {
        const size_t li = (i % biglog::lvls.size());
        const size_t mi = (i % biglog::msgs.size());
        rowCount += biglog::msgs[mi].second.size();
        outFile << FORMAT(R"_([{}] ({}): {})_", biglog::lvls[li], i, biglog::msgs[mi].first) << std::endl;
    }
    outFile.close();
    return rowCount;
}

} // namespace biglog

// The purpose of this test is to check if all data in a text log file
// is properly loaded without any missing entry or corrupted data.
// This test is also used to check how the parsed data relates to the
// raw data in the text file.
TEST(Test_TextLogModel, DataReliability_001)
{
    const std::size_t rowCount = biglog::createLog();
    ASSERT_NE(rowCount, std::string::npos);

    auto fileConf = FileConf::make(biglog::templateFileName);
    fileConf->setFileName(biglog::logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);
    ASSERT_EQ(model.rowCount(), rowCount);
    // Ensures the log file occupies at least 3 chunks
    EXPECT_GE(model.chunksCount(), 3);

    tp::RowData rowData;
    std::size_t lei = 0; // index for the main log entries
    // ri = the index the rows in the model (including the NoMaching ones)
    for (std::size_t ri = 0; ri < rowCount; ++ri, ++lei)
    {
        const size_t lvi = (lei % biglog::lvls.size());
        const size_t msi = (lei % biglog::msgs.size());

        ASSERT_EQ(model.getRow(ri, rowData), ri);
        ASSERT_EQ(rowData.size(), 3);

        ASSERT_EQ(rowData.at(0), biglog::lvls[lvi]);
        ASSERT_EQ(std::stoi(rowData.at(1)), lei);

        std::size_t msgLine = 0;
        for (const auto &msg : biglog::msgs[msi].second)
        {
            if (++msgLine > 1)
            {
                ++ri;
                ASSERT_EQ(model.getRow(ri, rowData), ri);
                ASSERT_EQ(rowData.size(), 3);
                ASSERT_EQ(rowData.at(0), "");
                ASSERT_EQ(rowData.at(1), "");
            }
            ASSERT_EQ(rowData.at(2), msg);
        }
    }
}

// The purpose of this test is to ensure that indexing and searching can run in parallel.
TEST(Test_TextLogModel, IndexingAndSearching_001)
{
    const std::size_t rowCount = biglog::createLog();
    ASSERT_NE(rowCount, std::string::npos);

    auto fileConf = FileConf::make(biglog::templateFileName);
    fileConf->setFileName(biglog::logFileName);
    TextLogModel model(fileConf);
    // Make chunks available for searching on each 5 chunks parsed
    model.setChunksPerParse(5);

    tp::SearchParam param;
    param.pattern = "FATAL";
    param.type = tp::SearchType::SubString;
    param.flags.set(tp::SearchFlag::MatchCase);

    tp::SearchParams params = {param};
    tutl::LogModelSearchResults searchRes(model);
    model.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    model.startSearch(params, false);

    int modelSecs = 0;
    while (model.rowCount() < rowCount)
    {
        if (++modelSecs > 30)
        {
            // Show the current number of rows in the error message
            ASSERT_EQ(model.rowCount(), rowCount);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    EXPECT_EQ(model.rowCount(), rowCount);
    // There must be some searching results
    EXPECT_GT(searchRes.count(), 1);

    model.stopSearch();
    model.stop();
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

TEST(Test_TextLogModel, InvalidSearchRegex_001)
{
    auto fileConf = FileConf::make("text_0001_tmpl.json");
    fileConf->setFileName("text_0001.log");
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = R"_(?<))_";
    param.type = tp::SearchType::Regex;
    tp::SearchParams params = {param};

    tutl::NotificationHandler notif;

    tp::RowData rowData;
    model.searchFromCurrentThread(params, false);

    const auto notifications = notif.getNotifications();
    ASSERT_EQ(notifications.size(), 1);
    EXPECT_EQ(notifications.front().first, tp::NotifLevel::Error);
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
