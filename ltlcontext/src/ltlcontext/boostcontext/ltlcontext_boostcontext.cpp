#include "ltlcontext/ltlcontext.hpp"

#include "boost/context/all.hpp"

#include <new>
#include <assert.h>
#include <vector>

namespace ltl {
    
static_assert(sizeof(void*) == sizeof(intptr_t),  "size mismatch");

struct context
{
    context()
    : ctx()
    , data()
    {
    }
    
    boost::context::fcontext_t* ctx;
    void* data;
    void (*fn)(void*);

    context(context const&) = delete;
    context& operator=(context const&) = delete;
};
    
namespace {
    
void trampoline(intptr_t x)
{
    auto ctx = reinterpret_cast<context*>(x);
    ctx->fn(ctx->data);
}
    
struct secondary_context : context
{
    explicit secondary_context(std::size_t stack_size)
    : context()
    , stack(stack_size)
    {
    }

    std::vector<char> stack;
    
    secondary_context(secondary_context const&) = delete;
    secondary_context& operator=(secondary_context const&) = delete;
};
} // namespace
    
void jump(context* ofc, context* nfc)
{
    boost::context::jump_fcontext(ofc->ctx, nfc->ctx, reinterpret_cast<intptr_t>(nfc));
}
    
context* create_context(std::size_t stack_size, void (*fn)(void*), void* vp)
{
    auto ctx = new secondary_context(stack_size);

    // make_fcontext wants the pointer to start or end of the stack
    // the direction in which the stack grows is platform dependent, but this should all current platforms
    ctx->ctx = boost::context::make_fcontext(ctx->stack.data() + stack_size, stack_size, trampoline);
    ctx->data = vp;
    ctx->fn = fn;
    return ctx;
}
    
void destroy_context(context* ctx)
{
    delete static_cast<secondary_context*>(ctx);
}
    
namespace {
struct main_context : context
{
    main_context() : context(), mainctx()
    {
        ctx = &mainctx;
    }
    
    boost::context::fcontext_t mainctx;
    
    main_context(main_context const&) = delete;
    main_context& operator=(main_context const&) = delete;
};
} // namespace
    
context* create_main_context()
{
    return new main_context();
}
    
void destroy_main_context(context* ctx)
{
    delete static_cast<main_context*>(ctx);
}
    
} // namespace ltl
