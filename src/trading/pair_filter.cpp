#include "trading/pair_filter.h"
#include "utils/logger.h"
#include <algorithm>
#include <set>
#include <random>

namespace crypto_trading {
namespace trading {

using json = nlohmann::json;

// ========== StaticPairListFilter ==========

StaticPairListFilter::StaticPairListFilter(const std::vector<std::string>& whitelist)
    : whitelist_(whitelist)
{
}

std::vector<std::string> StaticPairListFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    if (whitelist_.empty())
    {
        return pairs;
    }

    std::set<std::string> whitelist_set(whitelist_.begin(), whitelist_.end());
    std::vector<std::string> result;

    for (const auto& pair : pairs)
    {
        if (whitelist_set.count(pair) > 0)
        {
            result.push_back(pair);
        }
    }

    LOG_DEBUG("{}: Filtered {} pairs to {} pairs", getName(), pairs.size(), result.size());
    return result;
}

void StaticPairListFilter::configure(const json& config)
{
    if (config.contains("whitelist") && config["whitelist"].is_array())
    {
        whitelist_ = config["whitelist"].get<std::vector<std::string>>();
    }
}

// ========== VolumePairListFilter ==========

std::vector<std::string> VolumePairListFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& tickers)
{
    // 创建带ticker数据的pair列表
    struct PairWithValue
    {
        std::string pair;
        double value;
    };

    std::vector<PairWithValue> pairs_with_value;

    for (const auto& pair : pairs)
    {
        auto it = tickers.find(pair);
        if (it == tickers.end())
        {
            continue;  // 没有ticker数据，跳过
        }

        double value = 0.0;
        switch (sort_key_)
        {
            case SortKey::QuoteVolume:
                value = it->second.quote_volume_24h;
                break;
            case SortKey::Volume:
                value = it->second.volume_24h;
                break;
            case SortKey::PriceChange:
                value = std::abs(it->second.price_change_percent_24h);
                break;
            case SortKey::Volatility:
                value = it->second.getVolatility();
                break;
        }

        // 检查最小值
        if (value < min_value_)
        {
            continue;
        }

        pairs_with_value.push_back({pair, value});
    }

    // 按value降序排序
    std::sort(pairs_with_value.begin(), pairs_with_value.end(),
              [](const PairWithValue& a, const PairWithValue& b) {
                  return a.value > b.value;
              });

    // 取前N个
    std::vector<std::string> result;
    int count = std::min(number_assets_, static_cast<int>(pairs_with_value.size()));
    for (int i = 0; i < count; ++i)
    {
        result.push_back(pairs_with_value[i].pair);
    }

    LOG_INFO("{}: Filtered {} pairs to top {} by volume", getName(), pairs.size(), result.size());
    return result;
}

void VolumePairListFilter::configure(const json& config)
{
    if (config.contains("number_assets"))
    {
        number_assets_ = config["number_assets"].get<int>();
    }

    if (config.contains("min_value"))
    {
        min_value_ = config["min_value"].get<double>();
    }

    if (config.contains("refresh_period"))
    {
        refresh_period_ = config["refresh_period"].get<int>();
    }

    if (config.contains("sort_key"))
    {
        std::string key = config["sort_key"].get<std::string>();
        if (key == "quoteVolume") sort_key_ = SortKey::QuoteVolume;
        else if (key == "volume") sort_key_ = SortKey::Volume;
        else if (key == "priceChange") sort_key_ = SortKey::PriceChange;
        else if (key == "volatility") sort_key_ = SortKey::Volatility;
    }
}

// ========== SpreadFilter ==========

std::vector<std::string> SpreadFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& tickers)
{
    std::vector<std::string> result;

    for (const auto& pair : pairs)
    {
        auto it = tickers.find(pair);
        if (it == tickers.end())
        {
            continue;
        }

        double spread_ratio = it->second.getSpreadRatio();
        if (spread_ratio <= max_spread_ratio_)
        {
            result.push_back(pair);
        }
        else
        {
            LOG_DEBUG("{}: Filtered out {} (spread ratio: {:.4f}%)",
                     getName(), pair, spread_ratio * 100);
        }
    }

    LOG_INFO("{}: Filtered {} pairs to {} (max spread: {:.2f}%)",
             getName(), pairs.size(), result.size(), max_spread_ratio_ * 100);
    return result;
}

