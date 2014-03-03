#ifndef LTL_TASK_QUEUE_HPP
#define LTL_TASK_QUEUE_HPP

#include <functional>
#include <memory>
#include <type_traits>

#include "ltl/future.hpp"
#include "ltl/promise.hpp"
#include "ltl/detail/task_queue_impl.hpp"
#include "ltl/detail/current_task_context.hpp"

namespace ltl {
    
class task_queue
{
public:
    explicit task_queue(char const* name = nullptr)
    : impl_(std::make_shared<detail::task_queue_impl>(name))
    {
    }
    
    task_queue(task_queue const&) =delete;
    task_queue& operator=(task_queue const&) =delete;
    
    void join()
    {
        impl_->join();
    }
    
    void enqueue(std::function<void()> task)
    {
        impl_->enqueue_resumable(std::move(task));
    }
    
    template <typename Function>
    future<typename std::result_of<Function()>::type> execute(Function&& task);
    
private:
    std::shared_ptr<detail::task_queue_impl> const impl_;
};
    
namespace detail {
    
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
    
template <typename Function>
inline future<typename std::result_of<Function()>::type> task_queue::execute(Function&& task)
{
    typedef typename std::result_of<Function()>::type result_type;
    detail::wrapped_function<result_type> wf(current_task_context::get()->get_task_queue()->shared_from_this(),
                                             std::forward<Function>(task));
    impl_->enqueue_resumable(wf);
    return wf.promise_->get_future();
}
    
    
template <typename Function, typename... Args>
future<typename std::result_of<Function(Args const&...)>::type> async(task_queue& tq, Function&& f, Args const&... args)
{
    return tq.execute([=](){
        return f(args...);
    });
}
    
template <typename Function>
future<typename std::result_of<Function()>::type> operator <<= (task_queue& tq, Function&& f)
{
    return async(tq, std::forward<Function>(f));
}
    
} // namespace ltl

#endif // LTL_TASK_QUEUE_HPP
