# 加密货币交易平台 - 需求文档

## 1. 项目概述

### 1.1 项目目标
构建一个高性能、可扩展的C++加密货币交易平台，支持多交易所对接，实现自动化交易策略执行、风险控制和实时监控。

### 1.2 核心特性
- **多交易所支持**：首期支持币安(Binance)，后续扩展至OKX、Bybit、Coinbase等
- **高性能交易引擎**：低延迟订单执行，支持现货和期货交易
- **策略框架**：支持自定义交易策略的开发和回测
- **风险管理**：实时风控、仓位管理、止损止盈
- **数据管理**：市场数据订阅、历史数据存储、实时行情推送

### 1.3 技术栈
- **开发语言**：C++17/20
- **网络库**：Boost.Beast (WebSocket/HTTP), Boost.Asio
- **JSON处理**：nlohmann/json
- **加密库**：OpenSSL (HMAC签名)
- **数据库**：Redis (缓存), PostgreSQL/TimescaleDB (历史数据)
- **日志系统**：spdlog
- **构建工具**：CMake
- **测试框架**：Google Test

---

## 2. 代码规范

### 2.1 命名规范

#### 文件命名
- 头文件：`snake_case.h` 或 `snake_case.hpp`
- 源文件：`snake_case.cpp`
- 示例：`binance_client.h`, `order_manager.cpp`

#### 类和结构体
- **类名**：`PascalCase`
- **成员变量**：`snake_case_` (带下划线后缀)
- **成员函数**：`camelCase` 或 `snake_case`
- 示例：
```cpp
class OrderManager {
private:
    std::string api_key_;
    int connection_timeout_;

public:
    void submitOrder();
    bool cancelOrder(const std::string& order_id);
};
```

#### 常量和枚举
- **常量**：`kPascalCase` 或 `UPPER_SNAKE_CASE`
- **枚举类**：`PascalCase`，枚举值 `PascalCase`
- 示例：
```cpp
constexpr int kMaxRetries = 3;
const std::string kBinanceBaseUrl = "https://api.binance.com";

enum class OrderSide {
    Buy,
    Sell
};

enum class OrderType {
    Limit,
    Market,
    StopLoss,
    StopLossLimit
};
```

#### 命名空间
- 全小写，使用下划线分隔：`crypto_trading`, `exchange_api`

### 2.2 版本控制规范

#### Git工作流程
- **分支管理**：main/master为主分支，功能开发使用feature分支
- **提交规范**：每个功能模块完成后必须提交到GitHub
- **提交信息格式**：
  ```
  [类型] 简短描述

  详细说明（可选）
  ```
  类型包括：feat（新功能）、fix（修复）、docs（文档）、refactor（重构）、test（测试）

#### 提交时机
- ✅ 完成一个完整的功能模块后立即提交
- ✅ 通过所有单元测试后提交
- ✅ 重要代码修改后提交
- ✅ 每日工作结束前提交当天进度
- ❌ 避免提交未完成或无法编译的代码

#### 提交检查清单
1. 代码能够成功编译
2. 所有相关测试通过
3. 更新相关文档（如有）
4. 检查是否有敏感信息（API密钥等）
5. 确认.gitignore正确配置

### 2.3 代码风格

#### 缩进和格式
- 使用4个空格缩进（不使用Tab）
- 每行代码不超过120字符
- 大括号风格：Allman或K&R风格统一使用
```cpp
// Allman 风格（推荐）
void function()
{
    if (condition)
    {
        doSomething();
    }
}
```

#### 注释规范
- 类和函数使用Doxygen风格注释
```cpp
/**
 * @brief 订单管理器类，负责订单的创建、修改和取消
 *
 * @details 支持多种订单类型，提供异步订单提交和同步状态查询
 */
class OrderManager
{
public:
    /**
     * @brief 提交限价订单
     * @param symbol 交易对符号 (e.g., "BTCUSDT")
     * @param side 订单方向 (Buy/Sell)
     * @param price 价格
     * @param quantity 数量
     * @return 订单ID，失败返回空字符串
     */
    std::string submitLimitOrder(
        const std::string& symbol,
        OrderSide side,
        double price,
        double quantity
    );
};
```

