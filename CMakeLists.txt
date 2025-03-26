cmake_minimum_required(VERSION 3.11)
project(VIRTUAL_KUSHTIA C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)

if(BUILD_TESTING)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0)
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

  include(CTest)
  include(Catch)

  enable_testing()

  # Collect test sources
  file(GLOB TEST_SOURCES ${CMAKE_SOURCE_DIR}/test/*.cpp)

  foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_EXE ${TEST_SRC} NAME_WE)
    add_executable(${TEST_EXE} ${TEST_SRC})
    set_target_properties(${TEST_EXE} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                 "${CMAKE_BINARY_DIR}/tests")

    target_link_libraries(${TEST_EXE} PRIVATE Catch2::Catch2WithMain)

    catch_discover_tests(
      ${TEST_EXE}
      REPORTER
      console
      EXTRA_ARGS
      --colour-mode
      ansi
      PROPERTIES
      OUTPUT_CAPTURE
      OFF)
  endforeach()

  return()
endif()

set(CROW_ENABLE_SSL ON)
set(CROW_INSTALL OFF)

FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow
  GIT_TAG v1.2.1.2
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Crow)
FetchContent_GetProperties(Crow)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
  GIT_TAG v0.19.0
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(httplib)
FetchContent_GetProperties(httplib)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIRS "${SRC_DIR}")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")
add_executable(server "${SRC_FILES}")
target_include_directories(server PRIVATE ${INCLUDE_DIRS})
target_link_libraries(server PRIVATE Crow::Crow httplib::httplib)
target_precompile_headers(server PRIVATE "${Crow_SOURCE_DIR}/include/crow.h"
                          "${httplib_SOURCE_DIR}/httplib.h")

add_custom_target(
  web_dir COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/web
                  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/web)
add_dependencies(server web_dir)
