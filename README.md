# 加密货币交易平台

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com)

一个高性能、可扩展的C++加密货币交易平台，支持多交易所对接、自动化交易策略执行和风险管理。

## ✨ 核心特性

- 🔌 **多交易所支持** - 支持币安(Binance)，可扩展至OKX、Bybit等
- ⚡ **高性能交易引擎** - 低延迟订单执行，P99延迟<100ms
- 📊 **策略框架** - 支持自定义交易策略开发和回测
- 🛡️ **风险管理** - 实时风控、仓位管理、止损止盈
- 💾 **数据管理** - 市场数据订阅、历史数据存储、实时行情

## 🏗️ 技术栈

- **语言**: C++17/20
- **网络**: Boost.Beast (WebSocket/HTTP), Boost.Asio
- **数据**: Redis (缓存), PostgreSQL/TimescaleDB (历史数据)
- **工具**: CMake, Google Test, spdlog

## 📋 前置要求

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libboost-all-dev \
    postgresql-server-dev-all \
    redis-server

# CentOS/RHEL
sudo yum install -y \
    gcc-c++ \
    cmake \
    openssl-devel \
    boost-devel \
    postgresql-devel \
    redis
```

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/yourusername/crypto-trading-platform.git
cd crypto-trading-platform
```

### 2. 构建项目

```bash
mkdir build && cd build
cmake ..
make -j4
```

### 3. 配置API密钥

```bash
cp config/api_keys.json.example config/api_keys.json
# 编辑 config/api_keys.json 填入你的API密钥
```

### 4. 运行示例

```bash
# 运行币安客户端示例
./bin/binance_client_demo

# 运行网格交易策略
./bin/trading_bot --strategy=grid --config=config/grid_config.json
```

## 📂 项目结构

```
crypto_trading/
├── include/              # 头文件
│   ├── exchange/        # 交易所接口
│   ├── trading/         # 交易核心
│   ├── strategy/        # 策略框架
│   ├── risk/            # 风控模块
│   ├── data/            # 数据管理
│   └── utils/           # 工具类
├── src/                 # 源文件
├── tests/               # 单元测试
├── examples/            # 示例代码
├── config/              # 配置文件
└── claude.md           # 详细需求文档
```

## 📖 文档

详细的技术文档和开发指南请参考：

- [完整需求文档](claude.md) - 包含架构设计、代码规范、开发阶段规划
- [API文档](docs/api.md) - 接口说明文档（待生成）
- [策略开发指南](docs/strategy_guide.md) - 如何开发自定义策略（待编写）

## 🔧 开发阶段

### 阶段1: 基础框架与币安API集成 (2-3周)
- [x] 项目初始化
- [ ] 币安REST API客户端
- [ ] 币安WebSocket客户端
- [ ] 基础数据结构定义

### 阶段2: 交易引擎与订单管理 (2-3周)
- [ ] OrderManager实现
- [ ] PositionManager实现
- [ ] 数据库集成

### 阶段3: 策略系统与风控模块 (3-4周)
- [ ] StrategyEngine实现
- [ ] 内置策略（网格、做市）
- [ ] RiskManager实现

### 阶段4: 多交易所支持与监控 (3-4周)
- [ ] 多交易所适配
- [ ] 监控与告警系统
- [ ] Web管理界面（可选）

## ⚙️ 配置示例

```json
{
    "exchange": {
        "name": "binance",
        "endpoints": {
            "rest": "https://api.binance.com",
            "websocket": "wss://stream.binance.com:9443/ws"
        }
    },
    "risk": {
        "max_order_value_usdt": 1000,
        "max_position_value_usdt": 5000,
        "max_daily_loss_usdt": 500
    }
}
```

## 🧪 测试

```bash
# 运行单元测试
cd build
ctest --output-on-failure

# 运行代码覆盖率
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage
```

## ⚠️ 风险提示

- 本项目仅供学习和研究使用
- 加密货币交易存在风险，请谨慎使用
- 使用前请在测试网充分测试
- API密钥请妥善保管，建议禁用提币权限

## 🤝 贡献指南

欢迎提交Issue和Pull Request！

1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 📞 联系方式

- 作者: wangshilin123
- Email: 1362975283@qq.com

## 🙏 致谢

- [币安API文档](https://binance-docs.github.io/apidocs/spot/en/)
- [Boost库](https://www.boost.org/)
- [nlohmann/json](https://github.com/nlohmann/json)

---

**⚡ 开始构建你的交易系统！**