void SpreadFilter::configure(const json& config)
{
    if (config.contains("max_spread_ratio"))
    {
        max_spread_ratio_ = config["max_spread_ratio"].get<double>();
    }
}

// ========== BlacklistFilter ==========

BlacklistFilter::BlacklistFilter(const std::vector<std::string>& blacklist)
    : blacklist_(blacklist)
{
}

std::vector<std::string> BlacklistFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    if (blacklist_.empty())
    {
        return pairs;
    }

    std::set<std::string> blacklist_set(blacklist_.begin(), blacklist_.end());
    std::vector<std::string> result;

    for (const auto& pair : pairs)
    {
        if (blacklist_set.count(pair) == 0)
        {
            result.push_back(pair);
        }
        else
        {
            LOG_DEBUG("{}: Filtered out blacklisted pair: {}", getName(), pair);
        }
    }

    LOG_INFO("{}: Filtered {} pairs to {} (removed {} blacklisted)",
             getName(), pairs.size(), result.size(), pairs.size() - result.size());
    return result;
}

void BlacklistFilter::configure(const json& config)
{
    if (config.contains("blacklist") && config["blacklist"].is_array())
    {
        blacklist_ = config["blacklist"].get<std::vector<std::string>>();
    }
}

// ========== PriceFilter ==========

std::vector<std::string> PriceFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& tickers)
{
    std::vector<std::string> result;

    for (const auto& pair : pairs)
    {
        auto it = tickers.find(pair);
        if (it == tickers.end())
        {
            continue;
        }

        double price = it->second.last_price;
        if (price >= min_price_ && price <= max_price_)
        {
            result.push_back(pair);
        }
    }

    LOG_INFO("{}: Filtered {} pairs to {} (price range: {}-{})",
             getName(), pairs.size(), result.size(), min_price_, max_price_);
    return result;
}

void PriceFilter::configure(const json& config)
{
    if (config.contains("min_price"))
    {
        min_price_ = config["min_price"].get<double>();
    }

    if (config.contains("max_price"))
    {
        max_price_ = config["max_price"].get<double>();
    }
}

// ========== VolatilityFilter ==========

std::vector<std::string> VolatilityFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& tickers)
{
    std::vector<std::string> result;

    for (const auto& pair : pairs)
    {
        auto it = tickers.find(pair);
        if (it == tickers.end())
        {
            continue;
        }

        double volatility = it->second.getVolatility();
        if (volatility >= min_volatility_ && volatility <= max_volatility_)
        {
            result.push_back(pair);
        }
    }

    LOG_INFO("{}: Filtered {} pairs to {} (volatility range: {:.2f}%-{:.2f}%)",
             getName(), pairs.size(), result.size(),
             min_volatility_ * 100, max_volatility_ * 100);
    return result;
}

void VolatilityFilter::configure(const json& config)
{
    if (config.contains("min_volatility"))
    {
        min_volatility_ = config["min_volatility"].get<double>();
    }

    if (config.contains("max_volatility"))
    {
        max_volatility_ = config["max_volatility"].get<double>();
    }
}

// ========== AgeFilter ==========

