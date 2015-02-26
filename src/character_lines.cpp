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

void draw()
{
    render::cover_panel(Panel::char_lines);

    const int CHARACTER_LINE_X0 = 0;
    const int CHARACTER_LINE_Y0 = 0;

    Pos pos(CHARACTER_LINE_X0, CHARACTER_LINE_Y0);

    Player& player = *map::player;

    //Health
    const string hp = to_str(player.get_hp());
    const string hp_max = to_str(player.get_hp_max(true));
    render::draw_text("HP:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 3;
    string str = hp + "/" + hp_max;
    render::draw_text(str, Panel::char_lines, pos, clr_red_lgt);
    pos.x += str.length() + 1;

    //Spirit
    const string spi    = to_str(player.get_spi());
    const string spi_max = to_str(player.get_spi_max());
    render::draw_text("SPI:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 4;
    str = spi + "/" + spi_max;
    render::draw_text(str, Panel::char_lines, pos, clr_blue_lgt);
    pos.x += str.length() + 1;

    //Sanity
    const int SHOCK = player.get_shock_total();
    const int INS = player.get_insanity();
    render::draw_text("INS:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 4;
    const Clr short_san_clr =
        SHOCK < 50  ? clr_green_lgt :
        SHOCK < 75  ? clr_yellow   :
        SHOCK < 100 ? clr_magenta  : clr_red_lgt;
    str = to_str(SHOCK) + "%/";
    render::draw_text(str, Panel::char_lines, pos, short_san_clr);
    pos.x += str.length();
    str = to_str(INS) + "%";
    render::draw_text(str, Panel::char_lines, pos, clr_magenta);
    pos.x += str.length() + 1;

    //Armor
    render::draw_text("ARM:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 4;
    const Item* const armor = player.get_inv().get_item_in_slot(Slot_id::body);

    if (armor)
    {
        str = static_cast<const Armor*>(armor)->get_armor_data_line(false);
        render::draw_text(str, Panel::char_lines, pos, clr_white);
        pos.x += str.length() + 1;
    }
    else
    {
        render::draw_text("N/A", Panel::char_lines, pos, clr_white);
        pos.x += 4;
    }

    //Electric lantern
    render::draw_text("L:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 2;
    str                 = "";
    Clr lantern_info_clr  = clr_white;

    for (const Item* const item : map::player->get_inv().general_)
    {
        if (item->get_id() == Item_id::electric_lantern)
        {
            const Device_lantern* const lantern = static_cast<const Device_lantern*>(item);
            str = to_str(lantern->nr_turns_left_);

            if (lantern->is_activated_)
            {
                lantern_info_clr  = clr_yellow;
            }

            break;
        }
    }

    if (str == "") {str = "N/A";}

    render::draw_text(str, Panel::char_lines, pos, lantern_info_clr);
    pos.x += str.length() + 1;

    //Wielded weapon
    pos.x += 1;
    const int X_POS_MISSILE = pos.x;

    Item* item_wielded = map::player->get_inv().get_item_in_slot(Slot_id::wielded);

    if (item_wielded)
    {
        const Clr item_clr = item_wielded->get_clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(
                item_wielded->get_tile(), Panel::char_lines, pos, item_clr);
        }
        else
        {
            render::draw_glyph(
                item_wielded->get_glyph(), Panel::char_lines, pos, item_clr);
        }

        pos.x += 2;

        const auto& data = item_wielded->get_data();
        //If thrown weapon, force melee info - otherwise use weapon context.
        const Item_ref_att_inf att_inf = data.main_att_mode == Main_att_mode::thrown ?
                                         Item_ref_att_inf::melee : Item_ref_att_inf::wpn_context;

        str = item_wielded->get_name(Item_ref_type::plain, Item_ref_inf::yes, att_inf);
        render::draw_text(str, Panel::char_lines, pos, clr_white);
        pos.x += str.length() + 1;
    }
    else
    {
        render::draw_text("Unarmed", Panel::char_lines, pos, clr_white);
    }

    pos.x = CHARACTER_LINE_X0;
    pos.y += 1;

    // Level and xp
    render::draw_text("LVL:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 4;
    str = to_str(dungeon_master::get_cLvl());
    render::draw_text(str, Panel::char_lines, pos, clr_white);
    pos.x += str.length() + 1;
    render::draw_text("NXT:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 4;
    str = dungeon_master::get_cLvl() >= PLAYER_MAX_CLVL ? "-" :
          to_str(dungeon_master::get_xp_to_next_lvl());
    render::draw_text(str, Panel::char_lines, pos, clr_white);
    pos.x += str.length() + 1;

    //Dungeon level
    render::draw_text("DLVL:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 5;
    str = map::dlvl > 0 ? to_str(map::dlvl) : "-";
    render::draw_text(str, Panel::char_lines, pos, clr_white);
    pos.x += str.length() + 1;

    //Turn number
    render::draw_text("T:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 2;
    str = to_str(game_time::get_turn());
    render::draw_text(str, Panel::char_lines, pos, clr_white);
    pos.x += str.length() + 1;

    //Encumbrance
    render::draw_text("ENC:", Panel::char_lines, pos, clr_menu_drk);
    pos.x += 4;
    const int ENC = map::player->get_enc_percent();
    str = to_str(ENC) + "%";
    const Clr enc_clr = ENC < 100 ? clr_green_lgt :
                        ENC < ENC_IMMOBILE_LVL ? clr_yellow : clr_red_lgt;
    render::draw_text(str, Panel::char_lines, pos, enc_clr);
    pos.x += str.length() + 1;

    //Thrown weapon
    pos.x = X_POS_MISSILE;

    auto* const item_missiles =
        map::player->get_inv().get_item_in_slot(Slot_id::thrown);

    if (item_missiles)
    {
        const Clr item_clr = item_missiles->get_clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(item_missiles->get_tile(), Panel::char_lines, pos, item_clr);
        }
        else
        {
            render::draw_glyph(item_missiles->get_glyph(), Panel::char_lines, pos, item_clr);
        }

        pos.x += 2;

        str = item_missiles->get_name(Item_ref_type::plural, Item_ref_inf::yes,
                                      Item_ref_att_inf::thrown);
        render::draw_text(str, Panel::char_lines, pos, clr_white);
        pos.x += str.length() + 1;
    }
    else
    {
        render::draw_text("No thrown weapon", Panel::char_lines, pos, clr_white);
    }

    pos.y += 1;
    pos.x = CHARACTER_LINE_X0;

    vector<Str_and_clr> props_line;
    map::player->get_prop_handler().get_props_interface_line(props_line);
    const int NR_PROPS = props_line.size();

    for (int i = 0; i < NR_PROPS; ++i)
    {
        const Str_and_clr& cur_prop_label = props_line[i];
        render::draw_text(cur_prop_label.str, Panel::char_lines, pos, cur_prop_label.clr);
        pos.x += cur_prop_label.str.length() + 1;
    }
}

} //Character_lines
