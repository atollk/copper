#include "../util.h"

using namespace std::chrono_literals;


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
