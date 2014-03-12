#ifndef LTL_PROMISE_HPP
#define LTL_PROMISE_HPP

#include <memory>

#include "ltl/future.hpp"
#include "ltl/detail/task_context.hpp"
#include "ltl/detail/current_task_context.hpp"
#include "ltl/detail/noexcept.hpp"

namespace ltl {
    
template <typename T>
class promise
{
public:
    explicit promise()
    : state_()
    , retrieved_(false)
    {
        auto get_await_queue = []()
        {
            auto ctx = current_task_context::get();
            auto tq = ctx ? ctx->get_task_queue() : nullptr;
            return tq ? tq->shared_from_this() : nullptr;
        };
   
        future<T> f {detail::use_private_interface, get_await_queue()};
        state_ = f.get_state(detail::use_private_interface);
    }
    
    promise(promise&& other) LTL_NOEXCEPT
    : state_(std::move(other.state_))
    , retrieved_(std::move(other.retrieved_))
    {
    }
    
    promise(promise const& other) = delete;
    
    promise& operator=(promise&& other)
    {
        promise p(other);
        state_.swap(p.state_);
        retrieved_ = p.retrieved_;
        return *this;
    }
    
    promise& operator=(promise const& other) = delete;
    
    template <typename U>
    void set_value(U const& value)
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        state_->set_value(value);
    }
    
    template <typename U>
    void set_value(U&& value)
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        state_->set_value(std::forward<U>(value));
    }
    
    void set_value()
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        state_->set_value();
    }
    
    void set_exception(std::exception_ptr p)
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        state_->set_exception(std::move(p));
    }
    
    future<T> get_future()
    {
        if (!state_) throw std::future_error(std::future_errc::no_state);
        if (retrieved_) throw std::future_error(std::future_errc::future_already_retrieved);
        retrieved_ = true;
        return future<T>(detail::use_private_interface, state_);
    }
    
    void swap(promise& other) LTL_NOEXCEPT
    {
        state_.swap(other.state_);
        std::swap(retrieved_, other.retrieved_);
    }
    
private:
    typedef typename future<T>::state state;
    std::shared_ptr<state> state_;
    bool retrieved_;
    
public:
    std::shared_ptr<state> const& get_state(detail::private_interface) const
    {
        return state_;
    }
};
    
template <typename T>
void swap(promise<T>& x, promise<T>& y) LTL_NOEXCEPT
{
    x.swap(y);
}
    
} // namespace ltl

#endif // LTL_PROMISE_HPP
