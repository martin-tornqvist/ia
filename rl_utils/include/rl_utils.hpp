#ifndef RL_UTILS_HPP
#define RL_UTILS_HPP

//------------------------------------------------------------------------------
// Trace level (use -D build flag to override)
//------------------------------------------------------------------------------
// Lvl of TRACE output in debug mode
// 0 : Disabled
// 1 : Standard
// 2 : Verbose
#ifndef TRACE_LVL

#define TRACE_LVL 1

#endif // TRACE_LVL

//------------------------------------------------------------------------------
// Custom trace output and assert functionality
//------------------------------------------------------------------------------
#ifdef NDEBUG // Release mode

#define ASSERT(check)

#define TRACE                     if (1) ; else std::cerr
#define TRACE_FUNC_BEGIN          if (1) ; else std::cerr
#define TRACE_FUNC_END            if (1) ; else std::cerr
#define TRACE_VERBOSE             if (1) ; else std::cerr
#define TRACE_FUNC_BEGIN_VERBOSE  if (1) ; else std::cerr
#define TRACE_FUNC_END_VERBOSE    if (1) ; else std::cerr

#define PANIC exit(EXIT_FAILURE)

#else // Debug mode

#define ASSERT(check)                                                   \
    do_not_call::assert_impl(check, #check, __FILE__, __LINE__, __func__)

#define TRACE if (TRACE_LVL < 1) ; else         \
        std::cerr                               \
            << "DEBUG: "                        \
            << __FILE__ << ", "                 \
            << __LINE__ << ", "                 \
            << __func__ << "():"                \
            << std::endl

#define TRACE_FUNC_BEGIN if (TRACE_LVL < 1) ; else      \
        std::cerr                                       \
            << "DEBUG: "                                \
            << __FILE__ << ", "                         \
            << __LINE__ << ", "                         \
            << __func__ << "() [BEGIN]"                 \
            << std::endl

#define TRACE_FUNC_END if (TRACE_LVL < 1) ; else        \
        std::cerr                                       \
            << "DEBUG: "                                \
            << __FILE__ << ", "                         \
            << __LINE__ << ", "                         \
            << __func__ << "() [END]"                   \
            << std::endl

#define TRACE_VERBOSE             if (TRACE_LVL < 2) ; else TRACE
#define TRACE_FUNC_BEGIN_VERBOSE  if (TRACE_LVL < 2) ; else TRACE_FUNC_BEGIN
#define TRACE_FUNC_END_VERBOSE    if (TRACE_LVL < 2) ; else TRACE_FUNC_END

#define PANIC ASSERT(false)

#endif // NDEBUG

// Print an error, for both debug and release builds
#define TRACE_ERROR_RELEASE std::cerr << "ERROR: "

//------------------------------------------------------------------------------
// Custom assert
// NOTE: Never call this function directly, use the "ASSERT" macro above
//------------------------------------------------------------------------------
namespace do_not_call
{

void assert_impl(const bool check,
                 const char* check_str,
                 const char* file,
                 const int line,
                 const char* func);

} // do_not_call

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <functional>
#include <random>
#include <ctime>

// This header must be supplied by the "user" project, and contain some
// necessary symbols such as "map_w" and "map_h".
#include "global.hpp"

// RL Utils includes
// NOTE: The user project only needs to include rl_utils.hpp (this file)
#include "array2.hpp"
#include "direction.hpp"
#include "flood.hpp"
#include "pathfind.hpp"
#include "pos.hpp"
#include "random.hpp"
#include "rect.hpp"
#include "misc.hpp"
#include "time.hpp"

#endif // RL_UTILS_HPP