#### 头文件保护
使用`#pragma once`或传统的include guards
```cpp
#pragma once

// 或者
#ifndef CRYPTO_TRADING_ORDER_MANAGER_H
#define CRYPTO_TRADING_ORDER_MANAGER_H
// ...
#endif
```

### 2.3 C++最佳实践

#### 智能指针
- 优先使用`std::unique_ptr`和`std::shared_ptr`，避免裸指针
- 使用`std::make_unique`和`std::make_shared`创建

#### RAII原则
- 资源管理遵循RAII（资源获取即初始化）
- 使用智能指针、std::lock_guard等管理资源

#### 异常处理
- 构造函数中可抛出异常
- 析构函数、移动构造函数标记为`noexcept`
- 网络API调用使用错误码或异常（统一处理）

#### const正确性
- 参数传递：大对象用`const&`，小对象值传递
- 成员函数：不修改状态的标记为`const`

```cpp
class MarketData
{
public:
    double getPrice(const std::string& symbol) const;
    void updatePrice(const std::string& symbol, double price);
};
```

### 2.4 项目结构

```
crypto_trading/
├── CMakeLists.txt
├── README.md
├── claude.md                  # 本文档
├── include/                   # 公共头文件
│   ├── exchange/             # 交易所接口
│   │   ├── exchange_interface.h
│   │   ├── binance/
│   │   │   ├── binance_client.h
│   │   │   ├── binance_websocket.h
│   │   │   └── binance_types.h
│   │   └── okx/              # 后续扩展
│   ├── trading/              # 交易核心
│   │   ├── order_manager.h
│   │   ├── position_manager.h
│   │   └── account_manager.h
│   ├── strategy/             # 策略框架
│   │   ├── strategy_base.h
│   │   └── strategy_engine.h
│   ├── risk/                 # 风控模块
│   │   ├── risk_manager.h
│   │   └── position_limiter.h
│   ├── data/                 # 数据管理
│   │   ├── market_data.h
│   │   ├── database_manager.h
│   │   └── redis_cache.h
│   └── utils/                # 工具类
│       ├── logger.h
│       ├── config.h
│       ├── crypto_utils.h   # 签名、加密
│       └── time_utils.h
├── src/                      # 源文件（对应include结构）
│   ├── exchange/
│   ├── trading/
│   ├── strategy/
│   ├── risk/
│   ├── data/
│   └── utils/
├── tests/                    # 单元测试
│   ├── exchange_tests/
│   ├── trading_tests/
│   └── strategy_tests/
├── examples/                 # 示例代码
│   ├── simple_market_making.cpp
│   └── grid_trading.cpp
├── config/                   # 配置文件
│   ├── config.json
│   └── api_keys.json        # Git忽略
└── third_party/             # 第三方库
    └── CMakeLists.txt
```

---

## 3. 功能需求

### 3.1 交易所接口模块

#### 3.1.1 REST API支持
- **账户信息**：查询余额、账户状态
- **订单操作**：下单、撤单、查询订单、订单历史
- **市场数据**：K线、深度、最新成交价
- **签名认证**：HMAC-SHA256签名

#### 3.1.2 WebSocket支持
- **私有流**：订单更新、账户变动、持仓变化
- **公共流**：实时行情、深度更新、成交流
- **心跳机制**：保持连接、自动重连
- **订阅管理**：动态订阅/取消订阅

#### 3.1.3 接口抽象
```cpp
class ExchangeInterface
{
public:
    virtual ~ExchangeInterface() = default;

    // REST API
    virtual std::string placeOrder(const OrderRequest& request) = 0;
    virtual bool cancelOrder(const std::string& order_id) = 0;
    virtual OrderInfo queryOrder(const std::string& order_id) = 0;
    virtual std::vector<Balance> getAccountBalance() = 0;

    // WebSocket
    virtual void subscribeOrderBook(const std::string& symbol) = 0;
    virtual void subscribeTrades(const std::string& symbol) = 0;
    virtual void subscribeUserData() = 0;

    // 回调设置
    virtual void setOrderUpdateCallback(OrderUpdateCallback callback) = 0;
    virtual void setMarketDataCallback(MarketDataCallback callback) = 0;
};
```

