#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace crypto_trading {
namespace utils {

/**
 * @brief 配置管理器
 *
 * @details
 * - 支持JSON配置文件加载
 * - 支持环境变量覆盖
 * - 支持多配置文件合并
 * - 线程安全
 * - 支持热加载
 * - 支持配置验证
 */
class ConfigManager
{
public:
    using json = nlohmann::json;

    /**
     * @brief 获取单例实例
     */
    static ConfigManager& getInstance();

    /**
     * @brief 加载配置文件
     * @param config_path 配置文件路径
     * @param merge 是否与现有配置合并
     * @return 是否加载成功
     */
    bool loadFromFile(const std::string& config_path, bool merge = false);

    /**
     * @brief 从JSON字符串加载配置
     * @param json_str JSON字符串
     * @param merge 是否合并
     * @return 是否加载成功
     */
    bool loadFromString(const std::string& json_str, bool merge = false);

    /**
     * @brief 保存配置到文件
     * @param config_path 配置文件路径
     * @return 是否保存成功
     */
    bool saveToFile(const std::string& config_path) const;

    /**
     * @brief 获取配置值（泛型）
     * @tparam T 返回值类型
     * @param key 配置键（支持点分隔路径，如 "exchange.binance.api_key"）
     * @param default_value 默认值
     * @return 配置值
     */
    template<typename T>
    T get(const std::string& key, const T& default_value = T{}) const;

    /**
     * @brief 获取配置值，优先从环境变量读取
     * @tparam T 返回值类型
     * @param key 配置键
     * @param env_var 环境变量名
     * @param default_value 默认值
     * @return 配置值
     */
    template<typename T>
    T getWithEnv(const std::string& key, const std::string& env_var, const T& default_value = T{}) const;

    /**
     * @brief 设置配置值
     * @tparam T 值类型
     * @param key 配置键
     * @param value 值
     */
    template<typename T>
    void set(const std::string& key, const T& value);

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return 是否存在
     */
    bool has(const std::string& key) const;

    /**
     * @brief 获取整个配置对象
     * @return JSON配置
     */
    const json& getConfig() const;

    /**
     * @brief 清空配置
     */
    void clear();

    /**
     * @brief 重新加载配置文件
     * @return 是否重载成功
     */
    bool reload();

    /**
     * @brief 验证配置完整性
     * @param required_keys 必需的配置键列表
     * @return 是否通过验证
     */
    bool validate(const std::vector<std::string>& required_keys) const;

    /**
     * @brief 获取配置摘要（用于调试）
     * @return 配置摘要字符串
     */
    std::string getSummary() const;

    // 禁用拷贝和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

private:
    ConfigManager() = default;

    // 从JSON路径获取值
    json getValue(const std::string& key) const;

    // 设置JSON路径的值
    void setValue(const std::string& key, const json& value);

    // 从环境变量读取
    std::optional<std::string> getEnvVar(const std::string& var_name) const;

    mutable std::mutex mutex_;
    json config_;
    std::string config_file_path_;
};

// 模板实现
template<typename T>
T ConfigManager::get(const std::string& key, const T& default_value) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    try
    {
        json value = getValue(key);
        if (value.is_null())
        {
            return default_value;
        }
        return value.get<T>();
    }
    catch (const std::exception&)
    {
        return default_value;
    }
}

template<typename T>
T ConfigManager::getWithEnv(const std::string& key, const std::string& env_var, const T& default_value) const
{
    // 优先从环境变量读取
    auto env_value = getEnvVar(env_var);
    if (env_value.has_value())
    {
        try
        {
            // 尝试转换环境变量值
            if constexpr (std::is_same_v<T, std::string>)
            {
                return env_value.value();
            }
            else if constexpr (std::is_same_v<T, int>)
            {
                return std::stoi(env_value.value());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return std::stod(env_value.value());
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                std::string val = env_value.value();
                std::transform(val.begin(), val.end(), val.begin(), ::tolower);
                return (val == "true" || val == "1" || val == "yes");
            }
        }
        catch (const std::exception&)
        {
            // 转换失败，回退到配置文件
        }
    }

    // 从配置文件读取
    return get<T>(key, default_value);
}

template<typename T>
void ConfigManager::set(const std::string& key, const T& value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    setValue(key, json(value));
}

} // namespace utils
} // namespace crypto_trading
