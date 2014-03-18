#ifndef LTL_GET_AND_COMPOSE_HPP
#define LTL_GET_AND_COMPOSE_HPP

namespace ltl {
    
namespace detail {

template <typename R, typename T, typename Function, typename State>
struct get_and_compose
{
    template <typename Func>
    get_and_compose(Func&& f, State const& s)
    : s_(s)
    , f_(std::forward<Func>(f))
    {
    }
    
    R operator()() const
    {
        return f_(s_->get());
    }
    
    State s_;
    Function f_;
};

template <typename R, typename Function, typename State>
struct get_and_compose<R, void, Function, State>
{
    template <typename Func>
    get_and_compose(Func&& f, State const& s)
    : s_(s)
    , f_(std::forward<Func>(f))
    {
    }
    
    R operator()() const
    {
        s_->get();
        return f_();
    }
    
    State s_;
    Function f_;
};

} // namespace detail
    
} // namespace ltl

#endif // LTL_GET_AND_COMPOSE_HPP