### 3.2 订单管理系统

#### 3.2.1 订单类型
- 市价单（Market Order）
- 限价单（Limit Order）
- 止损单（Stop Loss）
- 止盈限价单（Take Profit Limit）
- 条件单（Conditional Order）

#### 3.2.2 订单生命周期
```
新建 -> 已提交 -> [部分成交] -> 完全成交
              -> 已取消
              -> 拒绝/失败
```

#### 3.2.3 订单管理器功能
- 订单创建与验证
- 订单状态跟踪
- 订单本地缓存（Redis）
- 订单历史存储
- 批量订单操作

### 3.3 仓位管理

#### 3.3.1 仓位跟踪
- 实时持仓计算
- 未实现盈亏（PnL）
- 平均持仓成本
- 杠杆倍数管理（期货）

#### 3.3.2 仓位数据结构
```cpp
struct Position
{
    std::string symbol;
    double quantity;          // 持仓量（正=多，负=空）
    double average_price;     // 平均开仓价
    double unrealized_pnl;    // 未实现盈亏
    double realized_pnl;      // 已实现盈亏
    double margin_used;       // 占用保证金
    int leverage;             // 杠杆倍数
    TimePoint update_time;
};
```

### 3.4 策略框架

#### 3.4.1 策略基类
```cpp
class StrategyBase
{
public:
    virtual ~StrategyBase() = default;

    virtual void onMarketData(const MarketData& data) = 0;
    virtual void onOrderUpdate(const OrderUpdate& update) = 0;
    virtual void onTick() = 0;  // 定时器回调

    virtual void initialize() = 0;
    virtual void shutdown() = 0;

protected:
    // 提供给策略的接口
    std::string buyLimit(const std::string& symbol, double price, double qty);
    std::string sellLimit(const std::string& symbol, double price, double qty);
    bool cancelOrder(const std::string& order_id);
    Position getPosition(const std::string& symbol);
};
```

#### 3.4.2 内置策略示例
- 网格交易（Grid Trading）
- 做市策略（Market Making）
- 趋势跟踪（Trend Following）
- 套利策略（Arbitrage）

### 3.5 风险控制

#### 3.5.1 风控规则
- **单笔订单限额**：最大订单金额
- **总持仓限额**：单币种、总账户持仓上限
- **日损失限制**：每日最大亏损额
- **频率限制**：防止API限流
- **价格偏离检查**：防止异常价格订单

#### 3.5.2 风控管理器
```cpp
class RiskManager
{
public:
    bool validateOrder(const OrderRequest& order);
    bool checkPositionLimit(const std::string& symbol, double new_qty);
    bool checkDailyLoss();
    void updateRiskMetrics();

private:
    double max_order_value_;
    double max_position_value_;
    double max_daily_loss_;
    std::map<std::string, double> position_limits_;
};
```

### 3.6 数据管理

#### 3.6.1 市场数据
- **实时数据**：Tick、深度、成交
- **历史数据**：K线、历史成交
- **数据存储**：TimescaleDB（时序数据）
- **数据缓存**：Redis（实时数据）

#### 3.6.2 数据库模式
```sql
-- 订单表
CREATE TABLE orders (
    order_id VARCHAR(64) PRIMARY KEY,
    exchange VARCHAR(32),
    symbol VARCHAR(32),
    side VARCHAR(10),
    type VARCHAR(20),
    price DECIMAL(20, 8),
    quantity DECIMAL(20, 8),
    filled_quantity DECIMAL(20, 8),
    status VARCHAR(20),
    create_time TIMESTAMPTZ,
    update_time TIMESTAMPTZ
);

-- K线数据表（TimescaleDB hypertable）
CREATE TABLE klines (
    time TIMESTAMPTZ NOT NULL,
    symbol VARCHAR(32),
    interval VARCHAR(10),
    open DECIMAL(20, 8),
    high DECIMAL(20, 8),
    low DECIMAL(20, 8),
    close DECIMAL(20, 8),
    volume DECIMAL(20, 8)
);

SELECT create_hypertable('klines', 'time');
```

