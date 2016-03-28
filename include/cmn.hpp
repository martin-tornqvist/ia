#ifndef CMN_H
#define CMN_H

#include <string>

#include "colors.hpp"
#include "art.hpp"

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------
const size_t PLAYER_NAME_MAX_LEN            = 14;

const std::string version_str               = "v18.1";

const std::string tiles_img_name            = "images/gfx_16x24.png";
const std::string logo_img_name             = "images/main_menu_logo.png";
const std::string skull_img_name            = "images/skull.png";

const int SCREEN_BPP                        = 32;

const int MAP_W                             = 80;
const int MAP_H                             = 22;
const int MAP_W_HALF                        = MAP_W / 2;
const int MAP_H_HALF                        = MAP_H / 2;
const int NR_MAP_CELLS                      = MAP_W * MAP_H;

const int LOG_H                             = 2;
const int MAP_OFFSET_H                      = LOG_H;
const int CHAR_LINES_OFFSET_H               = MAP_OFFSET_H + MAP_H;
const int CHAR_LINES_H                      = 3;
const int SCREEN_W                          = MAP_W;
const int SCREEN_H                          = CHAR_LINES_OFFSET_H + CHAR_LINES_H;

const int DESCR_X0                          = MAP_W - 31;

const int PLAYER_START_X                    = 10;
const int PLAYER_START_Y                    = MAP_H_HALF;

const int PLAYER_MAX_CLVL                   = 12;

const int DLVL_LAST_EARLY_GAME              = 9;                        //Early     = DLVL 1  - 9
const int DLVL_FIRST_MID_GAME               = DLVL_LAST_EARLY_GAME + 1; //
const int DLVL_LAST_MID_GAME                = 19;                       //Mid       = DLVL 10 - 19
const int DLVL_FIRST_LATE_GAME              = DLVL_LAST_MID_GAME + 1;   //
const int DLVL_LAST                         = 30;                       //Late      = DLVL 20 - 30

const int MIN_DLVL_HARDER_TRAPS             = 6;

const int AUDIO_ALLOCATED_CHANNELS          = 16;

const size_t MS_DELAY_PLAYER_UNABLE_ACT     = 7;
const size_t MIN_MS_BETWEEN_SAME_SFX        = 60;

const int PLAYER_START_HP                   = 16;
const int PLAYER_START_SPI                  = 6;
const int HP_PER_LVL                        = 2;
const int SPI_PER_LVL                       = 1;

const int MIN_DMG_TO_WOUND                  = 5;

const int XP_FOR_DISRM_TRAP                 = 1;

const int     FOV_STD_RADI_INT              = 8;
const int     FOV_STD_W_INT                 = (FOV_STD_RADI_INT * 2) + 1;
const double  FOV_STD_RADI_DB               = double(FOV_STD_RADI_INT);
const double  FOV_STD_W_DB                  = double(FOV_STD_W_INT);
const int     FOV_MAX_RADI_INT              = FOV_STD_RADI_INT * 2;
const int     FOV_MAX_W_INT                 = (FOV_MAX_RADI_INT * 2) + 1;
const double  FOV_MAX_RADI_DB               = double(FOV_MAX_RADI_INT);
const double  FOV_MAX_W_DB                  = double(FOV_MAX_W_INT);

const int SND_DIST_NORMAL                   = FOV_STD_RADI_INT;
const int SND_DIST_LOUD                     = SND_DIST_NORMAL * 2;

const int THROW_RANGE_LMT                   = FOV_STD_RADI_INT + 3;

const int DYNAMITE_FUSE_TURNS               = 5;
const int EXPLOSION_STD_RADI                = 2;

const int ENC_IMMOBILE_LVL                  = 125;

const int NR_MG_PROJECTILES                 = 5;
const int NR_CELL_JUMPS_MG_PROJECTILES      = 2;

//NOTE: Number of rolls is reduced by one for each step away from the explosion center
const int EXPL_DMG_ROLLS                    = 5;
const int EXPL_DMG_SIDES                    = 6;
const int EXPL_DMG_PLUS                     = 10;
const int EXPL_MAX_DMG                      = (EXPL_DMG_ROLLS* EXPL_DMG_SIDES) + EXPL_DMG_PLUS;

const int POISON_DMG_N_TURN                 = 3;

const int SHOCK_FROM_OBSESSION              = 30;

const int INS_FROM_DISTURBING_ITEMS         = 5;

//How many "units" of weight the player can carry, without trait modifiers etc
const int PLAYER_CARRY_WEIGHT_BASE          = 500;

//Value used for limiting spawning over time and "breeder" monsters. The actual number of actors
//may sometimes go a bit above this number, e.g. due to a group of monsters spawning when the
//number of actors is near the limit. Summoning spells does not check this number at all (because
//their effects should not be arbitrarily limited by this) - so that may also push the number of
//actors above the limit. This number is treated as a soft limit.
const size_t MAX_NR_ACTORS_ON_MAP           = 125;

