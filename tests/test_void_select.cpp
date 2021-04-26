#include "../tests/util.h"

using namespace std::chrono_literals;

VOID_CHANNEL_TEST_CASE("Single pop select on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push());
        REQUIRE_THREADSAFE(chan.push());
        REQUIRE_THREADSAFE(chan.push());
    });
    auto result = 0;
    auto f = [&result]() { result++; };
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 3);
}

VOID_CHANNEL_TEST_CASE("Single push select on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.pop());
        REQUIRE_THREADSAFE(chan.pop());
        REQUIRE_THREADSAFE(!chan.try_pop());
    });
    REQUIRE_THREADSAFE(copper::select(chan << [] {}) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::select(chan << [] {}) == copper::channel_op_status::success);
}

VOID_CHANNEL_TEST_CASE("push select on one closed channels stops.", "[copper]") {
    auto chan = channel_t();
    chan.close();
    REQUIRE_THREADSAFE(copper::select(chan << [] {}) == copper::channel_op_status::closed);
}

VOID_CHANNEL_TEST_CASE("pop select on one closed channels stops.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push());
        chan.close();
    });
    auto result = 0;
    auto f = [&result]() { result++; };
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    task.wait();
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::closed);
}

VOID_CHANNEL_TEST_CASE("push select on one channels reacts to a later close.", "[copper]") {
    auto chan = channel_t();
    auto fut = std::async([&chan]() {
        REQUIRE_THREADSAFE(copper::try_select_for(
                               1s,
                               chan >> [] {}) == copper::channel_op_status::closed);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    chan.close();
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

VOID_CHANNEL_TEST_CASE("pop select on one filled channel and one empty channel works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1]() {
        REQUIRE_THREADSAFE(chan1.push());
        REQUIRE_THREADSAFE(chan1.push());
    });
    auto result = 0;
    auto f = [&result]() { result++; };
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
}

VOID_CHANNEL_TEST_CASE("pop select on two pre-filled channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(chan1.push());
        REQUIRE_THREADSAFE(chan2.push());
    });
    auto result = 0;
    auto f = [&result]() { result++; };
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
}

VOID_CHANNEL_TEST_CASE("push select on two channels works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(copper::select(
                               chan1 << [] {},
                               chan2 << [] {}) == copper::channel_op_status::success);
        REQUIRE_THREADSAFE(copper::select(
                               chan1 << [] {},
                               chan2 << [] {}) == copper::channel_op_status::success);
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

VOID_CHANNEL_TEST_CASE("pop select on two empty channel reacts to a later push.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto result = 0;
    auto f = [&result]() { result++; };
    auto fut = std::async([&chan1, &chan2, &f]() {
        REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan1.push());
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
    REQUIRE_THREADSAFE(result == 1);
}

VOID_CHANNEL_TEST_CASE("push select on two full channel reacts to a later pop.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    fill_channel(chan1);
    fill_channel(chan2);
    auto fut = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(copper::try_select_for(
                               1s,
                               chan1 << [] {},
                               chan2 << [] {}) == copper::channel_op_status::success);
    });
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan1.try_pop());
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

VOID_CHANNEL_TEST_CASE("select on two closed channels stops.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    chan1.close();
    chan2.close();
    REQUIRE_THREADSAFE(copper::select(
                           chan1 >> [] {},
                           chan2 << [] {}) == copper::channel_op_status::closed);
}

VOID_CHANNEL_TEST_CASE("select does not consider closed channels.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    chan2.close();
    REQUIRE_THREADSAFE(copper::try_select_for(
                           50ms,
                           chan1 >> [] {},
                           chan2 << [] {}) == copper::channel_op_status::unavailable);
}

VOID_CHANNEL_TEST_CASE("selecting on the same channel twice at once works correctly.", "[copper]") {
    auto chan = channel_t(1);
    auto result = 0;
    auto f = [&result]() { result++; };
    auto task = std::async([&chan, f]() {
        REQUIRE_THREADSAFE(copper::select(
                               chan << [] {},
                               chan >> f) == copper::channel_op_status::success);
    });
    REQUIRE_THREADSAFE(copper::select(
                           chan << [] {},
                           chan >> f) == copper::channel_op_status::success);
    task.wait();
    REQUIRE_THREADSAFE(result == 1);
}

VOID_CHANNEL_TEST_CASE("select on two channels reacts to a later close.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto fut = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(copper::try_select_for(
                               1s,
                               chan1 >> [] {},
                               chan2 >> [] {}) == copper::channel_op_status::closed);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    chan1.close();
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    chan2.close();
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

VOID_CHANNEL_TEST_CASE("select on a closed channel and an open channel is cancelled when the second channel closes.",
                       "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    chan1.close();
    auto fut = std::async([&chan1, &chan2]() {
        REQUIRE_THREADSAFE(copper::try_select_for(
                               1s,
                               chan1 >> [] {},
                               chan2 >> [] {}) == copper::channel_op_status::closed);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    chan2.close();
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

VOID_CHANNEL_TEST_CASE("Multiple parallel pop selects can be used with the same channel.", "[copper]") {
    auto chan1 = channel_t();
    auto fut1 = std::async([&chan1] {
        auto chan2 = channel_t();
        REQUIRE_THREADSAFE(copper::select(
                               chan1 >> [] {},
                               chan2 >> [] {}) == copper::channel_op_status::success);
    });
    auto fut2 = std::async([&chan1] {
        auto chan2 = channel_t();
        REQUIRE_THREADSAFE(copper::select(
                               chan1 >> [] {},
                               chan2 >> [] {}) == copper::channel_op_status::success);
    });
    auto fut3 = std::async(
        [&chan1] { REQUIRE_THREADSAFE(copper::select(chan1 >> [] {}) == copper::channel_op_status::success); });
    auto fut4 = std::async([&chan1] { REQUIRE_THREADSAFE(chan1.pop()); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(chan1.push());
    REQUIRE_THREADSAFE(chan1.push());
    REQUIRE_THREADSAFE(chan1.push());
    REQUIRE_THREADSAFE(chan1.push());
}

VOID_CHANNEL_TEST_CASE("Multiple parallel push selects can be used with the same channel.", "[copper]") {
    auto chan1 = channel_t();
    fill_channel(chan1);
    auto fut1 = std::async([&chan1] {
        auto chan2 = channel_t();
        fill_channel(chan2);
        REQUIRE_THREADSAFE(copper::select(
                               chan1 << [] {},
                               chan2 << [] {}) == copper::channel_op_status::success);
    });
    auto fut2 = std::async([&chan1] {
        auto chan2 = channel_t();
        fill_channel(chan2);
        REQUIRE_THREADSAFE(copper::select(
                               chan1 << [] {},
                               chan2 << [] {}) == copper::channel_op_status::success);
    });
    auto fut3 = std::async(
        [&chan1] { REQUIRE_THREADSAFE(copper::select(chan1 << [] {}) == copper::channel_op_status::success); });
    auto fut4 = std::async([&chan1] { REQUIRE_THREADSAFE(chan1.push()); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(chan1.pop());
    REQUIRE_THREADSAFE(chan1.pop());
    REQUIRE_THREADSAFE(chan1.pop());
    REQUIRE_THREADSAFE(chan1.pop());
}