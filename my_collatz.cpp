#include "sharedresults.hpp"
#include <cassert>

uint64_t myCalcCollatz(SharedResults &sr, InfInt n) {
    uint64_t count = 0;
    assert(n > 0);
    while (n != 1) {
        uint64_t cnt = 0;

        if (sr.atomicRead(n, cnt))
            return count + cnt;

        count++;
        sr.atomicWrite(n, count);

        if (n % 2 == 1) {
            n *= 3;
            n += 1;
        }
        else {
            n /= 2;
        }
    }

    return count;
}