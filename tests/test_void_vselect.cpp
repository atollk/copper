#include "../tests/util.h"

using namespace std::chrono_literals;

VOID_CHANNEL_TEST_CASE("Single pop vselect on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push());
        REQUIRE_THREADSAFE(chan.push());
        REQUIRE_THREADSAFE(chan.push());
    });
    auto result = 0;
    REQUIRE_THREADSAFE(copper::vselect(chan >> copper::_, [&result] { result++; }) ==
                       copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::vselect(chan >> copper::_, [&result] { result++; }) ==
                       copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
    REQUIRE_THREADSAFE(copper::vselect(chan >> copper::_, [&result] { result++; }) ==
                       copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 3);
}

VOID_CHANNEL_TEST_CASE("Single push vselect on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.pop());
        REQUIRE_THREADSAFE(chan.pop());
        REQUIRE_THREADSAFE(!chan.try_pop());
    });
    REQUIRE_THREADSAFE(copper::vselect(chan << copper::_, [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::vselect(chan << copper::_, [] {}) == copper::channel_op_status::success);
}

VOID_CHANNEL_TEST_CASE("pop vselect on one filled channel and one empty channel works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1]() {
        REQUIRE_THREADSAFE(chan1.push());
        REQUIRE_THREADSAFE(chan1.push());
    });
    auto result = 0;
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> copper::_,
                           [&result] { result++; },
                           chan2 >> copper::_,
                           [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> copper::_,
                           [&result] { result++; },
                           chan2 >> copper::_,
                           [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
}

VOID_CHANNEL_TEST_CASE("pop vselect on two pre-filled channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(chan1.push());
        REQUIRE_THREADSAFE(chan2.push());
    });
    auto result = 0;
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> copper::_,
                           [&result] { result = 1; },
                           chan2 >> copper::_,
                           [&result] { result = 2; }) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> copper::_,
                           [&result] { result = 3; },
                           chan2 >> copper::_,
                           [&result] { result = 4; }) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 4);
}

VOID_CHANNEL_TEST_CASE("push vselect on two channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(copper::vselect(
                               chan1 << copper::_,
                               [] {},
                               chan2 << copper::_,
                               [] {}) == copper::channel_op_status::success);
        REQUIRE_THREADSAFE(copper::vselect(
                               chan1 << copper::_,
                               [] {},
                               chan2 << copper::_,
                               [] {}) == copper::channel_op_status::success);
    });
    std::this_thread::yield();

    auto chan1_n = 0;
    auto chan2_n = 0;
    if (chan1.try_pop_for(100ms)) {
        chan1_n += 1;
    }
    if (chan1.try_pop_for(100ms)) {
        chan1_n += 1;
    }
    if (chan2.try_pop_for(100ms)) {
        chan2_n += 1;
    }
    if (chan2.try_pop_for(100ms)) {
        chan2_n += 1;
    }
    REQUIRE_THREADSAFE(chan1_n + chan2_n == 2);
}

VOID_CHANNEL_TEST_CASE("vselect allows the callables to run in parallel.", "[copper]") {
    auto chan1 = channel_t();
    auto counter = std::atomic<int>(0);
    auto f = [&chan1, &counter]() {
        auto x = 0;
        const auto status = copper::vselect(chan1 >> copper::_, [&counter] {
            counter += 1;
            while (counter < 2) {
                std::this_thread::yield();
            }
        });
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    };
    auto task1 = std::async(f);
    auto task2 = std::async(f);

    REQUIRE_THREADSAFE(chan1.push());
    REQUIRE_THREADSAFE(chan1.push());
    task1.wait();
    task2.wait();
}
