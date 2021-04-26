#include "../tests/util.h"

using namespace std::chrono_literals;

CHANNEL_TEST_CASE("Single pop select on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push(1));
        REQUIRE_THREADSAFE(chan.push(2));
        REQUIRE_THREADSAFE(chan.push(3));
    });
    auto result = 0;
    auto f = [&result](int x) { result = x; };
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 3);
}

CHANNEL_TEST_CASE("Single push select on channels works correctly.", "[copper]") {
    auto chan = channel_t();
    auto f = []() { return 1; };
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.pop().value() == 1);
        REQUIRE_THREADSAFE(chan.pop().value() == 1);
        REQUIRE_THREADSAFE(!chan.try_pop());
    });
    REQUIRE_THREADSAFE(copper::select(chan << f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(copper::select(chan << f) == copper::channel_op_status::success);
}

CHANNEL_TEST_CASE("push select on one closed channels stops.", "[copper]") {
    auto chan = channel_t();
    chan.close();
    const auto f = []() { return 0; };
    REQUIRE_THREADSAFE(copper::select(chan << f) == copper::channel_op_status::closed);
}

CHANNEL_TEST_CASE("pop select on one closed channels stops.", "[copper]") {
    auto chan = channel_t();
    auto task = std::async([&chan]() {
        REQUIRE_THREADSAFE(chan.push(1));
        chan.close();
    });
    auto result = 0;
    const auto f = [&result](int x) { result = x; };
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    task.wait();
    REQUIRE_THREADSAFE(copper::select(chan >> f) == copper::channel_op_status::closed);
}

CHANNEL_TEST_CASE("push select on one channels reacts to a later close.", "[copper]") {
    auto chan = channel_t();
    const auto f = [](int) {};
    auto fut = std::async([&chan, &f]() {
        REQUIRE_THREADSAFE(copper::try_select_for(1s, chan >> f) == copper::channel_op_status::closed);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    chan.close();
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

CHANNEL_TEST_CASE("pop select on one filled channel and one empty channel works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto task = std::async([&chan1]() {
        REQUIRE_THREADSAFE(chan1.push(1));
        REQUIRE_THREADSAFE(chan1.push(2));
    });
    auto result = 0;
    auto f = [&result](int x) { result = x; };
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 1);
    REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    REQUIRE_THREADSAFE(result == 2);
}

CHANNEL_TEST_CASE("pop select on two pre-filled channels works correctly.", "[copper]") {
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

CHANNEL_TEST_CASE("push select on two channels works correctly.", "[copper]") {
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

CHANNEL_TEST_CASE("pop select on two empty channel reacts to a later push.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto result = 0;
    const auto f = [&result](int x) { result = x; };
    auto fut = std::async([&chan1, &chan2, &f]() {
        REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan1.push(1));
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
    REQUIRE_THREADSAFE(result == 1);
}

CHANNEL_TEST_CASE("push select on two full channel reacts to a later pop.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    fill_channel(chan1, 1);
    fill_channel(chan2, 1);
    const auto f = []() { return 1; };
    auto fut = std::async([&chan1, &chan2, &f]() {
        REQUIRE_THREADSAFE(copper::try_select_for(1s, chan1 << f, chan2 << f) == copper::channel_op_status::success);
    });
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    REQUIRE_THREADSAFE(chan1.try_pop().value() == 1);
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

CHANNEL_TEST_CASE("select on two closed channels stops.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    chan1.close();
    chan2.close();
    const auto f1 = [](int) {};
    const auto f2 = []() { return 0; };
    REQUIRE_THREADSAFE(copper::select(chan1 >> f1, chan2 << f2) == copper::channel_op_status::closed);
}

CHANNEL_TEST_CASE("select does not consider closed channels.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    chan2.close();
    REQUIRE_THREADSAFE(copper::try_select_for(
                           50ms,
                           chan1 >> [](int) {},
                           chan2 << [] { return 0; }) == copper::channel_op_status::unavailable);
}

CHANNEL_TEST_CASE("selecting on the same channel twice at once works correctly.", "[copper]") {
    auto chan = channel_t(1);
    auto result = 0;
    const auto f1 = []() { return 1; };
    const auto f2 = [&result](int x) { result = x; };
    auto task = std::async([&chan, f1, f2]() {
        REQUIRE_THREADSAFE(copper::select(chan << f1, chan >> f2) == copper::channel_op_status::success);
    });
    REQUIRE_THREADSAFE(copper::select(chan << f1, chan >> f2) == copper::channel_op_status::success);
    task.wait();
    REQUIRE_THREADSAFE(result == 1);
}

CHANNEL_TEST_CASE("select on two channels reacts to a later close.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    const auto f = [](int) {};
    auto fut = std::async([&chan1, &chan2, &f]() {
        REQUIRE_THREADSAFE(copper::try_select_for(1s, chan1 >> f, chan2 >> f) == copper::channel_op_status::closed);
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

CHANNEL_TEST_CASE("select on a closed channel and an open channel is cancelled when the second channel closes.",
                  "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    chan1.close();
    const auto f = [](int) {};
    auto fut = std::async([&chan1, &chan2, &f]() {
        REQUIRE_THREADSAFE(copper::try_select_for(1s, chan1 >> f, chan2 >> f) == copper::channel_op_status::closed);
    });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(100ms) == std::future_status::timeout);
    chan2.close();
    std::this_thread::yield();
    REQUIRE_THREADSAFE(fut.wait_for(50ms) == std::future_status::ready);
}

CHANNEL_TEST_CASE("Multiple parallel pop selects can be used with the same channel.", "[copper]") {
    auto chan1 = channel_t();
    const auto f = [](int) {};
    auto fut1 = std::async([&chan1, f] {
        auto chan2 = channel_t();
        REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    });
    auto fut2 = std::async([&chan1, f] {
        auto chan2 = channel_t();
        REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f) == copper::channel_op_status::success);
    });
    auto fut3 = std::async(
        [&chan1, f] { REQUIRE_THREADSAFE(copper::select(chan1 >> f) == copper::channel_op_status::success); });
    auto fut4 = std::async([&chan1] { REQUIRE_THREADSAFE(chan1.pop()); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(chan1.push(0));
    REQUIRE_THREADSAFE(chan1.push(0));
    REQUIRE_THREADSAFE(chan1.push(0));
    REQUIRE_THREADSAFE(chan1.push(0));
}

CHANNEL_TEST_CASE("Multiple parallel push selects can be used with the same channel.", "[copper]") {
    auto chan1 = channel_t();
    fill_channel(chan1);
    const auto f = [] { return 0; };
    auto fut1 = std::async([&chan1, f] {
        auto chan2 = channel_t();
        fill_channel(chan2);
        REQUIRE_THREADSAFE(copper::select(chan1 << f, chan2 << f) == copper::channel_op_status::success);
    });
    auto fut2 = std::async([&chan1, f] {
        auto chan2 = channel_t();
        fill_channel(chan2);
        REQUIRE_THREADSAFE(copper::select(chan1 << f, chan2 << f) == copper::channel_op_status::success);
    });
    auto fut3 = std::async(
        [&chan1, f] { REQUIRE_THREADSAFE(copper::select(chan1 << f) == copper::channel_op_status::success); });
    auto fut4 = std::async([&chan1] { REQUIRE_THREADSAFE(chan1.push(0)); });
    std::this_thread::yield();
    REQUIRE_THREADSAFE(chan1.pop());
    REQUIRE_THREADSAFE(chan1.pop());
    REQUIRE_THREADSAFE(chan1.pop());
    REQUIRE_THREADSAFE(chan1.pop());
}

CHANNEL_TEST_CASE(
    "Multi-select, single-select, and normal pop on channel all have the same chance of being used when running in parallel.",
    "[copper]") {
    constexpr int N = 100;

    auto chan1 = channel_t(1);
    auto keep_running = std::atomic<bool>(true);
    auto result = std::vector<int>();
    auto result_mutex = std::mutex();
    const auto push1 = [&result, &result_mutex](int) {
        const auto lock = std::lock_guard(result_mutex);
        result.push_back(1);
    };
    const auto push2 = [&result, &result_mutex](int) {
        const auto lock = std::lock_guard(result_mutex);
        result.push_back(2);
    };
    const auto push3 = [&result, &result_mutex](int) {
        const auto lock = std::lock_guard(result_mutex);
        result.push_back(3);
    };

    auto fut1 = std::async([&chan1, &push1, &keep_running] {
        auto chan2 = channel_t();
        while (keep_running) {
            REQUIRE_THREADSAFE(copper::select(
                                   chan1 >> push1,
                                   chan2 >> [](int) { FAIL(); }) == copper::channel_op_status::success);
        }
    });
    auto fut2 = std::async([&chan1, &push2, &keep_running] {
        while (keep_running) {
            REQUIRE_THREADSAFE(copper::select(chan1 >> push2) == copper::channel_op_status::success);
        }
    });
    auto fut3 = std::async([&chan1, &push3, &keep_running] {
        while (keep_running) {
            push3(chan1.pop().value());
        }
    });

    SECTION("Instant pushes.") {
        for (auto i = 0; i < N; ++i) {
            REQUIRE_THREADSAFE(chan1.push(1));
        }
    }
    SECTION("Sleep between pushes.") {
        for (auto i = 0; i < N; ++i) {
            std::this_thread::sleep_for(2ms);
            REQUIRE_THREADSAFE(chan1.push(1));
        }

        result_mutex.lock();
        const auto count_1 = std::count(result.begin(), result.end(), 1);
        const auto count_2 = std::count(result.begin(), result.end(), 2);
        const auto count_3 = std::count(result.begin(), result.end(), 3);
        result_mutex.unlock();

        REQUIRE_THREADSAFE(count_1 > 0);
        REQUIRE_THREADSAFE(count_2 > 0);
        REQUIRE_THREADSAFE(count_3 > 0);
    }

    keep_running = false;
    while (fut1.wait_for(1ms) == std::future_status::timeout || fut2.wait_for(1ms) == std::future_status::timeout ||
           fut3.wait_for(1ms) == std::future_status::timeout) {
        (void)chan1.try_push(1);
    }

    REQUIRE_THREADSAFE(result.size() >= N);
}

CHANNEL_TEST_CASE(
    "Multi-select, single-select, and normal push on channel all have the same chance of being used when running in parallel.",
    "[copper]") {
    constexpr int N = 100;

    auto chan1 = channel_t(1);
    auto keep_running = std::atomic<bool>(true);
    auto result = std::vector<int>();

    auto fut1 = std::async([&chan1, &keep_running] {
        auto chan2 = channel_t();
        while (keep_running) {
            REQUIRE_THREADSAFE(copper::select(
                                   chan1 << [] { return 1; },
                                   chan2 >> [](int) { FAIL(); }) == copper::channel_op_status::success);
        }
    });
    auto fut2 = std::async([&chan1, &keep_running] {
        while (keep_running) {
            REQUIRE_THREADSAFE(copper::select(chan1 << [] { return 2; }) == copper::channel_op_status::success);
        }
    });
    auto fut3 = std::async([&chan1, &keep_running] {
        while (keep_running) {
            REQUIRE_THREADSAFE(chan1.push_func([] { return 3; }) == copper::channel_op_status::success);
        }
    });

    SECTION("Instant pushes.") {
        std::this_thread::sleep_for(10ms);
        for (auto i = 0; i < N; ++i) {
            std::this_thread::yield();
            result.push_back(chan1.pop().value());
        }
    }
    SECTION("Sleep between pushes.") {
        for (auto i = 0; i < N; ++i) {
            std::this_thread::sleep_for(2ms);
            result.push_back(chan1.pop().value());
        }

        const auto count_1 = std::count(result.begin(), result.end(), 1);
        const auto count_2 = std::count(result.begin(), result.end(), 2);
        const auto count_3 = std::count(result.begin(), result.end(), 3);

        REQUIRE_THREADSAFE(count_1 > 0);
        REQUIRE_THREADSAFE(count_2 > 0);
        REQUIRE_THREADSAFE(count_3 > 0);
    }

    REQUIRE_THREADSAFE(result.size() >= N);
    keep_running = false;
    while (fut1.wait_for(1ms) == std::future_status::timeout || fut2.wait_for(1ms) == std::future_status::timeout ||
           fut3.wait_for(1ms) == std::future_status::timeout) {
        (void)chan1.try_pop();
    }
}

CHANNEL_TEST_CASE("Triple select works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto chan3 = channel_t();
    for (auto n = 0; n < 100; ++n) {
        auto sum = 0;
        const auto f = [&sum](int i) { sum += i; };
        auto task = std::async([&chan1, &chan2, &chan3, f]() {
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f) ==
                               copper::channel_op_status::success);
        });
        REQUIRE_THREADSAFE(chan1.push(0));
        REQUIRE_THREADSAFE(chan2.push(2));
        REQUIRE_THREADSAFE(chan3.push(4));
        task.wait();
        REQUIRE_THREADSAFE(sum == 6);
    }
}

CHANNEL_TEST_CASE("Quadruple select works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto chan3 = channel_t();
    auto chan4 = channel_t();
    for (auto n = 0; n < 100; ++n) {
        auto sum = 0;
        const auto f = [&sum](int i) { sum += i; };
        auto task = std::async([&chan1, &chan2, &chan3, &chan4, f]() {
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f) ==
                               copper::channel_op_status::success);
        });
        REQUIRE_THREADSAFE(chan1.push(0));
        REQUIRE_THREADSAFE(chan2.push(2));
        REQUIRE_THREADSAFE(chan3.push(4));
        REQUIRE_THREADSAFE(chan4.push(1));
        task.wait();
        REQUIRE_THREADSAFE(sum == 7);
    }
}

CHANNEL_TEST_CASE("Quintuple select works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto chan3 = channel_t();
    auto chan4 = channel_t();
    auto chan5 = channel_t();
    for (auto n = 0; n < 100; ++n) {
        auto sum = 0;
        const auto f = [&sum](int i) { sum += i; };
        auto task = std::async([&chan1, &chan2, &chan3, &chan4, &chan5, f]() {
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f) ==
                               copper::channel_op_status::success);
        });
        REQUIRE_THREADSAFE(chan1.push(0));
        REQUIRE_THREADSAFE(chan2.push(2));
        REQUIRE_THREADSAFE(chan3.push(4));
        REQUIRE_THREADSAFE(chan4.push(1));
        REQUIRE_THREADSAFE(chan5.push(3));
        task.wait();
        REQUIRE_THREADSAFE(sum == 10);
    }
}

CHANNEL_TEST_CASE("Sextuple select works correctly.", "[copper]") {
    auto chan1 = channel_t();
    auto chan2 = channel_t();
    auto chan3 = channel_t();
    auto chan4 = channel_t();
    auto chan5 = channel_t();
    auto chan6 = channel_t();
    for (auto n = 0; n < 100; ++n) {
        auto sum = 0;
        const auto f = [&sum](int i) { sum += i; };
        auto task = std::async([&chan1, &chan2, &chan3, &chan4, &chan5, &chan6, f]() {
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f, chan6 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f, chan6 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f, chan6 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f, chan6 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f, chan6 >> f) ==
                               copper::channel_op_status::success);
            REQUIRE_THREADSAFE(copper::select(chan1 >> f, chan2 >> f, chan3 >> f, chan4 >> f, chan5 >> f, chan6 >> f) ==
                               copper::channel_op_status::success);
        });
        REQUIRE_THREADSAFE(chan1.push(0));
        REQUIRE_THREADSAFE(chan2.push(2));
        REQUIRE_THREADSAFE(chan3.push(4));
        REQUIRE_THREADSAFE(chan4.push(1));
        REQUIRE_THREADSAFE(chan5.push(3));
        REQUIRE_THREADSAFE(chan6.push(5));
        task.wait();
        REQUIRE_THREADSAFE(sum == 15);
    }
}