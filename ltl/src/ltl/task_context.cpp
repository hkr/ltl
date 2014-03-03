#include "ltl/detail/task_context.hpp"
#include "ltl/detail/current_task_context.hpp"

#include <cstdlib>
#include <assert.h>

namespace ltl {
  
task_context::task_context(context* main,
                           std::function<void(std::shared_ptr<task_context> const&)> finished,
                           detail::task_queue_impl* tq)
: stack_(64 * 1024)
, func_()
, keep_alive_()
, main_(main)
, own_()
, finished_(std::move(finished))
, task_queue_(tq)
{
//    printf("created task_context %016X\n", this);
}

std::shared_ptr<task_context> task_context::create(context* main,
                                                   std::function<void(std::shared_ptr<task_context> const&)> finished,
                                                   detail::task_queue_impl* tq)
{
    return std::make_shared<task_context>(main, std::move(finished), tq);
}
    
void task_context::yield()
{
    assert(current_task_context::get());
    current_task_context::set(nullptr);
//    printf("task_context::yield %016X\n", this);
    jump(own_, main_);
}

void task_context::resume()
{
    assert(!current_task_context::get());
    current_task_context::set(this);

//    printf("task_context::resume %016X\n", this);
    jump(main_, own_);
}

void task_context::trampoline(context_data_t instance)
{
    task_context* const self = reinterpret_cast<task_context*>(instance);
    
    self->func_();
    self->finished_(self->keep_alive_);
//    printf("task_context::finished_ %016X\n", self);
    self->keep_alive_.reset();
    
    self->yield();
    
    std::abort();
}

void task_context::reset(std::function<void()>&& f)
{
//    printf("task_context::activate %016X\n", this);
    func_ = std::move(f);
    own_ = create_context(&stack_.front(), stack_.size() * sizeof(void*), &trampoline, reinterpret_cast<context_data_t>(this));
    keep_alive_ = shared_from_this();
}

detail::task_queue_impl* task_context::get_task_queue()
{
    return task_queue_;
}
    
} // nameaspace ltl