std::vector<std::string> AgeFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    if (!market_provider_)
    {
        LOG_WARN("{}: No market provider set, returning all pairs", getName());
        return pairs;
    }

    auto markets = market_provider_();
    std::map<std::string, MarketInfo> market_map;
    for (const auto& market : markets)
    {
        market_map[market.symbol] = market;
    }

    std::vector<std::string> result;
    auto now = std::chrono::system_clock::now();

    for (const auto& pair : pairs)
    {
        auto it = market_map.find(pair);
        if (it == market_map.end())
        {
            continue;
        }

        auto days_listed = std::chrono::duration_cast<std::chrono::hours>(
            now - it->second.listed_date).count() / 24;

        if (days_listed >= min_days_listed_)
        {
            result.push_back(pair);
        }
        else
        {
            LOG_DEBUG("{}: Filtered out {} (listed {} days < {} days required)",
                     getName(), pair, days_listed, min_days_listed_);
        }
    }

    LOG_INFO("{}: Filtered {} pairs to {} (min {} days listed)",
             getName(), pairs.size(), result.size(), min_days_listed_);
    return result;
}

void AgeFilter::configure(const json& config)
{
    if (config.contains("min_days_listed"))
    {
        min_days_listed_ = config["min_days_listed"].get<int>();
    }
}

// ========== OffsetFilter ==========

std::vector<std::string> OffsetFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    std::vector<std::string> result;

    int start = std::min(offset_, static_cast<int>(pairs.size()));
    int end = pairs.size();

    if (number_assets_ > 0)
    {
        end = std::min(start + number_assets_, static_cast<int>(pairs.size()));
    }

    for (int i = start; i < end; ++i)
    {
        result.push_back(pairs[i]);
    }

    LOG_INFO("{}: Filtered {} pairs to {} (offset: {}, count: {})",
             getName(), pairs.size(), result.size(), offset_, number_assets_);
    return result;
}

void OffsetFilter::configure(const json& config)
{
    if (config.contains("offset"))
    {
        offset_ = config["offset"].get<int>();
    }

    if (config.contains("number_assets"))
    {
        number_assets_ = config["number_assets"].get<int>();
    }
}

// ========== ShuffleFilter ==========

std::vector<std::string> ShuffleFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    std::vector<std::string> result = pairs;

    if (seed_ == 0)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(result.begin(), result.end(), g);
    }
    else
    {
        std::mt19937 g(seed_);
        std::shuffle(result.begin(), result.end(), g);
    }

    LOG_INFO("{}: Shuffled {} pairs (seed: {})",
             getName(), result.size(), seed_);
    return result;
}

void ShuffleFilter::configure(const json& config)
{
    if (config.contains("seed"))
    {
        seed_ = config["seed"].get<unsigned int>();
    }
}

// ========== PerformanceFilter ==========

std::vector<std::string> PerformanceFilter::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    if (!performance_provider_)
    {
        LOG_WARN("{}: No performance provider set, returning all pairs", getName());
        return pairs;
    }

    auto performance_map = performance_provider_();
    std::vector<std::string> result;

    for (const auto& pair : pairs)
    {
        auto it = performance_map.find(pair);
        if (it == performance_map.end())
        {
            // 没有性能数据，保留
            result.push_back(pair);
            continue;
        }

        if (it->second >= min_profit_)
        {
            result.push_back(pair);
        }
        else
        {
            LOG_DEBUG("{}: Filtered out {} (profit: {:.2f}% < {:.2f}%)",
                     getName(), pair, it->second * 100, min_profit_ * 100);
        }
    }

    LOG_INFO("{}: Filtered {} pairs to {} (min profit: {:.2f}%)",
             getName(), pairs.size(), result.size(), min_profit_ * 100);
    return result;
}

void PerformanceFilter::configure(const json& config)
{
    if (config.contains("min_profit"))
    {
        min_profit_ = config["min_profit"].get<double>();
    }
}

// ========== ProducerPairList ==========

std::vector<std::string> ProducerPairList::filter(
    const std::vector<std::string>& /* pairs */,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    if (!remote_provider_)
    {
        LOG_ERROR("{}: No remote pair provider set", getName());
        return {};
    }

    auto result = remote_provider_();

    LOG_INFO("{}: Received {} pairs from producer '{}'",
             getName(), result.size(), producer_name_);
    return result;
}

void ProducerPairList::configure(const json& config)
{
    if (config.contains("producer_name"))
    {
        producer_name_ = config["producer_name"].get<std::string>();
    }
}

