#ifndef LTL_TASK_QUEUE_IMPL_HPP
#define LTL_TASK_QUEUE_IMPL_HPP

#include <functional>
#include <memory>

namespace ltl {
namespace detail {

class task_queue_impl : public std::enable_shared_from_this<detail::task_queue_impl>
{
public:
    explicit task_queue_impl(char const* name = nullptr);
    ~task_queue_impl();
    task_queue_impl(task_queue_impl const&) =delete;
    task_queue_impl& operator=(task_queue_impl const&) =delete;
    
    void push_back_resumable(std::function<void()> task);
    void push_back(std::function<void()> task);
    void push_front(std::function<void()> task);
    
    void join();
    
private:
    struct impl;
    std::unique_ptr<impl> impl_;
};
    
} // namespace detail
} // namespace ltl

#endif // LTL_TASK_QUEUE_HPP
