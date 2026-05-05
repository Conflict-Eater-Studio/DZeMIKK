#ifndef DZEMIKK_THREAD_POOL_H
#define DZEMIKK_THREAD_POOL_H

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <typeindex>
#include <functional>

namespace dzemikk {
/**
 * @brief Simple thread pool for background task execution.
 *
 * Used mainly for asynchronous asset loading and processing jobs.
 */
class ThreadPool {
  public:
    /**
     * @brief Single job executed by worker threads.
     */
    struct AssetJob {
        std::string path;
        std::function<void()> execute;
    };

    ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * @brief Starts worker threads.
     */
    void start();

    /**
     * @brief Starts worker threads.
     */
    void stop();

    /**
     * @brief Adds job to execution queue.
     */
    void enqueue(AssetJob job);

  private:
    /** @brief Worker threads */
    std::vector<std::thread> _workers;

    /** @brief Number of threads in pool */
    size_t _threadCount;

    /** @brief Job queue */
    std::queue<AssetJob> _jobs;

    /** @brief Protects job queue */
    std::mutex _queueMutex;

    /** @brief Condition variable for worker wake-up */
    std::condition_variable _cv;

    /** @brief Pool running state */
    bool _running = false;
};
} // namespace dzemikk
#endif // DZEMIKK_THREAD_POOL_H