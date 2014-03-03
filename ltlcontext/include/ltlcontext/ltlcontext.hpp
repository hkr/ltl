#ifndef LTLCONTEXT_HPP
#define LTLCONTEXT_HPP

#include <cstddef>
#include <stdint.h>

namespace ltl {

struct context;

typedef intptr_t context_data_t;
    
context_data_t jump(context* ofc, context const* nfc, context_data_t vp);

context* create_context(void* sp, std::size_t size, void (*fn)(context_data_t));

context* create_main_context();
void destroy_main_context(context*);
    
} // namespace ltl

#endif // LTLCONTEXT_HPP

