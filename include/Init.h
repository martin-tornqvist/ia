#ifndef INIT_H
#define INIT_H

#include "DebugMode.h"

#include <assert.h>

//-------------------------------------------------------------------
// OPTIONS
//-------------------------------------------------------------------
//Lvl of TRACE output in debug mode
// 0 : Disabled
// 1 : Standard
// 2 : Verbose
#define TRACE_LVL 2

//Enable rendering and delays during map gen for evaluation/demo purposes
//Comment out to disable, uncomment to enable
#define DEMO_MODE 1
//-------------------------------------------------------------------

#ifdef NDEBUG
#define TRACE if (1) ; else cerr
#define TRACE_VERBOSE if (1) ; else cerr
#else
#define TRACE if (TRACE_LVL < 1) ; else cerr \
  << endl \
  << __FILE__ << ", " \
  << __func__ << "()" << ", " \
  << __LINE__ << ":" << endl
#define TRACE_VERBOSE if (TRACE_LVL < 2) ; else TRACE
#endif

namespace Init {

extern bool isCheatVisionEnabled;
extern bool quitToMainMenu;

void initIO();
void cleanupIO();

void initGame();
void cleanupGame();

void initSession();
void cleanupSession();

} //Init

#endif
