#ifndef LTL_FUTURE_HPP
#define LTL_FUTURE_HPP

#include <type_traits>
#include <memory>

#include "ltl/detail/future_state.hpp"
#include "ltl/detail/private.hpp"
#include "ltl/traits.hpp"
#include "ltl/detail/task_queue_impl.hpp"
#include "ltl/detail/wrapped_function.hpp"
#include "ltl/detail/noexcept.hpp"

namespace ltl {
    
template <typename T>
class future
{
public:
    typedef detail::future_state<T> state;
    typedef T value_type;

    future() LTL_NOEXCEPT
    : state_()
    {
    }
   
    bool valid() const LTL_NOEXCEPT
    {
        return state_ != nullptr;
    }
    
    template <typename Function>
    future<typename std::result_of<Function(future<T>)>::type> then(Function&& func)
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        using result_future = future<typename std::result_of<Function(future<T>)>::type> ;
        return state_->template then<result_future>(std::bind(std::forward<Function>(func), future<T>(detail::use_private_interface, state_)));
    }
    
    void swap(future& other) LTL_NOEXCEPT
    {
        state_swap(other.state_);
    }
    
    bool ready() const
    {
        return state_ ? state_->ready() : false;
    }
    
    void wait() const
    {
        if (!valid())
            return;
        
        return state_->wait();
    }
    
    typename state::get_result_type get() const
    {
        wait();
        if (state_)
        {
            return state_->get();
        }
                
        throw std::future_error(std::future_errc::no_state);
    }
    
    typename std::conditional<is_future<T>::value, T, future<T>>::type unwrap();
    
private:
    std::shared_ptr<state> state_;
    
public:
    std::shared_ptr<state> const& get_state(detail::private_interface) const
    {
        return state_;
    }
    
    future(detail::private_interface, detail::promised)
    : state_(std::make_shared<state>())
    {
    }
    
    future(detail::private_interface, std::shared_ptr<detail::task_queue_impl> const& tq)
    : state_(std::make_shared<state>(tq))
    {
    }
    
    future(detail::private_interface, std::shared_ptr<state> const& s)
    : state_(s)
    {
    }
};
  
template <typename T>
void swap(future<T>& x, future<T>& y) LTL_NOEXCEPT
{
    x.swap(y);
}
 
template <typename T>
future<T> make_ready_future(T&& value)
{
    future<T> f(detail::use_private_interface, detail::promised());
    f.get_state(detail::use_private_interface)->set_value(std::forward<T>(value));
    return std::move(f);
}

inline future<void> make_ready_future()
{
    future<void> f(detail::use_private_interface, detail::promised());
    f.get_state(detail::use_private_interface)->set_value();
    return std::move(f);
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
            throw std::future_error(std::future_errc::no_state);
        
        auto other_state = other.get_state(detail::use_private_interface);
        future<T> f(detail::use_private_interface, other.get_state(detail::use_private_interface)->await_queue);
        auto s = f.get_state(detail::use_private_interface);
        
        other.then([=](future<future<T>> ff) mutable {
            auto wrapped_state = ff.get_state(detail::use_private_interface);
            
            try
            {
                ff.get(); // throws
                wrapped_state->continue_with([=]() mutable {
                    try
                    {
                        s->set_value(wrapped_state->get().get());
                    }
                    catch(...)
                    {
                        s->set_exception(std::current_exception());
                    }
                });
            }
            catch(...)
            {
                s->set_exception(std::current_exception());
            }
        });
        return std::move(f);
    }
    
    inline future<void> operator()(future<future<void>>&& other) const
    {
        if (!other.valid())
            throw std::future_error(std::future_errc::no_state);
        
        auto other_state = other.get_state(detail::use_private_interface);
        future<void> f(detail::use_private_interface, other_state->await_queue);
        auto s = f.get_state(detail::use_private_interface);
        
        other.then([=](future<future<void>> ff) {
            auto wrapped_state = ff.get_state(detail::use_private_interface);
            
            try
            {
                ff.get(); // throws
                wrapped_state->continue_with([=]() mutable {
                    try
                    {
                        wrapped_state->get().get();
                    }
                    catch(...)
                    {
                        s->set_exception(std::current_exception());
                    }
                });
            }
            catch(...)
            {
                s->set_exception(std::current_exception());
            }
        });
        return std::move(f);
    }
};
    
detail::task_queue_impl& get_async_task_queue();
    
} // namespace detail
    
template <typename T>
inline typename std::conditional<is_future<T>::value, T, future<T>>::type future<T>::unwrap()
{
    return detail::unwrap()(future<T>(detail::use_private_interface, state_));
}
    
template <typename Function>
future<typename std::result_of<Function()>::type> async(Function&& f)
{
    typedef typename std::result_of<Function()>::type result_type;
    detail::wrapped_function<result_type> wf(std::forward<Function>(f), 0);
    detail::get_async_task_queue().push_back_resumable(wf);
    return wf.promise_->get_future();
}
    
template <typename Function, typename... Args>
future<typename std::result_of<Function(Args...)>::type> async(Function&& f, Args &&... args)
{
    return async(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
}
    
} // namespace ltl

#endif // LTL_FUTURE_HPP
