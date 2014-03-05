#ifndef LTL_FUTURE_COMBINATORS_HPP
#define LTL_FUTURE_COMBINATORS_HPP

#include <atomic>
#include <iterator>
#include <algorithm>
#include <vector>
#include <ltl/future.hpp>
#include <ltl/detail/tuple_meta.hpp>

namespace ltl {

template <typename InputIterator>
future<void> when_all_ready(InputIterator first, InputIterator last)
{
    auto const diff = std::distance(first, last);
    if (diff <= 0)
        return make_future();
    
    typedef typename std::iterator_traits<InputIterator>::difference_type diff_t;
    
    auto counter = std::make_shared<std::atomic<diff_t>>(diff);
    auto prm = std::make_shared<promise<void>>();
    auto dec = std::bind([=](){ if (counter->fetch_sub(1) == 1) prm->set_value(); }); // bind to ignore all arguments
    
    std::for_each(first, last,
                  [=](typename std::iterator_traits<InputIterator>::reference f)
    {
        if (!f.valid())
            dec();
        else
            f.get_state(detail::use_private_interface)->continue_with(dec);
    });
    
    return prm->get_future();
}
 
template <typename InputIterator>
future<std::vector<typename std::iterator_traits<InputIterator>::value_type::value_type>>
when_all(InputIterator first, InputIterator last)
{
    typedef typename std::iterator_traits<InputIterator>::value_type future_type;
    typedef typename future_type::value_type value_type;
    static_assert(is_future<future_type>::value, "InputIterator::value_type must be future<T>");
    
    auto fs = std::make_shared<std::vector<future_type>>(first, last);
    
    return when_all_ready(std::begin(*fs), std::end(*fs)).then([=]() {
        std::vector<typename future_type::value_type> result(fs->size());
        std::transform(std::begin(*fs), std::end(*fs), std::begin(result), [](future_type& x) {
            return x.get();
        });
        return result;
    });
}
    
namespace detail {

struct to_future_void
{
    typedef future<void> type;
    template <typename T>
    type operator()(future<T> const& x) const
    {
        if (auto&& state = x.get_state(use_private_interface))
            return state->template then<future<void>>(std::bind([](){}));
        else
            return make_future();
    }
};

template <typename T>
struct future_get
{
    typedef typename T::value_type type;
    type operator()(T& x) const
    {
        return x.get();
    }
};
    
} // namespace detail
    
template <typename... Ts>
future<void> when_all_ready(std::tuple<Ts...>& fs)
{
    typedef std::tuple<Ts...> TupleOfFutures;
    std::vector<future<void>> vf;
    vf.reserve(std::tuple_size<TupleOfFutures>::value);
    detail::tuple_transform<TupleOfFutures>::apply(fs, std::back_inserter(vf), detail::to_future_void());
    return when_all_ready(std::begin(vf), std::end(vf));
}
    
template <typename... Ts>
future<typename detail::tuple_map<detail::future_get, Ts...>::type>
    when_all(std::tuple<Ts...>& fs)
{
    auto fsc = std::make_shared<std::tuple<Ts...>>(fs);
    return when_all_ready(fs).then([=]() {
        return detail::tuple_map<detail::future_get, Ts...>()(*fsc);
    });
}
    
} // namespace ltl

#endif // LTL_FUTURE_COMBINATORS_HPP
