
#include <copper.h>
#include <chrono>
#include <future>
#include <iostream>

using namespace std::chrono_literals;

struct loggo {
    loggo() { std::cout << "log()" << std::endl; }
    loggo(const loggo&) { std::cout << "log(const log&)" << std::endl; }
    loggo(loggo&&) noexcept { std::cout << "log(log&&)" << std::endl; }
    ~loggo() { std::cout << "~log()" << std::endl; }
    loggo& operator=(const loggo&) = delete;
    loggo& operator=(loggo&&) { std::cout << "operator=(log&&)" << std::endl; }
};

int main() {
    using copper::_;
    copper::buffered_channel<loggo> chan;
    loggo x;
    std::cout << std::endl;
    (void)copper::select(
        chan >> x,
        [] { std::cout << 1 << std::endl; },
        chan << x,
        [] { std::cout << 2 << std::endl; });
    std::cout << std::endl;
    (void)copper::select(
        chan >> x,
        [] { std::cout << 1 << std::endl; },
        chan << std::move(x),
        [] { std::cout << 2 << std::endl; });
}