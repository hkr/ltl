#ifndef LTL_PRIVATE_HPP
#define LTL_PRIVATE_HPP

namespace ltl {
namespace detail {
    
struct private_interface_tag {};
typedef private_interface_tag(*private_interface)();
inline private_interface_tag use_private_interface() { return {}; }
    
} // namespace detail
} // namespace ltl

#endif // LTL_PRIVATE_HPP
