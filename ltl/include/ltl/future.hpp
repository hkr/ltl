#ifndef LTL_FUTURE_HPP
#define LTL_FUTURE_HPP

#include <type_traits>
#include <mutex>
#include <memory>

#include "ltl/detail/future_state.hpp"
#include "ltl/detail/result_of.hpp"

namespace ltl {
    
template <typename T>
class future
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
        return state_ != nullptr;
    }
    
    template <typename Function>
    future<typename result_of<Function, T>::type> then(Function&& func)
    {
        typedef future<typename result_of<Function, T>::type> result_future;
        return valid() ? state_->template then<result_future>(std::forward<Function>(func)) : result_future();
    }
    
    void swap(future& other)
    {
        state_swap(other.state_);
    }
    
    T* poll()
    {
        return state_ ? state_->poll() : nullptr;
    }
    
    void wait()
    {
        if (!valid())
            return;
        
        return state_->wait();
    }
    
    T get()
    {
        wait();
        return *poll();
    }
    
private:
    std::shared_ptr<state> state_;
    
public:
    // private interface
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
    
    
} // namespace ltl

#endif // LTL_FUTURE_HPP
