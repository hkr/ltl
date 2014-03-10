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
    T apply(future<T> f) const
    {
        if (f.ready())
            return f.get();
        
        task_context* ctx = current_task_context::get();
        if (!ctx)
        {
            return f.get();
        }
        
        auto&& resume_task = [=]() {
            ctx->resume();
        };
        
        auto s = f.get_state(detail::use_private_interface);
        
        s->continue_with([&]() {
            s->await_queue->push_front([=]() {
                resume_task();
            });
        });
        
        ctx->yield();
        
        return f.get();
    }
    
    void apply(future<void> f) const
    {
        if (f.ready())
            return;
        
        task_context* ctx = current_task_context::get();
        if (!ctx)
        {
            f.wait();
            return;
        }
        
        auto&& resume_task = [=]() {
            ctx->resume();
        };
        
        auto s = f.get_state(detail::use_private_interface);
        
        s->continue_with([&]() {
            s->await_queue->push_front([=]() {
                resume_task();
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
