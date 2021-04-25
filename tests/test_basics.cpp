#include "../tests/util.h"

using namespace std::chrono_literals;

CHANNEL_TEST_CASE("pushing and popping some rvalues and lvalues.", "[copper]") {
    auto chan = channel_t();
    auto x = 0;
    (void)chan.try_push(x);
    (void)chan.try_push(std::move(x));
    (void)chan.try_pop();
}

CHANNEL_TEST_CASE("Non-copyable types can be channel messages.", "[copper]") {
    auto chan = copper::channel<is_buffered, std::unique_ptr<int>>();
    (void)chan.try_push(std::make_unique<int>(0));
    (void)chan.try_pop();
}

CHANNEL_TEST_CASE("channel::push blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 1);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(1)); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan.pop().value() == 1);
}

CHANNEL_TEST_CASE("channel::pop blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop() == 1); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan.push(1));
}

CHANNEL_TEST_CASE("channel::try_push returns false until channel::pop is called in parallel.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    REQUIRE_THREADSAFE(!chan.try_push(1));
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 2); });
    while (!chan.try_push(2))
        std::this_thread::yield();
}

CHANNEL_TEST_CASE("channel::try_pop returns false until channel::push is called in parallel.", "[copper]") {
    auto chan = channel_t();
    REQUIRE_THREADSAFE(!chan.try_pop());
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(2)); });
    while (!chan.try_pop())
        std::this_thread::yield();
}

CHANNEL_TEST_CASE("channel::try_push_for blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_push_for(1, 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 2); });
    REQUIRE_THREADSAFE(chan.try_push_for(2, 10s));
}

CHANNEL_TEST_CASE("channel::try_pop_for blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_pop_for(100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(2)); });
    REQUIRE_THREADSAFE(chan.try_pop_for(10s).value() == 2);
}

CHANNEL_TEST_CASE("channel::try_push_until blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_push_until(1, start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 2); });
    REQUIRE_THREADSAFE(chan.try_push_until(2, start + 10s));
}

CHANNEL_TEST_CASE("channel::try_pop_until blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_pop_until(start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(2)); });
    REQUIRE_THREADSAFE(chan.try_pop_until(start + 10s).value() == 2);
}

CHANNEL_TEST_CASE("channel::push fails after close.", "[copper]") {
    auto chan = channel_t();
    chan.close();
    REQUIRE_THREADSAFE(!chan.push(1));
}

CHANNEL_TEST_CASE("close checks function correctly.", "[copper]") {
    auto chan = channel_t();
    REQUIRE_THREADSAFE(!chan.is_read_closed());
    REQUIRE_THREADSAFE(!chan.is_write_closed());
    if (is_buffered) {
        REQUIRE_THREADSAFE(chan.push(1));
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

CHANNEL_TEST_CASE("channel::push is cancelled by close.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(!chan.push(1)); });
    REQUIRE_THREADSAFE(task.wait_for(100ms) == std::future_status::timeout);
    chan.close();
}

CHANNEL_TEST_CASE("channel::pop is cancelled by close.", "[copper]") {
    auto chan = channel_t();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(!chan.pop()); });
    REQUIRE_THREADSAFE(task.wait_for(100ms) == std::future_status::timeout);
    chan.close();
}

CHANNEL_TEST_CASE("channel::clear functions correctly.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 0);
    std::this_thread::sleep_for(100ms);
    chan.clear();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 1); });
    REQUIRE_THREADSAFE(chan.push(1));
}

TEMPLATE_TEST_CASE("channel::clear notifies waiting pushes.", "[copper]", int, void) {
    auto chan = copper::buffered_channel<TestType>(2);
    fill_channel(chan);

    const auto task = std::async([&chan]() {
        if constexpr (std::is_void_v<TestType>) {
            REQUIRE_THREADSAFE(chan.push());
            REQUIRE_THREADSAFE(chan.push());
        } else {
            REQUIRE_THREADSAFE(chan.push(0));
            REQUIRE_THREADSAFE(chan.push(0));
        }
    });
    while (task.wait_for(10ms) == std::future_status::deferred)
        std::this_thread::yield();
    REQUIRE_THREADSAFE(chan.clear() == 2);
    REQUIRE_THREADSAFE(task.wait_for(10s) == std::future_status::ready);
    REQUIRE_THREADSAFE(chan.clear() == 2);
}

#ifdef TOLLKO_USE_RECURSIVE_MUTEX
CHANNEL_TEST_CASE("channel::pop_func and ::push_func are able to access the same channel.", "[copper]") {
    auto chan = channel_t();
    SECTION("pop_func") {
        const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(0)); });
        const auto f = [&chan](auto&&) {
            (void) chan.try_push(1);
            (void) chan.try_pop();
            chan.close();
        };
        const auto status = chan.pop_func(f);
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    }SECTION("push_func") {
        const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 0); });
        const auto f = [&chan]() {
            (void) chan.try_push(1);
            (void) chan.try_pop();
            chan.close();
            return 0;
        };
        const auto status = chan.push_func(f);
        REQUIRE_THREADSAFE(status == copper::channel_op_status::success);
    }
}

#endif