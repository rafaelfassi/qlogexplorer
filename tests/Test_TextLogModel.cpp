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
        {"The medicine had an immediate effect on my headache. ", {"The medicine had an immediate effect on my headache. "}},
        {"He was too late\nconsequently, he missed the meeting.", {"He was too late", "consequently, he missed the meeting."}},
        {"\"quotes\" with many spaces .", {"\"quotes\" with many spaces ."}},
        {" Error 'inserting'  record:\n\t\tWhat:\tconstraint violation\r\n\tINSERT INTO tbl(a,b) VALUES(1,?)",
         {"Error 'inserting'  record:", "\t\tWhat:\tconstraint violation", "\tINSERT INTO tbl(a,b) VALUES(1,?)"}}
    };

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

        std::ofstream outFile(logFileName, std::ios::trunc);
        if (!outFile.is_open())
        {
            return std::string::npos;
        }

        for (std::size_t i = 0; i < logEntries; ++i)
        {
            const size_t li = (i % lvls.size());
            const size_t mi = (i % msgs.size());
            rowCount += msgs[mi].second.size();
            outFile << FORMAT(R"_([{}] ({}): {})_", lvls[li], i, msgs[mi].first) << std::endl;
        }
        outFile.close();
        return rowCount;
    }
} // namespace biglog

namespace log01
{
    static const std::string logFileName{"TextLogMode_Log01.log"};
    static const std::string templateFileName{"TextLogMode_Log01_tmpl.json"};
    static std::string_view logContent{
        "INFO    2022-02-18 15:37:10.354 0xBF32 System starting...\n"
        "WARNING 2022-02-19 15:37:13.427 0xBF32 Not in UTC timezone\n"
        "INFO    2022-02-18 15:37:12.137 0xBF32 System initialized\n"
        "INFO    2022-02-19 15:14:02.003 0xAC05 Starting 'UpstreamUpdater' service error\n"
        "INFO    2022-02-19 15:14:10.351 0xF1D2 New user signup: 'john_constantine'\n"
        "ERROR   2022-02-19 15:14:10.437 0xF1D2 DB Error: unique constraint violation at username:\n"
        "\tINSERT INTO user (name, username, age)\r\n"
        "\t\tVALUES ('John Constantine', 'john_constantine', 99)\n"
        "WARNING 2022-02-19 15:14:22.519 0xAC05 The symbol 'XNUW' was not found in upstream\n"
        "INFO    2022-02-20 15:14:23.627 0xAC3F Starting payment for order '7315642'\n"
        "INFO    2022-02-19 15:14:23.712 0xAC05 New symbol 'ZDSP' found in upstream\n"
        "ERROR   2022-02-20 15:17:15.753 0xAC3F Payment gateway has returned error 3456\n"
        "WARNING 2022-02-19 15:17:16.524 0xAC3F Order '7315642' failed to complete payment\n"
        "INFO    2022-02-19 15:18:16.524 0xFFD4 System shutdown requested (force: true)\n"
        "WARNING 2022-02-19 15:18:26.643 0xFFD4 Could not stop \"UpstreamUpdater service\". killing it.\n"
        "ERROR   2022-02-19 15:18:26.644 0xAC05 An unexpected error has occurred.\tWhat: Network socket ptr is null.\n"
        "INFO    2022-02-18 15:19:02.472 0x3F22 System starting...\n"
    };

    static bool createLog()
    {
        static bool created = false;
        if (created)
        {
            return true;
        }

        std::ofstream outFile(logFileName, std::ios::trunc);
        if (!outFile.is_open())
        {
            return false;
        }
        outFile << logContent;
        outFile.close();
        return true;
    }
} // namespace log01

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
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
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

TEST(Test_TextLogModel, SearchSubString_002)
{
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = "has occurred.\tWhat:";
    param.type = tp::SearchType::SubString;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), "An unexpected error has occurred.\tWhat: Network socket ptr is null.");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}

TEST(Test_TextLogModel, SearchSubString_003)
{
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = "VALUES ('John Constantine', 'john_constantine', 99)";
    param.type = tp::SearchType::SubString;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), "\t\tVALUES ('John Constantine', 'john_constantine', 99)");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}

TEST(Test_TextLogModel, InvalidSearchRegex_001)
{
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
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
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
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

TEST(Test_TextLogModel, SearchRegex_002)
{
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = "has occurred.\tWhat:";
    param.type = tp::SearchType::Regex;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), "An unexpected error has occurred.\tWhat: Network socket ptr is null.");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}

TEST(Test_TextLogModel, SearchRegex_003)
{
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = "VALUES \\('John Constantine', 'john_constantine', 99\\)";
    param.type = tp::SearchType::Regex;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), "\t\tVALUES ('John Constantine', 'john_constantine', 99)");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}

TEST(Test_TextLogModel, SearchRegex_004)
{
    ASSERT_TRUE(log01::createLog());

    auto fileConf = FileConf::make(log01::templateFileName);
    fileConf->setFileName(log01::logFileName);
    TextLogModel model(fileConf);

    ASSERT_EQ(model.readFile(false), ReadFileResult::NormalExit);

    tp::SearchParam param;
    param.pattern = R"_(^\s+VALUES\s*\('[\w\s]+',\s*'[a-z_]+',\s*\d{2,3}\)$)_";
    param.type = tp::SearchType::Regex;
    tp::SearchParams params = {param};

    tp::RowData rowData;
    tutl::LogModelSearchResults searchRes(model);
    model.searchFromCurrentThread(params, false);

    ASSERT_TRUE(searchRes.nextRow(rowData));
    ASSERT_EQ(rowData.size(), 4);
    EXPECT_EQ(rowData.at(3), "\t\tVALUES ('John Constantine', 'john_constantine', 99)");

    ASSERT_FALSE(searchRes.nextRow(rowData));
}
