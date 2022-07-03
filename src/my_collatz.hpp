#ifndef MY_COLLATZ_HPP
#define MY_COLLATZ_HPP

#include "sharedresults.hpp"
#include <cassert>
#include <stack>

uint64_t t_myCalcCollatz(std::shared_ptr<SharedResults> sr, InfInt n) {
    uint64_t count = 0;
    std::stack<InfInt> intStack;
    auto clearStack = [&](uint64_t initialCount) {
        while (!intStack.empty()) {
            InfInt val = intStack.top();
            intStack.pop();
            sr->atomicWrite(val, ++initialCount);
        }
    };

    assert(n > 0);
    while (n != 1) {
        uint64_t cnt;
        if (sr->atomicRead(n, cnt)) {
            clearStack(cnt);
            return count + cnt;
        }

        intStack.push(n);
        count++;
        if (n % 2 == 1) {
            n *= 3;
            n += 1;
        } else {
            n /= 2;
        }
    }
    clearStack(0);
    return count;
}

uint64_t p_myCalcCollatz(uint64_t* preprocessedResults, bool preprocessing, InfInt n) {
    uint64_t count = 0;
    std::stack<InfInt> intStack;
    static bool calculated[BUCKET_COUNT + 1] = {false};

    static auto tryRead = [&](InfInt &n, uint64_t &cnt) {
        if (n > BUCKET_COUNT)
            return false;

        unsigned long long ull_n = n.toUnsignedLongLong();
        if (!calculated[ull_n])
            return false;

        count = preprocessedResults[ull_n];
        return true;
    };

    static auto tryWrite = preprocessing ?
           std::function<void(InfInt &n, uint64_t count)>([&](InfInt &n, uint64_t count) {
                unsigned long long ull_n = n.toUnsignedLongLong();
                preprocessedResults[ull_n] = count;
                calculated[ull_n] = true;
            }) :
           std::function<void(InfInt &n, uint64_t count)>([&](InfInt &n, uint64_t count) {
                ;  // void on purpose
            });

    auto clearStack = [&](uint64_t initialCount) {
        if (preprocessing)
            while (!intStack.empty()) {
                InfInt val = intStack.top();
                intStack.pop();
                tryWrite(val, ++initialCount);
            }
    };

    assert(n > 0);
    while (n != 1) {
        uint64_t cnt;
        if (tryRead(n, cnt)) {
            clearStack(cnt);
            return count + cnt;
        }

        intStack.push(n);
        count++;
        if (n % 2 == 1) {
            n *= 3;
            n += 1;
        } else {
            n /= 2;
        }
    }
    clearStack(0);
    return count;
}

#endif /*MY_COLLATZ_HPP*/