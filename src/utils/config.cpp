#include "utils/config.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

namespace crypto_trading {
namespace utils {

ConfigManager& ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadFromFile(const std::string& config_path, bool merge)
{
    std::lock_guard<std::mutex> lock(mutex_);

    try
    {
        std::ifstream file(config_path);
        if (!file.is_open())
        {
            return false;
        }

        json new_config = json::parse(file);
        file.close();

        if (merge && !config_.is_null())
        {
            // 递归合并JSON对象
            config_.merge_patch(new_config);
        }
        else
        {
            config_ = new_config;
        }

        config_file_path_ = config_path;
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ConfigManager::loadFromString(const std::string& json_str, bool merge)
{
    std::lock_guard<std::mutex> lock(mutex_);

    try
    {
        json new_config = json::parse(json_str);

        if (merge && !config_.is_null())
        {
            config_.merge_patch(new_config);
        }
        else
        {
            config_ = new_config;
        }

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ConfigManager::saveToFile(const std::string& config_path) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    try
    {
        std::ofstream file(config_path);
        if (!file.is_open())
        {
            return false;
        }

        file << config_.dump(4);  // 格式化输出，缩进4空格
        file.close();
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ConfigManager::has(const std::string& key) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    json value = getValue(key);
    return !value.is_null();
}

const ConfigManager::json& ConfigManager::getConfig() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

void ConfigManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    config_.clear();
    config_file_path_.clear();
}

bool ConfigManager::reload()
{
    if (config_file_path_.empty())
    {
        return false;
    }
    return loadFromFile(config_file_path_, false);
}

bool ConfigManager::validate(const std::vector<std::string>& required_keys) const
{
    for (const auto& key : required_keys)
    {
        if (!has(key))
        {
            return false;
        }
    }
    return true;
}

std::string ConfigManager::getSummary() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;

    oss << "Configuration Summary:\n";
    oss << "File: " << (config_file_path_.empty() ? "N/A" : config_file_path_) << "\n";
    oss << "Keys: " << config_.size() << "\n";
    oss << "Content:\n" << config_.dump(2);

    return oss.str();
}

ConfigManager::json ConfigManager::getValue(const std::string& key) const
{
    // 支持点分隔路径：exchange.binance.api_key
    std::vector<std::string> keys;
    std::istringstream iss(key);
    std::string token;

    while (std::getline(iss, token, '.'))
    {
        keys.push_back(token);
    }

    json current = config_;
    for (const auto& k : keys)
    {
        if (current.is_object() && current.contains(k))
        {
            current = current[k];
        }
        else
        {
            return json();  // 返回null
        }
    }

    return current;
}

void ConfigManager::setValue(const std::string& key, const json& value)
{
    // 支持点分隔路径
    std::vector<std::string> keys;
    std::istringstream iss(key);
    std::string token;

    while (std::getline(iss, token, '.'))
    {
        keys.push_back(token);
    }

    if (keys.empty())
    {
        return;
    }

    // 递归创建嵌套对象
    json* current = &config_;
    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
        if (!current->is_object())
        {
            *current = json::object();
        }

        if (!current->contains(keys[i]) || !(*current)[keys[i]].is_object())
        {
            (*current)[keys[i]] = json::object();
        }

        current = &(*current)[keys[i]];
    }

    if (!current->is_object())
    {
        *current = json::object();
    }

    (*current)[keys.back()] = value;
}

std::optional<std::string> ConfigManager::getEnvVar(const std::string& var_name) const
{
    const char* value = std::getenv(var_name.c_str());
    if (value != nullptr)
    {
        return std::string(value);
    }
    return std::nullopt;
}

} // namespace utils
} // namespace crypto_trading
