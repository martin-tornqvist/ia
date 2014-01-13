#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#include <string>

const unsigned int PLAYER_NAME_MAX_LENGTH = 12;

const std::string gameVersionStr = "v16.0 BETA";

const std::string tilesImgName         = "images/gfx_16x24.png";
const std::string mainMenuLogoImgName  = "images/main_menu_logo.png";

const int SCREEN_BPP = 32;

const int MAP_W               = 80;
const int MAP_H               = 22;
const int MAP_W_HALF          = MAP_W / 2;
const int MAP_H_HALF          = MAP_H / 2;
const int LOG_H               = 1;
const int MAP_OFFSET_H        = LOG_H;
const int CHAR_LINES_OFFSET_H = MAP_OFFSET_H + MAP_H;
const int CHAR_LINES_H        = 3;
const int SCREEN_W            = MAP_W;
const int SCREEN_H            = CHAR_LINES_OFFSET_H + CHAR_LINES_H;

const int PLAYER_START_X      = 10;
const int PLAYER_START_Y      = MAP_H_HALF;

const int PLAYER_MAX_CLVL = 30;

const int FIRST_CAVERN_LEVEL            = 20;
const int LAST_CAVERN_LEVEL             = 29;
const int LAST_ROOM_AND_CORRIDOR_LEVEL  = FIRST_CAVERN_LEVEL - 2;

const int AUDIO_ALLOCATED_CHANNELS = 16;

const int DELAY_PLAYER_WAITING        = 0;
const int DELAY_PLAYER_UNABLE_TO_ACT  = 7;

const int PLAYER_START_SPI                = 6;
const int SHOCK_TAKEN_FROM_CASTING_SPELLS = 10;
const int MTH_LVL_SPELLS_SPI_BON          = 20;
const int MTH_LVL_NEW_PWR                 = 15;

const int MIN_DLVL_NASTY_TRAPS = 6;

const int     FOV_STD_RADI_INT  = 8;
const int     FOV_STD_W_INT     = (FOV_STD_RADI_INT * 2) + 1;
const double  FOV_STD_RADI_DB   = double(FOV_STD_RADI_INT);
const double  FOV_STD_W_DB      = double(FOV_STD_W_INT);
const int     FOV_MAX_RADI_INT  =  FOV_STD_RADI_INT * 2;
const int     FOV_MAX_W_INT     = (FOV_MAX_RADI_INT * 2) + 1;
const double  FOV_MAX_RADI_DB   = double(FOV_MAX_RADI_INT);
const double  FOV_MAX_W_DB      = double(FOV_MAX_W_INT);

const int THROWING_RANGE_LIMIT  = FOV_STD_RADI_INT + 3;

const int DYNAMITE_FUSE_TURNS = 5;
const int EXPLOSION_STD_RADI  = 2;

const int NR_MG_PROJECTILES                       = 5;
const int NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES    = 2;

const int CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP = 50;

const std::string cancelInfoStr = " | space/esc to cancel";

#endif
