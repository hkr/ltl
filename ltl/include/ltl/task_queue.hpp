#ifndef LTL_TASK_QUEUE_HPP
#define LTL_TASK_QUEUE_HPP

#include <functional>
#include <memory>
#include <type_traits>

#include "ltl/future.hpp"
#include "ltl/promise.hpp"

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
    
    template <typename Function>
    future<typename std::result_of<Function()>::type> push_back_resumable(Function&& task)
    {
        return push_pack_impl(std::forward<Function>(task), &detail::task_queue_impl::push_back_resumable);
    }
    
    template <typename Function>
    future<typename std::result_of<Function()>::type> push_back(Function&& task)
    {
        return push_pack_impl(std::forward<Function>(task), &detail::task_queue_impl::push_back);
    }
    
    template <typename Function>
    void push_back_and_forget(Function&& task)
    {
        impl_->push_back(std::forward<Function>(task));
    }
    
private:
    template <typename Function, typename PushMemFn>
    future<typename std::result_of<Function()>::type> push_pack_impl(Function&& task, PushMemFn push)
    {
        typedef typename std::result_of<Function()>::type result_type;
        detail::wrapped_function<result_type> wf(std::forward<Function>(task), 0);
        ((*impl_).*push)(wf);
        return wf.promise_->get_future();
    }
    
private:
    std::shared_ptr<detail::task_queue_impl> impl_;
};

template <typename Function>
future<typename std::result_of<Function()>::type> operator <<= (task_queue& tq, Function&& f)
{
    return tq.push_back_resumable(std::forward<Function>(f));
}
    
} // namespace ltl

#endif // LTL_TASK_QUEUE_HPP
