#ifndef LTLCONTEXT_HPP
#define LTLCONTEXT_HPP

#include <cstddef>
#include <stdint.h>

namespace ltl {

struct context;

typedef intptr_t context_data_t;
    
context_data_t jump(context* ofc, context const* nfc, void* vp);
context* create_context(void* sp, std::size_t size, void (*fn)(context_data_t));
    
} // namespace ltl

#endif // LTLCONTEXT_HPP

