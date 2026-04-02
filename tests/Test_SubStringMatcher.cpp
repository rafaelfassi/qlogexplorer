#include "gtest/gtest.h"
#include "match/SubStringMatcher.h"

TEST(Test_SubStringMatcher, TextCaseInsensitive_001)
{
    tp::SearchParam param;
    param.pattern = "cde";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_002)
{
    tp::SearchParam param;
    param.pattern = "cdf";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_FALSE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_003)
{
    tp::SearchParam param;
    param.pattern = "abc";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_004)
{
    tp::SearchParam param;
    param.pattern = "a";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_005)
{
    tp::SearchParam param;
    param.pattern = "f";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_006)
{
    tp::SearchParam param;
    param.pattern = "Ef";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.match("abcdef"));
}

TEST(Test_SubStringMatcher, TextCaseInsensitive_007)
{
    tp::SearchParam param;
    param.pattern = "aBc";
    param.type = tp::SearchType::SubString;
    SubStringMatcher matcher(param);
    EXPECT_TRUE(matcher.match("abCdef"));
}
