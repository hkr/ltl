#include "ltlcontext/ltlcontext.hpp"

#include <windows.h>
#include <assert.h>
#include <crtdbg.h>

#include <new>

namespace ltl {
    
struct context
{
	void(*fn)(void*);
	void* fiber;
	void* data;
};

void jump(context*, context* nfc)
{
	assert(nfc);
	SwitchToFiber(nfc->fiber);
}

namespace {
void WINAPI fiberproc(void* x)
{
	auto ctx = static_cast<context*>(x);
	ctx->fn(ctx->data);
}
} // namespace

context* create_context(std::size_t stack_size, void (*fn)(void*), void* vp)
{
	auto ctx = new context();
	ctx->fn = fn;
	ctx->data = vp;
	ctx->fiber = CreateFiber(stack_size, fiberproc, ctx);
	return ctx;
}

void destroy_context(context* ctx)
{
	if (!ctx)
		return;

	DeleteFiber(ctx->fiber);
	delete ctx;
}

context* create_main_context()
{
	auto ctx = new context();
	ctx->fiber = ConvertThreadToFiber(nullptr);
	return ctx;
}

void destroy_main_context(context* x)
{
	delete x;
	ConvertFiberToThread();
}
    
} // namespace ltl

