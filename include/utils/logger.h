#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>

namespace crypto_trading {
namespace utils {

/**
 * @brief 高性能日志管理器
 *
 * @details
 * - 支持控制台和文件双输出
 * - 自动日志轮转（按大小和数量）
 * - 异步日志写入，性能优化
 * - 线程安全
 * - 支持运行时日志级别调整
 */
class Logger
{
public:
    /**
     * @brief 日志级别枚举
     */
    enum class Level
    {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical,
        Off
    };

    /**
     * @brief 日志配置结构
     */
    struct Config
    {
        std::string name = "crypto_trading";          // 日志名称
        std::string file_path = "logs/trading.log";   // 日志文件路径
        size_t max_file_size = 10 * 1024 * 1024;      // 单文件最大10MB
        size_t max_files = 10;                         // 最多保留10个文件
        Level console_level = Level::Info;             // 控制台日志级别
        Level file_level = Level::Debug;               // 文件日志级别
        bool enable_console = true;                    // 启用控制台输出
        bool enable_file = true;                       // 启用文件输出
        bool async_mode = true;                        // 异步模式
        size_t async_queue_size = 8192;                // 异步队列大小
        std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v";  // 日志格式
    };

    /**
     * @brief 初始化日志系统
     * @param config 日志配置
     */
    static void initialize(const Config& config);

    /**
     * @brief 关闭日志系统
     */
    static void shutdown();

    /**
     * @brief 获取默认日志器
     * @return 共享指针的日志器
     */
    static std::shared_ptr<spdlog::logger> get();

    /**
     * @brief 获取指定名称的日志器
     * @param name 日志器名称
     * @return 共享指针的日志器
     */
    static std::shared_ptr<spdlog::logger> get(const std::string& name);

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    static void setLevel(Level level);

    /**
     * @brief 设置控制台日志级别
     * @param level 日志级别
     */
    static void setConsoleLevel(Level level);

    /**
     * @brief 设置文件日志级别
     * @param level 日志级别
     */
    static void setFileLevel(Level level);

    /**
     * @brief 刷新日志缓冲区
     */
    static void flush();

private:
    static spdlog::level::level_enum toSpdlogLevel(Level level);
    static std::shared_ptr<spdlog::logger> default_logger_;
    static Config current_config_;
};

} // namespace utils
} // namespace crypto_trading

// 便捷宏定义
#define LOG_TRACE(...)    crypto_trading::utils::Logger::get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    crypto_trading::utils::Logger::get()->debug(__VA_ARGS__)
#define LOG_INFO(...)     crypto_trading::utils::Logger::get()->info(__VA_ARGS__)
#define LOG_WARN(...)     crypto_trading::utils::Logger::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    crypto_trading::utils::Logger::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) crypto_trading::utils::Logger::get()->critical(__VA_ARGS__)
