#ifndef CMN_HPP
#define CMN_HPP

#include <string>

#include "colors.hpp"
#include "art.hpp"

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------
const size_t player_name_max_len            = 14;

const std::string version_str               = "v18.2";

const std::string tiles_img_name            = "images/gfx_16x24.png";
const std::string logo_img_name             = "images/main_menu_logo.png";
const std::string skull_img_name            = "images/skull.png";

const int screen_bpp                        = 32;

const int map_w                             = 80;
const int map_h                             = 22;
const int map_w_half                        = map_w / 2;
const int map_h_half                        = map_h / 2;
const int nr_map_cells                      = map_w * map_h;

const int log_h                             = 2;
const int map_offset_h                      = log_h;
const int char_lines_offset_h               = map_offset_h + map_h;
const int char_lines_h                      = 3;
const int screen_w                          = map_w;
const int screen_h                          = char_lines_offset_h + char_lines_h;

const int descr_x0                          = map_w - 31;

const int player_start_x                    = 10;
const int player_start_y                    = map_h_half;

const int player_max_clvl                   = 12;

//NOTE:
//Early : dlvl 1  - 9
//Mid   : dlvl 10 - 19
//Late  : dlvl 20 - 30
const int dlvl_last_early_game              = 9;
const int dlvl_first_mid_game               = dlvl_last_early_game + 1;
const int dlvl_last_mid_game                = 19;
const int dlvl_first_late_game              = dlvl_last_mid_game + 1;
const int dlvl_last                         = 30;

const int min_dlvl_harder_traps             = 6;

const int audio_allocated_channels          = 16;

const size_t ms_delay_player_unable_act     = 7;
const size_t min_ms_between_same_sfx        = 60;

const int player_start_hp                   = 16;
const int player_start_spi                  = 6;
const int hp_per_lvl                        = 2;
const int spi_per_lvl                       = 1;

const int min_dmg_to_wound                  = 5;

const int xp_for_disarm_trap                 = 1;

const int     fov_std_radi_int              = 8;
const int     fov_std_w_int                 = (fov_std_radi_int * 2) + 1;
const double  fov_std_radi_db               = double(fov_std_radi_int);
const double  fov_std_w_db                  = double(fov_std_w_int);
const int     fov_max_radi_int              = fov_std_radi_int * 2;
const int     fov_max_w_int                 = (fov_max_radi_int * 2) + 1;
const double  fov_max_radi_db               = double(fov_max_radi_int);
const double  fov_max_w_db                  = double(fov_max_w_int);

const int snd_dist_normal                   = fov_std_radi_int;
const int snd_dist_loud                     = snd_dist_normal * 2;

const int throw_range_lmt                   = fov_std_radi_int + 3;

const int dynamite_fuse_turns               = 5;
const int expl_std_radi                = 2;

const int enc_immobile_lvl                  = 125;

const int nr_mg_projectiles                 = 5;
const int nr_cell_jumps_mg_projectiles      = 2;

//NOTE: Number of rolls is reduced by one for each step away from the explosion center
const int expl_dmg_rolls                    = 5;
const int expl_dmg_sides                    = 6;
const int expl_dmg_plus                     = 10;
const int expl_max_dmg                      = (expl_dmg_rolls* expl_dmg_sides) + expl_dmg_plus;

const int poison_dmg_n_turn                 = 3;

const int shock_from_obsession              = 30;

const int ins_from_disturbing_items         = 5;

//How many "units" of weight the player can carry, without trait modifiers etc
const int player_carry_weight_base          = 500;

//Value used for limiting spawning over time and "breeder" monsters. The actual number of actors
//may sometimes go a bit above this number, e.g. due to a group of monsters spawning when the
//number of actors is near the limit. Summoning spells does not check this number at all (because
//their effects should not be arbitrarily limited by this) - so that may also push the number of
//actors above the limit. This number is treated as a soft limit.
const size_t max_nr_actors_on_map           = 125;

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
//This is typically used for functions such as item identification and property
//applying to enable/disable printing to the message log, animating, or other
//such "side effects". For example when loading a saved game, we may want to do
//these things silently.
enum class Verbosity
{
    silent,
    verbose,
};

enum class UpdateScreen
{
    no,
    yes
};

enum class InvType
{
    slots,
    backpack
};

enum class DidAction
{
    yes,
    no
};

enum class PassTime
{
    yes,
    no
};

enum class ConsumeItem
{
    yes,
    no
};

enum class UnequipAllowed
{
    yes,
    no
};

enum class ItemRefType
{
    plain,
    a,
    plural,
    END
};

enum class ItemRefInf
{
    none,
    yes
};

enum class ItemRefAttInf
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

enum class LiquidType
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

enum class DmgType
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

enum class DmgMethod
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

enum class AttMode
{
    none,
    melee,
    thrown,
    ranged
};

enum class AllowWound
{
    no,
    yes
};

enum class ShockLvl
{
    none   = 0,
    mild   = 2,
    some   = 4,
    heavy  = 12
};

enum class GameEntryMode
{
    new_game,
    load_game
};

enum class SpawnRate
{
    never,
    extremely_rare,
    very_rare,
    rare,
    common,
    very_common
};

enum class VerDir
{
    up,
    down
};

enum class ActorState
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

enum class LgtSize
{
    none,
    small, //3x3
    fov
};

enum class MorePromptOnMsg
{
    no,
    yes
};

enum class InfScreenType
{
    scrolling,
    single_screen
};

struct StrAndClr
{
    StrAndClr() :
        str(""),
        clr(clr_black) {}

    StrAndClr(const std::string& text, const Clr& color) :
        str(text),
        clr(color) {}

    StrAndClr& operator=(const StrAndClr& other)
    {
        str = other.str;
        clr = other.clr;
        return *this;
    }

    std::string str;
    Clr clr;
};

enum class ItemType
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

struct ItemName
{
    ItemName(const std::string& name,
              const std::string& name_pl,
              const std::string& name_a)
    {
        names[size_t(ItemRefType::plain)]   = name;
        names[size_t(ItemRefType::plural)]  = name_pl;
        names[size_t(ItemRefType::a)]       = name_a;
    }

    ItemName()
    {
        for (size_t i = 0; i < size_t(ItemRefType::END); i++)
        {
            names[i] = "";
        }
    }

    std::string names[size_t(ItemRefType::END)];
};

struct ItemAttMsgs
{
    ItemAttMsgs() :
        player  (""),
        other   ("") {}

    ItemAttMsgs(const std::string& player_, const std::string& other_) :
        player  (player_),
        other   (other_) {}

    std::string player, other;
};

#endif //GLOBAL_HPP
