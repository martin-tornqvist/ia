#ifndef INIT_H
#define INIT_H

#include "DebugMode.h"

#include <assert.h>

//-------------------------------------------------------------------
// OPTIONS
//-------------------------------------------------------------------
//Lvl of trace output in debug mode
// 0 : Disabled
// 1 : Standard
// 2 : Verbose
#define TRACE_LVL 1

//Enable rendering and delays during map gen for evaluation/demo purposes
//Comment out to disable, uncomment to enable
#define DEMO_MODE 1
//-------------------------------------------------------------------

#ifdef NDEBUG
#define trace if (1) ; else cerr
#define traceVerbose if (1) ; else cerr
#else
#define trace if (TRACE_LVL < 1) ; else cerr
#define traceVerbose if (TRACE_LVL < 2) ; else cerr
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
