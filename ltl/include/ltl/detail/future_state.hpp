#ifndef LTL_FUTURE_STATE_HPP
#define LTL_FUTURE_STATE_HPP

#include <type_traits>
#include <mutex>
#include <memory>
#include <deque>

#include "ltl/detail/task_queue_impl.hpp"

namespace ltl {
    
namespace detail {

struct promised {};

template <typename Function, typename PtrToArg>
void invoke(Function const& func, PtrToArg* ptr)
{
    func(*ptr);
}

template <typename Function>
void invoke(Function const& func, void* ptr)
{
    func();
}

template <typename ValueHolder, typename ValueType>
void assign_value(ValueHolder& holder, ValueType* ptr)
{
    holder.reset(new ValueType(*ptr));
}

inline void assign_value(bool& holder, void*)
{
    holder = true;
}
    
template <typename ValueHolder>
typename ValueHolder::pointer get_value(ValueHolder const& holder)
{
    return holder.get();
}

inline void* get_value(bool const& holder)
{
    return holder ? reinterpret_cast<void*>(1) : nullptr;
}

template <typename T>
struct select_continuation
{
    typedef std::function<void(T const&)> type;
};
    
template <>
struct select_continuation<void>
{
    typedef std::function<void()> type;
};

    
template <typename T>
struct future_state
{
    typedef typename std::conditional<std::is_void<T>::value, bool, std::unique_ptr<T>>::type ValueHolder;
    typedef std::deque<typename select_continuation<T>::type> continuations_container;
    typedef std::shared_ptr<detail::task_queue_impl> await_queue_type;
    
    mutable std::mutex mutex;
    ValueHolder value;
    continuations_container continuations;
    await_queue_type await_queue;
    
    explicit future_state(await_queue_type const& tq = await_queue_type())
    : value()
    , await_queue(tq)
    {
    }
    
    template <class U>
    void set_value(U&& v)
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (value)
                return;
            
            assign_value(value, &v);
            cs.swap(continuations);
        }
        
        for(auto&& f : cs)
            invoke(f, &v);
    }
    
    void set_value()
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (value)
                return;
            
            assign_value(value, reinterpret_cast<void*>(1));
            cs.swap(continuations);
        }
        
        for(auto&& f : cs)
            invoke(f, 0);
    }
    
    T* poll() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return get_value(value);
    }
};

} // namespace detail

} // namespace ltl

#endif // LTL_FUTURE_HPP
