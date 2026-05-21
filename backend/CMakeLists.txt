cmake_minimum_required(VERSION 3.14)
project(DataCollectorBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

add_executable(DataCollectorBackend main.cpp)

target_link_libraries(DataCollectorBackend PRIVATE 
    nlohmann_json::nlohmann_json
    pthread
)