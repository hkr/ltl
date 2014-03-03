#include "ltlcontext/ltlcontext.hpp"

#include "ucontext.h"
#include <assert.h>

#include <new>

extern "C" void ltlcontex_trampoline(void* fn, void* data)
{
    typedef void (*func_type)(ltl::context_data_t);
    func_type f = reinterpret_cast<func_type>(fn);
    f(reinterpret_cast<ltl::context_data_t>(data));
}

namespace ltl {
    
struct context
{
    context()
    {
        getcontext(&value);
    }
    ucontext_t value;
};
    
static_assert(sizeof(context) >= sizeof(ucontext_t), "sizeof context to small");
    
void jump(context* ofc, context* nfc)
{
    swapcontext(&ofc->value, &nfc->value);
}

context* create_context(void* stack, std::size_t size, void (*fn)(context_data_t), context_data_t vp)
{
    if (size < sizeof(context))
        return nullptr;
    
    context* ret = new (stack) context();
    stack = static_cast<uint8_t*>(stack) + sizeof(context);
    size -= sizeof(context);
    
    ucontext_t* ctx = &ret->value;
    ctx->uc_stack.ss_size = size;
    ctx->uc_stack.ss_sp = stack;
    ctx->uc_link = 0;
    makecontext(ctx, reinterpret_cast<void(*)()>(ltlcontex_trampoline), 2, reinterpret_cast<void*>(fn), reinterpret_cast<void*>(vp));
    
    return ret;
}

context* create_main_context()
{
    return new context();
}

void destroy_main_context(context* ctx)
{
    delete ctx;
}
    
} // namespace ltl

