#pragma once

#include "pair_filter.h"
#include "pair_types.h"
#include "utils/logger.h"
#include "utils/config.h"
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>

namespace crypto_trading {
namespace trading {

/**
 * @brief 交易对列表管理器
 *
 * @details
 * - 支持动态生成交易对列表
 * - 支持多级过滤器链
 * - 支持定时刷新
 * - 线程安全
 */
class PairListManager
{
public:
    using json = nlohmann::json;
    using TickerProvider = std::function<std::map<std::string, TickerInfo>()>;
    using MarketProvider = std::function<std::vector<MarketInfo>()>;

    PairListManager();
    ~PairListManager();

    /**
     * @brief 从配置加载
     * @param config 配置对象
     */
    void loadFromConfig(const json& config);

    /**
     * @brief 设置Ticker数据提供者
     */
    void setTickerProvider(TickerProvider provider) { ticker_provider_ = provider; }

    /**
     * @brief 设置市场信息提供者
     */
    void setMarketProvider(MarketProvider provider) { market_provider_ = provider; }

    /**
     * @brief 添加过滤器
     */
    void addFilter(std::shared_ptr<PairFilter> filter);

    /**
     * @brief 清空过滤器
     */
    void clearFilters();

    /**
     * @brief 刷新交易对列表
     */
    void refresh();

    /**
     * @brief 获取当前交易对列表
     */
    std::vector<std::string> getPairs() const;

    /**
     * @brief 获取交易对数量
     */
    size_t getPairCount() const;

    /**
     * @brief 检查交易对是否在列表中
     */
    bool hasPair(const std::string& pair) const;

    /**
     * @brief 启动自动刷新
     * @param interval_seconds 刷新间隔（秒）
     */
    void startAutoRefresh(int interval_seconds);

    /**
     * @brief 停止自动刷新
     */
    void stopAutoRefresh();

    /**
     * @brief 是否正在自动刷新
     */
    bool isAutoRefreshing() const { return auto_refresh_running_; }

    /**
     * @brief 获取上次刷新时间
     */
    std::chrono::system_clock::time_point getLastRefreshTime() const { return last_refresh_time_; }

    /**
     * @brief 获取统计信息
     */
    json getStatistics() const;

private:
    void autoRefreshLoop();

    mutable std::mutex mutex_;
    std::vector<std::string> pairs_;
    std::vector<std::shared_ptr<PairFilter>> filters_;

    TickerProvider ticker_provider_;
    MarketProvider market_provider_;

    std::chrono::system_clock::time_point last_refresh_time_;

    // 自动刷新
    std::atomic<bool> auto_refresh_running_{false};
    std::unique_ptr<std::thread> auto_refresh_thread_;
    int refresh_interval_ = 1800;  // 默认30分钟

    // 统计
    std::atomic<int> refresh_count_{0};
    std::atomic<int> filter_count_{0};
};

} // namespace trading
} // namespace crypto_trading
