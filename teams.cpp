#include <memory>
#include <utility>
#include <future>

#include "lib/threadraii/ThreadRAII.h"
#include "teams.hpp"
#include "contest.hpp"
#include "collatz.hpp"
#include "my_collatz.hpp"

//TODO: powciskać wszędzie timery
//TODO: pomyśleć o push/emplace_back i wektorach zamiast tablic

ContestResult TeamNewThreads::runContestImpl(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzNewThreadsTimer");

    std::condition_variable cond;
    std::mutex mtx;
    std::vector<std::thread> threadStack(contestInput.size());
    uint64_t activeThreads = 0, workDone = 0;

    auto work = ([&contestInput, &workDone, &cond, &activeThreads, &mtx]() {
        uint64_t id;
        {
            std::unique_lock<std::mutex> lock(mtx);
            id = workDone;
        }
        calcCollatz(contestInput[id]);
        {
            std::unique_lock<std::mutex> lock(mtx);
            workDone++;
            activeThreads--;
        }
        cond.notify_one();
    });

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        if (workDone < contestInput.size()) {
            while (activeThreads == this->getSize()) {
                cond.wait(lock); // wait until one of the children wakes me up
            }
            activeThreads++;
            threadStack.emplace_back(std::move(createThread(work)));
        }
        else
            break;
    }

    for (auto & t : threadStack)
        t.join();

    return res;
}

//TODO: poniżej interpretacja dla bloków wątków
/*ContestResult TeamNewThreads::runContestImpl(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzNewThreadsTimer");

    std::thread threads[this->getSize()];
    uint64_t work_done = 0;
    while (work_done < contestInput.size()) {
        for (uint64_t j = 0; j < this->getSize() || work_done == contestInput.size(); j++) {
            threads[j] = std::thread(calcCollatz, contestInput[work_done]);
        }

        for (uint32_t i = 0; i < this->getSize(); i++) {
            threads[i].join();
            work_done++;
        }
    }

    return res;
}*/

ContestResult TeamConstThreads::runContestImpl(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzConstThreadsTimer");
    std::thread threads[this->getSize()];

    for (uint64_t i = 0; i < this->getSize(); i++) {
       threads[i] = std::move(createThread([this, i, &res, &contestInput]() {
           for (uint64_t j = i; j < contestInput.size(); j += this->getSize()) {
               res[j] = calcCollatz(contestInput[j]);
           }
       }));
    }

    for (uint32_t i = 0; i < this->getSize(); i++)
        threads[i].join();

    return res;
}

//TODO: alternatywa dla wektora - tablica
ContestResult TeamPool::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzTeamPoolTimer");

    std::future<uint64_t> futures[contestInput.size()];
    for (uint64_t i = 0; i < contestInput.size(); i++) {
        futures[i] = this->pool.push(calcCollatz, contestInput[i]);
    }

    for (uint64_t i = 0; i < contestInput.size(); i++) {
        res[i] = futures[i].get();
    }

    return res;
}

ContestResult TeamNewProcesses::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzTeamNewProcessesTimer");



    return res;
}

ContestResult TeamConstProcesses::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzTeamConstProcessesTimer");



    return res;
}

ContestResult TeamAsync::runContest(ContestInput const & contestInput) {
    ContestResult res;
    res.resize(contestInput.size());
    //rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzTeamAsyncTimer");
    std::future<uint64_t> futures[contestInput.size()];

    if (!this->getSharedResults()) {
        for (uint64_t i = 0; i < contestInput.size(); i++) {
            futures[i] = std::async(std::launch::async, calcCollatz, contestInput[i]);
        }
    }
    else {
        for (uint64_t i = 0; i < contestInput.size(); i++) {
            futures[i] = std::async(std::launch::async, [&contestInput, this, i](){
                return myCalcCollatz(*this->getSharedResults(), contestInput[i]);
            });
        }
    }

    for (uint64_t i = 0; i < contestInput.size(); i++) {
        res[i] = futures[i].get();
    }

    return res;
}
