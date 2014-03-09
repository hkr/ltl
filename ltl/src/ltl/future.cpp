#include "ltl/future.hpp"

#include "ltl/detail/task_queue_impl.hpp"

#include <mutex>

namespace ltl {
namespace detail {

namespace {

std::once_flag flag;
std::unique_ptr<task_queue_impl> instance;
    
void init()
{
    instance.reset(new detail::task_queue_impl());
}
        
} // namespace
    
detail::task_queue_impl& get_async_task_queue()
{
    std::call_once(flag, init);
    return *instance;
}
    
} // namespace detail
} // nameaspace ltl