---

## 4. 技术要求

### 4.1 性能指标
- **订单延迟**：REST API订单提交延迟 < 100ms（P99）
- **WebSocket延迟**：消息接收延迟 < 50ms
- **吞吐量**：支持每秒100+订单提交
- **并发连接**：支持10+交易对实时订阅

### 4.2 可靠性
- **故障恢复**：网络断开自动重连，订单状态恢复
- **数据一致性**：订单状态与交易所严格同步
- **容错机制**：API限流处理、异常订单回滚
- **幂等性**：订单操作支持幂等（防止重复提交）

### 4.3 安全性
- **API密钥管理**：密钥加密存储，不硬编码
- **签名验证**：所有私有API请求签名
- **权限控制**：API Key仅开放必要权限（禁止提币）
- **日志脱敏**：日志中不记录完整API密钥

### 4.4 可扩展性
- **多交易所适配**：新增交易所只需实现`ExchangeInterface`
- **策略插件化**：策略继承`StrategyBase`即可集成
- **配置驱动**：交易参数、风控规则支持配置文件修改
- **模块解耦**：各模块独立，便于单独测试和升级

---

## 5. 开发阶段规划

### 阶段1：基础框架与币安API集成（2-3周）

#### 5.1.1 任务列表
- [ ] 项目初始化（CMake配置、第三方库集成）
- [ ] 日志系统集成（spdlog）
- [ ] 配置管理模块（JSON配置文件读取）
- [ ] 币安REST API客户端
  - [ ] HTTP请求封装（Boost.Beast）
  - [ ] HMAC-SHA256签名实现
  - [ ] 基础API接口：账户信息、下单、撤单、查询订单
- [ ] 币安WebSocket客户端
  - [ ] WebSocket连接管理
  - [ ] 心跳与重连机制
  - [ ] 订阅用户数据流（订单更新）
  - [ ] 订阅市场数据流（深度、成交）
- [ ] 基础数据结构定义
  - [ ] OrderRequest、OrderInfo
  - [ ] Balance、Position
  - [ ] MarketData（Ticker、OrderBook、Trade）

#### 5.1.2 交付物
- 可运行的币安API客户端Demo
- 能够下单、撤单、查询余额
- WebSocket实时接收订单更新
- 完整的单元测试（覆盖率>80%）

#### 5.1.3 验收标准
- 成功连接币安测试网
- 完成10笔模拟交易（限价单+市价单）
- WebSocket稳定运行24小时无断连

---

### 阶段2：交易引擎与订单管理（2-3周）

#### 5.2.1 任务列表
- [ ] OrderManager实现
  - [ ] 订单创建与验证
  - [ ] 订单状态机管理
  - [ ] 订单本地缓存（内存+Redis）
  - [ ] 订单与交易所同步
- [ ] PositionManager实现
  - [ ] 实时持仓计算
  - [ ] 盈亏计算（PnL）
  - [ ] 持仓事件通知
- [ ] AccountManager实现
  - [ ] 账户余额管理
  - [ ] 资金变动记录
- [ ] 数据库集成
  - [ ] PostgreSQL/TimescaleDB连接池
  - [ ] 订单历史存储
  - [ ] 持仓快照存储
- [ ] Redis缓存层
  - [ ] 实时订单缓存
  - [ ] 市场数据缓存

#### 5.2.2 交付物
- 完整的订单管理系统
- 持仓实时跟踪
- 订单历史查询API
- 压力测试报告（100订单/秒）

#### 5.2.3 验收标准
- 订单状态与交易所100%一致
- 持仓计算准确率100%
- 订单提交延迟P99 < 100ms
- 系统连续运行72小时无内存泄漏

---

### 阶段3：策略系统与风控模块（3-4周）

