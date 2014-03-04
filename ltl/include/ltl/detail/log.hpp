#ifndef LTL_LOG_HPP
#define LTL_LOG_HPP

//#define LTL_ENABLE_LOGGING

#ifdef LTL_ENABLE_LOGGING
#  include <stdio.h>
#  define LTL_LOG(...) printf(__VA_ARGS__)
#else
#  define LTL_LOG(...)
#endif

#endif // LTL_LOG_HPP
