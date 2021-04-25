#include "../tests/util.h"

TEST_CASE("buffered_channel buffers correctly.", "[copper]") {
    auto chan = copper::buffered_channel<int>();
    REQUIRE_THREADSAFE(chan.push(1));
    REQUIRE_THREADSAFE(chan.push(2));
    REQUIRE_THREADSAFE(chan.push(3));
    REQUIRE_THREADSAFE(chan.push(-6));
    REQUIRE_THREADSAFE(chan.push(4));
    REQUIRE_THREADSAFE(chan.pop() == 1);
    REQUIRE_THREADSAFE(chan.pop() == 2);
    REQUIRE_THREADSAFE(chan.pop() == 3);
    REQUIRE_THREADSAFE(chan.pop() == -6);
    REQUIRE_THREADSAFE(chan.pop() == 4);
    REQUIRE_THREADSAFE(!chan.try_pop());
}

TEST_CASE("select still can pop from a non-empty closed buffered_channel.", "[copper]") {
    auto chan1 = copper::buffered_channel<int>(5);
    auto chan2 = copper::buffered_channel<int>(5);
    REQUIRE_THREADSAFE(chan1.push(1));
    fill_channel(chan1);
    chan1.close();
    chan2.close();
    const auto f1 = [](int) {};
    const auto f2 = []() { return 0; };
    REQUIRE_THREADSAFE(copper::select(chan1 >> f1, chan2 << f2) == copper::channel_op_status::success);
}
