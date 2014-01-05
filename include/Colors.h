#ifndef COLORS_H
#define COLORS_H

#include <SDL_video.h>

//Standard terminal colors (somewhat modified...)
//===========================================================
const SDL_Color clrBlack              = {  0,   0,   0, 0};
const SDL_Color clrGray               = {128, 128, 128, 0};
const SDL_Color clrWhite              = {192, 192, 192, 0};
const SDL_Color clrWhiteHigh          = {255, 255, 255, 0};

const SDL_Color clrRed                = {128,   0,   0, 0};
const SDL_Color clrRedLgt             = {255,   0,   0, 0}; //(220, 0, 0, 0};

const SDL_Color clrGreen              = {  0, 128,   0, 0};
const SDL_Color clrGreenLgt           = {  0, 255,   0, 0};

const SDL_Color clrYellow             = {255, 255,   0, 0};
const SDL_Color clrBlue               = {  0,   0, 139, 0};
const SDL_Color clrBlueLgt            = { 92,  92, 255, 0};

const SDL_Color clrMagenta            = {139,   0, 139, 0};
const SDL_Color clrMagentaLgt         = {255,   0, 255, 0};

const SDL_Color clrCyan               = {  0, 128, 128, 0};
const SDL_Color clrCyanLgt            = {  0, 255, 255, 0};

const SDL_Color clrBrown              = {153, 102,  61, 0};
//===========================================================

//Colors taken from Nosferatu movie screen shots
//===========================================================
const SDL_Color clrNosferatuSepia     = {139, 114,  57, 0};
const SDL_Color clrNosferatuSepiaLgt  = {225, 205, 154, 0};
const SDL_Color clrNosferatuSepiaDrk  = { 81,  72,  43, 0};

const SDL_Color clrNosferatuTeal      = { 57, 157, 155, 0};
const SDL_Color clrNosferatuTealLgt   = { 88, 226, 228, 0};
const SDL_Color clrNosferatuTealDrk   = { 20,  54,  51, 0};
//===========================================================

const SDL_Color clrBrownDrk           = { 96,  64,  32, 0};
const SDL_Color clrBrownGray          = { 92,  84,  78, 0};

const SDL_Color clrMsgGood        = clrGreenLgt;
const SDL_Color clrMsgBad         = clrRedLgt;

#endif
