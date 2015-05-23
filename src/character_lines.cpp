#include "character_lines.hpp"

#include "cmn_types.hpp"
#include "game_time.hpp"
#include "colors.hpp"
#include "render.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "dungeon_master.hpp"
#include "map_gen.hpp"
#include "player_bon.hpp"
#include "inventory.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "text_format.hpp"
#include "item.hpp"
#include "item_device.hpp"
#include "utils.hpp"

using namespace std;

namespace character_lines
{

const int X_NAME    = 0;
const int X_HP      = PLAYER_NAME_MAX_LEN + 1;
const int X_SPI     = X_HP  + 8;
const int X_INS     = X_SPI + 8;

const int X_WIELDED = X_INS + 14;

const int X_XP      = 0;
const int X_DLVL    = X_XP      + 14;
const int X_ENC     = X_DLVL    + 8;
const int X_TURN    = X_ENC     + 9;

const int X_THROWN  = X_WIELDED;

void draw()
{
    render::cover_panel(Panel::char_lines);

    Player& player = *map::player;

    Pos pos(0, 0);

    //Name
    pos.x = X_NAME;
    string str = player.name_a();
    render::draw_text(str, Panel::char_lines, pos, clr_white);

    //Health
    pos.x = X_HP;
    const string hp     = to_str(player.hp());
    const string hp_max = to_str(player.hp_max(true));
    str = "H:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    str = hp + "/" + hp_max;
    render::draw_text(str, Panel::char_lines, pos, clr_red_lgt);

    //Spirit
    pos.x = X_SPI;
    const string spi        = to_str(player.spi());
    const string spi_max    = to_str(player.spi_max());
    str = "S:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    str = spi + "/" + spi_max;
    render::draw_text(str, Panel::char_lines, pos, clr_blue_lgt);

    //Insanity
    pos.x = X_INS;
    const int SHOCK = player.shock_tot();
    const int INS = player.ins();
    str = "INS:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    const Clr short_san_clr =
        SHOCK < 50  ? clr_green_lgt :
        SHOCK < 75  ? clr_yellow   :
        SHOCK < 100 ? clr_magenta  : clr_red_lgt;
    str = to_str(SHOCK) + "%/";
    render::draw_text(str, Panel::char_lines, pos, short_san_clr);
    pos.x += str.length();
    str = to_str(INS) + "%";
    render::draw_text(str, Panel::char_lines, pos, clr_magenta);

    //Wielded weapon
    pos.x = X_WIELDED;

    Item* item_wielded = map::player->inv().item_in_slot(Slot_id::wielded);

    if (item_wielded)
    {
        const Clr item_clr = item_wielded->clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(item_wielded->tile(), Panel::char_lines, pos, item_clr);
        }
        else //ASCII mode
        {
            render::draw_glyph(item_wielded->glyph(), Panel::char_lines, pos, item_clr);
        }

        pos.x += 2;

        const auto& data = item_wielded->data();

        //If thrown weapon, force melee info - otherwise use weapon context.
        const Item_ref_att_inf att_inf = data.main_att_mode == Main_att_mode::thrown ?
                                         Item_ref_att_inf::melee : Item_ref_att_inf::wpn_context;

        str = item_wielded->name(Item_ref_type::plain, Item_ref_inf::yes, att_inf);

        text_format::first_to_upper(str);

        render::draw_text(str, Panel::char_lines, pos, clr_white);
        pos.x += str.length() + 1;
    }
    else
    {
        str = "Unarmed";
        render::draw_text(str, Panel::char_lines, pos, clr_gray);
        pos.x += str.length() + 1;
    }

    //----------------------------------------------------------------------------- SECOND ROW
    ++pos.y;

    // Level and xp
    pos.x = X_XP;
    str = "XP:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    str = to_str(dungeon_master::clvl());
    if (dungeon_master::clvl() < PLAYER_MAX_CLVL)
    {
        //Not at maximum character level
        str += "(" + to_str(dungeon_master::xp_to_next_lvl()) + ")";
    }
    render::draw_text(str, Panel::char_lines, pos, clr_white);

    //Dungeon level
    pos.x = X_DLVL;
    str = "DLVL:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    str = map::dlvl > 0 ? to_str(map::dlvl) : "-";
    render::draw_text(str, Panel::char_lines, pos, clr_white);

    //Turn number
    pos.x = X_TURN;
    str = "T:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    str = to_str(game_time::turn());
    render::draw_text(str, Panel::char_lines, pos, clr_white);

    //Encumbrance
    pos.x = X_ENC;
    str = "ENC:";
    render::draw_text(str, Panel::char_lines, pos, clr_menu_drk);
    pos.x += str.length();
    const int ENC = map::player->enc_percent();
    str = to_str(ENC) + "%";
    const Clr enc_clr = ENC < 100 ? clr_green_lgt :
                        ENC < ENC_IMMOBILE_LVL ? clr_yellow : clr_red_lgt;
    render::draw_text(str, Panel::char_lines, pos, enc_clr);

    //Thrown weapon
    pos.x = X_THROWN;

    auto* const item_missiles = map::player->inv().item_in_slot(Slot_id::thrown);

    if (item_missiles)
    {
        const Clr item_clr = item_missiles->clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(item_missiles->tile(), Panel::char_lines, pos, item_clr);
        }
        else
        {
            render::draw_glyph(item_missiles->glyph(), Panel::char_lines, pos, item_clr);
        }

        pos.x += 2;

        str = item_missiles->name(Item_ref_type::plural, Item_ref_inf::yes,
                                  Item_ref_att_inf::thrown);

        text_format::first_to_upper(str);

        render::draw_text(str, Panel::char_lines, pos, clr_white);
        pos.x += str.length() + 1;
    }
    else
    {
        render::draw_text("No thrown weapon", Panel::char_lines, pos, clr_gray);
    }

    //----------------------------------------------------------------------------- THIRD ROW
    ++pos.y;
    pos.x = 0;

    vector<Str_and_clr> props_line;
    map::player->prop_handler().props_interface_line(props_line);
    const int NR_PROPS = props_line.size();

    for (int i = 0; i < NR_PROPS; ++i)
    {
        const Str_and_clr& cur_prop_label = props_line[i];
        render::draw_text(cur_prop_label.str, Panel::char_lines, pos, cur_prop_label.clr);
        pos.x += cur_prop_label.str.length() + 1;
    }
}

} //Character_lines
