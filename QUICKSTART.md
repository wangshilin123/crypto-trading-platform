# 快速入门指南

## 环境要求

- **编译器**: GCC 9+ / Clang 10+ / MSVC 2019+
- **CMake**: 3.15+
- **依赖**: OpenSSL, Boost (1.75+)

## 安装依赖（Ubuntu/Debian）

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev
```

## 构建项目

```bash
# 1. 克隆项目
cd /home/trade

# 2. 创建构建目录
mkdir build && cd build

# 3. 配置CMake（会自动下载 spdlog, nlohmann/json, googletest）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 编译（使用4个线程）
make -j4

# 5. 查看生成的文件
ls -la bin/
```

## 运行测试

```bash
# 在 build 目录下
cd build

# 运行所有单元测试
ctest --output-on-failure

# 或直接运行测试可执行文件
./bin/utils_test

# 查看测试覆盖的模块
./bin/utils_test --gtest_list_tests
```

预期输出：
```
[==========] Running XX tests from X test suites.
[----------] Global test environment set-up.
...
[  PASSED  ] XX tests.
```

## 运行示例程序

### 基础用法示例

```bash
# 在 build 目录下
./bin/basic_usage_example
```

这个示例演示：
- 日志系统的初始化和使用
- 配置文件的加载和读取
- 时间工具的各种功能
- 加密工具（HMAC签名、Base64等）
- 字符串工具的使用

### 完整应用演示

```bash
# 使用默认配置
./bin/full_demo_example

# 或指定配置文件
./bin/full_demo_example ../config/config.json
```

这个示例模拟一个完整的交易应用，包括：
- 市场数据处理
- 订单下单流程
- 性能监控

查看生成的日志：
```bash
tail -f ../logs/trading_platform.log
```

## 集成到你的项目

### 方式1：直接使用工具库

在你的 CMakeLists.txt 中：

```cmake
# 添加本项目的子目录
add_subdirectory(path/to/crypto_trading)

# 链接工具库
add_executable(your_app your_app.cpp)
target_link_libraries(your_app PRIVATE utils_lib)
```

在你的代码中：

```cpp
#include "utils/logger.h"
#include "utils/config.h"

int main() {
    // 初始化日志
    Logger::initialize();

    // 加载配置
    auto& config = ConfigManager::getInstance();
    config.loadFromFile("config.json");

    LOG_INFO("Your application started");

    return 0;
}
```

### 方式2：作为静态库安装

```bash
cd build
sudo make install

# 头文件会安装到 /usr/local/include
# 库文件会安装到 /usr/local/lib
```

然后在你的 CMakeLists.txt 中：

```cmake
find_library(UTILS_LIB utils_lib)
target_link_libraries(your_app PRIVATE ${UTILS_LIB})
```

## 配置API密钥（用于交易）

```bash
# 复制示例配置
cp config/api_keys.json.example config/api_keys.json

# 编辑配置文件，填入你的API密钥
nano config/api_keys.json
```

⚠️ **重要**: `api_keys.json` 已在 `.gitignore` 中，不会被提交到版本控制。

## 常见问题

### Q: 编译时找不到 OpenSSL

```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# macOS
brew install openssl
export OPENSSL_ROOT_DIR=/usr/local/opt/openssl
```

### Q: 编译时找不到 Boost

```bash
# Ubuntu/Debian
sudo apt-get install libboost-all-dev

# macOS
brew install boost
```

### Q: 测试失败

确保有写权限创建测试目录：
```bash
mkdir -p test_logs test_config
chmod 755 test_logs test_config
```

### Q: 日志文件没有生成

确保 logs 目录存在：
```bash
mkdir -p logs
chmod 755 logs
```

## 性能调优

### 日志系统

生产环境建议配置：
```cpp
Logger::Config config;
config.async_mode = true;              // 异步模式
config.async_queue_size = 16384;       // 更大的队列
config.console_level = Logger::Level::Warn;  // 控制台只显示警告
config.file_level = Logger::Level::Info;     // 文件记录信息级别
```

### 配置管理

避免频繁重载配置：
```cpp
// 启动时加载一次
config.loadFromFile("config.json");

// 需要时才重载
if (config_changed) {
    config.reload();
}
```

## 下一步

基础设施已完成，接下来可以：

1. **实现币安API客户端** (阶段1)
   - REST API 封装
   - WebSocket 连接
   - 订单管理

2. **阅读完整文档**
   - `IMPLEMENTATION.md` - 实现细节
   - `claude.md` - 完整需求和规划

3. **查看示例代码**
   - `examples/basic_usage_example.cpp`
   - `examples/full_demo_example.cpp`

## 获取帮助

- 查看测试代码了解详细用法
- 阅读头文件中的 Doxygen 注释
- 参考 `claude.md` 中的代码规范

祝开发顺利！🚀
