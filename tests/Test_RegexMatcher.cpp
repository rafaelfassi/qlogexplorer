#include "TestUtils.h"
#include "match/RegexMatcher.h"

TEST(Test_RegexMatcher, TextCaseInsensitive_001)
{
    std::string_view chunkData = "Simple text1\nSimple text2\nSimple text3";
    std::string_view rowData = "Simple text2";
    std::string_view parsedRow = "Simple text2";

    tp::SearchParam param;
    param.pattern = "Simple text2";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, chunkData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, rowData));
    EXPECT_TRUE(matcher.match(parsedRow));
    EXPECT_TRUE(matcher.canUseRawMatch());
}

TEST(Test_RegexMatcher, TextCaseInsensitive_002)
{
    std::string_view chunkData = "Simple text1\nSimple text2\nSimple text3";
    std::string_view rowData = "Simple text2";
    std::string_view parsedRow = "Simple text2";

    tp::SearchParam param;
    param.pattern = "^Simple text2$";
    param.type = tp::SearchType::Regex;
    RegexMatcher matcher(param);

    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, chunkData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, rowData));
    EXPECT_TRUE(matcher.match(parsedRow));
    EXPECT_TRUE(matcher.canUseRawMatch());
}

TEST(Test_RegexMatcher, TextCaseInsensitive_003)
{
    std::string_view chunkData = "Simple text1\nSimple text2\nSimple text3";
    std::string_view rowData = "Simple text2";
    std::string_view parsedRow = "Simple text2";

    tp::SearchParam param;
    param.pattern = "(^|\\s+)Simple text2($|.)";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, chunkData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, rowData));
    EXPECT_TRUE(matcher.match(parsedRow));
    EXPECT_FALSE(matcher.canUseRawMatch());
}

TEST(Test_RegexMatcher, TextCaseInsensitive_004)
{
    std::string_view chunkData = "Simple text1\nSimple text2\nSimple text3";
    std::string_view rowData = "Simple text2";
    std::string_view parsedRow = "Simple text2";

    tp::SearchParam param;
    param.pattern = "\\ASimple text2\\z";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, chunkData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, rowData));
    EXPECT_TRUE(matcher.match(parsedRow));
    EXPECT_TRUE(matcher.canUseRawMatch());
}

TEST(Test_RegexMatcher, TextCaseInsensitive_005)
{
    std::string_view chunkData = "Simple text1\nSimple text2\nSimple text3";
    std::string_view rowData = "Simple text2";
    std::string_view parsedRow = "Simple text2";

    tp::SearchParam param;
    param.pattern = "(\\A|^)Simple text2(\\z|$)";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, chunkData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, rowData));
    EXPECT_TRUE(matcher.match(parsedRow));
    EXPECT_FALSE(matcher.canUseRawMatch());
}

TEST(Test_RegexMatcher, TextCaseInsensitive_006)
{
    std::string_view chunkData = "Simple text1\nSimple text2\nSimple text3";
    std::string_view rowData = "Simple text2";
    std::string_view parsedRow = "Simple text2";

    tp::SearchParam param;
    param.pattern = "Simpl[^a] text2";
    param.type = tp::SearchType::Regex;
    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, chunkData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, rowData));
    EXPECT_TRUE(matcher.match(parsedRow));
    EXPECT_TRUE(matcher.canUseRawMatch());
}

TEST(Test_RegexMatcher, JsonCaseInsensitive_001)
{
    std::string_view rawData = R"_(Has \"quotes\" linebreak\nand tab\tend.)_";
    std::string_view parsedData = R"_(Has "quotes" linebreak and tab end.)_";

    tp::SearchParam param;
    param.pattern = R"_(Has "quotes" linebreak and tab end.)_";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseInsensitive_002)
{
    std::string_view rawData = R"_(Has \"quotes\" linebreak\nand tab\tend.)_";
    std::string_view parsedData = R"_(Has "quotes" linebreak and tab end.)_";

    tp::SearchParam param;
    param.pattern = R"_(Has\s"quotes"\slinebreak\sand\stab\send.)_";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseInsensitive_003)
{
    std::string_view rawData = R"_(Has \u0022quotes\u0022 win linebreak\r\nand tab\u0009finished)_";
    std::string_view parsedData = R"_(Has "quotes" win linebreak and tab finished)_";

    tp::SearchParam param;
    param.pattern = R"_(Has "quotes" win linebreak and tab finished)_";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseInsensitive_004)
{
    std::string_view rawData = R"_(Has \u0022quotes\u0022 win linebreak\r\nand tab\u0009finished)_";
    std::string_view parsedData = R"_(Has "quotes" win linebreak and tab finished)_";

    tp::SearchParam param;
    param.pattern = R"_(Has\s"quotes"\swin\slinebreak\sand\stab\sfinished)_";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseInsensitive_005)
{
    std::string_view rawData = R"_(Has backslash \u005C return\r and slash\u002F.)_";
    std::string_view parsedData = R"_(Has backslash \ return and slash/.)_";

    tp::SearchParam param;
    param.pattern = R"_(Has backslash \\ return and slash/.)_";
    param.type = tp::SearchType::Regex;

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseSensitive_006)
{
    std::string_view rawData = R"_(Has backslash \u005c return\r and slash\u002Ffinished)_";
    std::string_view parsedData = R"_(Has backslash \ return and slash/finished)_";

    tp::SearchParam param;
    param.pattern = R"_(Has\sbackslash\s+\\\s*return\sand\s+slash\/finished)_";
    param.type = tp::SearchType::Regex;
    param.flags.set(tp::SearchFlag::MatchCase);

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseSensitive_007)
{
    std::string_view rawData = R"_(\ntest backslash\u005Creturn\r.)_";
    std::string_view parsedData = R"_( test backslash\return.)_";

    tp::SearchParam param;
    param.pattern = R"_(test backslash\\return.)_";
    param.type = tp::SearchType::Regex;
    param.flags.set(tp::SearchFlag::MatchCase);

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_RegexMatcher, JsonCaseSensitive_008)
{
    std::string_view rawData = R"_(e\r.)_";
    std::string_view parsedData = R"_(e .)_";

    tp::SearchParam param;
    param.pattern = R"_(e \.)_";
    param.type = tp::SearchType::Regex;
    param.flags.set(tp::SearchFlag::MatchCase);

    RegexMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}
