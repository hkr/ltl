#include "ltlcontext/ltlcontext.hpp"

#include "boost/context/all.hpp"

namespace ltl {
    
context_data_t jump(context* ofc, context const* nfc, context_data_t vp)
{
    return boost::context::jump_fcontext(reinterpret_cast<boost::context::fcontext_t*>(ofc),
                                         reinterpret_cast<boost::context::fcontext_t const*>(nfc),
                                         vp);
}
    
context* create_context(void* stack, std::size_t size, void (*fn)(context_data_t))
{
    return reinterpret_cast<context*>(boost::context::make_fcontext(stack, size, fn));
}
    

context* create_main_context()
{
    return reinterpret_cast<context*>(new boost::context::fcontext_t());
}
    
void destroy_main_context(context* ctx)
{
    if (!ctx)
        return;
    
    delete reinterpret_cast<boost::context::fcontext_t*>(ctx);
}

} // namespace ltl
