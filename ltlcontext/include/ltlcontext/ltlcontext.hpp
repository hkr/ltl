#ifndef LTLCONTEXT_HPP
#define LTLCONTEXT_HPP

#include <cstddef>
#include <stdint.h>

namespace ltl {
    
struct context;

typedef intptr_t context_data_t;
    
// creates a context on the stack
// stack_size default size of the stack in bytes (may be ignored by the implementation)
// fn function to be called when the context is first jumped to
// vp data passed to fn
context* create_context(std::size_t stack_size, void (*fn)(context_data_t), context_data_t vp);

// destroy the context
void destroy_context(context* ctx);
    
// switch contexts from ofc to nfc,
void jump(context* ofc, context* nfc);
    
// creates the main context for the current thread
context* create_main_context();
    
// destroys the main context for the current thread
void destroy_main_context(context*);
    
} // namespace ltl

#endif // LTLCONTEXT_HPP

