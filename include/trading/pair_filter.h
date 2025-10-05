#pragma once

#include "pair_types.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <functional>

namespace crypto_trading {
namespace trading {

/**
 * @brief 交易对过滤器接口
 */
class PairFilter
{
public:
    using json = nlohmann::json;

    virtual ~PairFilter() = default;

    /**
     * @brief 过滤交易对列表
     * @param pairs 输入的交易对列表
     * @param tickers Ticker数据映射
     * @return 过滤后的交易对列表
     */
    virtual std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) = 0;

    /**
     * @brief 获取过滤器名称
     */
    virtual std::string getName() const = 0;

    /**
     * @brief 从配置初始化
     */
    virtual void configure(const json& config) = 0;
};

/**
 * @brief 静态白名单过滤器
 */
class StaticPairListFilter : public PairFilter
{
public:
    StaticPairListFilter() = default;
    explicit StaticPairListFilter(const std::vector<std::string>& whitelist);

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "StaticPairList"; }
    void configure(const json& config) override;

    void setWhitelist(const std::vector<std::string>& whitelist) { whitelist_ = whitelist; }

private:
    std::vector<std::string> whitelist_;
};

/**
 * @brief 按交易量过滤器
 */
class VolumePairListFilter : public PairFilter
{
public:
    VolumePairListFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "VolumePairList"; }
    void configure(const json& config) override;

private:
    int number_assets_ = 20;         // 返回的交易对数量
    SortKey sort_key_ = SortKey::QuoteVolume;
    double min_value_ = 0.0;         // 最小交易量/交易额
    int refresh_period_ = 1800;      // 刷新周期（秒）
};

/**
 * @brief 价差过滤器
 */
class SpreadFilter : public PairFilter
{
public:
    SpreadFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "SpreadFilter"; }
    void configure(const json& config) override;

private:
    double max_spread_ratio_ = 0.005;  // 最大价差比例（0.5%）
};

/**
 * @brief 黑名单过滤器
 */
class BlacklistFilter : public PairFilter
{
public:
    BlacklistFilter() = default;
    explicit BlacklistFilter(const std::vector<std::string>& blacklist);

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "BlacklistFilter"; }
    void configure(const json& config) override;

    void setBlacklist(const std::vector<std::string>& blacklist) { blacklist_ = blacklist; }
    void addToBlacklist(const std::string& pair) { blacklist_.push_back(pair); }

private:
    std::vector<std::string> blacklist_;
};

/**
 * @brief 价格过滤器
 */
class PriceFilter : public PairFilter
{
public:
    PriceFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "PriceFilter"; }
    void configure(const json& config) override;

private:
    double min_price_ = 0.0;
    double max_price_ = std::numeric_limits<double>::max();
};

/**
 * @brief 波动率过滤器
 */
class VolatilityFilter : public PairFilter
{
public:
    VolatilityFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "VolatilityFilter"; }
    void configure(const json& config) override;

private:
    double min_volatility_ = 0.0;
    double max_volatility_ = std::numeric_limits<double>::max();
};

/**
 * @brief 上市时间过滤器
 */
class AgeFilter : public PairFilter
{
public:
    using MarketProvider = std::function<std::vector<MarketInfo>()>;

    AgeFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "AgeFilter"; }
    void configure(const json& config) override;

    void setMarketProvider(MarketProvider provider) { market_provider_ = provider; }

private:
    int min_days_listed_ = 10;
    MarketProvider market_provider_;
};

/**
 * @brief 偏移过滤器
 */
class OffsetFilter : public PairFilter
{
public:
    OffsetFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "OffsetFilter"; }
    void configure(const json& config) override;

private:
    int offset_ = 0;
    int number_assets_ = 0;  // 0 = 全部
};

/**
 * @brief 随机打乱过滤器
 */
class ShuffleFilter : public PairFilter
{
public:
    ShuffleFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "ShuffleFilter"; }
    void configure(const json& config) override;

private:
    unsigned int seed_ = 0;  // 0 = 使用随机种子
};

/**
 * @brief 性能过滤器
 */
class PerformanceFilter : public PairFilter
{
public:
    using PerformanceProvider = std::function<std::map<std::string, double>()>;

    PerformanceFilter() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "PerformanceFilter"; }
    void configure(const json& config) override;

    void setPerformanceProvider(PerformanceProvider provider) { performance_provider_ = provider; }

private:
    double min_profit_ = 0.0;
    PerformanceProvider performance_provider_;
};

/**
 * @brief 多实例协同（从生产者获取交易对列表）
 */
class ProducerPairList : public PairFilter
{
public:
    using RemotePairProvider = std::function<std::vector<std::string>()>;

    ProducerPairList() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "ProducerPairList"; }
    void configure(const json& config) override;

    void setRemotePairProvider(RemotePairProvider provider) { remote_provider_ = provider; }

private:
    RemotePairProvider remote_provider_;
    std::string producer_name_;
};

/**
 * @brief 按市值排序
 */
class MarketCapPairList : public PairFilter
{
public:
    using MarketProvider = std::function<std::vector<MarketInfo>()>;

    MarketCapPairList() = default;

    std::vector<std::string> filter(
        const std::vector<std::string>& pairs,
        const std::map<std::string, TickerInfo>& tickers
    ) override;

    std::string getName() const override { return "MarketCapPairList"; }
    void configure(const json& config) override;

    void setMarketProvider(MarketProvider provider) { market_provider_ = provider; }

private:
    int number_assets_ = 20;
    int max_rank_ = 100;  // 只考虑市值排名前100的币
    MarketProvider market_provider_;
};

/**
 * @brief 过滤器工厂
 */
class PairFilterFactory
{
public:
    using json = nlohmann::json;

    static std::shared_ptr<PairFilter> create(const std::string& method);
    static std::shared_ptr<PairFilter> createFromConfig(const json& config);
};

} // namespace trading
} // namespace crypto_trading
