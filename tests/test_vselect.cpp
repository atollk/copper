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

CHANNEL_TEST_CASE("pop vselect on two pre-filled channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(chan1.push(1));
        REQUIRE_THREADSAFE(chan2.push(2));
    });
    auto a = 0;
    auto sum = 0;
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> a,
                           [&a, &sum] { sum += a; },
                           chan2 >> a,
                           [&a, &sum] { sum += a; }) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::vselect(
                           chan1 >> a,
                           [&a, &sum] { sum += a; },
                           chan2 >> a,
                           [&a, &sum] { sum += a; }) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(sum == 3);
}

CHANNEL_TEST_CASE("push vselect on two channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(copper::vselect(
                               chan1 << 1,
                               [] {},
                               chan2 << 1,
                               [] {}) == copper::channel_op_status::success);
        REQUIRE_THREADSAFE(copper::vselect(
                               chan1 << 2,
                               [] {},
                               chan2 << 2,
                               [] {}) == copper::channel_op_status::success);
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

CHANNEL_TEST_CASE("vselect executes the correct callable.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto x = 0;
    auto pops = std::vector<int>();
    auto task = std::async([&chan1, &chan2, &x, &pops]() {
        auto status = copper::vselect(
            chan1 >> x,
            [&pops, &x] { pops.push_back(x); },
            chan2 >> x,
            [&pops, &x] { pops.push_back(x); });
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
        status = copper::vselect(
            chan1 >> x,
            [&pops, &x] { pops.push_back(x); },
            chan2 >> x,
            [&pops, &x] { pops.push_back(x); });
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    });
    std::this_thread::yield();

    REQUIRE_THREADSAFE(chan1.push(1));
    REQUIRE_THREADSAFE(chan1.push(2));
    task.wait();
    REQUIRE_THREADSAFE(pops.size() == 2);
    REQUIRE_THREADSAFE(pops[0] == 1);
    REQUIRE_THREADSAFE(pops[1] == 2);
}

CHANNEL_TEST_CASE("vselect allows the callables to run in parallel.", "[copper]") {
    auto chan1 = channel_t();
    auto counter = std::atomic<int>(0);
    auto f = [&chan1, &counter]() {
        auto x = 0;
        const auto status = copper::vselect(chan1 >> x, [&counter] {
            counter += 1;
            while (counter < 2) {
                std::this_thread::yield();
            }
        });
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    };
    auto task1 = std::async(f);
    auto task2 = std::async(f);

    REQUIRE_THREADSAFE(chan1.push(1));
    REQUIRE_THREADSAFE(chan1.push(2));
    task1.wait();
    task2.wait();
}