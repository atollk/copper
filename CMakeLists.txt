cmake_minimum_required(VERSION 3.19)

project(copper CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Options
option(COPPER_ENABLE_TESTS "enable unit testing for Copper" ON)
option(COPPER_DISALLOW_MUTEX_RECURSION "replace std::recursive_mutex by std::mutex in Copper" OFF)

# copper library target
add_library(copper INTERFACE include/copper.h)
target_include_directories(copper INTERFACE include)
target_compile_features(copper INTERFACE cxx_std_17)
if (COPPER_DISALLOW_MUTEX_RECURSION)
    target_compile_definitions(copper INTERFACE COPPER_DISALLOW_MUTEX_RECURSION)
endif ()

# Catch2 testing framework
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.6
)
FetchContent_MakeAvailable(Catch2)

# Unit tests
add_executable(copper_tests
        tests/main.cpp
        tests/test_basics.cpp
        tests/test_channel_read_view.cpp
        tests/test_channel_write_view.cpp
        tests/test_iterators.cpp
        tests/test_select.cpp
        tests/test_vselect.cpp
        tests/test_advanced.cpp
        tests/test_void_basics.cpp
        tests/test_void_select.cpp
        tests/test_void_vselect.cpp
        )
target_link_libraries(copper_tests PRIVATE Catch2 copper Threads::Threads)
target_compile_features(copper_tests PRIVATE cxx_std_20)
if (WIN32 AND (MINGW OR MSYS))
    target_compile_options(copper_tests PRIVATE "-Wl,--default-image-base-low")
endif()

if (COPPER_ENABLE_TESTS)
    enable_testing()
    add_test(copper_tests copper_tests)
endif()
