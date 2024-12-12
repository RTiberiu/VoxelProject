#include "FairSemaphore.h"

FairSemaphore::FairSemaphore(int InitialCount) : Count(InitialCount) {}

void FairSemaphore::Acquire() {
    std::unique_lock<std::mutex> Lock(Mutex);
    std::condition_variable thisCondition;
    waitQueue.push(&thisCondition);

    while (Count == 0 || waitQueue.front() != &thisCondition) {
        thisCondition.wait(Lock);
    }

    waitQueue.pop();
    --Count;
}

void FairSemaphore::Release() {
    std::lock_guard<std::mutex> Lock(Mutex);
    ++Count;
    if (!waitQueue.empty()) {
        waitQueue.front()->notify_one();  // Notify the thread at the front of the queue
    }
}