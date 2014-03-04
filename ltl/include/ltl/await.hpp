#ifndef LTL_AWAIT_HPP
#define LTL_AWAIT_HPP

#include "ltl/detail/current_task_context.hpp"
#include "ltl/future.hpp"

namespace ltl {
    
template <typename> class future;
    
struct await_value
{
    template <class Future>
    typename Future::value_type operator()(Future&& f) const
    {
        return apply(f);
    }
    
    template <class T>
    T apply(future<T>& f) const
    {
        T value;
        task_context* ctx = current_task_context::get();
        
        auto&& resume_task = [&](T const& x) {
            value = x;
            ctx->resume();
        };
        
        f.then([&](T const& x) {
            f.get_state()->await_queue->execute_next([=]() {
                resume_task(x);
            });
        });
        
        ctx->yield();
        
        return value;
    }
    
    void apply(future<void>& f) const
    {
        task_context* ctx = current_task_context::get();
        
        auto&& resumeTask = [&]() {
            ctx->resume();
        };
        
        f.then([&]() {
            f.get_state()->await_queue->execute_next([=]() {
                resumeTask();
            });
        });
        
        ctx->yield();
    }
};

extern const await_value await;

template <typename T>
T operator|=(await_value const& a, future<T> f)
{
    return a.apply(f);
}

    
} // namespace ltl

#endif // LTL_AWAIT_HPP
