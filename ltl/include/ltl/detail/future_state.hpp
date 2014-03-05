#ifndef LTL_FUTURE_STATE_HPP
#define LTL_FUTURE_STATE_HPP

#include <type_traits>
#include <mutex>
#include <memory>
#include <deque>

#include "ltl/detail/task_queue_impl.hpp"
#include "ltl/detail/block.hpp"
#include "ltl/detail/private.hpp"

namespace ltl {
    
namespace detail {

struct promised {};

template <typename Function, typename ValueHolder>
void invoke(Function const& func, ValueHolder const& holder)
{
    func(*holder);
}

template <typename Function>
void invoke(Function const& func, bool const&)
{
    func();
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

template <typename T, typename R>
struct invoke_and_set_value
{
    template <typename Function, typename State, class ValueHolder>
    void operator()(State& rs, Function&& func, ValueHolder const& holder)
    {
        rs.set_value(func(*holder));
    }
};

template <typename T>
struct invoke_and_set_value<T, void>
{
    template <typename Function, typename State, class ValueHolder>
    void operator()(State& rs, Function&& func, ValueHolder const& holder)
    {
        func(*holder);
        rs.set_value();
    }
};

template <typename R>
struct invoke_and_set_value<void, R>
{
    template <typename Function, typename State, class ValueHolder>
    void operator()(State& rs, Function&& func, ValueHolder const& holder)
    {
        rs.set_value(func());
    }
};

template <>
struct invoke_and_set_value<void, void>
{
    template <typename Function, typename State, class ValueHolder>
    void operator()(State& rs, Function&& func, ValueHolder const&)
    {
        func();
        rs.set_value();
    }
};

template <typename T, typename R>
struct add_continuation
{
    template <typename Function, typename Continuations, typename State>
    void operator()(Continuations& continuations, std::shared_ptr<State> const& rs, Function&& func) const
    {
        continuations.emplace_back([=](T const& x)
        {
           rs->set_value(func(x));
        });
    }
};

template <typename R>
struct add_continuation<void, R>
{
    template <typename Function, typename Continuations, typename State>
    void operator()(Continuations& continuations, std::shared_ptr<State> const& rs, Function&& func) const
    {
        continuations.emplace_back([=]()
        {
           rs->set_value(func());
        });
    }
};

template <typename T>
struct add_continuation<T, void>
{
    template <typename Function, typename Continuations, typename State>
    void operator()(Continuations& continuations, std::shared_ptr<State> const& rs, Function&& func) const
    {
        continuations.emplace_back([=](T const& x)
        {
           func(x);
           rs->set_value();
        });
    }
    
    template <typename Function, typename Continuations>
    void operator()(Continuations& continuations, Function&& func) const
    {
        continuations.emplace_back([=](T const& x){
            func(x);
        });
    }
};

template <>
struct add_continuation<void, void>
{
    template <typename Function, typename Continuations, typename State>
    void operator()(Continuations& continuations, std::shared_ptr<State> const& rs, Function&& func) const
    {
        continuations.emplace_back([=](){
            func();
            rs->set_value();
        });
    }
    
    template <typename Function, typename Continuations>
    void operator()(Continuations& continuations, Function&& func) const
    {
        continuations.emplace_back([=](){
            func();
        });
    }
};

template <typename T>
struct future_state_base
{
    typedef typename std::conditional<std::is_void<T>::value, bool, std::unique_ptr<T>>::type ValueHolder;
    typedef std::deque<typename select_continuation<T>::type> continuations_container;
    typedef std::shared_ptr<detail::task_queue_impl> await_queue_type;
    
    mutable std::mutex mutex;
    ValueHolder value;
    continuations_container continuations;
    await_queue_type await_queue;
    std::unique_ptr<detail::block> block;
    
    explicit future_state_base(await_queue_type const& tq)
    : value()
    , await_queue(tq)
    {
    }
    
    future_state_base(future_state_base const&) =delete;
    future_state_base& operator=(future_state_base const&) =delete;

    void wait()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (auto x = get_value(value))
                return;
            
            if (!block)
            {
                block.reset(new detail::block());
                continuations.push_back(std::bind(&detail::block::signal, block.get()));
            }
        }
        
        block->wait();
    }
    
    T* poll() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return get_value(value);
    }
    
    template <typename Future, typename Function>
    Future then(Function&& func)
    {
        Future fr { detail::use_private_interface, detail::promised() };
        auto&& rs = fr.get_state(detail::use_private_interface);
        
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!value)
            {
                add_continuation<T, typename Future::value_type>()(continuations, rs, std::forward<Function>(func));
                return fr;
            }
        }
        
        invoke_and_set_value<T, typename Future::value_type>()(*rs, func, value);
        return fr;
    }
    
    template <typename Function>
    void continue_with(Function&& func)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!value)
            {
                add_continuation<T, void>()(continuations, std::forward<Function>(func));
                return;
            }
        }
        invoke(std::forward<Function>(func), value);
    }
    
protected:
    ~future_state_base() {}
};

    
template <typename T>
struct future_state : future_state_base<T>
{
    typedef future_state_base<T> base_type;
    typedef typename base_type::await_queue_type await_queue_type;
    typedef typename base_type::continuations_container continuations_container;
    
    explicit future_state(await_queue_type const& tq = await_queue_type())
    : base_type(tq)
    {
    }
    
    template <class U>
    void set_value(U&& v)
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(this->mutex);
            if (this->value)
                return;
            
            this->value.reset(new T(std::forward<U>(v)));
            cs.swap(this->continuations);
        }
        
        for(auto&& f : cs)
            f(v);
    }

	T const& get() const
	{
		// no lock required because value does not change once it's set
		return *get_value(value);
	}
};
    
template <>
struct future_state<void> : future_state_base<void>
{
    typedef future_state_base<void> base_type;
    typedef base_type::await_queue_type await_queue_type;
    typedef base_type::continuations_container continuations_container;
    
    explicit future_state(await_queue_type const& tq = await_queue_type())
    : base_type(tq)
    {
    }
    
	void get() const
	{

	}

    void set_value()
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (value)
                return;
            
            this->value = true;
            cs.swap(continuations);
        }
        
        for(auto&& f : cs)
            f();
    }
};
    
} // namespace detail

} // namespace ltl

#endif // LTL_FUTURE_HPP
