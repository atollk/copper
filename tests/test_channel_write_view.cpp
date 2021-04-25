#include "../tests/util.h"

using copper::channel_write_view;

CHANNEL_TEST_CASE("channel_write_view<channel>::push blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 1);
    auto view = chan.write_view();
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.push(1)); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan.pop().value() == 1);
}

CHANNEL_TEST_CASE("channel::pop blocks until channel_write_view<channel>::push.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.write_view();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop() == 1); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(task.wait_for(50ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(view.push(1));
}

CHANNEL_TEST_CASE("channel_write_view<channel>::try_push returns false until channel::pop is called in parallel.",
                  "[copper]") {
    for (int i = 0; i < 10'000; ++i) {
        auto chan = channel_t();
        fill_channel(chan, 2);
        auto view = chan.write_view();
        REQUIRE_THREADSAFE(!view.try_push(1));
        const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 2); });
        while (!view.try_push(2))
            std::this_thread::yield();
    }
}

CHANNEL_TEST_CASE("channel::try_pop returns false until channel_write_view<channel>::push is called in parallel.",
                  "[copper]") {
    auto chan = channel_t();
    auto view = chan.write_view();
    REQUIRE_THREADSAFE(!chan.try_pop());
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.push(2)); });
    auto pop_result = chan.try_pop();
    while (!pop_result) {
        std::this_thread::yield();
        pop_result = chan.try_pop();
    }
    REQUIRE_THREADSAFE(pop_result.value() == 2);
}

CHANNEL_TEST_CASE("channel_write_view<channel>::try_push_for blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    auto view = chan.write_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!view.try_push_for(1, 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 2); });
    REQUIRE_THREADSAFE(view.try_push_for(2, 10s));
}

CHANNEL_TEST_CASE("channel::try_pop_for blocks until channel_write_view<channel>::push.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.write_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_pop_for(100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.push(2)); });
    REQUIRE_THREADSAFE(chan.try_pop_for(10s).value() == 2);
}

CHANNEL_TEST_CASE("channel_write_view<channel>::try_push_until blocks until channel::pop.", "[copper]") {
    auto chan = channel_t();
    fill_channel(chan, 2);
    auto view = chan.write_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!view.try_push_until(1, start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 2); });
    REQUIRE_THREADSAFE(view.try_push_until(2, start + 10s));
}

CHANNEL_TEST_CASE("channel::try_pop_until blocks until channel_write_view<channel>::push.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.write_view();
    const auto start = tnow();
    REQUIRE_THREADSAFE(!chan.try_pop_until(start + 100ms));
    REQUIRE_THREADSAFE(tnow() >= start + 100ms);
    const auto task = std::async([&view]() { REQUIRE_THREADSAFE(view.push(2)); });
    REQUIRE_THREADSAFE(chan.try_pop_until(start + 10s).value() == 2);
}

CHANNEL_TEST_CASE("channel_write_view::clear functions correctly.", "[copper]") {
    auto chan = channel_t();
    auto view = chan.write_view();
    fill_channel(chan, 0);
    std::this_thread::sleep_for(100ms);
    view.clear();
    const auto task = std::async([&chan]() { REQUIRE_THREADSAFE(chan.pop().value() == 1); });
    REQUIRE_THREADSAFE(view.push(1));
}