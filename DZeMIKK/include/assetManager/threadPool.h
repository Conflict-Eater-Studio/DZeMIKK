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
    std::vector<std::thread> workers;

    std::queue<AssetJob> jobs;
    std::mutex queueMutex;
    std::condition_variable cv;

    bool running = true;
};
}
