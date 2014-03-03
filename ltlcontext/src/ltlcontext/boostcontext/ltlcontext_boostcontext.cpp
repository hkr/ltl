#include "ltlcontext/ltlcontext.hpp"

#include "boost/context/all.hpp"

#include <new>
#include <assert.h>

namespace ltl {
    
static_assert(sizeof(void*) >= sizeof(context_data_t),  "size mismatch");

struct context
{
    context() : ctx(), data() {}
    boost::context::fcontext_t* ctx;
    context_data_t data;
    
    context(context const&) = delete;
    context& operator=(context const&) = delete;
};
    
void jump(context* ofc, context* nfc)
{
    boost::context::jump_fcontext(ofc->ctx, nfc->ctx, nfc->data);
}
    
context* create_context(void* stack, std::size_t size, void (*fn)(context_data_t), context_data_t vp)
{
    context* ctx = new context();
    stack = static_cast<char*>(stack) + sizeof(context);
    size -= 2 * sizeof(context); // waste some space, but allow make_fcontext to place fcontext_t at either end of the stack
    ctx->ctx = boost::context::make_fcontext(stack, size, fn);
    ctx->data = vp;
    return ctx;
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
