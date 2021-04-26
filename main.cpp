
#include <copper.h>
#include <chrono>
#include <future>
#include <iostream>

using namespace std::chrono_literals;

int main() {
    using copper::_;
    copper::buffered_channel<int> chan;
    copper::buffered_channel<void> vchan;
    int x = 0;
    int y = 0;
    (void)copper::select(
        chan >> x, [] { std::cout << 1; }, chan << y, [] { std::cout << 2; }, vchan >> _, [] { std::cout << 3; },
        vchan << _, [] { std::cout << 4; });
}