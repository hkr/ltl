#ifndef LTL_TASK_QUEUE_HPP
#define LTL_TASK_QUEUE_HPP

#include <functional>
#include <memory>
#include <type_traits>

#include "ltl/future.hpp"
#include "ltl/promise.hpp"

namespace ltl {
    
namespace detail {
    
template <typename R>
struct wrapped_function
{
    template <typename Function>
    explicit wrapped_function(Function&& task)
    : task_(std::move(task))
    , promise_(std::make_shared<promise<R>>())
    {
        
    }
    
    void operator()() const
    {
        promise_->set_value(task_());
    }
    
    std::function<R()> task_;
    std::shared_ptr<promise<R>> promise_;
};
    
} // namespace detail
    
class task_queue
{
public:
    task_queue();
    ~task_queue();
    
    task_queue(task_queue const&) =delete;
    task_queue& operator=(task_queue const&) =delete;
    
    void enqueue(std::function<void()> task);
    
    template <typename Function>
    future<typename std::result_of<Function()>::type> execute(Function&& task)
    {
        typedef typename std::result_of<Function()>::type result_type;
        detail::wrapped_function<result_type> wf(std::forward<Function>(task));
        enqueue(wf);
        return wf.promise_->get_future();
    }
    
private:
    struct impl;
    std::unique_ptr<impl> const impl_;
};


    
} // namespace ltl

#endif // LTL_TASK_QUEUE_HPP
