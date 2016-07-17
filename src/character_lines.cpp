#include "character_lines.hpp"

#include "game_time.hpp"
#include "colors.hpp"
#include "render.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "dungeon_master.hpp"
#include "mapgen.hpp"
#include "player_bon.hpp"
#include "inventory.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "text_format.hpp"
#include "item.hpp"
#include "item_device.hpp"

namespace character_lines
{

void draw()
{
    const int       x_wielded_default           = 43;
    const size_t    min_nr_steps_to_nxt_label   = 3;

    const Panel panel = Panel::char_lines;

    render::cover_panel(panel);

    Player& player = *map::player;

    //Hit points
    P p(0, 0);

    std::string str = "HP:";
    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    str = to_str(player.hp()) + "/" + to_str(player.hp_max(true));

    render::draw_text(str, panel, p, clr_red_lgt);

    //Spirit
    p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

    str = "SP:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    str = to_str(player.spi()) + "/" + to_str(player.spi_max());

    render::draw_text(str, panel, p, clr_magenta);

    //Insanity
    p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

    str = "Ins:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    const int shock = player.shock_tot();
    const int ins   = player.ins();

    const Clr short_san_clr =
        shock < 50  ? clr_green     :
        shock < 75  ? clr_yellow    :
        shock < 100 ? clr_magenta   : clr_red_lgt;

    str = to_str(shock) + "%/";

    render::draw_text(str, panel, p, short_san_clr);

    p.x += str.size();

    str = to_str(ins) + "%";

    render::draw_text(str, panel, p, clr_magenta);

    // Experience
    p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

    str = "Exp:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    str = to_str(dungeon_master::clvl());

    if (dungeon_master::clvl() < player_max_clvl)
    {
        //Not at maximum character level
        str += "(" + to_str(dungeon_master::xp_to_next_lvl()) + ")";
    }

    render::draw_text(str, panel, p, clr_white);

    //Wielded weapon
    p.x = std::max(x_wielded_default, int(p.x + str.size() + 1));

    int x_wielded = p.x;

    const Item* wpn = player.inv().item_in_slot(SlotId::wpn);

    if (!wpn)
    {
        wpn = &player.unarmed_wpn();
    }

    const Clr item_clr = wpn->clr();

    if (config::is_tiles_mode())
    {
        render::draw_tile(wpn->tile(), panel, p, item_clr);
    }
    else //Text mode
    {
        render::draw_glyph(wpn->glyph(), panel, p, item_clr);
    }

    p.x += 2;

    const auto& data = wpn->data();

    //If mainly a thrown weapon, force melee info - otherwise use weapon context.
    const ItemRefAttInf att_inf = data.main_att_mode == AttMode::thrown ?
                                     ItemRefAttInf::melee :
                                     ItemRefAttInf::wpn_context;

    str = wpn->name(ItemRefType::plain,
                    ItemRefInf::yes,
                    att_inf);

    text_format::first_to_upper(str);

    render::draw_text(str, panel, p, clr_white);

    //----------------------------------------------------------------------------- SECOND ROW
    ++p.y;
    p.x = 0;

    //Dungeon level
    str = "Dlvl:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    str = map::dlvl > 0 ? to_str(map::dlvl) : "-";

    render::draw_text(str, panel, p, clr_white);

    //Encumbrance
    p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

    str = "W:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    const int enc = player.enc_percent();

    str = to_str(enc) + "%";

    const Clr enc_clr = enc < 100 ? clr_white :
                        enc < enc_immobile_lvl ? clr_yellow : clr_red_lgt;

    render::draw_text(str, panel, p, enc_clr);

    //Armor
    const Item* const body_item = player.inv().item_in_slot(SlotId::body);

    if (body_item)
    {
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        const Clr clr = body_item->clr();

        if (config::is_tiles_mode())
        {
            const TileId tile = body_item->tile();

            render::draw_tile(tile, panel, p, clr);
        }
        else //Text mode
        {
            str = body_item->glyph();

            render::draw_text(str, panel, p, clr);
        }

        ++p.x;

        str = ":";

        render::draw_text(str, panel, p, clr_gray_drk);

        p.x += str.size();

        const Armor* const armor = static_cast<const Armor*>(body_item);

        str = armor->armor_points_str(false /* Do not include brackets */);

        render::draw_text(str, panel, p, clr_white);
    }

    //Lantern
    const Item* const lantern_item = player.inv().item_in_backpack(ItemId::lantern);

    if (lantern_item)
    {
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        const DeviceLantern* const lantern = static_cast<const DeviceLantern*>(lantern_item);

        if (config::is_tiles_mode())
        {
            const TileId tile = lantern_item->tile();

            render::draw_tile(tile, panel, p, clr_yellow);
        }
        else //Text mode
        {
            str = lantern_item->glyph();

            render::draw_text(str, panel, p, clr_yellow);
        }

        ++p.x;

        str = ":";

        render::draw_text(str, panel, p, clr_gray_drk);

        p.x += str.size();

        const Clr clr = lantern->is_activated_ ? clr_yellow : clr_white;

        str = lantern->is_activated_ ? "On" : "Off";

        str += "(" + to_str(lantern->nr_turns_left_) + ")";

        render::draw_text(str, panel, p, clr);
    }

    //Medical bag
    const Item* const medical_item = player.inv().item_in_backpack(ItemId::medical_bag);

    if (medical_item)
    {
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        const Clr clr = medical_item->clr();

        if (config::is_tiles_mode())
        {
            const TileId tile = medical_item->tile();

            render::draw_tile(tile, panel, p, clr);
        }
        else //Text mode
        {
            str = medical_item->glyph();

            render::draw_text(str, panel, p, clr);
        }

        ++p.x;

        str = ":";

        render::draw_text(str, panel, p, clr_gray_drk);

        p.x += str.size();

        const MedicalBag* const medical_bag = static_cast<const MedicalBag*>(medical_item);

        const int nr_suppl = medical_bag->nr_supplies_;

        str = to_str(nr_suppl);

        render::draw_text(str, panel, p, clr_white);
    }

    //Thrown item
    p.x = x_wielded;

    auto* const thr_item = player.inv().item_in_slot(SlotId::thrown);

    if (thr_item)
    {
        const Clr item_clr = thr_item->clr();

        if (config::is_tiles_mode())
        {
            render::draw_tile(thr_item->tile(), panel, p, item_clr);
        }
        else //Text mode
        {
            render::draw_glyph(thr_item->glyph(), panel, p, item_clr);
        }

        p.x += 2;

        str = thr_item->name(ItemRefType::plural,
                             ItemRefInf::yes,
                             ItemRefAttInf::thrown);

        text_format::first_to_upper(str);

        render::draw_text(str, panel, p, clr_white);
    }

    //----------------------------------------------------------------------------- THIRD ROW
    ++p.y;
    p.x = 0;

    std::vector<StrAndClr> props_line;

    player.prop_handler().props_interface_line(props_line);

    for (const StrAndClr& cur_prop_label : props_line)
    {
        render::draw_text(cur_prop_label.str, panel, p, cur_prop_label.clr);

        p.x += cur_prop_label.str.size() + 1;
    }

    //Turn number
    const int           turn        = game_time::turn();
    const std::string   turn_str    = to_str(turn);

    p.x = screen_w - turn_str.size() - 2; //"T" + ":"

    const bool is_free_step_turn = player.is_free_step_turn();

    const Clr turn_label_clr    = is_free_step_turn ? clr_black : clr_gray_drk;
    const Clr turn_label_bg_clr = is_free_step_turn ? clr_green : clr_black;

    render::draw_text("T", panel, p, turn_label_clr, turn_label_bg_clr);

    ++p.x;

    render::draw_text(":", panel, p, clr_gray_drk);

    ++p.x;

    render::draw_text(turn_str, panel, p, clr_white);
}

} //CharacterLines
