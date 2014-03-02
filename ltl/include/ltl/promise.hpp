#ifndef LTL_PROMISE_HPP
#define LTL_PROMISE_HPP

#include "ltl/future.hpp"

namespace ltl {
    
template <typename T>
class promise
{
public:
    promise()
    : state_()
    {
        future<T> f {detail::promised()};
        state_ = f.get_state();
    }
    
    promise(promise&& other)
    : state_(std::move(other.state_))
    {
    }
    
    promise(promise const& other) = delete;
    
    promise& operator=(promise&& other)
    {
        promise p(other);
        state_.swap(p.state_);
        return *this;
    }
    
    promise& operator=(promise const& other) = delete;
    
    void set_value(T const& value)
    {
        state_->set_value(value);
    }
    
    void set_value(T&& value)
    {
        state_->set_value(std::forward<T>(value));
    }
    
    future<T> get_future()
    {
        return future<T>(state_);
    }
    
    void swap(promise& other)
    {
        state_swap(other.state_);
    }
    
private:
    typedef typename future<T>::state state;
    std::shared_ptr<state> state_;
};
    
template <typename T>
void swap(promise<T>& x, promise<T>& y)
{
    x.swap(y);
}
    
} // namespace ltl

#endif // LTL_PROMISE_HPP
