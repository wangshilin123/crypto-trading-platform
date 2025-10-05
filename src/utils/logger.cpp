#include "utils/logger.h"
#include <spdlog/async.h>
#include <filesystem>
#include <iostream>

namespace crypto_trading {
namespace utils {

std::shared_ptr<spdlog::logger> Logger::default_logger_ = nullptr;
Logger::Config Logger::current_config_;

void Logger::initialize(const Config& config)
{
    try
    {
        current_config_ = config;

        // 确保日志目录存在
        if (config.enable_file)
        {
            std::filesystem::path log_path(config.file_path);
            std::filesystem::create_directories(log_path.parent_path());
        }

        std::vector<spdlog::sink_ptr> sinks;

        // 控制台输出
        if (config.enable_console)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(toSpdlogLevel(config.console_level));
            console_sink->set_pattern(config.pattern);
            sinks.push_back(console_sink);
        }

        // 文件输出（带轮转）
        if (config.enable_file)
        {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config.file_path,
                config.max_file_size,
                config.max_files
            );
            file_sink->set_level(toSpdlogLevel(config.file_level));
            file_sink->set_pattern(config.pattern);
            sinks.push_back(file_sink);
        }

        // 创建日志器
        if (config.async_mode)
        {
            // 异步模式：高性能，适合生产环境
            spdlog::init_thread_pool(config.async_queue_size, 1);
            default_logger_ = std::make_shared<spdlog::async_logger>(
                config.name,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        }
        else
        {
            // 同步模式：调试时使用
            default_logger_ = std::make_shared<spdlog::logger>(
                config.name,
                sinks.begin(),
                sinks.end()
            );
        }

        // 设置默认级别
        default_logger_->set_level(spdlog::level::trace);  // 允许所有级别，由sink控制
        default_logger_->flush_on(spdlog::level::warn);    // warn及以上立即刷新

        // 注册为默认日志器
        spdlog::register_logger(default_logger_);
        spdlog::set_default_logger(default_logger_);

        LOG_INFO("Logger initialized successfully: {}", config.name);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        throw;
    }
}

void Logger::shutdown()
{
    if (default_logger_)
    {
        LOG_INFO("Logger shutting down...");
        default_logger_->flush();
        spdlog::shutdown();
        default_logger_.reset();
    }
}

std::shared_ptr<spdlog::logger> Logger::get()
{
    if (!default_logger_)
    {
        // 自动初始化为默认配置
        initialize();
    }
    return default_logger_;
}

std::shared_ptr<spdlog::logger> Logger::get(const std::string& name)
{
    auto logger = spdlog::get(name);
    if (!logger)
    {
        // 如果不存在，创建一个新的日志器
        logger = default_logger_->clone(name);
        spdlog::register_logger(logger);
    }
    return logger;
}

void Logger::setLevel(Level level)
{
    if (default_logger_)
    {
        default_logger_->set_level(toSpdlogLevel(level));
    }
}

void Logger::setConsoleLevel(Level level)
{
    if (default_logger_)
    {
        auto sinks = default_logger_->sinks();
        if (!sinks.empty() && current_config_.enable_console)
        {
            sinks[0]->set_level(toSpdlogLevel(level));
        }
    }
}

void Logger::setFileLevel(Level level)
{
    if (default_logger_)
    {
        auto sinks = default_logger_->sinks();
        size_t file_sink_idx = current_config_.enable_console ? 1 : 0;
        if (sinks.size() > file_sink_idx && current_config_.enable_file)
        {
            sinks[file_sink_idx]->set_level(toSpdlogLevel(level));
        }
    }
}

void Logger::flush()
{
    if (default_logger_)
    {
        default_logger_->flush();
    }
}

spdlog::level::level_enum Logger::toSpdlogLevel(Level level)
{
    switch (level)
    {
        case Level::Trace:    return spdlog::level::trace;
        case Level::Debug:    return spdlog::level::debug;
        case Level::Info:     return spdlog::level::info;
        case Level::Warn:     return spdlog::level::warn;
        case Level::Error:    return spdlog::level::err;
        case Level::Critical: return spdlog::level::critical;
        case Level::Off:      return spdlog::level::off;
        default:              return spdlog::level::info;
    }
}

} // namespace utils
} // namespace crypto_trading
