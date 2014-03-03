#ifndef LTLCONTEXT_HPP
#define LTLCONTEXT_HPP

#include <cstddef>
#include <stdint.h>

namespace ltl {
    
struct context;

typedef intptr_t context_data_t;
    
// creates a context on the stack
// sp stack pointer
// size size of the stack in bytes
// fn function to be called when the context is first jumped to
// vp data passed to fn
context* create_context(void* sp, std::size_t size, void (*fn)(context_data_t), context_data_t vp);

// switch contexts from ofc to nfc,
void jump(context* ofc, context* nfc);
    
// creates the main context for the current thread
context* create_main_context();
    
// destroys the main context for the current thread
void destroy_main_context(context*);
    
} // namespace ltl

#endif // LTLCONTEXT_HPP

