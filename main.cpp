
#include <copper.h>
#include <chrono>
#include <future>
#include <iostream>

using namespace std::chrono_literals;

copper::buffered_channel<int> randoms;

int main() {
    constexpr auto N = 100;
    auto ts = std::array<std::future<void>, N>();
    for (auto& t : ts) {
        t = std::async(work);
    }
    std::this_thread::sleep_for(10s);
    randoms.close();
}