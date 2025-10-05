#include "utils/time_utils.h"
#include "utils/logger.h"
#include <iomanip>
#include <sstream>
#include <thread>

namespace crypto_trading {
namespace utils {

int64_t TimeUtils::nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int64_t TimeUtils::nowUs()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int64_t TimeUtils::nowSec()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

TimePoint TimeUtils::now()
{
    return std::chrono::system_clock::now();
}

std::string TimeUtils::toIso8601(const TimePoint& tp)
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_time;

#ifdef _WIN32
    gmtime_s(&tm_time, &timer);
#else
    gmtime_r(&timer, &tm_time);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_time, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

std::string TimeUtils::toString(const TimePoint& tp, const std::string& format)
{
    auto timer = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_time;

#ifdef _WIN32
    localtime_s(&tm_time, &timer);
#else
    localtime_r(&timer, &tm_time);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_time, format.c_str());
    return oss.str();
}

TimePoint TimeUtils::fromIso8601(const std::string& iso_str)
{
    std::tm tm_time = {};
    std::istringstream ss(iso_str);
    ss >> std::get_time(&tm_time, "%Y-%m-%dT%H:%M:%S");

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm_time));

    // 解析毫秒部分
    size_t dot_pos = iso_str.find('.');
    if (dot_pos != std::string::npos)
    {
        std::string ms_str = iso_str.substr(dot_pos + 1, 3);
        int ms = std::stoi(ms_str);
        tp += std::chrono::milliseconds(ms);
    }

    return tp;
}

TimePoint TimeUtils::fromTimestampMs(int64_t timestamp_ms)
{
    return TimePoint(std::chrono::milliseconds(timestamp_ms));
}

int64_t TimeUtils::toTimestampMs(const TimePoint& tp)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()
    ).count();
}

int64_t TimeUtils::durationMs(const TimePoint& start, const TimePoint& end)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void TimeUtils::sleepMs(int64_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ScopedTimer 实现
ScopedTimer::ScopedTimer(const std::string& name)
    : name_(name)
    , start_(TimeUtils::now())
{
}

ScopedTimer::~ScopedTimer()
{
    auto elapsed = elapsedMs();
    LOG_DEBUG("{} took {} ms", name_, elapsed);
}

int64_t ScopedTimer::elapsedMs() const
{
    return TimeUtils::durationMs(start_, TimeUtils::now());
}

} // namespace utils
} // namespace crypto_trading
