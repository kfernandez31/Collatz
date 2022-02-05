#ifndef SHAREDRESULTS_HPP
#define SHAREDRESULTS_HPP

#include <unordered_map>
#include <shared_mutex>
#include <functional>

#include "contest.hpp"
#include "lib/infint/InfInt.h"

#define BUCKET_COUNT 999983 // magic prime number

class SharedResults {
public:
    SharedResults() : bucketCount(BUCKET_COUNT) {}

    virtual bool atomicRead(InfInt &n, uint64_t &count) = 0;
    virtual bool atomicWrite(InfInt &n, uint64_t &count) = 0;

protected:
    const InfInt bucketCount;
};

class SharedThreadResults : public SharedResults {
public:
    SharedThreadResults() : map(BUCKET_COUNT, this->hashFun) {}

    bool atomicRead(InfInt &n, uint64_t &count) override;
    bool atomicWrite(InfInt &n, uint64_t &count) override;

private:
    std::shared_timed_mutex mtx;
    std::function<uint64_t(InfInt)> hashFun = [](InfInt x) {
        size_t seed = x.getVal().size();
        for (ELEM_TYPE& i : x.getVal()) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    };
    std::unordered_map<InfInt, uint64_t, decltype(hashFun)> map;
};

class SharedProcessResults : public SharedResults {
public:
    SharedProcessResults() {
        this->calculated.fill(false);
    }

    bool atomicRead(InfInt &n, uint64_t &count) override;
    bool atomicWrite(InfInt &n, uint64_t &count) override;
    virtual void initMap(ContestInput const & contestInput);

private:
    std::array<uint64_t, BUCKET_COUNT + 1> map;
    std::array<bool, BUCKET_COUNT + 1> calculated;
};

#endif
