#ifndef LTL_RESULT_OF_HPP
#define LTL_RESULT_OF_HPP

#include <type_traits>

namespace ltl {
    
template <typename Function, typename Arg>
struct result_of : std::result_of<Function(Arg)> {};

template <typename Function>
struct result_of<Function, void> : std::result_of<Function()> {};
    
} // namespace ltl

#endif // LTL_RESULT_OF_HPP
