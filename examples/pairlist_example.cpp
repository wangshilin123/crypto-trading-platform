#include "trading/pair_list_manager.h"
#include "utils/logger.h"
#include "utils/config.h"
#include <iostream>
#include <random>

using namespace crypto_trading::trading;
using namespace crypto_trading::utils;

/**
 * @brief 模拟的Ticker数据提供者
 */
std::map<std::string, TickerInfo> getMockTickers()
{
    std::map<std::string, TickerInfo> tickers;

    std::vector<std::string> pairs = {
        "BTC/USDT", "ETH/USDT", "BNB/USDT", "XRP/USDT", "ADA/USDT",
        "SOL/USDT", "DOT/USDT", "DOGE/USDT", "AVAX/USDT", "SHIB/USDT",
        "MATIC/USDT", "UNI/USDT", "LINK/USDT", "ATOM/USDT", "LTC/USDT",
        "BCH/USDT", "FIL/USDT", "ETC/USDT", "XLM/USDT", "ALGO/USDT"
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> price_dis(10.0, 50000.0);
    std::uniform_real_distribution<> volume_dis(1000000.0, 100000000.0);
    std::uniform_real_distribution<> change_dis(-10.0, 10.0);

    for (const auto& pair : pairs)
    {
        TickerInfo ticker;
        ticker.symbol = pair;
        ticker.last_price = price_dis(gen);
        ticker.bid = ticker.last_price * 0.999;
        ticker.ask = ticker.last_price * 1.001;
        ticker.high_24h = ticker.last_price * 1.05;
        ticker.low_24h = ticker.last_price * 0.95;
        ticker.volume_24h = volume_dis(gen) / ticker.last_price;
        ticker.quote_volume_24h = volume_dis(gen);
        ticker.price_change_percent_24h = change_dis(gen);
        ticker.timestamp = std::chrono::system_clock::now();

        tickers[pair] = ticker;
    }

    return tickers;
}

/**
 * @brief 模拟的市场信息提供者
 */
std::vector<MarketInfo> getMockMarkets()
{
    std::vector<std::string> pairs = {
        "BTC/USDT", "ETH/USDT", "BNB/USDT", "XRP/USDT", "ADA/USDT",
        "SOL/USDT", "DOT/USDT", "DOGE/USDT", "AVAX/USDT", "SHIB/USDT",
        "MATIC/USDT", "UNI/USDT", "LINK/USDT", "ATOM/USDT", "LTC/USDT",
        "BCH/USDT", "FIL/USDT", "ETC/USDT", "XLM/USDT", "ALGO/USDT"
    };

    std::vector<MarketInfo> markets;
    for (const auto& pair : pairs)
    {
        MarketInfo market;
        market.symbol = pair;

        // 解析交易对
        size_t pos = pair.find('/');
        market.base = pair.substr(0, pos);
        market.quote = pair.substr(pos + 1);

        market.type = PairType::Spot;
        market.active = true;
        market.min_amount = 0.001;
        market.max_amount = 10000.0;
        market.min_price = 0.00001;
        market.max_price = 1000000.0;
        market.min_cost = 10.0;
        market.amount_precision = 8;
        market.price_precision = 2;
        market.maker_fee = 0.001;
        market.taker_fee = 0.001;

        markets.push_back(market);
    }

    return markets;
}

void demonstrateStaticWhitelist()
{
    std::cout << "\n=== 示例 1: 静态白名单 ===" << std::endl;

    PairListManager manager;

    // 设置数据提供者
    manager.setTickerProvider(getMockTickers);
    manager.setMarketProvider(getMockMarkets);

    // 创建静态白名单过滤器
    auto whitelist_filter = std::make_shared<StaticPairListFilter>();
    whitelist_filter->setWhitelist({"BTC/USDT", "ETH/USDT", "BNB/USDT"});

    manager.addFilter(whitelist_filter);

    // 刷新
    manager.refresh();

    // 获取结果
    auto pairs = manager.getPairs();
    std::cout << "Selected pairs (" << pairs.size() << "):" << std::endl;
    for (const auto& pair : pairs)
    {
        std::cout << "  - " << pair << std::endl;
    }
}

void demonstrateVolumeFilter()
{
    std::cout << "\n=== 示例 2: 按交易量筛选前5名 ===" << std::endl;

    PairListManager manager;
    manager.setTickerProvider(getMockTickers);
    manager.setMarketProvider(getMockMarkets);

    // 创建交易量过滤器
    auto volume_filter = std::make_shared<VolumePairListFilter>();
    nlohmann::json config = {
        {"number_assets", 5},
        {"sort_key", "quoteVolume"},
        {"min_value", 0}
    };
    volume_filter->configure(config);

    manager.addFilter(volume_filter);
    manager.refresh();

    auto pairs = manager.getPairs();
    auto tickers = getMockTickers();

    std::cout << "Top 5 by volume:" << std::endl;
    for (const auto& pair : pairs)
    {
        auto it = tickers.find(pair);
        if (it != tickers.end())
        {
            std::cout << "  - " << pair
                     << " (Volume: $" << it->second.quote_volume_24h / 1000000.0 << "M)"
                     << std::endl;
        }
    }
}

void demonstrateMultipleFilters()
{
    std::cout << "\n=== 示例 3: 多重过滤器组合 ===" << std::endl;

    PairListManager manager;
    manager.setTickerProvider(getMockTickers);
    manager.setMarketProvider(getMockMarkets);

    // 第一步：选择交易量前10
    auto volume_filter = std::make_shared<VolumePairListFilter>();
    volume_filter->configure({
        {"number_assets", 10},
        {"sort_key", "quoteVolume"}
    });
    manager.addFilter(volume_filter);

    // 第二步：过滤价差
    auto spread_filter = std::make_shared<SpreadFilter>();
    spread_filter->configure({{"max_spread_ratio", 0.005}});
    manager.addFilter(spread_filter);

    // 第三步：黑名单
    auto blacklist_filter = std::make_shared<BlacklistFilter>();
    blacklist_filter->setBlacklist({"DOGE/USDT", "SHIB/USDT"});
    manager.addFilter(blacklist_filter);

    manager.refresh();

    auto pairs = manager.getPairs();
    std::cout << "Filtered pairs (" << pairs.size() << "):" << std::endl;
    for (const auto& pair : pairs)
    {
        std::cout << "  - " << pair << std::endl;
    }

    // 显示统计信息
    auto stats = manager.getStatistics();
    std::cout << "\nStatistics:" << std::endl;
    std::cout << stats.dump(2) << std::endl;
}

void demonstrateConfigFile()
{
    std::cout << "\n=== 示例 4: 从配置文件加载 ===" << std::endl;

    // 加载配置
    auto& config = ConfigManager::getInstance();
    if (!config.loadFromFile("config/pairlist_config.json"))
    {
        std::cerr << "Failed to load config file" << std::endl;
        return;
    }

    PairListManager manager;
    manager.setTickerProvider(getMockTickers);
    manager.setMarketProvider(getMockMarkets);

    // 从配置加载过滤器
    auto pairlist_config = config.getConfig();
    manager.loadFromConfig(pairlist_config);

    manager.refresh();

    auto pairs = manager.getPairs();
    std::cout << "Pairs from config (" << pairs.size() << "):" << std::endl;
    for (const auto& pair : pairs)
    {
        std::cout << "  - " << pair << std::endl;
    }
}

void demonstrateAutoRefresh()
{
    std::cout << "\n=== 示例 5: 自动刷新 ===" << std::endl;

    PairListManager manager;
    manager.setTickerProvider(getMockTickers);
    manager.setMarketProvider(getMockMarkets);

    auto volume_filter = std::make_shared<VolumePairListFilter>();
    volume_filter->configure({
        {"number_assets", 5},
        {"sort_key", "quoteVolume"}
    });
    manager.addFilter(volume_filter);

    // 初始刷新
    manager.refresh();
    std::cout << "Initial pairs: " << manager.getPairCount() << std::endl;

    // 启动自动刷新（每5秒）
    manager.startAutoRefresh(5);
    std::cout << "Auto refresh started (5s interval)" << std::endl;

    // 运行一段时间
    for (int i = 0; i < 3; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(6));
        auto stats = manager.getStatistics();
        std::cout << "Refresh count: " << stats["refresh_count"] << std::endl;
    }

    manager.stopAutoRefresh();
    std::cout << "Auto refresh stopped" << std::endl;
}

int main()
{
    // 初始化日志
    Logger::Config log_config;
    log_config.file_path = "logs/pairlist_example.log";
    log_config.console_level = Logger::Level::Info;
    log_config.file_level = Logger::Level::Debug;
    Logger::initialize(log_config);

    std::cout << "=== 交易对配置系统示例 ===" << std::endl;

    try
    {
        demonstrateStaticWhitelist();
        demonstrateVolumeFilter();
        demonstrateMultipleFilters();
        demonstrateConfigFile();
        demonstrateAutoRefresh();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    Logger::shutdown();

    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}
