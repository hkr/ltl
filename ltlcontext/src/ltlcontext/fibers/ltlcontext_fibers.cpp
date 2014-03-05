#include "ltlcontext/ltlcontext.hpp"

#include <windows.h>
#include <assert.h>
#include <crtdbg.h>

#include <new>

#define LTL_ASSERT(cond) do { if (!cond) DebugBreak(); } while (false)

namespace ltl {
    


struct context
{
	explicit context(void* fiber) : value(fiber)
    {
    }
    void* value;
};
    
void jump(context*, context* nfc)
{
	LTL_ASSERT(nfc);
	LTL_ASSERT(nfc->value);
	SwitchToFiber(nfc->value);
}

context* create_context(void* stack, std::size_t size, void (*fn)(context_data_t), context_data_t vp)
{
    if (size < sizeof(context))
        return nullptr;

	// TODO: rework how to allocate stack, create_context interface does not work well with Fibers
	// TODO: pass allocator function instead
    
	// casting function pointer types is ugly, but undefined bahavior, but this is platform-dependent anyway
	auto fiber = CreateFiber(0, reinterpret_cast<LPFIBER_START_ROUTINE>(fn), reinterpret_cast<LPVOID>(vp));
	if (!fiber)
	{
		DWORD err = GetLastError();
		int i = 0;
		++i;
	}
	LTL_ASSERT(fiber);
	return new (stack)context(fiber);
}

context* create_main_context()
{
	auto fiber = ConvertThreadToFiber(nullptr);
	LTL_ASSERT(fiber);
	return new context(fiber);
}

void destroy_main_context(context* ctx)
{
    delete ctx;
}
    
} // namespace ltl

