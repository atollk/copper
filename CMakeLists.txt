cmake_minimum_required(VERSION 3.14)

project(copper CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Options
option(COPPER_ENABLE_TESTS "enable unit testing for Copper" ON)
option(COPPER_COLLECT_TEST_COVERAGE "collect coverage data when running tests (only supported with G++)" OFF)
option(COPPER_ENABLE_FLAKY_TESTS "enable flaky (unreliable) unit testing for Copper" ON)
option(COPPER_DISALLOW_MUTEX_RECURSION "replace std::recursive_mutex by std::mutex in Copper" OFF)

# copper library target
add_library(copper INTERFACE)
target_sources(copper INTERFACE include/copper.h)
target_include_directories(copper INTERFACE include)
target_compile_features(copper INTERFACE cxx_std_17)
if (COPPER_DISALLOW_MUTEX_RECURSION)
    target_compile_definitions(copper INTERFACE COPPER_DISALLOW_MUTEX_RECURSION)
endif ()
if (MSVC)
    target_compile_options(copper INTERFACE bigobj)
endif()


if (COPPER_ENABLE_TESTS)
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
            tests/template_instantiations.cpp
            tests/channels/test_basics.cpp
            tests/channels/test_void_basics.cpp
            tests/views/test_channel_read_view.cpp
            tests/views/test_channel_write_view.cpp
            tests/iterators/test_channel_pop_iterator.cpp
            tests/iterators/test_channel_push_iterator.cpp
            tests/select/test_select.cpp
            tests/select/test_vselect.cpp
            tests/select/test_void_select.cpp
            tests/select/test_void_vselect.cpp)
    target_link_libraries(copper_tests PRIVATE Catch2 copper Threads::Threads)
    target_compile_features(copper_tests PRIVATE cxx_std_20)
    if (NOT COPPER_ENABLE_FLAKY_TESTS)
        target_compile_definitions(copper_tests PRIVATE "COPPER_DISABLE_FLAKY_TESTS")
    endif()
    if (COPPER_COLLECT_TEST_COVERAGE)
        target_compile_options(copper_tests PRIVATE "--coverage")
        target_link_options(copper_tests PRIVATE "--coverage")
    endif()

    enable_testing()
    add_test(copper_tests copper_tests)
endif()
