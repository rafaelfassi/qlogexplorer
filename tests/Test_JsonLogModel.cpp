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

    rowData.clear();
    EXPECT_EQ(model.getRow(0, rowData), 0);
    EXPECT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has "quotes" linebreak and tab finished)_");

    rowData.clear();
    EXPECT_EQ(model.getRow(1, rowData), 1);
    EXPECT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has "quotes" win linebreak and tab finished)_");

    rowData.clear();
    EXPECT_EQ(model.getRow(2, rowData), 2);
    EXPECT_EQ(rowData.size(), 7);
    EXPECT_EQ(rowData.at(6), R"_(Has backslash \ return double whitespace and slash/finished)_");
}
