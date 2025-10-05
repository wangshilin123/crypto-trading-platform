#include <gtest/gtest.h>
#include "utils/logger.h"
#include <filesystem>

using namespace crypto_trading::utils;

class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 清理之前的日志
        std::filesystem::remove_all("test_logs");
    }

    void TearDown() override
    {
        Logger::shutdown();
        std::filesystem::remove_all("test_logs");
    }
};

TEST_F(LoggerTest, InitializeDefault)
{
    Logger::Config config;
    config.file_path = "test_logs/test.log";
    config.enable_console = false;  // 测试时禁用控制台输出

    ASSERT_NO_THROW(Logger::initialize(config));
    ASSERT_NE(Logger::get(), nullptr);
}

TEST_F(LoggerTest, LogMessages)
{
    Logger::Config config;
    config.file_path = "test_logs/messages.log";
    config.enable_console = false;
    config.file_level = Logger::Level::Debug;

    Logger::initialize(config);

    LOG_DEBUG("Debug message: {}", 123);
    LOG_INFO("Info message: {}", "test");
    LOG_WARN("Warning message");
    LOG_ERROR("Error message: {:.2f}", 3.14159);

    Logger::flush();

    // 检查日志文件是否创建
    ASSERT_TRUE(std::filesystem::exists("test_logs/messages.log"));
}

TEST_F(LoggerTest, LogRotation)
{
    Logger::Config config;
    config.file_path = "test_logs/rotation.log";
    config.max_file_size = 1024;  // 1KB
    config.max_files = 3;
    config.enable_console = false;

    Logger::initialize(config);

    // 写入大量日志触发轮转
    for (int i = 0; i < 100; ++i)
    {
        LOG_INFO("This is log message number {} with some extra text to make it longer", i);
    }

    Logger::flush();
    ASSERT_TRUE(std::filesystem::exists("test_logs/rotation.log"));
}

TEST_F(LoggerTest, AsyncMode)
{
    Logger::Config config;
    config.file_path = "test_logs/async.log";
    config.async_mode = true;
    config.async_queue_size = 4096;
    config.enable_console = false;

    ASSERT_NO_THROW(Logger::initialize(config));

    for (int i = 0; i < 1000; ++i)
    {
        LOG_INFO("Async log message {}", i);
    }

    Logger::shutdown();
    ASSERT_TRUE(std::filesystem::exists("test_logs/async.log"));
}

TEST_F(LoggerTest, MultipleLoggers)
{
    Logger::Config config;
    config.file_path = "test_logs/main.log";
    config.enable_console = false;

    Logger::initialize(config);

    auto logger1 = Logger::get("module1");
    auto logger2 = Logger::get("module2");

    ASSERT_NE(logger1, nullptr);
    ASSERT_NE(logger2, nullptr);
    ASSERT_NE(logger1, logger2);

    logger1->info("Message from module1");
    logger2->info("Message from module2");
}

TEST_F(LoggerTest, LogLevelControl)
{
    Logger::Config config;
    config.file_path = "test_logs/levels.log";
    config.enable_console = false;
    config.file_level = Logger::Level::Warn;

    Logger::initialize(config);

    LOG_DEBUG("This should not appear");
    LOG_INFO("This should not appear");
    LOG_WARN("This should appear");
    LOG_ERROR("This should appear");

    Logger::flush();

    // 读取日志文件验证
    std::ifstream log_file("test_logs/levels.log");
    std::string content((std::istreambuf_iterator<char>(log_file)),
                        std::istreambuf_iterator<char>());

    ASSERT_EQ(content.find("This should not appear"), std::string::npos);
    ASSERT_NE(content.find("This should appear"), std::string::npos);
}
