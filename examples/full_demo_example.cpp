#include "utils/logger.h"
#include "utils/config.h"
#include "utils/time_utils.h"
#include "utils/crypto_utils.h"
#include <iostream>
#include <thread>
#include <vector>

using namespace crypto_trading::utils;

/**
 * @brief 完整的交易平台基础设施演示
 *
 * 展示如何在实际应用中组合使用所有工具
 */

class TradingApplication
{
public:
    TradingApplication(const std::string& config_file)
    {
        initializeLogger();
        loadConfiguration(config_file);
        LOG_INFO("Trading application initialized");
    }

    void run()
    {
        LOG_INFO("Starting trading application...");

        // 模拟交易流程
        simulateMarketDataProcessing();
        simulateOrderPlacement();
        simulatePerformanceMonitoring();

        LOG_INFO("Trading application stopped");
    }

    ~TradingApplication()
    {
        Logger::shutdown();
    }

private:
    void initializeLogger()
    {
        Logger::Config log_config;
        log_config.name = "trading_platform";
        log_config.file_path = "logs/trading_platform.log";
        log_config.max_file_size = 50 * 1024 * 1024;  // 50MB
        log_config.max_files = 20;
        log_config.console_level = Logger::Level::Info;
        log_config.file_level = Logger::Level::Debug;
        log_config.async_mode = true;
        log_config.async_queue_size = 16384;

        Logger::initialize(log_config);
    }

    void loadConfiguration(const std::string& config_file)
    {
        auto& config = ConfigManager::getInstance();

        // 尝试从文件加载，失败则使用默认配置
        if (!config.loadFromFile(config_file))
        {
            LOG_WARN("Config file {} not found, using default config", config_file);
            useDefaultConfig();
        }
        else
        {
            LOG_INFO("Config loaded from {}", config_file);
        }

        // 验证必需配置
        std::vector<std::string> required_keys = {
            "exchange.name",
            "exchange.endpoints.rest",
            "risk.max_order_value_usdt"
        };

        if (!config.validate(required_keys))
        {
            LOG_ERROR("Configuration validation failed!");
            throw std::runtime_error("Invalid configuration");
        }

        printConfig();
    }

