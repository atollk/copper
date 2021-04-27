cmake_minimum_required(VERSION 3.18)

project(copper CXX)

include(FetchContent)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.5
)
FetchContent_MakeAvailable(Catch2)

add_library(copper INTERFACE include/copper.h)
target_include_directories(copper INTERFACE include)
set_property(TARGET copper PROPERTY CXX_STANDARD 20)

if (COPPER_DISALLOW_MUTEX_RECURSION)
    target_compile_definitions(copper INTERFACE COPPER_DISALLOW_MUTEX_RECURSION)
endif ()

target_compile_definitions(copper INTERFACE TOLLKO_USE_RECURSIVE_MUTEX)

add_executable(copper_tests
        tests/main.cpp
        tests/test_basics.cpp
        tests/test_channel_read_view.cpp
        tests/test_channel_write_view.cpp
        tests/test_iterators.cpp
        tests/test_select.cpp
        tests/test_advanced.cpp
        tests/test_void_basics.cpp
        )
target_link_libraries(copper_tests PRIVATE Catch2 copper Threads::Threads)
set_property(TARGET copper_tests PROPERTY CXX_STANDARD 20)

enable_testing()
add_test(copper_tests copper_tests)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE copper Threads::Threads)
set_property(TARGET main PROPERTY CXX_STANDARD 20)
