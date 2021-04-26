
#include <copper.h>
#include <chrono>
#include <future>
#include <iostream>

using namespace std::chrono_literals;

int main() {
    copper::buffered_channel<int> chan;
    int x = 0;
    copper::select(chan << x, [] {});
}