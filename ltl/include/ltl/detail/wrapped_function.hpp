#ifndef LTL_WRAPPED_FUNCTION_HPP
#define LTL_WRAPPED_FUNCTION_HPP

#include <memory>
#include <functional>

#include "ltl/promise.hpp"

namespace ltl {
namespace detail {

class task_queue_impl;
 
typedef int not_a_copy_ctor;

template <typename R>
struct wrapped_function
{
	template <typename Function>
    wrapped_function(Function&& task, not_a_copy_ctor)
    : task_(std::forward<Function>(task))
    , promise_(std::make_shared<promise<R>>())
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