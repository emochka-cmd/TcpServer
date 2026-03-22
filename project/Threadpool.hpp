#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <condition_variable>
#include <type_traits>
#include <utility>
#include <vector>
 
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;   
    std::mutex queMutex;
    std::condition_variable cv;
    bool stop;       

public:
    explicit ThreadPool(size_t countThreads = 4) : stop(false) {
        for (size_t i = 0; i < countThreads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queMutex);
                        cv.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                       
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }
 
    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&)                 = delete;
    ThreadPool& operator=(ThreadPool&&)      = delete;
 
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;
 
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return std::invoke(std::move(f), std::move(args)...);
            }
        );
 
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queMutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task] { (*task)(); });
        }
        cv.notify_one();
        return res;
    }
 
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queMutex);
            stop = true;
        }
        cv.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};

