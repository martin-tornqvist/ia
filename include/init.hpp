#ifndef INIT_H
#define INIT_H

#include "debug_mode.hpp"

#include <iostream>

//-----------------------------------------------------------------------------
// OPTIONS
//-----------------------------------------------------------------------------
//Lvl of TRACE output in debug mode
// 0 : Disabled
// 1 : Standard
// 2 : Verbose
#ifndef TRACE_LVL

#define TRACE_LVL 1

#endif //TRACE_LVL

//Enable rendering and delays during map gen for evaluation/demo purposes
//Comment out to disable, uncomment to enable
//#define DEMO_MODE 1
//-----------------------------------------------------------------------------

#ifdef NDEBUG

#define IA_ASSERT(check)

#define TRACE                     if (1) ; else std::cerr
#define TRACE_FUNC_BEGIN          if (1) ; else std::cerr
#define TRACE_FUNC_END            if (1) ; else std::cerr
#define TRACE_VERBOSE             if (1) ; else std::cerr
#define TRACE_FUNC_BEGIN_VERBOSE  if (1) ; else std::cerr
#define TRACE_FUNC_END_VERBOSE    if (1) ; else std::cerr

#else //NDEBUG not defined

#define IA_ASSERT(check) init::ia_assert(check, #check, __FILE__, __LINE__, __func__)

#define TRACE if (TRACE_LVL < 1) ; else std::cerr \
  << std::endl \
  << "T:" << to_str(game_time::turn()) << " " \
  << __FILE__ << ", " \
  << __LINE__ << ", " \
  << __func__ << "():" << std::endl

#define TRACE_FUNC_BEGIN if (TRACE_LVL < 1) ; else std::cerr \
  << std::endl \
  << "T:" << to_str(game_time::turn()) << " " \
  << __FILE__ << ", " \
  << __LINE__ << ", " \
  << __func__ << "()..." << std::endl

#define TRACE_FUNC_END if (TRACE_LVL < 1) ; else std::cerr \
  << std::endl \
  << "T:" << to_str(game_time::turn()) << " " \
  << __FILE__ << ", " \
  << __LINE__ << ", " \
  << __func__ << "() [DONE]" << std::endl

#define TRACE_VERBOSE             if (TRACE_LVL < 2) ; else TRACE
#define TRACE_FUNC_BEGIN_VERBOSE  if (TRACE_LVL < 2) ; else TRACE_FUNC_BEGIN
#define TRACE_FUNC_END_VERBOSE    if (TRACE_LVL < 2) ; else TRACE_FUNC_END

#endif //NDEBUG

namespace init
{

extern bool is_cheat_vision_enabled;
extern bool quit_to_main_menu;

//Do not call this function directly, use "IA_ASSERT" macro above
void ia_assert(const bool check,
               const char* check_str,
               const char* file,
               const int line,
               const char* func);

void init_io();
void cleanup_io();

void init_game();
void cleanup_game();

void init_session();
void cleanup_session();

} //init

#endif
