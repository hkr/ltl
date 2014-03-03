#ifndef LTL_CURRENT_TASK_CONTEXT_HPP
#define LTL_CURRENT_TASK_CONTEXT_HPP

#include "ltl/detail/task_context.hpp"

namespace ltl {
namespace current_task_context {

task_context* get();
    
void set(task_context*);
    
} // namespace current_task_context
} // namespace ltl

#endif // LTL_CURRENT_TASK_CONTEXT_HPP
