/**
 * Tests for the basic functions (pushes, pops, close, clear) of void channels.
 */

#include "../util.h"

using namespace std::chrono_literals;

VOID_CHANNEL_TEST_CASE("channel::push blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push()); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan.pop());
}

VOID_CHANNEL_TEST_CASE("channel::pop blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop()); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan.push());
}

VOID_CHANNEL_TEST_CASE("channel::try_push returns false until channel::pop is called in parallel.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    REQUIRE_THREADSAFE(!chan.try_push());
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop()); });
    while (!chan.try_push())
        std::this_thread::yield();
}

VOID_CHANNEL_TEST_CASE("channel::try_pop returns false until channel::push is called in parallel.", "[copper]") {
    auto chan = channel_t();
    REQUIRE_THREADSAFE(!chan.try_pop());
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push()); });
    while (!chan.try_pop())
        std::this_thread::yield();
}

VOID_CHANNEL_TEST_CASE("channel::try_push_for blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_push_for(100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop()); });
    REQUIRE_THREADSAFE(chan.try_push_for(10s));
}

VOID_CHANNEL_TEST_CASE("channel::try_pop_for blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_pop_for(100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push()); });
    REQUIRE_THREADSAFE(chan.try_pop_for(10s));
}

VOID_CHANNEL_TEST_CASE("channel::try_push_until blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_push_until(start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop()); });
    REQUIRE_THREADSAFE(chan.try_push_until(start + 10s));
}

VOID_CHANNEL_TEST_CASE("channel::try_pop_until blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_pop_until(start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push()); });
    REQUIRE_THREADSAFE(chan.try_pop_until(start + 10s));
}

VOID_CHANNEL_TEST_CASE("channel::push fails after close.", "[copper]") {
    auto chan = channel_t();
    chan.close();
    REQUIRE_THREADSAFE(!chan.push());
}

VOID_CHANNEL_TEST_CASE("close checks function correctly.", "[copper]") {
    auto chan = channel_t();
    REQUIRE_THREADSAFE(!chan.is_read_closed());
    REQUIRE_THREADSAFE(!chan.is_write_closed());
    if (is_buffered) {
        REQUIRE_THREADSAFE(chan.push());
    }
    chan.close();
    REQUIRE_THREADSAFE(chan.is_write_closed());
    if (is_buffered) {
        REQUIRE_THREADSAFE(!chan.is_read_closed());
        REQUIRE_THREADSAFE(chan.pop());
        REQUIRE_THREADSAFE(chan.is_read_closed());
    } else {
        REQUIRE_THREADSAFE(chan.is_read_closed());
    }
}

VOID_CHANNEL_TEST_CASE("channel::push is cancelled by close.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(!chan.push()); });
    REQUIRE_THREADSAFE(task.wait_for(100ms) == std::future_status::timeout);
    chan.close();
}

VOID_CHANNEL_TEST_CASE("channel::pop is cancelled by close.", "[copper]") {
    auto chan = channel_t();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(!chan.pop()); });
    REQUIRE_THREADSAFE(task.wait_for(100ms) == std::future_status::timeout);
    chan.close();
}

VOID_CHANNEL_TEST_CASE("channel::clear functions correctly.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    std::this_thread::sleep_for(100ms);
    chan.clear();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop()); });
    REQUIRE_THREADSAFE(chan.push());
}

#ifndef COPPER_DISALLOW_MUTEX_RECURSION
VOID_CHANNEL_TEST_CASE("channel::pop_func and ::push_func are able to access the same channel.", "[copper]") {
    auto chan = channel_t();
    SECTION("pop_func") {
        const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push()); });
        const auto f = [&chan]() {
            (void)chan.try_push();
            (void)chan.try_pop();
            chan.close();
        };
        const auto status = chan.pop_func(f);
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    }
    SECTION("push_func") {
        const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop()); });
        const auto f = [&chan]() {
            (void)chan.try_push();
            (void)chan.try_pop();
            chan.close();
        };
        const auto status = chan.push_func(f);
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    }
}
#endif