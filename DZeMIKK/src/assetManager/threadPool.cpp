#include "assetManager/threadPool.h"

dzemikk::ThreadPool::ThreadPool(size_t threadCount) : running(true) {
    workers.reserve(threadCount);
}

void dzemikk::ThreadPool::start() {
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
        workers.emplace_back([this]() {
            while (true) {
                AssetJob job;

                {
                    std::unique_lock lock(queueMutex);

                    cv.wait(lock, [this]() { return !jobs.empty() || !running; });

                    if (!running && jobs.empty())
                        return;

                    job = std::move(jobs.front());
                    jobs.pop();
                }

                job.execute();
            }
        });
    }
}

void dzemikk::ThreadPool::enqueue(AssetJob job) {
    {
        std::lock_guard lock(queueMutex);
        jobs.push(std::move(job));
    }

    cv.notify_one();
}

void dzemikk::ThreadPool::stop() {
    {
        std::lock_guard lock(queueMutex);
        running = false;
    }

    cv.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable())
            worker.join();
    }

    workers.clear();
}

dzemikk::ThreadPool::~ThreadPool() {
    stop();
}