    void useDefaultConfig()
    {
        auto& config = ConfigManager::getInstance();

        std::string default_config = R"({
            "exchange": {
                "name": "binance",
                "endpoints": {
                    "rest": "https://testnet.binance.vision",
                    "websocket": "wss://testnet.binance.vision/ws"
                },
                "timeout_ms": 5000,
                "rate_limit": {
                    "orders_per_second": 10,
                    "requests_per_minute": 1200
                }
            },
            "trading": {
                "default_symbols": ["BTCUSDT", "ETHUSDT"],
                "order_timeout_ms": 30000
            },
            "risk": {
                "max_order_value_usdt": 1000,
                "max_position_value_usdt": 5000,
                "max_daily_loss_usdt": 500
            },
            "logging": {
                "level": "info",
                "enable_performance_logs": true
            }
        })";

        config.loadFromString(default_config);
    }

    void printConfig()
    {
        auto& config = ConfigManager::getInstance();

        LOG_INFO("=== Configuration Summary ===");
        LOG_INFO("Exchange: {}", config.get<std::string>("exchange.name"));
        LOG_INFO("REST Endpoint: {}", config.get<std::string>("exchange.endpoints.rest"));
        LOG_INFO("Max Order Value: ${}", config.get<double>("risk.max_order_value_usdt"));
        LOG_INFO("Max Position Value: ${}", config.get<double>("risk.max_position_value_usdt"));
    }

    void simulateMarketDataProcessing()
    {
        LOG_INFO("=== Market Data Processing ===");

        auto market_logger = Logger::get("market_data");

        // 模拟处理市场数据
        for (int i = 0; i < 5; ++i)
        {
            ScopedTimer timer("process_market_data");

            // 模拟接收行情数据
            double btc_price = 50000.0 + (rand() % 1000 - 500);
            double eth_price = 3000.0 + (rand() % 100 - 50);

            market_logger->info("Market update - BTC: ${:.2f}, ETH: ${:.2f}",
                              btc_price, eth_price);

            // 模拟数据处理延迟
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void simulateOrderPlacement()
    {
        LOG_INFO("=== Order Placement Simulation ===");

        auto order_logger = Logger::get("order_manager");
        auto& config = ConfigManager::getInstance();

        // 获取配置参数
        double max_order_value = config.get<double>("risk.max_order_value_usdt");
        std::string exchange = config.get<std::string>("exchange.name");

        // 模拟下单
        struct Order
        {
            std::string symbol;
            std::string side;
            std::string type;
            double price;
            double quantity;
        };

        std::vector<Order> orders = {
            {"BTCUSDT", "BUY", "LIMIT", 50000.0, 0.01},
            {"ETHUSDT", "SELL", "LIMIT", 3000.0, 0.5},
            {"BNBUSDT", "BUY", "MARKET", 0.0, 10.0}
        };

        for (const auto& order : orders)
        {
            ScopedTimer timer(StringUtils::format("place_order_%s", order.symbol.c_str()));

            // 风控检查
            double order_value = order.price * order.quantity;
            if (order.type != "MARKET" && order_value > max_order_value)
            {
                order_logger->error("Order rejected: value ${:.2f} exceeds limit ${:.2f}",
                                  order_value, max_order_value);
                continue;
            }

            // 构建签名
            std::string timestamp = std::to_string(TimeUtils::nowMs());
            std::vector<std::string> params;
            params.push_back("symbol=" + order.symbol);
            params.push_back("side=" + order.side);
            params.push_back("type=" + order.type);
            params.push_back("quantity=" + std::to_string(order.quantity));
            if (order.price > 0)
            {
                params.push_back("price=" + std::to_string(order.price));
            }
            params.push_back("timestamp=" + timestamp);

            std::string query_string = StringUtils::join(params, "&");
            std::string signature = CryptoUtils::hmacSha256(query_string, "test_secret");

            order_logger->info("Order placed: {} {} {} @ {:.2f} x {:.4f}",
                             order.symbol, order.side, order.type,
                             order.price, order.quantity);
            order_logger->debug("Query: {}", query_string);
            order_logger->debug("Signature: {}", signature.substr(0, 16) + "...");

            // 模拟网络延迟
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    }

    void simulatePerformanceMonitoring()
    {
        LOG_INFO("=== Performance Monitoring ===");

        auto perf_logger = Logger::get("performance");

        // 监控指标
        struct Metrics
        {
            int64_t start_time;
            int total_orders;
            int successful_orders;
            int failed_orders;
            double total_volume_usdt;
        };

        Metrics metrics;
        metrics.start_time = TimeUtils::nowMs();
        metrics.total_orders = 15;
        metrics.successful_orders = 13;
        metrics.failed_orders = 2;
        metrics.total_volume_usdt = 25000.50;

        int64_t uptime_ms = TimeUtils::nowMs() - metrics.start_time;
        double uptime_sec = uptime_ms / 1000.0;

        perf_logger->info("=== Performance Report ===");
        perf_logger->info("Uptime: {:.2f} seconds", uptime_sec);
        perf_logger->info("Total Orders: {}", metrics.total_orders);
        perf_logger->info("Success Rate: {:.1f}%",
                        100.0 * metrics.successful_orders / metrics.total_orders);
        perf_logger->info("Total Volume: ${:.2f}", metrics.total_volume_usdt);
        perf_logger->info("Avg Orders/sec: {:.2f}",
                        metrics.total_orders / uptime_sec);

        // 保存运行时指标
        auto& config = ConfigManager::getInstance();
        config.set("runtime.metrics.total_orders", metrics.total_orders);
        config.set("runtime.metrics.success_rate",
                  100.0 * metrics.successful_orders / metrics.total_orders);
        config.set("runtime.metrics.total_volume_usdt", metrics.total_volume_usdt);
        config.set("runtime.metrics.last_update", TimeUtils::toIso8601(TimeUtils::now()));

        perf_logger->info("Metrics saved to configuration");
    }
};

int main(int argc, char* argv[])
{
    try
    {
        std::string config_file = "config/config.json";
        if (argc > 1)
        {
            config_file = argv[1];
        }

        std::cout << "=== Crypto Trading Platform Demo ===" << std::endl;
        std::cout << "Using config file: " << config_file << std::endl;
        std::cout << std::endl;

        TradingApplication app(config_file);
        app.run();

        std::cout << std::endl;
        std::cout << "=== Demo Completed Successfully ===" << std::endl;
        std::cout << "Check logs/trading_platform.log for detailed logs" << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
