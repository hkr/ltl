#include "ltlcontext/ltlcontext.hpp"

#include <windows.h>
#include <assert.h>
#include <crtdbg.h>

#include <new>

namespace ltl {
        
void jump(context*, context* nfc)
{
	assert(nfc);
	SwitchToFiber(nfc);
}

context* create_context(std::size_t stack_size, void (*fn)(context_data_t), context_data_t vp)
{
	return static_cast<context*>(CreateFiber(stack_size, reinterpret_cast<LPFIBER_START_ROUTINE>(fn), reinterpret_cast<LPVOID>(vp)));
}

void destroy_context(context* ctx)
{
	DeleteFiber(ctx);
}

context* create_main_context()
{
	return static_cast<context*>(ConvertThreadToFiber(nullptr));
}

void destroy_main_context(context*)
{
	ConvertFiberToThread();
}
    
} // namespace ltl

