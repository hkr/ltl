#ifndef LTL_TASK_QUEUE_IMPL_HPP
#define LTL_TASK_QUEUE_IMPL_HPP

#include <functional>
#include <memory>
#include <type_traits>

namespace ltl {
namespace detail {

class task_queue_impl : public std::enable_shared_from_this<detail::task_queue_impl>
{
public:
    explicit task_queue_impl(char const* name = nullptr);
    ~task_queue_impl();
    task_queue_impl(task_queue_impl const&) =delete;
    task_queue_impl& operator=(task_queue_impl const&) =delete;
    
    void enqueue_resumable(std::function<void()> task);
    void execute_next(std::function<void()> task);
    
    void join();
    
private:
    struct impl;
    std::unique_ptr<impl> impl_;
};
    
} // namespace detail
} // namespace ltl

#endif // LTL_TASK_QUEUE_HPP
