cmake_minimum_required(VERSION 3.15)
project(Direct3D12Renderer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 查找 src 文件夹中的所有 .cpp 文件
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 添加可执行文件
add_definitions(-DUNICODE -D_UNICODE)
add_executable(Direct3D12Renderer WIN32 src/main.cpp src/Renderer.cpp)

# 链接 Direct3D12 和 DXGI 库
target_link_libraries(Direct3D12Renderer d3d12 dxgi)

