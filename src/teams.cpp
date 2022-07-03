#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "teams.hpp"
#include "contest.hpp"
#include "collatz.hpp"
#include "my_collatz.hpp"

#define SYSERR(fun, err)                                                      \
    do {                                                                      \
        fprintf(stderr, "Runtime error: %s returned %d in %s at %s:%d\n%s\n", \
             fun, err, __func__, __FILE__, __LINE__, std::strerror(err));     \
        exit(EXIT_FAILURE);                                                   \
                                                                              \
    } while (0)

//TODO: powciskać wszędzie timery

ContestResult TeamNewThreads::runContestImpl(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());

    std::condition_variable cond;
    std::mutex mtx;
    uint64_t activeThreads = 0, workIdx = 0;

    auto regularWork = [&](size_t idx) {
        res[idx] = calcCollatz(contestInput[idx]);
        {
            std::unique_lock<std::mutex> lock(mtx);
            activeThreads--;
        }
        cond.notify_one();
    };

    auto memoisedWork = [&](size_t idx) {
        res[idx] = t_myCalcCollatz(this->getSharedResults(), contestInput[idx]);
        {
            std::unique_lock<std::mutex> lock(mtx);
            activeThreads--;
        }
        cond.notify_one();
    };

    auto work = this->getSharedResults()?
        std::function<void(size_t)>([&](size_t idx) { memoisedWork(idx); }) :
        std::function<void(size_t)>([&](size_t idx) { regularWork(idx); });

    while (workIdx < contestInput.size()) {
        std::unique_lock<std::mutex> lock(mtx);
        auto foo = [&]{ return workIdx < contestInput.size() && activeThreads < this->getSize(); };
        cond.wait(lock, foo);
        while (foo()) {
            activeThreads++;
            std::thread t = createThread(work, workIdx);
            t.detach(); //TODO: lepiej join
            workIdx++;
        }
    }

    {
        std::unique_lock<std::mutex> lock(mtx);
        cond.wait(lock, [&] { return activeThreads == 0; });
    }

    return res;
}

ContestResult TeamConstThreads::runContestImpl(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    std::thread threads[this->getSize()];

    auto regularWork = [this, &res, &contestInput](size_t i) {
        for (uint64_t j = i; j < contestInput.size(); j += this->getSize())
            res[j] = calcCollatz(contestInput[j]);
    };

    auto memoisedWork = [this, &res, &contestInput](size_t i) {
        for (uint64_t j = i; j < contestInput.size(); j += this->getSize())
            res[j] = t_myCalcCollatz(this->getSharedResults(), contestInput[j]);
    };

    auto work = this->getSharedResults()?
                std::function<void(size_t)>([&](size_t idx) { memoisedWork(idx); }) :
                std::function<void(size_t)>([&](size_t idx) { regularWork(idx); });

    for (uint64_t i = 0; i < this->getSize(); i++)
       threads[i] = std::move(createThread(work, i));

    for (uint32_t i = 0; i < this->getSize(); i++)
        threads[i].join();

    return res;
}

//TODO: alternatywa dla wektora - tablica
ContestResult TeamPool::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());

    std::future<uint64_t> futures[contestInput.size()];
    if (this->getSharedResults())
        for (uint64_t i = 0; i < contestInput.size(); i++) {
            futures[i] = this->pool.push(t_myCalcCollatz, this->getSharedResults(), contestInput[i]);
        }
    else
        for (uint64_t i = 0; i < contestInput.size(); i++)
            futures[i] = this->pool.push(calcCollatz, contestInput[i]);


    for (uint64_t i = 0; i < contestInput.size(); i++)
        res[i] = futures[i].get();

    return res;
}

ContestResult TeamNewProcesses::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());

    return res;
}

ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());

    InfInt max_input = *max_element(contestInput.begin(), contestInput.end());
    size_t max_idx = std::min(max_input, InfInt(BUCKET_COUNT)).toUnsignedInt();

    void* mapped_mem_all = nullptr;
    if ((mapped_mem_all = mmap(nullptr, sizeof(uint64_t) * (max_idx + 1 + contestInput.size()),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)
        ) == MAP_FAILED
    ) SYSERR("mmap", -1);

    // arrays of space `max_idx + 1` and `contestInput.size()` respectively;
    auto *preprocessedResults = (uint64_t*) (static_cast<char*>(mapped_mem_all));
    auto *finalResults = (uint64_t*) (static_cast<char*>(mapped_mem_all) + max_idx + 1);

    uint64_t idx;
    pid_t pid;

    for (idx = 1; idx <= max_idx; idx++)
        preprocessedResults[idx] = p_myCalcCollatz(preprocessedResults, true, max_input);
    //TODO: rozłożyć pracę

    auto regularWork = [this, &res, &contestInput](size_t i) {
        for (uint64_t j = i; j < contestInput.size(); j += this->getSize())
            res[j] = calcCollatz(contestInput[j]);
    };
    auto memoisedWork = [this, &res, &contestInput, &preprocessedResults](size_t i) {
        for (uint64_t j = i; j < contestInput.size(); j += this->getSize())
            res[j] = p_myCalcCollatz(preprocessedResults, false, contestInput[j]);
    };
    auto work = this->getSharedResults()?
                std::function<void(size_t)>([&](size_t i) { memoisedWork(i); }) :
                std::function<void(size_t)>([&](size_t i) { regularWork(i); });

    for (idx = 0; idx < this->getSize(); idx++)
        switch (pid = fork()) {
            case -1: /* error */
                SYSERR("fork", -1);
            case 0:  /* child */
                //std::cout << "[" << getpid() << "]" << "I've just been spawned" << std::endl;
                work(idx);
                munmap(preprocessedResults, sizeof(uint64_t) * (max_idx + 1));
                munmap(finalResults, sizeof(uint64_t) * contestInput.size());
                exit(EXIT_SUCCESS);
            default: /* parent */
                //std::cout << "[" << getpid() << "]" << "spawned child " << pid << std::endl;
                break;
        }

    for (idx = 0; idx < this->getSize(); idx++) {
        //std::cout << "waiting for my child..." << std::endl;
        if (wait(nullptr) == -1)
            SYSERR("wait", -1);
    }

    for (uint64_t i = 0; i < contestInput.size(); i++)
        res[i] = finalResults[i];

    munmap(mapped_mem_all, sizeof(uint64_t) * (max_idx + 1 + contestInput.size()));
    return res;
}

ContestResult TeamAsync::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    std::future<uint64_t> futures[contestInput.size()];

    if (!this->getSharedResults())
        for (uint64_t i = 0; i < contestInput.size(); i++)
            futures[i] = std::async(calcCollatz, contestInput[i]);
    else
        for (uint64_t i = 0; i < contestInput.size(); i++)
            futures[i] = std::async([&contestInput, this, i](){
                return t_myCalcCollatz(this->getSharedResults(), contestInput[i]);
            });

    for (uint64_t i = 0; i < contestInput.size(); i++)
        res[i] = futures[i].get();

    return res;
}
