#include "../tests/util.h"

using namespace std::chrono_literals;

CHANNEL_TEST_CASE("Single pop vselect on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push(1));
        REQUIRE_THREADSAFE(chan.push(2));
        REQUIRE_THREADSAFE(chan.push(3));
    });
    auto result = 0;
    REQUIRE_THREADSAFE(copper::vselect(chan >> result, [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::vselect(chan >> result, [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
    REQUIRE_THREADSAFE(copper::vselect(chan >> result, [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 3);
}

CHANNEL_TEST_CASE("Single push vselect on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto f = []() { return 1; };
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.pop().value() == 1);
        REQUIRE_THREADSAFE(chan.pop().value() == 1);
        REQUIRE_THREADSAFE(!chan.try_pop());
    });
    REQUIRE_THREADSAFE(copper::vselect(chan << 1, [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::vselect(chan << 1, [] {}) == copper::channel_op_status::success);
}

CHANNEL_TEST_CASE("pop vselect on one filled channel and one empty channel works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1]() {
        REQUIRE_THREADSAFE(chan1.push(1));
        REQUIRE_THREADSAFE(chan1.push(2));
    });
    auto result = 0;
    auto f = [&result](int x) { result = x; };
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> result,
                           [] {},
                           chan2 >> result,
                           [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> result,
                           [] {},
                           chan2 >> result,
                           [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
}

/*
CHANNEL_TEST_CASE("pop vselect on two pre-filled channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(chan1.push(1));
        REQUIRE_THREADSAFE(chan2.push(2));
    });
    auto result = 0;
    auto f = [&result](int x) { result += x; };
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 3);
}

CHANNEL_TEST_CASE("push vselect on two channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto f1 = []() { return 1; };
    auto f2 = []() { return 2; };
    auto task = std::async([&chan1, &chan2, f1, f2]() {
        REQUIRE_THREADSAFE(copper::select(chan1 << f1, chan2 << f1) == copper::channel_op_status::success);
        REQUIRE_THREADSAFE(copper::select(chan1 << f2, chan2 << f2) == copper::channel_op_status::success);
    });
    std::this_thread::yield();

    auto val1 = 0, val2 = 0;
    if (const auto pop1 = chan1.try_pop_for(100ms); pop1) {
        val1 = pop1.value();
    } else {
        val1 = chan2.pop().value();
    }
    if (const auto pop1 = chan1.try_pop_for(100ms); pop1) {
        val2 = pop1.value();
    } else {
        val2 = chan2.pop().value();
    }

    if (val1 == 1) {
        REQUIRE_THREADSAFE(val2 == 2);
    } else {
        REQUIRE_THREADSAFE(val1 == 2);
        REQUIRE_THREADSAFE(val2 == 1);
    }
}
 */