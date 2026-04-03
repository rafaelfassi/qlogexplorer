#include "TestUtils.h"
#include "model/JsonLogModel.h"

TEST(Test_JsonLogModel, Test_001)
{
    auto fileConf = FileConf::make(tp::FileType::Json);
    fileConf->setFileName("json_0001.json");
    JsonLogModel model(fileConf);
    tp::RowData rowData;

    EXPECT_TRUE(model.loadForTesting());
    EXPECT_EQ(model.rowCount(), 3);

    EXPECT_EQ(model.getRow(0, rowData), 0);
    EXPECT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has "quotes" linebreak and tab finished)_");

    EXPECT_EQ(model.getRow(1, rowData), 1);
    EXPECT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has "quotes" win linebreak and tab finished)_");

    EXPECT_EQ(model.getRow(2, rowData), 2);
    EXPECT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has backslash \ return double whitespace and slash/finished)_");
}

TEST(Test_JsonLogModel, Integrity_001)
{
    const bool forceRecreate = true;
    const std::size_t rowCount = 3 * g_chunkSize;
    QFile file("Test_JsonLogModel-Integrity_001.json");

    std::vector<QString> lvls{"FATAL", "ERROR", "WARNING", "INFO", "DEBUG"};
    std::vector<std::pair<QString, std::string>> msgs{
        std::make_pair(R"_(Some\tlog\u0009with\ttabs)_", R"_(Some log with tabs)_"),
        std::make_pair(R"_(This\nlog\u000Ahas\r\nnewline\n)_", R"_(This log has newline )_"),
        std::make_pair(R"_(\ntest backslash\u005Creturn\r.)_", R"_( test backslash\return .)_"),
        std::make_pair(R"_(\"quotes\" with many spaces \t\r\n.)_", R"_("quotes" with many spaces .)_")};

    if (forceRecreate || !file.exists())
    {
        if (file.exists())
        {
            EXPECT_TRUE(file.remove());
        }

        EXPECT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));

        QTextStream out(&file);
        for (std::size_t i = 0; i < rowCount; ++i)
        {
            const size_t li = (i % lvls.size());
            const size_t mi = (i % msgs.size());
            out << QString(R"_({"LogLevel":"%1","LineNumber": %2,"LogMessage":"%3"})_")
                       .arg(lvls[li])
                       .arg(i)
                       .arg(msgs[mi].first)
                << Qt::endl;
        }

        file.close();
    }

    auto fileConf = FileConf::make(tp::FileType::Json);
    fileConf->setFileName(utl::toStr(file.fileName()));
    JsonLogModel model(fileConf);

    ASSERT_TRUE(model.loadForTesting());
    ASSERT_EQ(model.rowCount(), rowCount);

    tp::RowData rowData;
    for (std::size_t i = 0; i < rowCount; ++i)
    {
        const size_t li = (i % lvls.size());
        const size_t mi = (i % msgs.size());

        ASSERT_EQ(model.getRow(i, rowData), i);
        ASSERT_EQ(rowData.size(), 3);
        ASSERT_EQ(rowData.at(0), lvls[li].toStdString());
        ASSERT_EQ(std::stoi(rowData.at(1)), i);
        ASSERT_EQ(rowData.at(2), msgs[mi].second);
    }
}
