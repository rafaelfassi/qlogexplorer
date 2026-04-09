#include "TestUtils.h"
#include "match/SubStringMatcher.h"

TEST(Test_SubStringMatcher, TextCaseInsensitive_001)
{
    tp::SearchParam param;
    param.pattern = "cde";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, "abcdef"));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, "abcdef"));
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_002)
{
    tp::SearchParam param;
    param.pattern = "cdf";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_FALSE(matcher.preMatchRawText(tp::FileType::Text, true, "abcdef"));
    EXPECT_FALSE(matcher.preMatchRawText(tp::FileType::Text, false, "abcdef"));
    EXPECT_FALSE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_003)
{
    tp::SearchParam param;
    param.pattern = "abc";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, "abcdef"));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, "abcdef"));
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_004)
{
    tp::SearchParam param;
    param.pattern = "a";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, "abcdef"));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, "abcdef"));
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_005)
{
    tp::SearchParam param;
    param.pattern = "f";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, "abcdef"));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, "abcdef"));
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_006)
{
    tp::SearchParam param;
    param.pattern = "Ef";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, "abcdef"));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, "abcdef"));
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_007)
{
    tp::SearchParam param;
    param.pattern = "aBc";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, true, "abCdef"));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Text, false, "abCdef"));
    EXPECT_TRUE(matcher.match("abCdef"));
}

TEST(Test_SubStringMatcher, JsonCaseInsensitive_001)
{
    std::string_view rawData = R"_(Has \"quotes\" linebreak\nand tab\tend.)_";
    const auto parsedData = R"_(Has "quotes" linebreak and tab end.)_";
    tp::SearchParam param;
    param.pattern = "HAS \"quotes\" linebreak And taB end.";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_SubStringMatcher, JsonCaseInsensitive_002)
{
    std::string_view rawData = R"_(Has \u0022quotes\u0022 win linebreak\r\nand tab\u0009finished)_";
    const auto parsedData = R"_(Has "quotes" win linebreak and tab finished)_";
    tp::SearchParam param;
    param.pattern = "Has \"quotes\" win linebreak and tab finished";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}

TEST(Test_SubStringMatcher, JsonCaseSensitive_001)
{
    std::string_view rawData = R"_(Has backslash \u005C return\r and slash\u002f.)_";
    const auto parsedData = R"_(Has backslash \ return and slash/.)_";
    tp::SearchParam param;
    param.pattern = "Has backslash \\ return and slash/.";
    param.type = tp::SearchType::SubString;
    param.flags.set(tp::SearchFlag::MatchCase);
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, true, rawData));
    EXPECT_TRUE(matcher.preMatchRawText(tp::FileType::Json, false, rawData));
    EXPECT_TRUE(matcher.match(parsedData));
}
