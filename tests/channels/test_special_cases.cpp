/**
 * Tests for special cases, e.g. regression tests for former bugs.
 */

#include "../util.h"
#include <string>
#include <thread>

CHANNEL_TEST_CASE("GitHub Issue #23", "[copper]") {
    {
		copper::buffered_channel<std::string> channel;
        std::vector<std::thread*> ths;
        for (size_t i = 0; i < 5; i++) {
            ths.push_back(new std::thread([&]() {
                for (auto& item : channel) {
                }
            }));
        }
        channel.close();
        for (auto item : ths) {
            item->join();
        }
    }
}