#ifndef COLORS_HPP
#define COLORS_HPP

#include <SDL_video.h>

typedef SDL_Color Clr;

//-----------------------------------------------------------------------------
// Standard colors
//-----------------------------------------------------------------------------
const Clr clr_black             = {  0,   0,   0, 0};
const Clr clr_gray_xdrk         = { 30,  30,  30, 0};
const Clr clr_gray_drk          = { 48,  48,  48, 0};
const Clr clr_gray              = {128, 128, 128, 0};
const Clr clr_white             = {192, 192, 192, 0};
const Clr clr_white_lgt         = {255, 255, 255, 0};

const Clr clr_red               = {128,   0,   0, 0};
const Clr clr_red_lgt           = {255,   0,   0, 0};

const Clr clr_green_drk         = {  0, 64,    0, 0};
const Clr clr_green             = {  0, 128,   0, 0};
const Clr clr_green_lgt         = {  0, 255,   0, 0};

const Clr clr_yellow_drk        = {205, 205,   0, 0};
const Clr clr_yellow            = {255, 255,   0, 0};

const Clr clr_blue              = {  0,   0, 168, 0};
const Clr clr_blue_lgt          = { 30, 144, 255, 0};

const Clr clr_magenta           = {139,   0, 139, 0};
const Clr clr_magenta_lgt       = {255,   0, 255, 0};

const Clr clr_cyan              = {  0, 128, 128, 0};
const Clr clr_cyan_lgt          = {  0, 255, 255, 0};

const Clr clr_brown             = {153, 102,  61, 0};
const Clr clr_brown_drk         = { 96,  64,  32, 0};
const Clr clr_brown_gray        = { 83,  76,  66, 0};
const Clr clr_brown_gray_drk    = { 34,  32,  26, 0};

const Clr clr_violet            = {128,   0, 255, 0};
const Clr clr_violet_drk        = { 64,   0, 128, 0};

const Clr clr_orange            = {255, 128,   0, 0};

//-----------------------------------------------------------------------------
// Colors taken from Nosferatu movie screen shots
//-----------------------------------------------------------------------------
const Clr clr_nosf_sepia        = {139, 114,  57, 0};
const Clr clr_nosf_sepia_lgt    = {225, 205, 154, 0};
const Clr clr_nosf_sepia_drk    = { 81,  72,  43, 0};

const Clr clr_nosf_teal         = { 57, 157, 155, 0};
const Clr clr_nosf_teal_lgt     = { 88, 226, 228, 0};
const Clr clr_nosf_teal_drk     = { 20,  54,  51, 0};

//-----------------------------------------------------------------------------
// Standard interface colors
//-----------------------------------------------------------------------------
const Clr clr_text              = clr_white;

const Clr clr_menu_highlight    = clr_brown_gray;
const Clr clr_menu_drk          = clr_brown_gray_drk;

const Clr clr_popup_label       = clr_brown_gray_drk;

const Clr clr_title             = clr_brown_gray;

const Clr clr_msg_good          = clr_green_lgt;
const Clr clr_msg_bad           = clr_red_lgt;
const Clr clr_msg_note          = clr_orange;

//-----------------------------------------------------------------------------
// Misc colors
//-----------------------------------------------------------------------------
const Clr clr_allied_mon        = clr_green;

//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------
bool is_clr_equal(const Clr& clr1, const Clr& clr2);

void div_clr(Clr& clr, const double div);

#endif // COLORS_HPP
