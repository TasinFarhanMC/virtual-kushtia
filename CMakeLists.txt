cmake_minimum_required(VERSION 3.11)
project(VIRTUAL_KUSHTIA C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_QUIET OFF)
set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)

include(FetchContent)

option(OATPP_BUILD_TESTS OFF)
option(OATPP_LINK_TEST_LIBRARY OFF)

# FetchContent_Declare(
#     oatpp
#     GIT_REPOSITORY https://github.com/oatpp/oatpp.git
#     GIT_TAG 1.3.0-latest
#     GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(oatpp)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr
    GIT_TAG 1.11.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow
    GIT_TAG v1.2.1.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Crow)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIRS "${SRC_DIR}")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")

add_executable(virtual-kushtia "${SRC_FILES}")

add_custom_target(web_dir
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/web
)

add_dependencies(virtual-kushtia web_dir)
target_include_directories(virtual-kushtia PRIVATE ${INCLUDE_DIRS})
target_link_libraries(virtual-kushtia PRIVATE cpr::cpr Crow::Crow)
target_precompile_headers(virtual-kushtia PRIVATE "${SRC_DIR}/phc.hpp")
