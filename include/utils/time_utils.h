#pragma once

#include <chrono>
#include <string>
#include <ctime>

namespace crypto_trading {
namespace utils {

using TimePoint = std::chrono::system_clock::time_point;
using Duration = std::chrono::milliseconds;

/**
 * @brief 时间工具类
 */
class TimeUtils
{
public:
    /**
     * @brief 获取当前时间戳（毫秒）
     */
    static int64_t nowMs();

    /**
     * @brief 获取当前时间戳（微秒）
     */
    static int64_t nowUs();

    /**
     * @brief 获取当前时间戳（秒）
     */
    static int64_t nowSec();

    /**
     * @brief 获取当前时间点
     */
    static TimePoint now();

    /**
     * @brief 时间戳转字符串（ISO 8601格式）
     * @param tp 时间点
     * @return ISO格式字符串 (e.g., "2025-10-05T10:30:45.123Z")
     */
    static std::string toIso8601(const TimePoint& tp);

    /**
     * @brief 时间戳转字符串（自定义格式）
     * @param tp 时间点
     * @param format 格式字符串（strftime格式）
     * @return 格式化字符串
     */
    static std::string toString(const TimePoint& tp, const std::string& format = "%Y-%m-%d %H:%M:%S");

    /**
     * @brief ISO字符串转时间点
     * @param iso_str ISO格式字符串
     * @return 时间点
     */
    static TimePoint fromIso8601(const std::string& iso_str);

    /**
     * @brief Unix时间戳（毫秒）转时间点
     * @param timestamp_ms 毫秒时间戳
     * @return 时间点
     */
    static TimePoint fromTimestampMs(int64_t timestamp_ms);

    /**
     * @brief 时间点转Unix时间戳（毫秒）
     * @param tp 时间点
     * @return 毫秒时间戳
     */
    static int64_t toTimestampMs(const TimePoint& tp);

    /**
     * @brief 计算两个时间点之间的毫秒差
     * @param start 开始时间
     * @param end 结束时间
     * @return 毫秒差
     */
    static int64_t durationMs(const TimePoint& start, const TimePoint& end);

    /**
     * @brief 休眠指定毫秒数
     * @param ms 毫秒数
     */
    static void sleepMs(int64_t ms);
};

/**
 * @brief 性能计时器（RAII）
 */
class ScopedTimer
{
public:
    explicit ScopedTimer(const std::string& name);
    ~ScopedTimer();

    // 获取已经过的时间（毫秒）
    int64_t elapsedMs() const;

private:
    std::string name_;
    TimePoint start_;
};

} // namespace utils
} // namespace crypto_trading
