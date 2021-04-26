
#include <copper.h>
#include <chrono>
#include <future>
#include <iostream>

using namespace std::chrono_literals;

copper::buffered_channel<int> randoms;

void work() {
    auto r = copper::_detail::thread_local_randomizer::create();
    std::cout << "work" << std::endl;
    auto run = true;
    auto next_r = std::optional<int>();
    while (run) {
        if (!next_r) {
            next_r = r(10'000);
            std::this_thread::sleep_for(1s);
        }
        run = copper::select(
                  randoms <<
                      [&next_r] {
                          std::cout << "<" << std::flush;
                          auto v = next_r.value();
                          next_r.reset();
                          return v;
                      },
                  randoms >>
                      [](int x) {
                          std::cout << ">" << std::flush;
                          if (x % 100 == 0) {
                              std::cout << x / 1000 << " " << std::flush;
                          }
                      }) == copper::channel_op_status::success;
    }
}

int main() {
    constexpr auto N = 100;
    auto ts = std::array<std::future<void>, N>();
    for (auto& t : ts) {
        t = std::async(work);
    }
    std::this_thread::sleep_for(10s);
    randoms.close();
}