// ========== MarketCapPairList ==========

std::vector<std::string> MarketCapPairList::filter(
    const std::vector<std::string>& pairs,
    const std::map<std::string, TickerInfo>& /* tickers */)
{
    if (!market_provider_)
    {
        LOG_WARN("{}: No market provider set, returning all pairs", getName());
        return pairs;
    }

    auto markets = market_provider_();

    // 创建市场映射
    std::map<std::string, MarketInfo> market_map;
    for (const auto& market : markets)
    {
        market_map[market.symbol] = market;
    }

    // 筛选有市值数据的交易对
    struct PairWithMarketCap
    {
        std::string pair;
        double market_cap;
        int rank;
    };

    std::vector<PairWithMarketCap> pairs_with_cap;

    for (const auto& pair : pairs)
    {
        auto it = market_map.find(pair);
        if (it == market_map.end())
        {
            continue;
        }

        // 过滤市值排名
        if (it->second.market_cap_rank > 0 && it->second.market_cap_rank <= max_rank_)
        {
            pairs_with_cap.push_back({
                pair,
                it->second.market_cap,
                it->second.market_cap_rank
            });
        }
    }

    // 按市值降序排序
    std::sort(pairs_with_cap.begin(), pairs_with_cap.end(),
              [](const PairWithMarketCap& a, const PairWithMarketCap& b) {
                  return a.market_cap > b.market_cap;
              });

    // 取前N个
    std::vector<std::string> result;
    int count = std::min(number_assets_, static_cast<int>(pairs_with_cap.size()));
    for (int i = 0; i < count; ++i)
    {
        result.push_back(pairs_with_cap[i].pair);
    }

    LOG_INFO("{}: Filtered {} pairs to top {} by market cap",
             getName(), pairs.size(), result.size());
    return result;
}

void MarketCapPairList::configure(const json& config)
{
    if (config.contains("number_assets"))
    {
        number_assets_ = config["number_assets"].get<int>();
    }

    if (config.contains("max_rank"))
    {
        max_rank_ = config["max_rank"].get<int>();
    }
}

// ========== PairFilterFactory ==========

std::shared_ptr<PairFilter> PairFilterFactory::create(const std::string& method)
{
    if (method == "StaticPairList")
    {
        return std::make_shared<StaticPairListFilter>();
    }
    else if (method == "VolumePairList")
    {
        return std::make_shared<VolumePairListFilter>();
    }
    else if (method == "SpreadFilter")
    {
        return std::make_shared<SpreadFilter>();
    }
    else if (method == "BlacklistFilter")
    {
        return std::make_shared<BlacklistFilter>();
    }
    else if (method == "PriceFilter")
    {
        return std::make_shared<PriceFilter>();
    }
    else if (method == "VolatilityFilter")
    {
        return std::make_shared<VolatilityFilter>();
    }
    else if (method == "AgeFilter")
    {
        return std::make_shared<AgeFilter>();
    }
    else if (method == "OffsetFilter")
    {
        return std::make_shared<OffsetFilter>();
    }
    else if (method == "ShuffleFilter")
    {
        return std::make_shared<ShuffleFilter>();
    }
    else if (method == "PerformanceFilter")
    {
        return std::make_shared<PerformanceFilter>();
    }
    else if (method == "ProducerPairList")
    {
        return std::make_shared<ProducerPairList>();
    }
    else if (method == "MarketCapPairList")
    {
        return std::make_shared<MarketCapPairList>();
    }

    LOG_ERROR("Unknown pair filter method: {}", method);
    return nullptr;
}

std::shared_ptr<PairFilter> PairFilterFactory::createFromConfig(const json& config)
{
    if (!config.contains("method"))
    {
        LOG_ERROR("Filter config missing 'method' field");
        return nullptr;
    }

    std::string method = config["method"].get<std::string>();
    auto filter = create(method);

    if (filter)
    {
        filter->configure(config);
    }

    return filter;
}

} // namespace trading
} // namespace crypto_trading
