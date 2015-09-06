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

namespace character_lines
{

const int X_NAME    = 0;
const int X_HP      = PLAYER_NAME_MAX_LEN + 1;
const int X_SPI     = X_HP  + 8;
const int X_INS     = X_SPI + 8;

const int X_WIELDED = 44;

const int X_XP      = 0;
const int X_DLVL    = X_XP      + 12;
const int X_ENC     = X_DLVL    + 5;
const int X_ARM     = X_ENC     + 7;
const int X_LANTERN = X_ARM     + 5;
const int X_MEDICAL = X_LANTERN + 7;

const int X_THROWN  = X_WIELDED;

void draw()
{
    const Panel panel = Panel::char_lines;

    render::cover_panel(panel);

    Player& player = *map::player;

    Pos pos(0, 0);

    //Name
    pos.x = X_NAME;

    std::string str = player.name_a();

    render::draw_text(str, panel, pos, clr_white);

    //Health
    pos.x = X_HP;

    if (config::is_tiles_mode())
    {
        render::draw_tile(Tile_id::heart, panel, pos, clr_red_lgt);
    }
    else //Text mode
    {
        render::draw_text("H:", panel, pos, clr_menu_drk);
    }

    pos.x += 2;

    const std::string hp     = to_str(player.hp());
    const std::string hp_max = to_str(player.hp_max(true));

    str = hp + "/" + hp_max;

    render::draw_text(str, panel, pos, clr_red_lgt);

    //Spirit
    pos.x = X_SPI;

    if (config::is_tiles_mode())
    {
        render::draw_tile(Tile_id::elder_sign, panel, pos, clr_magenta);
    }
    else //Text mode
    {
        render::draw_text("S:", panel, pos, clr_menu_drk);
    }

    pos.x += 2;

    const std::string spi       = to_str(player.spi());
    const std::string spi_max   = to_str(player.spi_max());

    str = spi + "/" + spi_max;

    render::draw_text(str, panel, pos, clr_magenta);

    //Insanity
    pos.x = X_INS;

    if (config::is_tiles_mode())
    {
        render::draw_tile(Tile_id::brain, panel, pos, clr_blue_lgt);
    }
    else //Text mode
    {
        render::draw_text("I:", panel, pos, clr_menu_drk);
    }

    pos.x += 2;

//    render::draw_text(":", panel, pos, clr_menu_drk);

//    ++pos.x;

    const int SHOCK = player.shock_tot();
    const int INS = player.ins();

    const Clr short_san_clr =
        SHOCK < 50  ? clr_green_lgt :
        SHOCK < 75  ? clr_yellow   :
        SHOCK < 100 ? clr_magenta  : clr_red_lgt;

    str = to_str(SHOCK) + "%/";

    render::draw_text(str, panel, pos, short_san_clr);

    pos.x += str.length();
    str = to_str(INS) + "%";

    render::draw_text(str, panel, pos, clr_magenta);

    //Wielded weapon
    pos.x = X_WIELDED;

    Item* item_wielded = map::player->inv().item_in_slot(Slot_id::wpn);

    if (item_wielded)
    {
        const Clr item_clr = item_wielded->clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(item_wielded->tile(), panel, pos, item_clr);
        }
        else //Text mode
        {
            render::draw_glyph(item_wielded->glyph(), panel, pos, item_clr);
        }

        pos.x += 2;

        const auto& data = item_wielded->data();

        //If thrown weapon, force melee info - otherwise use weapon context.
        const Item_ref_att_inf att_inf = data.main_att_mode == Main_att_mode::thrown ?
                                         Item_ref_att_inf::melee : Item_ref_att_inf::wpn_context;

        str = item_wielded->name(Item_ref_type::plain, Item_ref_inf::yes, att_inf);

        text_format::first_to_upper(str);

        render::draw_text(str, panel, pos, clr_white);
        pos.x += str.length() + 1;
    }
    else //Not wielding a weapon
    {
        //TODO: Draw player unarmed weapon instead of "Unarmed"

        str = "Unarmed";
        render::draw_text(str, panel, pos, clr_gray);
        pos.x += str.length() + 1;
    }

    //----------------------------------------------------------------------------- SECOND ROW
    ++pos.y;

    // Level and xp
    pos.x = X_XP;

    str = "LVL:";

    render::draw_text(str, panel, pos, clr_menu_drk);

    pos.x += str.length();

    str = to_str(dungeon_master::clvl());

    if (dungeon_master::clvl() < PLAYER_MAX_CLVL)
    {
        //Not at maximum character level
        str += "(" + to_str(dungeon_master::xp_to_next_lvl()) + ")";
    }

    render::draw_text(str, panel, pos, clr_white);

    //Dungeon level
    pos.x = X_DLVL;

