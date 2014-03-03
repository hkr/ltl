#include "ltl/detail/task_queue_impl.hpp"

#include "ltlcontext/ltlcontext.hpp"
#include "ltl/detail/task_context.hpp"
#include "ltl/detail/current_task_context.hpp"

#include <condition_variable>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>

#include "stdio.h"
#include "assert.h"

namespace ltl {
namespace detail {

    int counter = 0;
    
using namespace std::placeholders;
    
struct task_queue_impl::impl
{
    int instance;
    explicit impl(task_queue_impl* task_queue)
    : main_context_(create_main_context(), &destroy_main_context)
    , mutex_()
    , join_(false)
    , cv_()
    , queue_()
    , thread_([&]() {
        auto&& work_to_do = [&](){ return !queue_.empty() || join_; };
        
        while (true)
        {
            work_item wi;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                
                if (!work_to_do())
                {
                    //printf("waiting... %d\n", instance);
                    cv_.wait(lock, work_to_do);
                    //printf("waited %d\n", instance);
                }
                
                if (join_)
                    break;
                
                wi.swap(queue_.front());
                queue_.pop_front();
            }
            
            if (wi.first)
            {
                if (wi.second == Resumable)
                {
                    //printf("run_in_new_context %d\n", instance);
                    run_in_new_context(std::move(wi.first));
                    //printf("done with run_in_new_context %d\n", instance);
                }
                else
                {
                    //printf("wi.first() %d\n", instance);
                    wi.first();
                }
            }
        }
        
        printf("leaving thread loop %d\n", instance);
    })
    , task_queue_(task_queue)
    {
        instance = ++counter;
    }

    void run_in_new_context(std::function<void()>&& func)
    {
        assert(!current_task_context::get());
        
        std::shared_ptr<task_context> ctx;
        if (free_task_contexts_.empty())
        {
            ctx = task_context::create(main_context_.get(), std::bind(&impl::task_complete, this, _1),
                                       task_queue_);
        }
        else
        {
            ctx = free_task_contexts_.back();
            free_task_contexts_.pop_back();
        }
        //printf("run_in_new_context %d %016X\n", instance, ctx.get());

        ctx->activate(std::move(func));
        ctx->resume();
    }
    
    ~impl()
    {
         printf("~impl\n");
        join();
    }
    
    void enqueue_resumable(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_back(work_item(std::move(task), Resumable));
        if (queue_.size() == 1)
            cv_.notify_one();
    }
    
    void execute_next(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_front(work_item(std::move(task), First));
        if (queue_.size() == 1)
            cv_.notify_one();
    }
    
    void join()
    {
        if (thread_.joinable())
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.clear();
                join_ = true;
                cv_.notify_one();
            }
            thread_.join();
        }
    }
    
    void task_complete(std::shared_ptr<task_context> const& ctx)
    {
        free_task_contexts_.push_back(ctx);
    }
    
    enum scheduling_policy { Resumable, First };
    typedef std::function<void()> task;
    typedef std::pair<task, scheduling_policy> work_item;
    
    std::unique_ptr<context, void(*)(context*)> main_context_;
    mutable std::mutex mutex_;
    bool join_;
    std::condition_variable cv_;
    std::deque<work_item> queue_;
    std::thread thread_;
    std::vector<std::shared_ptr<task_context>> free_task_contexts_;
    task_queue_impl* const task_queue_;
};
    
task_queue_impl::task_queue_impl()
: impl_(new impl(this))
{
    
}

task_queue_impl::~task_queue_impl()
{
    
}

void task_queue_impl::enqueue_resumable(std::function<void()> task)
{
    impl_->enqueue_resumable(std::move(task));
}
    
void task_queue_impl::execute_next(std::function<void()> task)
{
    impl_->execute_next(std::move(task));
}

void task_queue_impl::join()
{
    impl_->join();
}
    
} // namespace detail
} // namespace ltl
