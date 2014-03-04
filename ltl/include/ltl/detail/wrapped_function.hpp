#ifndef LTL_WRAPPED_FUNCTION_HPP
#define LTL_WRAPPED_FUNCTION_HPP

#include <memory>
#include <functional>

#include "ltl/promise.hpp"
#include "ltl/detail/task_context.hpp"

namespace ltl {
namespace detail {

class task_queue_impl;
    
template <typename R>
struct wrapped_function
{
    template <typename Function>
    explicit wrapped_function(task_context* ctx, Function&& task)
    : task_(std::move(task))
    , promise_(ctx
               ? std::make_shared<promise<R>>(ctx->get_task_queue()->shared_from_this())
               : std::make_shared<promise<R>>())
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