#include "TestUtils.h"
#include "match/RegexMatcher.h"

TEST(Test_RegexMatcher, JsonCaseInsensitive_001)
{
    std::string_view rawData = R"_(Has \"quotes\" linebreak\nand tab\tend.)_";
    const auto parsedData = R"_(Has "quotes" linebreak and tab end.)_";
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
    const auto parsedData = R"_(Has "quotes" linebreak and tab end.)_";
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
    const auto parsedData = R"_(Has "quotes" win linebreak and tab finished)_";
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
    const auto parsedData = R"_(Has "quotes" win linebreak and tab finished)_";
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
    const auto parsedData = R"_(Has backslash \ return and slash/.)_";
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
    const auto parsedData = R"_(Has backslash \ return and slash/finished)_";
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
    const auto parsedData = R"_( test backslash\return.)_";
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
