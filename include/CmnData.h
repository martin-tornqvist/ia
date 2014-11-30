#ifndef CMN_DATA_H
#define CMN_DATA_H

#include <string>

const size_t PLAYER_NAME_MAX_LEN = 12;

const std::string gameVersionStr = "v17.0";

const std::string tilesImgName         = "images/gfx_16x24.png";
const std::string mainMenuLogoImgName  = "images/main_menu_logo.png";

const int SCREEN_BPP = 32;

const int MAP_W               = 80;
const int MAP_H               = 22;
const int MAP_W_HALF          = MAP_W / 2;
const int MAP_H_HALF          = MAP_H / 2;

const int LOG_H               = 2;
const int MAP_OFFSET_H        = LOG_H;
const int CHAR_LINES_OFFSET_H = MAP_OFFSET_H + MAP_H;
const int CHAR_LINES_H        = 3;
const int SCREEN_W            = MAP_W;
const int SCREEN_H            = CHAR_LINES_OFFSET_H + CHAR_LINES_H;

const int DESCR_X0            = MAP_W - 31;

const int PLAYER_START_X      = 10;
const int PLAYER_START_Y      = MAP_H_HALF;

const int PLAYER_MAX_CLVL = 10;

const int FIRST_CAVERN_LVL            = 20;
const int LAST_CAVERN_LVL             = 29;
const int LAST_ROOM_AND_CORRIDOR_LVL  = FIRST_CAVERN_LVL - 2;

const int AUDIO_ALLOCATED_CHANNELS = 16;

const int DELAY_PLAYER_UNABLE_TO_ACT = 7;

const int PLAYER_START_HP   = 16;
const int PLAYER_START_SPI  = 6;
const int HP_PER_LVL        = 3;
const int SPI_PER_LVL       = 1;

const int MIN_DLVL_HARDER_TRAPS = 6;

const int     FOV_STD_RADI_INT  = 8;
const int     FOV_STD_W_INT     = (FOV_STD_RADI_INT * 2) + 1;
const double  FOV_STD_RADI_DB   = double(FOV_STD_RADI_INT);
const double  FOV_STD_W_DB      = double(FOV_STD_W_INT);
const int     FOV_MAX_RADI_INT  =  FOV_STD_RADI_INT * 2;
const int     FOV_MAX_W_INT     = (FOV_MAX_RADI_INT * 2) + 1;
const double  FOV_MAX_RADI_DB   = double(FOV_MAX_RADI_INT);
const double  FOV_MAX_W_DB      = double(FOV_MAX_W_INT);

const int SND_DIST_NORMAL       = FOV_STD_RADI_INT;
const int SND_DIST_LOUD         = SND_DIST_NORMAL * 2;

const int THROW_RANGE_LMT  = FOV_STD_RADI_INT + 3;

const int DYNAMITE_FUSE_TURNS = 5;
const int EXPLOSION_STD_RADI  = 2;

const int ENC_IMMOBILE_LVL = 125;

const int NR_MG_PROJECTILES                     = 5;
const int NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES  = 2;

//Note: Explosion damage is reduced with distance from center
const int EXPL_DMG_ROLLS  = 5;
const int EXPL_DMG_SIDES  = 6;
const int EXPL_DMG_PLUS   = 10;
const int EXPL_MAX_DMG    = (EXPL_DMG_ROLLS* EXPL_DMG_SIDES) + EXPL_DMG_PLUS;

//Value used for limiting spawning over time and "breeder" monsters. The actual number of
//actors may sometimes go a bit above this number, e.g. due to a group of monsters
//spawning when the number of actors is near the limit. Summoning spells don't check
//this number at all (because their effect shouldn't be arbitrarily blocked by meta) -
//so that may also push the number of actors above the limit. (It's a soft limit)
const size_t MAX_NR_ACTORS_ON_MAP = 125;

const std::string infoScrCmdInfo =
  " [2/8 down/up j/k] to navigate [space/esc] to exit ";

const std::string cancelInfoStrNoSpace  = "[space/esc] to cancel";
const std::string cancelInfoStr         = " " + cancelInfoStr;
const std::string msgDisarmNoTrap       = "I find nothing there to disarm.";
const std::string msgMonPreventCmd      = "Not while an enemy is near.";

#endif
