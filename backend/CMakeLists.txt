cmake_minimum_required(VERSION 3.14)
project(DataCollectorBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=*,-fuchsia-*,-llvm-*")
endif()

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_executable(DataCollectorBackend main.cpp server.cpp data_collector.cpp utils/data_structures.cpp utils/logger.cpp utils/serializers.cpp)

target_link_libraries(DataCollectorBackend PRIVATE 
    nlohmann_json::nlohmann_json
    pthread
)