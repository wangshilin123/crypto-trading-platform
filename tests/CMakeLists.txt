cmake_minimum_required(VERSION 3.15)

# 工具库测试
add_executable(utils_test
    utils_tests/logger_test.cpp
    utils_tests/config_test.cpp
    utils_tests/crypto_utils_test.cpp
    utils_tests/time_utils_test.cpp
)

target_link_libraries(utils_test
    PRIVATE
    utils_lib
    GTest::gtest_main
)

# 注册测试
include(GoogleTest)
gtest_discover_tests(utils_test)
