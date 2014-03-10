#ifndef LTL_NOEXCEPT_HPP
#define LTL_NOEXCEPT_HPP

#ifndef _MSC_VER
#define LTL_NOEXCEPT noexcept
#else
#define LTL_NOEXCEPT throw()
#endif

#endif // LTL_NOEXCEPT_HPP
