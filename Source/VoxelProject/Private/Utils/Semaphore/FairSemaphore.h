#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

class FairSemaphore {
public:
	// Constructor to initialize the semaphore with the initial count
	FairSemaphore(int InitialCount);

	// Acquire a permit from the semaphore
	void Acquire();

	// Release a permit to the semaphore
	void Release();

private:
	std::mutex Mutex;
	std::condition_variable Condition;
	std::queue<std::condition_variable*> waitQueue;
	int Count;

};
