#ifndef LTL_WRAPPED_FUNCTION_HPP
#define LTL_WRAPPED_FUNCTION_HPP

#include <memory>
#include <functional>

#include "ltl/promise.hpp"

namespace ltl {
namespace detail {

class task_queue_impl;
    
template <typename R>
struct wrapped_function
{
    template <typename Function>
    explicit wrapped_function(std::shared_ptr<detail::task_queue_impl> const& await_queue, Function&& task)
    : task_(std::move(task))
    , promise_(std::make_shared<promise<R>>(await_queue))
    {
    }
    
    void operator()() const
    {
        apply(static_cast<R*>(nullptr));
    }
    
    template <typename U>
    void apply(U*) const
    {
        promise_->set_value(task_());
    }
    
    void apply(void*) const
    {
        task_();
        promise_->set_value();
    }
    
    std::function<R()> task_;
    std::shared_ptr<promise<R>> promise_;
};

} // namespace detail
} // namespace ltl

#endif // LTL_WRAPPED_FUNCTION_HPP