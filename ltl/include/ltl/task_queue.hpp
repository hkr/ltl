#include <thread>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <functional>

namespace ltl {
    
class task_queue
{
public:
    task_queue();
    ~task_queue();
    
    task_queue(task_queue const&) =delete;
    task_queue& operator=(task_queue const&) =delete;
    
    void enqueue(std::function<void()> task);
    
private:
    mutable std::mutex mutex_;
    bool join_;
    std::condition_variable cv_;
    std::deque<std::function<void()>> queue_;
    std::thread thread_;
};
    
} // namespace ltl

