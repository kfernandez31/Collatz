#ifndef SHAREDRESULTS_HPP
#define SHAREDRESULTS_HPP

#include <unordered_map>
#include <shared_mutex>
#include <functional>
#include <semaphore.h>

#include "contest.hpp"
#include "lib/infint/InfInt.h"

#define BUCKET_COUNT 999983 // magic prime number just under 10e6

class SharedResults {
public:
    SharedResults() : map(BUCKET_COUNT + 1, this->hashFun) {}
    ~SharedResults() = default;

    bool atomicRead(InfInt &n, uint64_t &count) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto search = map.find(n);
        if (search != map.end()) {
            count = search->second;
            return true;
        }

        return false;
    }

    bool atomicWrite(InfInt &n, uint64_t &count) {
        std::unique_lock<std::shared_mutex> lock(mtx);
        //map[n] = count;
        map.insert({n, count});
        return true;
    }

private:
    std::shared_mutex mtx;
    std::function<uint64_t(InfInt)> hashFun = [](InfInt x) {
        size_t seed = x.getVal().size();
        for (ELEM_TYPE& i : x.getVal()) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    };
    std::unordered_map<InfInt, uint64_t, decltype(hashFun)> map;
};

#endif
