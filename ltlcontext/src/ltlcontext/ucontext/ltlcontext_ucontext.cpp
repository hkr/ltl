#include "ltlcontext/ltlcontext.hpp"

#include <ucontext.h>
#include <assert.h>
#include <vector>

#include <new>

#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

extern "C" void ltlcontex_trampoline(void* fn, void* data)
{
    typedef void (*func_type)(ltl::context_data_t);
    func_type f = reinterpret_cast<func_type>(fn);
    f(reinterpret_cast<ltl::context_data_t>(data));
}

namespace ltl {
    
struct context
{
    explicit context(std::size_t stack_size = 0)
    : stack(stack_size)
    {
        getcontext(&value);
    }
    ucontext_t value;
    std::vector<char> stack;
};
    
static_assert(sizeof(context) >= sizeof(ucontext_t), "sizeof context to small");
    
void jump(context* ofc, context* nfc)
{
    swapcontext(&ofc->value, &nfc->value);
}

context* create_context(std::size_t stack_size, void (*fn)(context_data_t), context_data_t vp)
{
    context* ret = new context(stack_size);
    ucontext_t* ctx = &ret->value;
    ctx->uc_stack.ss_size = stack_size;
    ctx->uc_stack.ss_sp = ret->stack.data();
    ctx->uc_link = 0;
    makecontext(ctx, reinterpret_cast<void(*)()>(ltlcontex_trampoline), 2, reinterpret_cast<void*>(fn), reinterpret_cast<void*>(vp));
    
    return ret;
}
    
void destroy_context(context* ctx)
{
    delete ctx;
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

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif
