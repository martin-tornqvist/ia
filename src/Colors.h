#ifndef COLORS_H
#define COLORS_H

#include "SDL.h"

//struct Color {
//public:
//  Color() : r(0.f), g(0.f), b(0.f) {
//    setClrFactors(0.f, 0.f, 0.f);
//  }
//
//  Color(GLfloat red, GLfloat green, GLfloat blue) :
//    r(red), g(green), b(blue) {
//      setClrFactors(red, green, blue);
//  }
//
//  bool operator==(const Color& other) const {
//    return
//      int(r) == int(other.r) &&
//      int(g) == int(other.g) &&
//      int(b) == int(other.b);
//  }
//
//  bool operator!=(const Color& other) const {
//    return
//      int(r) != int(other.r) ||
//      int(g) != int(other.g) ||
//      int(b) != int(other.b);
//  }
//
//  GLfloat r;
//  GLfloat g;
//  GLfloat b;
//  GLfloat rFactor;
//  GLfloat gFactor;
//  GLfloat bFactor;
//private:
//  void setClrFactors(const GLfloat r_, const GLfloat g_, const GLfloat b_) {
//    rFactor = r_ / 255.f;
//    gFactor = g_ / 255.f;
//    bFactor = b_ / 255.f;
//  }
//};

//STANDARD TERMINAL COLORS (somewhat modified...)
//===========================================================
const SDL_Color clrBlack = {0, 0, 0, 0};
const SDL_Color clrGray = {128, 128, 128, 0};
const SDL_Color clrWhite = {192, 192, 192, 0};
const SDL_Color clrWhiteHigh = {255, 255, 255, 0};

const SDL_Color clrRed = {128, 0, 0, 0};
const SDL_Color clrRedLight = {255, 0, 0, 0}; //{220, 0, 0, 0};

const SDL_Color clrGreen = {0, 128, 0, 0};
const SDL_Color clrGreenLight = {0, 255, 0, 0};

const SDL_Color clrYellow = {255, 255, 0, 0};
const SDL_Color clrBlue = {0, 0, 238, 0};
const SDL_Color clrBlueLight = {92, 92, 255, 0};

const SDL_Color clrMagenta = {139, 0, 139, 0};
const SDL_Color clrMagentaLight = {255, 0, 255, 0};

const SDL_Color clrCyan = {0, 128, 128, 0};
const SDL_Color clrCyanLight = {0, 255, 255, 0};

const SDL_Color clrBrown = {153, 102, 61, 0};
//===========================================================

const SDL_Color	clrBrownDark = {96, 64, 32, 0};

const SDL_Color clrMessageGood = clrGreenLight;
const SDL_Color clrMessageBad = clrRedLight;

//The three armor/damage types from Blood
//const SDL_Color clrArmorRed             (172,   0,   0, 0};
//const SDL_Color clrArmorBlue            ( 75, 105, 243, 0};
//const SDL_Color clrArmorGreen           ( 60, 120,  60, 0};


#endif
