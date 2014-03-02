#ifndef LTL_FUTURE_THEN_BASE_HPP
#define LTL_FUTURE_THEN_BASE_HPP

#include <type_traits>
#include <mutex>
#include <memory>

namespace ltl {
    
template <typename> class future;
    
namespace detail {

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
};

template <typename T, typename Future, typename State, typename Function>
Future then_impl(State& state, Function&& func)
{
    Future fr { detail::promised() };
    auto&& rs = fr.get_state();
    
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (!state.value)
        {
            add_continuation<T, typename Future::value_type>()(state.continuations, rs, std::forward<Function>(func));
            return fr;
        }
    }
    
    invoke_and_set_value<T, typename Future::value_type>()(*rs, func, state.value);
    return fr;
}
    
template <typename Derived, typename T>
struct future_then_base
{
    template <typename Function>
    future<typename std::result_of<Function(T)>::type> then(Function func)
    {
        typedef typename std::result_of<Function(T)>::type result_type;
        auto&& state = static_cast<Derived*>(this)->get_state();
        if (!state)
            return future<result_type>();
        
        return then_impl<T, future<result_type>>(*state, std::move(func));
    }
    
protected:
    ~future_then_base() {}
};
    
template <typename Derived>
struct future_then_base<Derived, void>
{
    template <typename Function>
    future<typename std::result_of<Function()>::type> then(Function func)
    {
        typedef typename std::result_of<Function()>::type result_type;
        auto&& state = static_cast<Derived*>(this)->get_state();
        
        if (!state)
            return future<result_type>();
        
        return then_impl<void, future<result_type>>(*state, std::move(func));
    }
};

} // namespace detail
} // namespace ltl

#endif // LTL_FUTURE_HPP