#### 5.3.1 任务列表
- [ ] StrategyEngine实现
  - [ ] 策略加载与管理
  - [ ] 事件分发机制（订单、行情）
  - [ ] 策略生命周期管理
- [ ] StrategyBase基类
  - [ ] 统一策略接口
  - [ ] 交易API封装（buy/sell/cancel）
  - [ ] 数据订阅接口
- [ ] 内置策略实现
  - [ ] 网格交易策略
  - [ ] 简单做市策略
- [ ] RiskManager实现
  - [ ] 订单前置风控检查
  - [ ] 持仓限额控制
  - [ ] 日损失监控
  - [ ] API频率限制
- [ ] 回测框架（基础版本）
  - [ ] 历史数据回放
  - [ ] 策略性能评估

#### 5.3.2 交付物
- 可运行的策略引擎
- 2个以上示例策略
- 完整的风控系统
- 策略回测工具

#### 5.3.3 验收标准
- 策略能够自动执行交易
- 风控规则100%触发
- 网格策略通过回测验证
- 策略切换无订单遗漏

---

### 阶段4：多交易所支持与监控系统（3-4周）

#### 5.4.1 任务列表
- [ ] ExchangeInterface标准化
  - [ ] 统一交易所接口定义
  - [ ] 交易所工厂模式
- [ ] 新增交易所支持
  - [ ] OKX API集成
  - [ ] Bybit API集成（可选）
- [ ] 跨交易所套利支持
  - [ ] 多交易所价格监控
  - [ ] 套利机会检测
- [ ] 监控与告警系统
  - [ ] 系统健康检查
  - [ ] 性能指标采集（Prometheus格式）
  - [ ] 告警通知（邮件/Telegram）
- [ ] Web管理界面（可选）
  - [ ] 订单查看
  - [ ] 持仓监控
  - [ ] 策略控制

#### 5.4.2 交付物
- 支持2个以上交易所
- 跨交易所套利Demo
- 监控系统
- 运维文档

#### 5.4.3 验收标准
- 同时连接3个交易所稳定运行
- 套利策略成功执行10笔交易
- 监控系统覆盖所有关键指标
- 完整的故障恢复测试通过

---

## 6. 依赖库与环境

### 6.1 必需依赖
```bash
# Ubuntu/Debian
apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    postgresql-server-dev-all \
    redis-server

# C++库（通过CMake FetchContent或vcpkg安装）
- nlohmann/json (3.11.0+)
- spdlog (1.10.0+)
- Boost (1.75+)
- OpenSSL (1.1.1+)
- libpqxx (PostgreSQL C++ client)
- hiredis (Redis C client)
- cpp-httplib (可选，简单HTTP客户端)
```

### 6.2 开发工具
- **编译器**：GCC 9+ / Clang 10+ / MSVC 2019+
- **IDE**：VSCode / CLion / Visual Studio
- **调试工具**：GDB / LLDB / Valgrind
- **性能分析**：perf / gperftools

### 6.3 CMakeLists.txt示例
```cmake
cmake_minimum_required(VERSION 3.15)
project(CryptoTradingPlatform VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 依赖查找
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

# 包含目录
include_directories(include)

# 源文件
add_subdirectory(src)
add_subdirectory(tests)

# 主程序
add_executable(trading_bot src/main.cpp)
target_link_libraries(trading_bot
    PRIVATE
    exchange_lib
    trading_lib
    Boost::system
    OpenSSL::SSL
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)
```

---

## 7. 配置文件示例

### 7.1 config.json
```json
{
    "exchange": {
        "name": "binance",
        "endpoints": {
            "rest": "https://testnet.binance.vision",
            "websocket": "wss://testnet.binance.vision/ws"
        },
        "api_key_file": "config/api_keys.json",
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
        "max_daily_loss_usdt": 500,
        "position_limits": {
            "BTCUSDT": 0.1,
            "ETHUSDT": 2.0
        }
    },
    "database": {
        "postgres": {
            "host": "localhost",
            "port": 5432,
            "database": "crypto_trading",
            "user": "trader",
            "password_env": "DB_PASSWORD"
        },
        "redis": {
            "host": "localhost",
            "port": 6379,
            "db": 0
        }
    },
    "logging": {
        "level": "info",
        "file": "logs/trading.log",
        "max_size_mb": 100,
        "max_files": 10
    }
}
```

