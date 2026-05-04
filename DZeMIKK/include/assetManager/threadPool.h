#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <typeindex>
#include <functional>

namespace dzemikk {

class ThreadPool {
  public:
    struct AssetJob {
        std::string path;
        std::type_index type = typeid(void);
        std::function<void()> execute;
    };

    ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~ThreadPool();

    void start();
    void stop();

    void enqueue(AssetJob job);

  private:
    std::vector<std::thread> _workers;

    size_t _threadCount;
    std::queue<AssetJob> _jobs;
    std::mutex _queueMutex;
    std::condition_variable _cv;

    bool _running = true;
};
}
