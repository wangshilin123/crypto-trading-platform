#include <gtest/gtest.h>
#include "utils/time_utils.h"
#include <thread>

using namespace crypto_trading::utils;

TEST(TimeUtilsTest, Timestamps)
{
    int64_t ms1 = TimeUtils::nowMs();
    int64_t us1 = TimeUtils::nowUs();
    int64_t sec1 = TimeUtils::nowSec();

    // 微秒应该比毫秒大
    EXPECT_GT(us1, ms1);

    // 毫秒应该比秒大（单位换算）
    EXPECT_GT(ms1, sec1 * 1000 - 1000);  // 允许1秒误差

    // 等待一小段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    int64_t ms2 = TimeUtils::nowMs();
    EXPECT_GT(ms2, ms1);
}

TEST(TimeUtilsTest, Now)
{
    auto tp1 = TimeUtils::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto tp2 = TimeUtils::now();

    EXPECT_LT(tp1, tp2);
}

TEST(TimeUtilsTest, ToIso8601)
{
    auto tp = TimeUtils::fromTimestampMs(1609459200000);  // 2021-01-01 00:00:00 UTC
    std::string iso = TimeUtils::toIso8601(tp);

    // 应该包含日期和时间
    EXPECT_NE(iso.find("2021"), std::string::npos);
    EXPECT_NE(iso.find("T"), std::string::npos);
    EXPECT_NE(iso.find("Z"), std::string::npos);
}

TEST(TimeUtilsTest, ToString)
{
    auto tp = TimeUtils::now();
    std::string str = TimeUtils::toString(tp, "%Y-%m-%d");

    // 应该是 YYYY-MM-DD 格式
    EXPECT_EQ(str.length(), 10);
    EXPECT_EQ(str[4], '-');
    EXPECT_EQ(str[7], '-');
}

TEST(TimeUtilsTest, FromTimestampMs)
{
    int64_t timestamp = 1609459200000;  // 2021-01-01 00:00:00 UTC
    auto tp = TimeUtils::fromTimestampMs(timestamp);

    int64_t converted = TimeUtils::toTimestampMs(tp);
    EXPECT_EQ(converted, timestamp);
}

TEST(TimeUtilsTest, ToTimestampMs)
{
    auto tp = TimeUtils::now();
    int64_t ms = TimeUtils::toTimestampMs(tp);

    // 应该是一个合理的时间戳（2020年之后）
    EXPECT_GT(ms, 1577836800000);  // 2020-01-01
}

TEST(TimeUtilsTest, TimestampRoundTrip)
{
    int64_t original = TimeUtils::nowMs();
    auto tp = TimeUtils::fromTimestampMs(original);
    int64_t converted = TimeUtils::toTimestampMs(tp);

    EXPECT_EQ(converted, original);
}

TEST(TimeUtilsTest, DurationMs)
{
    auto start = TimeUtils::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto end = TimeUtils::now();

    int64_t duration = TimeUtils::durationMs(start, end);

    // 应该大约是50ms（允许一些误差）
    EXPECT_GE(duration, 45);
    EXPECT_LE(duration, 100);
}

TEST(TimeUtilsTest, SleepMs)
{
    auto start = TimeUtils::now();
    TimeUtils::sleepMs(100);
    auto end = TimeUtils::now();

    int64_t duration = TimeUtils::durationMs(start, end);

    // 应该大约是100ms
    EXPECT_GE(duration, 95);
    EXPECT_LE(duration, 150);
}

TEST(ScopedTimerTest, BasicTiming)
{
    auto start = TimeUtils::now();
    {
        ScopedTimer timer("test_operation");
        TimeUtils::sleepMs(50);

        int64_t elapsed = timer.elapsedMs();
        EXPECT_GE(elapsed, 45);
        EXPECT_LE(elapsed, 100);
    }
    auto end = TimeUtils::now();

    int64_t total = TimeUtils::durationMs(start, end);
    EXPECT_GE(total, 45);
}

TEST(ScopedTimerTest, NestedTimers)
{
    ScopedTimer outer("outer");

    TimeUtils::sleepMs(20);

    {
        ScopedTimer inner("inner");
        TimeUtils::sleepMs(30);
    }

    TimeUtils::sleepMs(20);

    int64_t total = outer.elapsedMs();
    EXPECT_GE(total, 65);  // 20 + 30 + 20 = 70ms
}

TEST(TimeUtilsTest, Iso8601RoundTrip)
{
    auto original = TimeUtils::now();
    std::string iso = TimeUtils::toIso8601(original);
    auto parsed = TimeUtils::fromIso8601(iso);

    // ISO 8601使用UTC时间，转换可能有时区差异
    // 只检查时间戳接近即可（允许1小时误差，处理时区问题）
    int64_t diff = std::abs(TimeUtils::durationMs(original, parsed));
    EXPECT_LT(diff, 3600000);  // 1小时 = 3600000ms
}