### 7.2 api_keys.json（Git忽略）
```json
{
    "binance": {
        "api_key": "YOUR_API_KEY",
        "secret_key": "YOUR_SECRET_KEY"
    },
    "okx": {
        "api_key": "",
        "secret_key": "",
        "passphrase": ""
    }
}
```

---

## 8. 测试策略

### 8.1 单元测试
- 每个类至少80%代码覆盖率
- 重点测试：订单管理、持仓计算、风控规则

### 8.2 集成测试
- 完整交易流程测试（下单->成交->持仓更新）
- 异常场景测试（网络断开、API错误）

### 8.3 压力测试
- 订单吞吐量测试（目标：100订单/秒）
- 长时间稳定性测试（72小时运行）
- 内存泄漏检测（Valgrind）

### 8.4 模拟交易
- 使用交易所测试网进行真实环境测试
- 策略7天模拟盘验证

---

## 9. 部署与运维

### 9.1 部署方式
- **开发环境**：本地编译运行
- **生产环境**：Docker容器化部署

### 9.2 监控指标
- 系统指标：CPU、内存、网络
- 业务指标：订单成功率、延迟、持仓、盈亏
- 告警：API错误、风控触发、异常订单

### 9.3 日志管理
- 分级日志：DEBUG/INFO/WARN/ERROR
- 日志轮转：按大小或时间自动轮转
- 敏感信息脱敏：API密钥、账户信息

---

## 10. 风险提示

### 10.1 技术风险
- **API变更**：交易所API可能升级或变更，需定期维护
- **网络延迟**：高频交易对网络质量要求高
- **系统故障**：需完善的故障恢复机制

### 10.2 交易风险
- **市场风险**：加密货币市场波动剧烈
- **滑点风险**：市价单可能产生较大滑点
- **流动性风险**：小币种可能流动性不足

### 10.3 安全风险
- **API密钥泄露**：需妥善保管，禁止提币权限
- **代码漏洞**：需严格代码审查和测试

---

## 11. 后续扩展方向

1. **高级策略**：机器学习预测、高频做市
2. **多账户管理**：支持多个交易账户
3. **期货合约**：支持永续合约、交割合约
4. **跨链套利**：DEX与CEX套利
5. **可视化界面**：实时监控Dashboard
6. **移动端监控**：手机App或小程序

---

## 12. 参考资源

### 12.1 交易所文档
- [币安API文档](https://binance-docs.github.io/apidocs/spot/en/)
- [OKX API文档](https://www.okx.com/docs-v5/en/)

### 12.2 技术文档
- [Boost.Asio文档](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [nlohmann/json](https://github.com/nlohmann/json)
- [spdlog](https://github.com/gabime/spdlog)

### 12.3 量化交易资源
- [QuantLib](https://www.quantlib.org/)
- [CCXT](https://github.com/ccxt/ccxt) (Python/JS参考)

---

**文档版本**：v1.0
**最后更新**：2025-10-04
**维护者**：开发团队

---

## 附录：快速开始指南

### A.1 环境搭建
```bash
# 1. 克隆项目
git clone <repo_url>
cd crypto_trading

# 2. 安装依赖
./scripts/install_dependencies.sh

# 3. 构建项目
mkdir build && cd build
cmake ..
make -j4

# 4. 配置API密钥
cp config/api_keys.json.example config/api_keys.json
# 编辑api_keys.json填入币安测试网密钥

# 5. 运行示例
./bin/binance_client_demo
```

### A.2 运行第一个策略
```bash
# 运行网格交易策略
./bin/trading_bot --strategy=grid --config=config/grid_config.json
```

### A.3 查看日志
```bash
tail -f logs/trading.log
```

---

**祝开发顺利！🚀**
