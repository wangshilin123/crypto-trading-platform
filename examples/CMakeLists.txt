cmake_minimum_required(VERSION 3.15)

# 基础设施使用示例
add_executable(basic_usage_example basic_usage_example.cpp)
target_link_libraries(basic_usage_example PRIVATE utils_lib)

# 完整应用示例
add_executable(full_demo_example full_demo_example.cpp)
target_link_libraries(full_demo_example PRIVATE utils_lib)
