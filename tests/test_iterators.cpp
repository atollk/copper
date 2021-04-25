#include "../tests/util.h"

using namespace std::chrono_literals;

CHANNEL_TEST_CASE("channel_pop_iterator<channel> can be used manually.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push(1));
        REQUIRE_THREADSAFE(chan.push(2));
        chan.close();
    });
    auto iter = chan.begin();
    const auto end = chan.end();
    REQUIRE_THREADSAFE(iter != chan.end());
    REQUIRE_THREADSAFE(iter != end);
    SECTION("Read first element.") {
        REQUIRE_THREADSAFE(*iter == 1);
        REQUIRE_THREADSAFE(*iter == 1);
        ++iter;
        REQUIRE_THREADSAFE(iter != chan.end());
        REQUIRE_THREADSAFE(iter != end);
        REQUIRE_THREADSAFE(*iter == 2);
        REQUIRE_THREADSAFE(*iter == 2);
        ++iter;
        REQUIRE_THREADSAFE(iter == chan.end());
        REQUIRE_THREADSAFE(iter == end);
    }
    SECTION("Skip first element.") {
        ++iter;
        REQUIRE_THREADSAFE(iter != chan.end());
        REQUIRE_THREADSAFE(iter != end);
        REQUIRE_THREADSAFE(*iter == 2);
        REQUIRE_THREADSAFE(*iter == 2);
        ++iter;
        REQUIRE_THREADSAFE(iter == chan.end());
        REQUIRE_THREADSAFE(iter == end);
    }
    SECTION("Skip both elements.") {
        ++iter;
        ++iter;
        REQUIRE_THREADSAFE(iter == chan.end());
        REQUIRE_THREADSAFE(iter == end);
    }
}

CHANNEL_TEST_CASE("channel_pop_iterator<channel> can be iterated over.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push(1));
        REQUIRE_THREADSAFE(chan.push(2));
        REQUIRE_THREADSAFE(chan.push(3));
        chan.close();
    });
    SECTION("channel") {
        const auto vec = std::vector<int>(chan.begin(), chan.end());
        REQUIRE_THAT(vec, Catch::Matchers::Equals(std::vector<int>{1, 2, 3}));
    }
    SECTION("view") {
        auto view = chan.read_view();
        const auto vec = std::vector<int>(view.begin(), view.end());
        REQUIRE_THAT(vec, Catch::Matchers::Equals(std::vector<int>{1, 2, 3}));
    }
}

CHANNEL_TEST_CASE("channel_push_iterator<channel> can iterated on.", "[copper]") {
    const auto vec = std::vector<int>{1, 2, 3};
    auto chan = channel_t();
    auto iter = copper::channel_push_iterator<decltype(chan)>();
    SECTION("channel") { iter = chan.push_iterator(); }
    SECTION("view") { iter = chan.write_view().push_iterator(); }
    auto task = std::async([iter, &vec]() { std::copy(vec.begin(), vec.end(), iter); });
    REQUIRE_THREADSAFE(chan.pop().value() == 1);
    REQUIRE_THREADSAFE(chan.pop().value() == 2);
    REQUIRE_THREADSAFE(chan.pop().value() == 3);
}
