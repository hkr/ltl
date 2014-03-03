#ifndef LTL_TASK_CONTEXT_HPP
#define LTL_TASK_CONTEXT_HPP

#include <memory>
#include <functional>
#include <vector>

#include "ltlcontext/ltlcontext.hpp"

namespace ltl {
    
namespace detail {
class task_queue_impl;
} // namespace detail
    
class task_context : public std::enable_shared_from_this<task_context>
{
public:
    static std::shared_ptr<task_context> create(context* main,
                                                std::function<void(std::shared_ptr<task_context> const&)> finished,
                                                detail::task_queue_impl* tq);
    
    task_context(context* main,
                 std::function<void(std::shared_ptr<task_context> const&)> finished,
                 detail::task_queue_impl* tq);
    
    task_context(task_context const&) =delete;
    task_context& operator=(task_context const&) =delete;
    
    void activate(std::function<void()>&& f);
    
    void yield();
    void resume();
    
    detail::task_queue_impl* get_task_queue()
    {
        return task_queue_;
    }
    
private:
    static void trampoline(context_data_t instance);

    
private:
    std::vector<void*> stack_;
    std::function<void()> func_;
    std::shared_ptr<task_context> keep_alive_;
    context* const main_;
    context* own_;
    std::function<void(std::shared_ptr<task_context> const&)> finished_;
    detail::task_queue_impl* const task_queue_;
};
    
} // namespace ltl

#endif // LTL_TASK_CONTEXT_HPP
