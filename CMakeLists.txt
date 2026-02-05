cmake_minimum_required(VERSION 3.12)
project(Gomoku VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# 下载nlohmann/json（header-only库）
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# 源文件
set(SOURCES
    src/main.cpp
    src/config.cpp
    src/game.cpp
    src/network.cpp
    src/ui.cpp
)

# 头文件
set(HEADERS
    src/config.hpp
    src/game.hpp
    src/network.hpp
    src/ui.hpp
)

# 可执行文件
add_executable(gomoku ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(gomoku 
    PRIVATE 
    nlohmann_json::nlohmann_json
    pthread
)

# 静态链接（用于Docker镜像优化）
if(STATIC_BUILD)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()
