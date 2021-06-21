---

![logo](docs/copper_logo_small.png)

---


[![Build & Test](https://github.com/atollk/copper/actions/workflows/tests.yml/badge.svg)](https://github.com/atollk/copper/actions/workflows/tests.yml)
[![codecov](https://codecov.io/gh/atollk/copper/branch/main/graph/badge.svg?token=BCAT88OAU8)](https://codecov.io/gh/atollk/copper)

**Copper** is a C++ library of a powerful queue object for communication between threads. It is based on Go's channels and follows the quote:

>Don't communicate by sharing memory; share memory by communicating.

See [**docs/motivation.adoc**](docs/motivation.adoc) for a high-level description of what Copper is
capable of and how it stands out from previous C / C++ implementations of queues and Go-like channels.

Copper has...
* only a single header to include.
* no deadlocks; no race conditions; no undefined behavior; no polling.
* support for multiple producers and multiple consumers.
* an API based on that of `std` to avoid style clashes.

---

## Quick Example
```c++
#include <future>
#include <iostream>
#include "copper.h"

copper::buffered_channel<int> channel_1;
copper::buffered_channel<int> channel_2;

void producer_1() {
    // Push the numbers 0 to 9 into channel_1.
    for (auto i = 0; i < 10; ++i) {
        (void) channel_1.push(i);
    }
    channel_1.close();
}

void producer_2() {
    // Push the numbers 0 to 9 into channel_2.
    for (auto i = 0; i < 10; ++i) {
        (void) channel_2.push(i);
    }
    channel_2.close();
}

void consumer() {
    // Until both channel_1 and channel_2 are closed, get the next message from either and print it.
    while (copper::select(
        channel_1 >> [](int x) { std::cout << "Message from producer 1: " << x << std::endl; },
        channel_2 >> [](int x) { std::cout << "Message from producer 2: " << x << std::endl; }
    ) == copper::channel_op_status::success);
}

int main() {
    const auto p1 = std::async(producer_1);
    const auto p2 = std::async(producer_2);
    const auto c = std::async(consumer);
    return 0;
}
```

---

##  More Information

### API Reference

See [docs/reference.adoc](docs/reference.adoc) for a detailed reference of the public code interface.

### Efficiency & Benchmarks

See [docs/techdetails.adoc](docs/techdetails.adoc) for technical information about efficiency
and some benchmarks.

---

## Dependency managers

### CPM

Include the project in your CMakeLists.txt
```cmake
CPMAddPackage(
  GITHUB_REPOSITORY atollk/copper
  VERSION 1.1
)

target_link_libraries(MyProject Threads::Threads copper)
```

