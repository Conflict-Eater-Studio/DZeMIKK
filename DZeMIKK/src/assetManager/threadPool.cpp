#include "assetManager/threadPool.h"

dzemikk::ThreadPool::ThreadPool(size_t threadCount) : _running(true), _threadCount(threadCount) {
    _workers.reserve(threadCount);
}

void dzemikk::ThreadPool::start() {
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
        _workers.emplace_back([this]() {
            while (true) {
                AssetJob job;

                {
                    std::unique_lock lock(_queueMutex);

                    _cv.wait(lock, [this]() { return !_jobs.empty() || !_running; });

                    if (!_running && _jobs.empty())
                        return;

                    job = std::move(_jobs.front());
                    _jobs.pop();
                }

                job.execute();
            }
        });
    }
}

void dzemikk::ThreadPool::enqueue(AssetJob job) {
    {
        std::lock_guard lock(_queueMutex);
        _jobs.push(std::move(job));
    }

    _cv.notify_one();
}

void dzemikk::ThreadPool::stop() {
    {
        std::lock_guard lock(_queueMutex);
        _running = false;
    }

    _cv.notify_all();

    for (auto& worker : _workers) {
        if (worker.joinable())
            worker.join();
    }

    _workers.clear();
}

dzemikk::ThreadPool::~ThreadPool() {
    stop();
}