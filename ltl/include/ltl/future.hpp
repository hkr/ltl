#ifndef LTL_FUTURE_HPP
#define LTL_FUTURE_HPP

#include <type_traits>
#include <mutex>
#include <memory>

#include "ltl/detail/future_state.hpp"
#include "ltl/detail/result_of.hpp"
#include "ltl/detail/private.hpp"
#include "ltl/traits.hpp"

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
    
    bool ready()
    {
        return state_ ? state_->poll() != nullptr : false;
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
        return *state_->poll();
    }
    
    typename std::conditional<is_future<T>::value, T, future<T>>::type unwrap();
    
private:
    std::shared_ptr<state> state_;
    
public:
    std::shared_ptr<state> const& get_state(detail::private_interface) const
    {
        return state_;
    }
    
    explicit future(detail::private_interface, detail::promised)
    : state_(std::make_shared<state>())
    {
    }
    
    explicit future(detail::private_interface, std::shared_ptr<detail::task_queue_impl> const& tq)
    : state_(std::make_shared<state>(tq))
    {
    }
    
    explicit future(detail::private_interface, std::shared_ptr<state> const& s)
    : state_(s)
    {
    }
};
  
template <typename T>
void swap(future<T>& x, future<T>& y)
{
    x.swap(y);
}
 
    
    
namespace detail {
    
struct unwrap
{
    template <typename T>
    future<T> operator()(future<T>&& other) const
    {
        return std::move(other);
    }
    
    
    template <typename T>
    future<T> operator()(future<future<T>>&& other) const
    {
        if (!other.valid())
            return future<T>();
        
        future<T> f(detail::use_private_interface, other.get_state(detail::use_private_interface)->await_queue);
        auto s = f.get_state(detail::use_private_interface);
        
        other.get_state(detail::use_private_interface)->continue_with([=](future<T> const& x){
            x.get_state(detail::use_private_interface)->continue_with([=](T const& x) {
                s->set_value(x);
            });
        });
        return std::move(f);
    }
    
    inline future<void> operator()(future<future<void>>&& other) const
    {
        if (!other.valid())
            return future<void>();
        
        future<void> f(detail::use_private_interface, other.get_state(detail::use_private_interface)->await_queue);
        auto s = f.get_state(detail::use_private_interface);
        other.get_state(detail::use_private_interface)->continue_with([=](future<void> const& x){
            x.get_state(detail::use_private_interface)->continue_with([=]() {
                s->set_value();
            });
        });
        return std::move(f);
    }
};
    
} // namespace detail
    
template <typename T>
inline typename std::conditional<is_future<T>::value, T, future<T>>::type future<T>::unwrap()
{
    return detail::unwrap()(future<T>(detail::use_private_interface, state_));
}
    
} // namespace ltl

#endif // LTL_FUTURE_HPP
