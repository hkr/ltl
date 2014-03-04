#ifndef LTL_TASK_QUEUE_HPP
#define LTL_TASK_QUEUE_HPP

#include <functional>
#include <memory>
#include <type_traits>

#include "ltl/future.hpp"
#include "ltl/promise.hpp"
#include "ltl/detail/task_queue_impl.hpp"
#include "ltl/detail/current_task_context.hpp"
#include "ltl/detail/wrapped_function.hpp"

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
    
    task_queue(task_queue&& other)
    : impl_(std::move(other.impl_))
    {
    }
    
    task_queue& operator=(task_queue&& other)
    {
        impl_ = std::move(other.impl_);
        return *this;
    }
    
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
    std::shared_ptr<detail::task_queue_impl> impl_;
};
    
template <typename Function>
inline future<typename std::result_of<Function()>::type> task_queue::execute(Function&& task)
{
    typedef typename std::result_of<Function()>::type result_type;
    detail::wrapped_function<result_type> wf(current_task_context::get(),
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
