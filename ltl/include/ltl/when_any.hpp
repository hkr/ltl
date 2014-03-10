#ifndef LTL_FUTURE_WHEN_ANY_HPP
#define LTL_FUTURE_WHEN_ANY_HPP

#include <functional>
#include <iterator>
#include <ltl/promise.hpp>

namespace ltl {

template <typename ForwardIterator>
future<std::size_t> when_any_ready(ForwardIterator first, ForwardIterator last)
{
    auto const diff = std::distance(first, last);
    if (diff <= 0)
        return make_ready_future(std::size_t(-1));
 
    auto prm = std::make_shared<promise<std::size_t>>();
    
    bool at_least_one_valid = false;
    std::size_t i = 0;
    for (auto it = first; it != last; ++i, ++it)
    {
        auto&& f = *first;
        if (f.valid())
        {
            at_least_one_valid = true;
            auto&& state = f.get_state(detail::use_private_interface);
            state->continue_with(std::bind([=](){ prm->set_value(i); }));
        }
    }
    
    if (!at_least_one_valid)
        prm->set_value(std::size_t(-1)); // to avoid blocking forever
    
    return prm->get_future();
}
    
} // namespace ltl

#endif // LTL_FUTURE_WHEN_ANY_HPP