    if (config::is_tiles_mode())
    {
        render::draw_tile(Tile_id::stairs_down, panel, pos, clr_menu_drk);
    }
    else //Text mode
    {
        render::draw_text(">:", panel, pos, clr_menu_drk);
    }

    pos.x += 2;

    str = map::dlvl > 0 ? to_str(map::dlvl) : "-";

    render::draw_text(str, panel, pos, clr_white);

    //Armor
    const Item* const body_item = map::player->inv().item_in_slot(Slot_id::body);

    if (body_item)
    {
        pos.x = X_ARM;

        const Clr clr = body_item->clr();

        if (config::is_tiles_mode())
        {
            const Tile_id tile = body_item->tile();

            render::draw_tile(tile, panel, pos, clr);

            pos.x += 2;
        }
        else //Text mode
        {
            str = body_item->glyph();

            render::draw_text(str, panel, pos, clr);

            ++pos.x;

            render::draw_text(":", panel, pos, clr_menu_drk);

            ++pos.x;
        }

        const Armor* const armor = static_cast<const Armor*>(body_item);

        str = armor->armor_points_str(false /* Do not include brackets */);

        render::draw_text(str, panel, pos, clr_white);
    }

    //Encumbrance
    pos.x = X_ENC;

    str = "W:";

    render::draw_text(str, panel, pos, clr_menu_drk);

    pos.x += str.length();

    const int ENC = map::player->enc_percent();

    str = to_str(ENC) + "%";

    const Clr enc_clr = ENC < 100 ? clr_white_high :
                        ENC < ENC_IMMOBILE_LVL ? clr_yellow : clr_red_lgt;

    render::draw_text(str, panel, pos, enc_clr);

    //Lantern
    const Item* const lantern_item = map::player->inv().item_in_backpack(Item_id::lantern);

    if (lantern_item)
    {
        pos.x = X_LANTERN;

        const Device_lantern* const lantern = static_cast<const Device_lantern*>(lantern_item);

        if (config::is_tiles_mode())
        {
            const Tile_id tile = lantern_item->tile();

            render::draw_tile(tile, panel, pos, clr_yellow);

            pos.x += 2;
        }
        else //Text mode
        {
            str = lantern_item->glyph();

            render::draw_text(str, panel, pos, clr_yellow);

            ++pos.x;

            render::draw_text(":", panel, pos, clr_menu_drk);

            ++pos.x;
        }

        const Clr clr = lantern->is_activated_ ? clr_yellow : clr_menu_drk;

        str = lantern->is_activated_ ? "On" : "Off";

        render::draw_text(str, panel, pos, clr);
    }

    //Medical bag
    const Item* const medical_item = map::player->inv().item_in_backpack(Item_id::medical_bag);

    if (medical_item)
    {
        pos.x = X_MEDICAL;

        const Clr clr = medical_item->clr();

        if (config::is_tiles_mode())
        {
            const Tile_id tile = medical_item->tile();

            render::draw_tile(tile, panel, pos, clr);

            pos.x += 2;
        }
        else //Text mode
        {
            str = medical_item->glyph();

            render::draw_text(str, panel, pos, clr);

            ++pos.x;

            render::draw_text(":", panel, pos, clr_menu_drk);

            ++pos.x;
        }

        const Medical_bag* const medical_bag = static_cast<const Medical_bag*>(medical_item);

        const int NR_SUPPL = medical_bag->nr_supplies();

        str = to_str(NR_SUPPL);

        render::draw_text(str, panel, pos, clr_white_high);
    }

    //Thrown item
    pos.x = X_THROWN;

    auto* const thr_item = map::player->inv().item_in_slot(Slot_id::thrown);

    if (thr_item)
    {
        const Clr item_clr = thr_item->clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(thr_item->tile(), panel, pos, item_clr);
        }
        else //Text mode
        {
            render::draw_glyph(thr_item->glyph(), panel, pos, item_clr);
        }

        pos.x += 2;

        str = thr_item->name(Item_ref_type::plural, Item_ref_inf::yes,
                             Item_ref_att_inf::thrown);

        text_format::first_to_upper(str);

        render::draw_text(str, panel, pos, clr_white);
        pos.x += str.length() + 1;
    }
//    else //No item equiped for throwing
//    {
//        render::draw_text("No throwing item", panel, pos, clr_gray);
//    }

    //----------------------------------------------------------------------------- THIRD ROW
    ++pos.y;
    pos.x = 0;

    std::vector<Str_and_clr> props_line;

    map::player->prop_handler().props_interface_line(props_line);

    const int NR_PROPS = props_line.size();

    for (int i = 0; i < NR_PROPS; ++i)
    {
        const Str_and_clr& cur_prop_label = props_line[i];
        render::draw_text(cur_prop_label.str, panel, pos, cur_prop_label.clr);
        pos.x += cur_prop_label.str.length() + 1;
    }
}

} //Character_lines
