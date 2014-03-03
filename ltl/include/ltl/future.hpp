#ifndef LTL_FUTURE_HPP
#define LTL_FUTURE_HPP

#include <type_traits>
#include <mutex>
#include <memory>

#include "ltl/detail/future_state.hpp"
#include "ltl/detail/future_then_base.hpp"
#include "ltl/detail/current_task_context.hpp"

namespace ltl {
    
template <typename T>
class future : public detail::future_then_base<future<T>, T>
{
public:
    typedef detail::future_state<T> state;
    typedef T value_type;

    future()
    : state_()
    {
    }
    
    explicit future(detail::promised)
    : state_(std::make_shared<state>())
    {
    }
    
    explicit future(std::shared_ptr<detail::task_queue_impl> const& tq)
    : state_(std::make_shared<state>(tq))
    {
    }
    
    explicit future(std::shared_ptr<state> const& s)
    : state_(s)
    {
    }
    
    future(future&& other)
    : state_(std::move(other.state_))
    {
    }
    
    future(future const& other) = delete;
    
    future& operator=(future&& other)
    {
        future f(other);
        state_.swap(f.state_);
        return *this;
    }
    
    future& operator=(future const& other) = delete;
    
    bool valid() const
    {
        return state_;
    }
    
    using detail::future_then_base<future<T>, T>::then;
    
    void swap(future& other)
    {
        state_swap(other.state_);
    }
    
    T* poll()
    {
        return state_ ? state_->poll() : nullptr;
    }
    
private:
    std::shared_ptr<state> state_;
    
public:
    std::shared_ptr<state> const& get_state() const
    {
        return state_;
    }
};
    
template <typename T>
void swap(future<T>& x, future<T>& y)
{
    x.swap(y);
}
    
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

#endif // LTL_FUTURE_HPP
