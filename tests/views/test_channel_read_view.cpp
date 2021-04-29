#include "../util.h"

using copper::channel_read_view;

CHANNEL_TEST_CASE("channel::push blocks until channel_read_view<channel>::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 1);
    auto view = chan.read_view();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(1)); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(view.pop().value() == 1);
    std::this_thread::yield();
}

CHANNEL_TEST_CASE("channel_read_view<channel>::pop blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.read_view();
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.pop() == 1); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan.push(1));
}

CHANNEL_TEST_CASE("channel::try_push returns false until channel_read_view<channel>::pop is called in parallel.",
                  "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    auto view = chan.read_view();
    REQUIRE_THREADSAFE(!chan.try_push(1));
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.pop().value() == 2); });
    while (!chan.try_push(2))
        std::this_thread::yield();
}

CHANNEL_TEST_CASE("channel_read_view<channel>::try_pop returns false until channel::push is called in parallel.",
                  "[copper]") {
    auto chan = channel_t();
    auto view = chan.read_view();
    REQUIRE_THREADSAFE(!view.try_pop());
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(2)); });
    while (!view.try_pop())
        std::this_thread::yield();
}

CHANNEL_TEST_CASE("channel::try_push_for blocks until channel_read_view<channel>::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    auto view = chan.read_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_push_for(1, 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.pop().value() == 2); });
    REQUIRE_THREADSAFE(chan.try_push_for(2, 10s));
}

CHANNEL_TEST_CASE("channel_read_view<channel>::try_pop_for blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.read_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!view.try_pop_for(100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(2)); });
    REQUIRE_THREADSAFE(view.try_pop_for(10s).value() == 2);
}

CHANNEL_TEST_CASE("channel::try_push_until blocks until channel_read_view<channel>::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    auto view = chan.read_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_push_until(1, start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.pop().value() == 2); });
    REQUIRE_THREADSAFE(chan.try_push_until(2, start + 10s));
}

CHANNEL_TEST_CASE("channel_read_view<channel>::try_pop_until blocks until channel::push.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.read_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!view.try_pop_until(start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.push(2)); });
    REQUIRE_THREADSAFE(view.try_pop_until(start + 10s).value() == 2);
}