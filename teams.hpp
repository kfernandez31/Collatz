#ifndef TEAMS_HPP
#define TEAMS_HPP

#include <thread>
#include <cassert>

#include "lib/rtimers/cxx11.hpp"
#include "lib/pool/cxxpool.h"

#include "contest.hpp"
#include "collatz.hpp"
//#include "my_collatz.hpp"
#include "sharedresults.hpp"

class Team {
public:
    explicit Team(uint32_t sizeArg): size(sizeArg) {
        assert(this->size > 0);
    }
    virtual ~Team() = default;

    virtual std::string getInnerName() = 0;
    std::shared_ptr<SharedResults> getSharedResults() { return sharedResults; };
    virtual ContestResult runContest(ContestInput const & contest) = 0;
    std::string getXname() { return this->getSharedResults() ? "X" : ""; }
    virtual std::string getTeamName() { return this->getInnerName() + this->getXname() + "<" + std::to_string(this->size) + ">"; }
    [[nodiscard]] uint32_t getSize() const { return this->size; }

private:
    uint32_t size;
protected:
    std::shared_ptr<SharedResults> sharedResults;
};

class TeamSolo : public Team {
public:
    explicit TeamSolo(uint32_t sizeArg): Team(1) {} // ignore size, don't share

    ContestResult runContest(ContestInput const & contestInput) override {
        ContestResult result;
        result.resize(contestInput.size());
        uint64_t idx = 0;

        rtimers::cxx11::DefaultTimer soloTimer("CalcCollatzSoloTimer");

        for (InfInt const & singleInput : contestInput) {
            auto scopedStartStop = soloTimer.scopedStart();
            result[idx] = calcCollatz(singleInput);
            idx++;
        }
        return result;
    }

    std::string getInnerName() override { return "TeamSolo"; }
};

class TeamThreads : public Team {
public:
    TeamThreads(uint32_t sizeArg, bool shareResults): Team(sizeArg), createdThreads(0){
        if (shareResults) {
            this->sharedResults.reset(new SharedThreadResults());
        }
    }
    
    template< class Function, class... Args >
    std::thread createThread(Function&& f, Args&&... args) {
        ++this->createdThreads;
        return std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
    }

    void resetThreads() { this->createdThreads = 0; } 
    [[nodiscard]] uint64_t getCreatedThreads() const { return this->createdThreads; }

private:
    uint64_t createdThreads;
};

class TeamNewThreads : public TeamThreads {
public:
    TeamNewThreads(uint32_t sizeArg, bool shareResults): TeamThreads(sizeArg, shareResults) {}

    ContestResult runContest(ContestInput const & contestInput) override {
        this->resetThreads();
        ContestResult result = this->runContestImpl(contestInput);
        assert(contestInput.size() == this->getCreatedThreads());
        return result;
    }

    virtual ContestResult runContestImpl(ContestInput const & contestInput);

    std::string getInnerName() override { return "TeamNewThreads"; }
};

class TeamConstThreads : public TeamThreads {
public:
    TeamConstThreads(uint32_t sizeArg, bool shareResults): TeamThreads(sizeArg, shareResults) {}

    ContestResult runContest(ContestInput const & contestInput) override {
        this->resetThreads();
        ContestResult result = this->runContestImpl(contestInput);
        assert(this->getSize() == this->getCreatedThreads());
        return result;
    }

    virtual ContestResult runContestImpl(ContestInput const & contestInput);

    std::string getInnerName() override { return "TeamConstThreads"; }
};

class TeamPool : public Team {
public:
    TeamPool(uint32_t sizeArg, bool shareResults): Team(sizeArg), pool(sizeArg) {
        if (shareResults) {
            this->sharedResults.reset(new SharedThreadResults());
        }
    }

    ContestResult runContest(ContestInput const & contestInput) override;

    std::string getInnerName() override { return "TeamPool"; }

private:
    cxxpool::thread_pool pool;
    std::shared_ptr<SharedThreadResults> sharedResults;
};

class TeamNewProcesses : public Team {
public:
    TeamNewProcesses(uint32_t sizeArg, bool shareResults): Team(sizeArg) {
        if (shareResults) {
            this->sharedResults.reset(new SharedProcessResults());
        }
    }

    ContestResult runContest(ContestInput const & contestInput) override;

    std::string getInnerName() override { return "TeamNewProcesses"; }
};

class TeamConstProcesses : public Team {
public:
    TeamConstProcesses(uint32_t sizeArg, bool shareResults): Team(sizeArg) {
        if (shareResults) {
            this->sharedResults.reset(new SharedProcessResults());
        }
    }

    ContestResult runContest(ContestInput const & contestInput) override;

    std::string getInnerName() override { return "TeamConstProcesses"; }
};

class TeamAsync : public Team {
public:
    TeamAsync(uint32_t sizeArg, bool shareResults): Team(1) {
        if (shareResults) {
            this->sharedResults.reset(new SharedThreadResults());
        }
    } // ignore size

    ContestResult runContest(ContestInput const & contestInput) override;

    std::string getInnerName() override { return "TeamAsync"; }
};

#endif