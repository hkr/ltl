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
#include "ltl/detail/result_of.hpp"
#include "ltl/detail/get_and_compose.hpp"

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
    
    // see N3721
    template <typename Function>
    future<typename std::result_of<Function(future<T>)>::type> then(Function&& func)
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        using result_future = future<typename std::result_of<Function(future<T>)>::type>;
        return state_->template then<result_future>(std::bind(std::forward<Function>(func), future<T>(detail::use_private_interface, state_)));
    }
    
    // see N3865
    template <typename Function>
    future<typename result_of<Function, T>::type> next(Function&& func)
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        using result_type = typename result_of<Function, T>::type;
        using result_future = future<result_type>;
        using continuation = detail::get_and_compose<result_type, T, Function, std::shared_ptr<state>>;
        return state_->template then<result_future>(continuation(std::forward<Function>(func), state_));
    }
    
    // see N3865
    template <typename Function>
    future recover(Function&& func)
    {
        return then([=](future f){
            if (f.has_value())
                return f.get();
            else
                return func(f.get());
        });
    }
    
    void swap(future& other) LTL_NOEXCEPT
    {
        state_swap(other.state_);
    }
    
    // see N3721
    bool ready() const
    {
        return state_ ? state_->ready() : false;
    }
    
    // see N3865
    bool has_value() const
    {
        return state_ ? state_->has_value() : false;
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
    
    // see N3865
    template <typename U>
    U value_or(U&& x)
    {
        return has_value() ? get() : x;
    }
    
    // see N3721
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
 
// see N3721
template <typename T>
future<T> make_ready_future(T&& value)
{
    future<T> f(detail::use_private_interface, detail::promised());
    f.get_state(detail::use_private_interface)->set_value(std::forward<T>(value));
    return std::move(f);
}

// see N3721
inline future<void> make_ready_future()
{
    future<void> f(detail::use_private_interface, detail::promised());
    f.get_state(detail::use_private_interface)->set_value();
    return std::move(f);
}

// see N3865
template <typename T, typename E>
inline future<T> make_exceptional_future(E e)
{
    future<T> f(detail::use_private_interface, detail::promised());
    f.get_state(detail::use_private_interface)->set_exception(std::make_exception_ptr(e));
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
    
    template <typename State, typename T>
    void static get_and_set_value(State& s, future<T> const& f)
    {
        s.set_value(f.get());
    }
    
    template <typename State>
    void static get_and_set_value(State& s, future<void> const& f)
    {
        f.get(); // get exception
        s.set_value();
    }
    
    template <typename T>
    future<T> operator()(future<future<T>>&& other) const
    {
        if (!other.valid())
            throw std::future_error(std::future_errc::no_state);
        
        auto other_state = other.get_state(detail::use_private_interface);
        future<T> f(detail::use_private_interface, other.get_state(detail::use_private_interface)->await_queue());
        auto s = f.get_state(detail::use_private_interface);
        
        other.then([=](future<future<T>> ff) mutable {
            auto f = ff.get_state(detail::use_private_interface)->get();
            auto wrapped_state = f.get_state(detail::use_private_interface);
            
            try
            {
                ff.get(); // throws
                wrapped_state->continue_with([=]() mutable {
                    try
                    {
                        get_and_set_value(*s, f);
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
