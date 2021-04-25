#ifndef CPP20_CHANNELS_TESTUTIL_H
#define CPP20_CHANNELS_TESTUTIL_H

#include <future>
#include "catch2/catch.hpp"
#include "copper.h"

using namespace std::chrono_literals;

#define CHANNEL_TEST_CASE(test_name, tags) \
    TEMPLATE_TEST_CASE_SIG(test_name, tags, ((bool is_buffered, typename T), is_buffered, T), (true, int), (false, int))

#define VOID_CHANNEL_TEST_CASE(test_name, tags)                                                             \
    TEMPLATE_TEST_CASE_SIG(test_name, tags, ((bool is_buffered, typename T), is_buffered, T), (true, void), \
                           (false, void))


#define channel_t                                   \
    ([](int n=5) {                                   \
        if constexpr (is_buffered)                     \
            return copper::channel<is_buffered, T>(n); \
        else                                           \
            return copper::channel<is_buffered, T>();  \
    })

inline std::chrono::time_point<std::chrono::high_resolution_clock> tnow() {
    return std::chrono::high_resolution_clock::now();
}

#define REQUIRE_THREADSAFE(condition)  \
    if (!static_cast<bool>(condition)) \
        FAIL("REQUIRE_THREADSAFE failed.");

template<bool is_buffered>
inline void fill_channel(copper::channel<is_buffered, int>& channel, int value = 0) {
    while (channel.try_push(value));
}

template<bool is_buffered>
inline void fill_channel(copper::channel<is_buffered, void>& channel) {
    while (channel.try_push());
}

#endif  // CPP20_CHANNELS_TESTUTIL_H
