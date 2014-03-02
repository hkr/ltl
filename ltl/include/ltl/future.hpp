#ifndef LTL_FUTURE_HPP
#define LTL_FUTURE_HPP

#include <type_traits>
#include <mutex>
#include <memory>

#include "ltl/detail/future_state.hpp"
#include "ltl/detail/future_then_base.hpp"

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
    
} // namespace ltl

#endif // LTL_FUTURE_HPP
