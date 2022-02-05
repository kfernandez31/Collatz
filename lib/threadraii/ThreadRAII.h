#ifndef THREADRAII_H
#define THREADRAII_H

#include <thread>

class ThreadRAII {
public:
    enum class DtorAction { join, detach };

    ThreadRAII() : action(DtorAction::join) {};
    ThreadRAII(std::thread&& t, DtorAction a)
            : action(a), t(std::move(t)) {}

    ~ThreadRAII()
    {
        if (t.joinable()) {
            if (action == DtorAction::join) {
                t.join();
            } else {
                t.detach();
            }
        }
    }
    std::thread& get() { return t; }
private:
    DtorAction action;
    std::thread t;
};

#endif /* THREADRAII_H */