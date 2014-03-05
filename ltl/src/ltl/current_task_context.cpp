#include "ltl/detail/current_task_context.hpp"

#ifdef __APPLE__

#include <pthread.h>
#include <thread>
#include <unordered_map>

#if 1

struct tlskey
{
    tlskey()
    {
        pthread_key_create(&value, 0);
    }
    
    ~tlskey()
    {
        pthread_key_delete(value);
    }
    
    pthread_key_t value;
};

static tlskey key;

ltl::task_context* ltl::current_task_context::get()
{
    ltl::task_context* ctx = static_cast<ltl::task_context*>(pthread_getspecific(key.value));
    return ctx;
}

void ltl::current_task_context::set(ltl::task_context* ctx)
{
    pthread_setspecific(key.value, ctx);
}

#else

static std::unordered_map<pthread_t, ltl::task_context*> map;
static std::mutex mutex;

ltl::task_context* ltl::current_task_context::get()
{
    std::lock_guard<std::mutex> lock(mutex);
    return map[pthread_self()];
}

void ltl::current_task_context::set(ltl::task_context* ctx)
{
    std::lock_guard<std::mutex> lock(mutex);
    map[pthread_self()] = ctx;;
}

#endif 

#else

#ifdef _MSC_VER
#define LTL_THREAD_LOCAL __declspec(thread)
#else
#define LTL_THREAD_LOCAL __thread
#endif

static LTL_THREAD_LOCAL ltl::task_context* value = nullptr;

ltl::task_context* ltl::current_task_context::get()
{
    return value;
}

void ltl::current_task_context::set(ltl::task_context* ctx)
{
    value = ctx;
}

#endif


