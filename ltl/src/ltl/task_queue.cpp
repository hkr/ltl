#include <thread>
#include "ltl/task_queue.hpp"

namespace ltl {
    
task_queue::task_queue()
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
    
task_queue::~task_queue()
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
    
void task_queue::enqueue(std::function<void()> task)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push_back(std::move(task));
    if (queue_.size() == 1)
        cv_.notify_one();
}

    
} // namespace ltl
