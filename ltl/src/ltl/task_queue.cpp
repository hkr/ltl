#include "ltl/task_queue.hpp"

#include <condition_variable>
#include <thread>
#include <mutex>
#include <deque>

namespace ltl {
    
struct task_queue::impl
{
    impl()
    : mutex_()
    , join_(false)
    , cv_()
    , queue_()
    , thread_([&](){
        
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [&](){ return !queue_.empty() || join_; });
                
                if (join_)
                    break;
                
                task.swap(queue_.front());
                queue_.pop_front();
            }
            
            if (task)
                task();
        }
    })
    {
    }
    
    ~impl()
    {
        if (thread_.joinable())
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                join_ = true;
                cv_.notify_one();
            }
            thread_.join();
        }
    }
    
    void enqueue(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(task));
        if (queue_.size() == 1)
            cv_.notify_one();
    }
    
    mutable std::mutex mutex_;
    bool join_;
    std::condition_variable cv_;
    std::deque<std::function<void()>> queue_;
    std::thread thread_;
};
    
task_queue::task_queue()
: impl_(new impl())
{
    
}
    
task_queue::~task_queue()
{

}
    
void task_queue::enqueue(std::function<void()> task)
{
    impl_->enqueue(std::move(task));
}

    
} // namespace ltl
