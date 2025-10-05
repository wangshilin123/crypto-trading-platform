#include <gtest/gtest.h>
#include "utils/config.h"
#include <fstream>
#include <filesystem>

using namespace crypto_trading::utils;

class ConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::filesystem::create_directories("test_config");
    }

    void TearDown() override
    {
        ConfigManager::getInstance().clear();
        std::filesystem::remove_all("test_config");
    }

    void createTestConfig(const std::string& path, const std::string& content)
    {
        std::ofstream file(path);
        file << content;
        file.close();
    }
};

TEST_F(ConfigTest, LoadFromFile)
{
    std::string config_json = R"({
        "exchange": {
            "name": "binance",
            "timeout_ms": 5000
        },
        "risk": {
            "max_order_value": 1000.0
        }
    })";

    createTestConfig("test_config/test.json", config_json);

    auto& config = ConfigManager::getInstance();
    ASSERT_TRUE(config.loadFromFile("test_config/test.json"));

    EXPECT_EQ(config.get<std::string>("exchange.name"), "binance");
    EXPECT_EQ(config.get<int>("exchange.timeout_ms"), 5000);
    EXPECT_DOUBLE_EQ(config.get<double>("risk.max_order_value"), 1000.0);
}

TEST_F(ConfigTest, LoadFromString)
{
    std::string json_str = R"({"key1": "value1", "key2": 123})";

    auto& config = ConfigManager::getInstance();
    ASSERT_TRUE(config.loadFromString(json_str));

    EXPECT_EQ(config.get<std::string>("key1"), "value1");
    EXPECT_EQ(config.get<int>("key2"), 123);
}

TEST_F(ConfigTest, GetWithDefault)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString(R"({"existing_key": "value"})");

    // 存在的键
    EXPECT_EQ(config.get<std::string>("existing_key", "default"), "value");

    // 不存在的键，返回默认值
    EXPECT_EQ(config.get<std::string>("non_existing_key", "default"), "default");
    EXPECT_EQ(config.get<int>("non_existing_key", 999), 999);
}

TEST_F(ConfigTest, NestedKeys)
{
    std::string json_str = R"({
        "level1": {
            "level2": {
                "level3": {
                    "value": "deep_value"
                }
            }
        }
    })";

    auto& config = ConfigManager::getInstance();
    config.loadFromString(json_str);

    EXPECT_EQ(config.get<std::string>("level1.level2.level3.value"), "deep_value");
}

TEST_F(ConfigTest, SetValue)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString("{}");

    config.set("new_key", "new_value");
    config.set("nested.key", 42);

    EXPECT_EQ(config.get<std::string>("new_key"), "new_value");
    EXPECT_EQ(config.get<int>("nested.key"), 42);
}

TEST_F(ConfigTest, HasKey)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString(R"({"key1": "value1"})");

    EXPECT_TRUE(config.has("key1"));
    EXPECT_FALSE(config.has("key2"));
}

TEST_F(ConfigTest, SaveToFile)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString(R"({"key": "value"})");

    ASSERT_TRUE(config.saveToFile("test_config/saved.json"));
    ASSERT_TRUE(std::filesystem::exists("test_config/saved.json"));

    // 重新加载验证
    ConfigManager::getInstance().clear();
    ASSERT_TRUE(config.loadFromFile("test_config/saved.json"));
    EXPECT_EQ(config.get<std::string>("key"), "value");
}

TEST_F(ConfigTest, MergeConfig)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString(R"({"key1": "value1", "key2": "value2"})");
    config.loadFromString(R"({"key2": "new_value2", "key3": "value3"})", true);

    EXPECT_EQ(config.get<std::string>("key1"), "value1");
    EXPECT_EQ(config.get<std::string>("key2"), "new_value2");  // 被覆盖
    EXPECT_EQ(config.get<std::string>("key3"), "value3");      // 新增
}

TEST_F(ConfigTest, GetWithEnv)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString(R"({"db_password": "config_password"})");

    // 设置环境变量
    setenv("TEST_DB_PASSWORD", "env_password", 1);

    // 环境变量优先
    EXPECT_EQ(config.getWithEnv<std::string>("db_password", "TEST_DB_PASSWORD", "default"),
              "env_password");

    // 清理环境变量
    unsetenv("TEST_DB_PASSWORD");

    // 回退到配置文件
    EXPECT_EQ(config.getWithEnv<std::string>("db_password", "TEST_DB_PASSWORD", "default"),
              "config_password");
}

TEST_F(ConfigTest, Validate)
{
    auto& config = ConfigManager::getInstance();
    config.loadFromString(R"({
        "exchange.name": "binance",
        "exchange.timeout": 5000,
        "risk.max_order": 1000
    })");

    std::vector<std::string> required_keys = {
        "exchange.name",
        "exchange.timeout",
        "risk.max_order"
    };

    EXPECT_TRUE(config.validate(required_keys));

    required_keys.push_back("missing.key");
    EXPECT_FALSE(config.validate(required_keys));
}

TEST_F(ConfigTest, Reload)
{
    createTestConfig("test_config/reload.json", R"({"version": 1})");

    auto& config = ConfigManager::getInstance();
    ASSERT_TRUE(config.loadFromFile("test_config/reload.json"));
    EXPECT_EQ(config.get<int>("version"), 1);

    // 修改文件
    createTestConfig("test_config/reload.json", R"({"version": 2})");

    // 重新加载
    ASSERT_TRUE(config.reload());
    EXPECT_EQ(config.get<int>("version"), 2);
}
