#ifndef LTL_FUTURE_STATE_HPP
#define LTL_FUTURE_STATE_HPP

#include <type_traits>
#include <mutex>
#include <memory>
#include <deque>
#include <future>

#include "ltl/detail/task_queue_impl.hpp"
#include "ltl/detail/block.hpp"
#include "ltl/detail/private.hpp"
#include "ltl/forward_declarations.hpp"

namespace ltl {
    
namespace detail {

struct promised {};
    
template <typename ValueHolder>
typename ValueHolder::pointer get_value(ValueHolder const& holder)
{
    return holder.get();
}

inline void* get_value(bool const& holder)
{
    return holder ? reinterpret_cast<void*>(1) : nullptr;
}

template <typename T, typename R>
struct invoke_and_set_value
{
    template <typename Function, typename State>
    void operator()(State& rs, Function&& func)
    {
        try
        {
            rs.set_value(func());
        }
        catch (...)
        {
            rs.set_exception(std::current_exception());
        }
    }
};

template <typename T>
struct invoke_and_set_value<T, void>
{
    template <typename Function, typename State>
    void operator()(State& rs, Function&& func)
    {
        try
        {
            func();
            rs.set_value();
        }
        catch (...)
        {
            rs.set_exception(std::current_exception());
        }
    }
};

template <typename T, typename R>
struct add_continuation
{
    template <typename Function, typename Continuations, typename State>
    void operator()(Continuations& continuations, std::shared_ptr<State> const& rs, Function&& func) const
    {
        continuations.emplace_back([=]() mutable
        {
            invoke_and_set_value<T, R>()(*rs, func);
        });
    }
};

template <typename T>
struct add_continuation<T, void>
{
    template <typename Function, typename Continuations, typename State>
    void operator()(Continuations& continuations, std::shared_ptr<State> const& rs, Function&& func) const
    {
		continuations.emplace_back([=]() mutable 
		{
            invoke_and_set_value<T, void>()(*rs, std::forward<Function>(func));
        });
    }
};

template <typename T>
class future_state_base : public std::enable_shared_from_this<future_state_base<T>>
{
public:
    typedef typename std::conditional<std::is_void<T>::value, bool, std::unique_ptr<T>>::type ValueHolder;
    typedef std::deque<std::function<void()>> continuations_container;
    typedef std::shared_ptr<detail::task_queue_impl> await_queue_type;
    
public:
    explicit future_state_base(await_queue_type const& tq)
    : value_()
    , await_queue_(tq)
    {
    }
    
    future_state_base(future_state_base const&) =delete;
    future_state_base& operator=(future_state_base const&) =delete;

    bool ready() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_ || exception_;
    }
    
    void wait()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (auto x = get_value(value_))
                return;
            
            if (!block_)
            {
                block_.reset(new detail::block());
                continuations_.push_back(std::bind(&detail::block::signal, block_.get()));
            }
        }
        
        block_->wait();
    }
    
    template <typename Future, typename Function>
    Future then(Function&& func)
    {
        Future fr { detail::use_private_interface, detail::promised() };
        auto&& rs = fr.get_state(detail::use_private_interface);
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!value_ && !exception_)
            {
                add_continuation<T, typename Future::value_type>()(continuations_, rs, std::forward<Function>(func));
                return fr;
            }
        }
        
        invoke_and_set_value<T, typename Future::value_type>()(*rs, func);
        return fr;
    }
    
    template <typename Function>
    void continue_with(Function&& func)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!value_ && !exception_)
            {
                continuations_.emplace_back([=]() mutable {
                    func(); // TODO: handle exception
                });
                return;
            }
        }
        func(); // TODO: handle exception
    }
    
    void set_exception(std::exception_ptr p)
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (value_ || exception_) throw std::future_error(std::future_errc::promise_already_satisfied);
            exception_ = std::move(p);
            cs.swap(this->continuations_);
        }
                
        for(auto&& f : cs)
            f();
    }
    
    await_queue_type await_queue() const
    {
        return await_queue_;
    }
    
protected:
    mutable std::mutex mutex_;
    ValueHolder value_;
    continuations_container continuations_;
    await_queue_type await_queue_;
    std::unique_ptr<detail::block> block_;
    std::exception_ptr exception_;
    
protected:
    ~future_state_base() {}
};

    
template <typename T>
struct future_state : future_state_base<T>
{
    typedef future_state_base<T> base_type;
    typedef typename base_type::await_queue_type await_queue_type;
    typedef typename base_type::continuations_container continuations_container;
    typedef T const& get_result_type;
    
    explicit future_state(await_queue_type const& tq = await_queue_type())
    : base_type(tq)
    {
    }
    
    template <class U>
    void set_value_impl(U&& v, bool allow_exception)
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            if (this->value_ || this->exception_)
            {
                if (allow_exception) throw std::future_error(std::future_errc::promise_already_satisfied);
                return;
            }
            
            this->value_.reset(new T(std::forward<U>(v)));
            cs.swap(this->continuations_);
        }
        
        for(auto&& f : cs)
            f();
    }
    
    template <class U>
    void set_value(U&& v)
    {
        set_value_impl(std::forward<U>(v), true);
    }

	T const& get() const
	{
		// no lock required because value does not change once it's set
        if (this->exception_) std::rethrow_exception(this->exception_);
		return *get_value(this->value_);
	}
};
    
template <>
struct future_state<void> : future_state_base<void>
{
    typedef future_state_base<void> base_type;
    typedef base_type::await_queue_type await_queue_type;
    typedef base_type::continuations_container continuations_container;
    typedef void get_result_type;
    
    explicit future_state(await_queue_type const& tq = await_queue_type())
    : base_type(tq)
    {
    }
    
	void get() const
	{
        if (this->exception_) std::rethrow_exception(this->exception_);
	}

    void set_value_impl(bool allow_exception)
    {
        continuations_container cs;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (value_ || this->exception_)
            {
                if (allow_exception) throw std::future_error(std::future_errc::promise_already_satisfied);
                return;
            }
            
            this->value_ = true;
            cs.swap(continuations_);
        }
        
        for(auto&& f : cs)
            f();
    }
    
    void set_value()
    {
        set_value_impl(true);
    }
};
    
} // namespace detail

} // namespace ltl

#endif // LTL_FUTURE_HPP