const std::string info_scr_tip              = "[space/esc] to exit";
const std::string info_scr_tip_scrollable   = "[2/8, down/up, j/k] " + info_scr_tip;
const std::string cancel_info_str_no_space  = "[space/esc] to cancel";
const std::string cancel_info_str           = " " + cancel_info_str_no_space;
const std::string drop_info_str             = " [shift+select] to drop";
const std::string msg_disarm_no_trap        = "I find nothing there to disarm.";
const std::string msg_mon_prevent_cmd       = "Not while an enemy is near.";
const std::string spell_resist_msg          = "The spell is resisted!";
const std::string spell_reflect_msg         = "The spell is reflected!";
const std::string spell_reflect_self_msg    = "There is a faint echo...";

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
//This is typically used for functions such as item identification and property applying to
//enable/disable printing to the message log and other such "side effects". For example when
//loading a saved game, we may want to do these things silently.
enum class Verbosity
{
    silent,
    verbose,
};

enum class Inv_type
{
    slots,
    backpack
};

enum class Did_action
{
    yes,
    no
};

enum class Pass_time
{
    yes,
    no
};

enum class Consume_item
{
    yes,
    no
};

enum class Unequip_allowed
{
    yes,
    no
};

enum class Item_ref_type
{
    plain,
    a,
    plural,
    END
};

enum class Item_ref_inf
{
    none,
    yes
};

enum class Item_ref_att_inf
{
    none,
    wpn_context,
    melee,
    ranged,
    thrown
};

enum class Article
{
    a,
    the
};

enum class Matl
{
    empty,
    stone,
    metal,
    plant,  //Grass, bushes, reeds, vines, fungi...
    wood,   //Trees, doors, benches...
    cloth,  //Carpet, silk (cocoons)...
    fluid
};

enum class Liquid_type
{
    water,
    mud,
    blood,
    acid,
    lava
};

enum class Condition
{
    breaking,
    shoddy,
    fine
};

enum class Dmg_type
{
    physical,
    fire,
    acid,
    electric,
    spirit,
    light,
    pure,
    END
};

enum class Dmg_method
{
    piercing,
    slashing,
    blunt_medium,
    blunt_heavy,
    kick,
    explosion,
    shotgun,
    elemental,
    forced, //Guaranteed to detroy the feature (silently - no messages)
    END
};

enum class Att_mode
{
    none,
    melee,
    thrown,
    ranged
};

enum class Allow_wound
{
    no,
    yes
};

enum class Shock_lvl
{
    none   = 0,
    mild   = 2,
    some   = 4,
    heavy  = 12
};

enum class Game_entry_mode
{
    new_game,
    load_game
};

enum class Spawn_rate
{
    never,
    extremely_rare,
    very_rare,
    rare,
    common,
    very_common
};

enum class Ver_dir
{
    up,
    down
};

enum class Actor_state
{
    alive,
    corpse,
    destroyed
};

enum class Axis
{
    hor,
    ver
};

enum class Lgt_size
{
    none,
    small, //3x3
    fov
};

enum class More_prompt_on_msg
{
    no,
    yes
};

enum class Inf_screen_type
{
    scrolling,
    single_screen
};

struct Cell_render_data
{
    Cell_render_data() :
        clr                             (clr_black),
        clr_bg                          (clr_black),
        tile                            (Tile_id::empty),
        glyph                           (' '),
        lifebar_length                  (-1),
        is_light_fade_allowed           (true),
        is_marked_lit                   (false),
        is_living_actor_seen_here       (false),
        is_aware_of_hostile_mon_here    (false),
        is_aware_of_allied_mon_here     (false) {}

    Clr     clr;
    Clr     clr_bg;
    Tile_id tile;
    char    glyph;
    int     lifebar_length;
    bool    is_light_fade_allowed;
    bool    is_marked_lit;
    bool    is_living_actor_seen_here;
    bool    is_aware_of_hostile_mon_here;
    bool    is_aware_of_allied_mon_here;
};

struct Str_and_clr
{
    Str_and_clr() :
        str(""),
        clr(clr_black) {}

    Str_and_clr(const std::string& text, const Clr& color) :
        str(text),
        clr(color) {}

    Str_and_clr& operator=(const Str_and_clr& other)
    {
        str = other.str;
        clr = other.clr;
        return *this;
    }

    std::string str;
    Clr clr;
};

enum class Item_type
{
    general,
    melee_wpn,
    ranged_wpn,
    throwing_wpn,
    ammo,
    ammo_mag,
    scroll,
    potion,
    device,
    rod,
    armor,
    head_wear,
    amulet,
    explosive,

    END_OF_EXTR_ITEMS,
    melee_wpn_intr,
    ranged_wpn_intr
};

struct Item_name
{
    Item_name(const std::string& name,
              const std::string& name_pl,
              const std::string& name_a)
    {
        names[size_t(Item_ref_type::plain)]   = name;
        names[size_t(Item_ref_type::plural)]  = name_pl;
        names[size_t(Item_ref_type::a)]       = name_a;
    }

    Item_name()
    {
        for (size_t i = 0; i < size_t(Item_ref_type::END); i++)
        {
            names[i] = "";
        }
    }

    std::string names[size_t(Item_ref_type::END)];
};

struct Item_att_msgs
{
    Item_att_msgs() :
        player  (""),
        other   ("") {}

    Item_att_msgs(const std::string& player_, const std::string& other_) :
        player  (player_),
        other   (other_) {}

    std::string player, other;
};

#endif //CMN_H
