#include "utils/logger.h"
#include "utils/config.h"
#include "utils/time_utils.h"
#include "utils/crypto_utils.h"
#include <iostream>

using namespace crypto_trading::utils;

int main()
{
    // ========== 1. 日志系统使用示例 ==========
    std::cout << "=== Logger Example ===" << std::endl;

    Logger::Config log_config;
    log_config.name = "example_app";
    log_config.file_path = "logs/example.log";
    log_config.console_level = Logger::Level::Info;
    log_config.file_level = Logger::Level::Debug;
    log_config.async_mode = true;

    Logger::initialize(log_config);

    LOG_INFO("Application started");
    LOG_DEBUG("Debug message with value: {}", 42);
    LOG_WARN("Warning: This is a warning message");
    LOG_ERROR("Error occurred: {}", "sample error");

    // 创建模块专用日志器
    auto module_logger = Logger::get("trading_module");
    module_logger->info("Module-specific log message");

    // ========== 2. 配置管理使用示例 ==========
    std::cout << "\n=== Config Example ===" << std::endl;

    auto& config = ConfigManager::getInstance();

    // 从JSON字符串加载配置
    std::string config_json = R"({
        "exchange": {
            "name": "binance",
            "endpoints": {
                "rest": "https://api.binance.com",
                "websocket": "wss://stream.binance.com:9443"
            },
            "timeout_ms": 5000
        },
        "trading": {
            "default_symbols": ["BTCUSDT", "ETHUSDT"],
            "max_orders_per_second": 10
        },
        "risk": {
            "max_order_value_usdt": 1000.0,
            "max_position_value_usdt": 5000.0
        }
    })";

    config.loadFromString(config_json);

    // 读取配置
    std::string exchange_name = config.get<std::string>("exchange.name");
    int timeout = config.get<int>("exchange.timeout_ms");
    double max_order = config.get<double>("risk.max_order_value_usdt");

    LOG_INFO("Exchange: {}, Timeout: {}ms, Max Order: ${}",
             exchange_name, timeout, max_order);

    // 设置配置
    config.set("runtime.start_time", TimeUtils::nowMs());
    config.set("runtime.version", "1.0.0");

    // 使用环境变量（优先级高于配置文件）
    setenv("API_KEY", "your_api_key_from_env", 1);
    std::string api_key = config.getWithEnv<std::string>(
        "exchange.api_key", "API_KEY", "default_key"
    );
    LOG_INFO("API Key: {}", api_key.substr(0, 8) + "...");  // 只显示前8位

    // ========== 3. 时间工具使用示例 ==========
    std::cout << "\n=== Time Utils Example ===" << std::endl;

    // 获取当前时间
    int64_t now_ms = TimeUtils::nowMs();
    int64_t now_sec = TimeUtils::nowSec();
    LOG_INFO("Current timestamp (ms): {}", now_ms);
    LOG_INFO("Current timestamp (sec): {}", now_sec);

    // 时间格式化
    auto now = TimeUtils::now();
    std::string iso_time = TimeUtils::toIso8601(now);
    std::string formatted = TimeUtils::toString(now, "%Y-%m-%d %H:%M:%S");

    LOG_INFO("ISO 8601: {}", iso_time);
    LOG_INFO("Formatted: {}", formatted);

    // 性能计时
    {
        ScopedTimer timer("sample_operation");

        // 模拟一些操作
        int sum = 0;
        for (int i = 0; i < 1000000; ++i)
        {
            sum += i;
        }

        LOG_INFO("Computation result: {}", sum);
    }  // 析构时自动打印耗时

    // 手动计时
    auto start = TimeUtils::now();
    TimeUtils::sleepMs(100);
    auto end = TimeUtils::now();
    int64_t duration = TimeUtils::durationMs(start, end);
    LOG_INFO("Operation took {} ms", duration);

    // ========== 4. 加密工具使用示例 ==========
    std::cout << "\n=== Crypto Utils Example ===" << std::endl;

    // HMAC-SHA256 签名（币安API常用）
    std::string query_string = "symbol=BTCUSDT&side=BUY&type=LIMIT&quantity=1&timestamp=";
    query_string += std::to_string(now_ms);

    std::string api_secret = "your_api_secret";
    std::string signature = CryptoUtils::hmacSha256(query_string, api_secret);

    LOG_INFO("Query: {}", query_string);
    LOG_INFO("Signature: {}", signature);

    // SHA256 哈希
    std::string data = "sensitive_data";
    std::string hash = CryptoUtils::sha256(data);
    LOG_INFO("SHA256 hash: {}", hash);

    // Base64 编码/解码
    std::string original = "Hello, Trading Platform!";
    std::string encoded = CryptoUtils::base64Encode(original);
    auto decoded = CryptoUtils::base64Decode(encoded);
    std::string decoded_str(decoded.begin(), decoded.end());

    LOG_INFO("Original: {}", original);
    LOG_INFO("Base64 encoded: {}", encoded);
    LOG_INFO("Decoded: {}", decoded_str);

    // URL 编码（API请求参数）
    std::string url_param = "price=50000.5&type=LIMIT";
    std::string encoded_url = CryptoUtils::urlEncode(url_param);
    std::string decoded_url = CryptoUtils::urlDecode(encoded_url);

    LOG_INFO("URL param: {}", url_param);
    LOG_INFO("URL encoded: {}", encoded_url);
    LOG_INFO("URL decoded: {}", decoded_url);

    // ========== 5. 字符串工具使用示例 ==========
    std::cout << "\n=== String Utils Example ===" << std::endl;

    std::string messy = "  BTCUSDT  ";
    std::string clean = StringUtils::trim(messy);
    std::string lower = StringUtils::toLower(clean);

    LOG_INFO("Original: '{}', Trimmed: '{}', Lower: '{}'", messy, clean, lower);

    // 字符串分割和拼接
    std::string symbols = "BTCUSDT,ETHUSDT,BNBUSDT";
    auto symbol_list = StringUtils::split(symbols, ',');

    LOG_INFO("Symbol count: {}", symbol_list.size());
    for (const auto& sym : symbol_list)
    {
        LOG_INFO("  - {}", sym);
    }

    std::string joined = StringUtils::join(symbol_list, " | ");
    LOG_INFO("Joined: {}", joined);

    // 字符串格式化
    std::string formatted_msg = StringUtils::format(
        "Order placed: %s, Price: %.2f, Qty: %.4f",
        "BTCUSDT", 50000.50, 0.0123
    );
    LOG_INFO("{}", formatted_msg);

    // ========== 6. 综合应用示例：模拟API请求签名 ==========
    std::cout << "\n=== Simulated API Request ===" << std::endl;

    // 构建API请求参数
    std::map<std::string, std::string> params;
    params["symbol"] = "BTCUSDT";
    params["side"] = "BUY";
    params["type"] = "LIMIT";
    params["quantity"] = "0.01";
    params["price"] = "50000.00";
    params["timestamp"] = std::to_string(TimeUtils::nowMs());

    // 拼接参数
    std::vector<std::string> param_parts;
    for (const auto& [key, value] : params)
    {
        param_parts.push_back(key + "=" + value);
    }
    std::string param_str = StringUtils::join(param_parts, "&");

    // 签名
    std::string request_signature = CryptoUtils::hmacSha256(param_str, api_secret);

    LOG_INFO("API Request:");
    LOG_INFO("  Endpoint: /api/v3/order");
    LOG_INFO("  Params: {}", param_str);
    LOG_INFO("  Signature: {}", request_signature);

    // ========== 保存运行时配置 ==========
    config.set("runtime.last_request_time", TimeUtils::nowMs());
    config.set("runtime.request_count", 1);
    config.saveToFile("config/runtime_config.json");

    LOG_INFO("Runtime config saved");

    // ========== 清理 ==========
    Logger::flush();
    Logger::shutdown();

    std::cout << "\n=== Example completed ===" << std::endl;
    std::cout << "Check logs/example.log for detailed logs" << std::endl;

    return 0;
}
