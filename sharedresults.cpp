#include "sharedresults.hpp"
#include "my_collatz.hpp"
#include <algorithm>
#include <cassert>

bool SharedThreadResults::atomicRead(InfInt &n, uint64_t &count) {
    std::shared_lock<std::shared_timed_mutex> lock(mtx);

    auto search = map.find(n);
    if (search != map.end()) {
        count = search->second;
        return true;
    }

    return false;
}

bool SharedThreadResults::atomicWrite(InfInt &n, uint64_t &count) {
    std::unique_lock<std::shared_timed_mutex> lock(mtx);
    map[n] = count;
    return true;
}

bool SharedProcessResults::atomicRead(InfInt &n, uint64_t &count) {
    if (n > bucketCount)
        return false;

    unsigned long long ull_n = n.toUnsignedLongLong();
    if (!calculated[ull_n])
        return false;

    count = map[ull_n];
    return true;
}

bool SharedProcessResults::atomicWrite(InfInt &n, uint64_t &count) {
    if (n > bucketCount)
        return false;

    unsigned long long ull_n = n.toUnsignedLongLong();
    assert(!calculated[ull_n]);

    count = map[ull_n];
    return true;
}

void SharedProcessResults::initMap(ContestInput const & contestInput) {
    InfInt max_input =* max_element(contestInput.begin(), contestInput.end());
    size_t max_idx = std::min(max_input, bucketCount).toUnsignedInt();

    for (size_t i = max_idx; i >= 1; i--) {
        map[i] = myCalcCollatz(*this, max_input);
    }
}