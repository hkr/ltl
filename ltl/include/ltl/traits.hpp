#ifndef LTL_TRAITS_HPP
#define LTL_TRAITS_HPP

#include "ltl/forward_declarations.hpp"

#include <type_traits>

namespace ltl {
    
template <typename T>
struct is_future : std::false_type {};

template <typename T>
struct is_future<future<T>> : std::true_type {};

template <typename T>
struct is_promise : std::false_type {};

template <typename T>
struct is_promise<promise<T>> : std::true_type {};
    
} // namespace ltl

#endif // LTL_TRAITS_HPP
