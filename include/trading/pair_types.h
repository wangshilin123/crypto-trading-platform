#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace crypto_trading {
namespace trading {

/**
 * @brief 交易对类型
 */
enum class PairType
{
    Spot,       // 现货
    Futures,    // 期货
    Margin      // 杠杆
};

/**
 * @brief 市场信息
 */
struct MarketInfo
{
    std::string symbol;              // 交易对符号 (e.g., "BTCUSDT", "BTC/USDT")
    std::string base;                // 基础货币 (e.g., "BTC")
    std::string quote;               // 计价货币 (e.g., "USDT")
    PairType type;                   // 交易对类型
    bool active;                     // 是否活跃

    // 交易限制
    double min_amount;               // 最小交易数量
    double max_amount;               // 最大交易数量
    double min_price;                // 最小价格
    double max_price;                // 最大价格
    double min_cost;                 // 最小成本（价格*数量）

    // 精度
    int amount_precision;            // 数量精度
    int price_precision;             // 价格精度

    // 费率
    double maker_fee;                // Maker手续费
    double taker_fee;                // Taker手续费

    // 上市信息
    std::chrono::system_clock::time_point listed_date;  // 上市时间

    // 市值信息（用于MarketCapPairList）
    double market_cap;               // 市值
    int market_cap_rank;             // 市值排名
};

/**
 * @brief Ticker数据
 */
struct TickerInfo
{
    std::string symbol;
    double last_price;               // 最新价格
    double bid;                      // 买一价
    double ask;                      // 卖一价
    double high_24h;                 // 24小时最高价
    double low_24h;                  // 24小时最低价
    double volume_24h;               // 24小时成交量（基础货币）
    double quote_volume_24h;         // 24小时成交额（计价货币）
    double price_change_percent_24h; // 24小时价格变化百分比
    std::chrono::system_clock::time_point timestamp;

    // 计算价差
    double getSpread() const { return ask - bid; }
    double getSpreadRatio() const { return (ask - bid) / ask; }

    // 计算波动率（简化版本）
    double getVolatility() const { return (high_24h - low_24h) / last_price; }
};

/**
 * @brief 排序键
 */
enum class SortKey
{
    QuoteVolume,    // 按计价货币成交量
    Volume,         // 按基础货币成交量
    PriceChange,    // 按价格变化
    Volatility      // 按波动率
};

} // namespace trading
} // namespace crypto_trading
