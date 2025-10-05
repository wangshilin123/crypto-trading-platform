#include "trading/pair_list_manager.h"
#include "utils/time_utils.h"

namespace crypto_trading {
namespace trading {

PairListManager::PairListManager()
{
    last_refresh_time_ = std::chrono::system_clock::now();
}

PairListManager::~PairListManager()
{
    stopAutoRefresh();
}

void PairListManager::loadFromConfig(const json& config)
{
    std::lock_guard<std::mutex> lock(mutex_);

    clearFilters();

    // 加载过滤器配置
    if (config.contains("pairlist_filters") && config["pairlist_filters"].is_array())
    {
        for (const auto& filter_config : config["pairlist_filters"])
        {
            auto filter = PairFilterFactory::createFromConfig(filter_config);
            if (filter)
            {
                filters_.push_back(filter);
                LOG_INFO("Loaded filter: {}", filter->getName());
            }
        }
    }

    // 加载刷新间隔
    if (config.contains("refresh_period"))
    {
        refresh_interval_ = config["refresh_period"].get<int>();
    }

    LOG_INFO("PairListManager configured with {} filters", filters_.size());
}

void PairListManager::addFilter(std::shared_ptr<PairFilter> filter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    filters_.push_back(filter);
    LOG_INFO("Added filter: {}", filter->getName());
}

void PairListManager::clearFilters()
{
    filters_.clear();
}

void PairListManager::refresh()
{
    auto start_time = utils::TimeUtils::now();

    // 获取所有可用市场
    std::vector<std::string> all_pairs;

    if (market_provider_)
    {
        auto markets = market_provider_();
        for (const auto& market : markets)
        {
            if (market.active)
            {
                all_pairs.push_back(market.symbol);
            }
        }
    }

    if (all_pairs.empty())
    {
        LOG_WARN("No pairs available from market provider");
        return;
    }

    LOG_INFO("Starting pair list refresh with {} initial pairs", all_pairs.size());

    // 获取ticker数据
    std::map<std::string, TickerInfo> tickers;
    if (ticker_provider_)
    {
        tickers = ticker_provider_();
        LOG_DEBUG("Fetched {} ticker data", tickers.size());
    }

    // 应用过滤器链
    std::vector<std::string> filtered_pairs = all_pairs;

    for (const auto& filter : filters_)
    {
        if (filtered_pairs.empty())
        {
            break;
        }

        filtered_pairs = filter->filter(filtered_pairs, tickers);
        filter_count_++;
    }

    // 更新交易对列表
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pairs_ = filtered_pairs;
        last_refresh_time_ = std::chrono::system_clock::now();
    }

    refresh_count_++;

    auto duration = utils::TimeUtils::durationMs(start_time, utils::TimeUtils::now());
    LOG_INFO("Pair list refreshed: {} pairs (took {}ms)", filtered_pairs.size(), duration);
}

std::vector<std::string> PairListManager::getPairs() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pairs_;
}

size_t PairListManager::getPairCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pairs_.size();
}

bool PairListManager::hasPair(const std::string& pair) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return std::find(pairs_.begin(), pairs_.end(), pair) != pairs_.end();
}

void PairListManager::startAutoRefresh(int interval_seconds)
{
    if (auto_refresh_running_)
    {
        LOG_WARN("Auto refresh already running");
        return;
    }

    refresh_interval_ = interval_seconds;
    auto_refresh_running_ = true;

    auto_refresh_thread_ = std::make_unique<std::thread>([this]() {
        autoRefreshLoop();
    });

    LOG_INFO("Started auto refresh (interval: {}s)", interval_seconds);
}

void PairListManager::stopAutoRefresh()
{
    if (!auto_refresh_running_)
    {
        return;
    }

    auto_refresh_running_ = false;

    if (auto_refresh_thread_ && auto_refresh_thread_->joinable())
    {
        auto_refresh_thread_->join();
    }

    LOG_INFO("Stopped auto refresh");
}

void PairListManager::autoRefreshLoop()
{
    while (auto_refresh_running_)
    {
        try
        {
            refresh();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Error in auto refresh: {}", e.what());
        }

        // 等待指定间隔
        for (int i = 0; i < refresh_interval_ && auto_refresh_running_; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

PairListManager::json PairListManager::getStatistics() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    json stats;
    stats["pair_count"] = pairs_.size();
    stats["filter_count"] = filters_.size();
    stats["refresh_count"] = refresh_count_.load();
    stats["total_filter_executions"] = filter_count_.load();
    stats["last_refresh_time"] = utils::TimeUtils::toIso8601(last_refresh_time_);
    stats["auto_refresh_running"] = auto_refresh_running_.load();
    stats["refresh_interval"] = refresh_interval_;

    // 过滤器名称列表
    json filter_names = json::array();
    for (const auto& filter : filters_)
    {
        filter_names.push_back(filter->getName());
    }
    stats["filters"] = filter_names;

    return stats;
}

} // namespace trading
} // namespace crypto_trading
