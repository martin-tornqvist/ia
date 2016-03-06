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

void draw()
{
    const int       X_WIELDED_DEFAULT           = 43;
    const size_t    MIN_NR_STEPS_TO_NXT_LABEL   = 3;

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
    p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

    str = "SP:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    str = to_str(player.spi()) + "/" + to_str(player.spi_max());

    render::draw_text(str, panel, p, clr_magenta);

    //Insanity
    p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

    str = "Ins:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    const int SHOCK = player.shock_tot();
    const int INS   = player.ins();

    const Clr short_san_clr =
        SHOCK < 50  ? clr_green     :
        SHOCK < 75  ? clr_yellow    :
        SHOCK < 100 ? clr_magenta   : clr_red_lgt;

    str = to_str(SHOCK) + "%/";

    render::draw_text(str, panel, p, short_san_clr);

    p.x += str.size();

    str = to_str(INS) + "%";

    render::draw_text(str, panel, p, clr_magenta);

    // Experience
    p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

    str = "Exp:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    str = to_str(dungeon_master::clvl());

    if (dungeon_master::clvl() < PLAYER_MAX_CLVL)
    {
        //Not at maximum character level
        str += "(" + to_str(dungeon_master::xp_to_next_lvl()) + ")";
    }

    render::draw_text(str, panel, p, clr_white);

    //Wielded weapon
    p.x = std::max(X_WIELDED_DEFAULT, int(p.x + str.size() + 1));

    int x_wielded = p.x;

    const Item* wpn = player.inv().item_in_slot(Slot_id::wpn);

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
    const Item_ref_att_inf att_inf = data.main_att_mode == Att_mode::thrown ?
                                     Item_ref_att_inf::melee :
                                     Item_ref_att_inf::wpn_context;

    str = wpn->name(Item_ref_type::plain,
                    Item_ref_inf::yes,
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
    p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

    str = "W:";

    render::draw_text(str, panel, p, clr_gray_drk);

    p.x += str.size();

    const int ENC = player.enc_percent();

    str = to_str(ENC) + "%";

    const Clr enc_clr = ENC < 100 ? clr_white :
                        ENC < ENC_IMMOBILE_LVL ? clr_yellow : clr_red_lgt;

    render::draw_text(str, panel, p, enc_clr);

    //Armor
    const Item* const body_item = player.inv().item_in_slot(Slot_id::body);

    if (body_item)
    {
        p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

        const Clr clr = body_item->clr();

        if (config::is_tiles_mode())
        {
            const Tile_id tile = body_item->tile();

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
    const Item* const lantern_item = player.inv().item_in_backpack(Item_id::lantern);

    if (lantern_item)
    {
        p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

        const Device_lantern* const lantern = static_cast<const Device_lantern*>(lantern_item);

        if (config::is_tiles_mode())
        {
            const Tile_id tile = lantern_item->tile();

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
    const Item* const medical_item = player.inv().item_in_backpack(Item_id::medical_bag);

    if (medical_item)
    {
        p.x += std::max(str.size() + 1, MIN_NR_STEPS_TO_NXT_LABEL);

        const Clr clr = medical_item->clr();

        if (config::is_tiles_mode())
        {
            const Tile_id tile = medical_item->tile();

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

        const Medical_bag* const medical_bag = static_cast<const Medical_bag*>(medical_item);

        const int NR_SUPPL = medical_bag->nr_supplies();

        str = to_str(NR_SUPPL);

        render::draw_text(str, panel, p, clr_white);
    }

    //Thrown item
    p.x = x_wielded;

    auto* const thr_item = player.inv().item_in_slot(Slot_id::thrown);

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

        str = thr_item->name(Item_ref_type::plural,
                             Item_ref_inf::yes,
                             Item_ref_att_inf::thrown);

        text_format::first_to_upper(str);

        render::draw_text(str, panel, p, clr_white);
    }

    //----------------------------------------------------------------------------- THIRD ROW
    ++p.y;
    p.x = 0;

    std::vector<Str_and_clr> props_line;

    player.prop_handler().props_interface_line(props_line);

    for (const Str_and_clr& cur_prop_label : props_line)
    {
        render::draw_text(cur_prop_label.str, panel, p, cur_prop_label.clr);

        p.x += cur_prop_label.str.size() + 1;
    }

    //Turn number
    const int           TURN        = game_time::turn();
    const std::string   turn_str    = to_str(TURN);

    p.x = SCREEN_W - turn_str.size() - 2; //"T" + ":"

    const bool IS_FREE_STEP_TURN = player.is_free_step_turn();

    const Clr turn_label_clr    = IS_FREE_STEP_TURN ? clr_black : clr_gray_drk;
    const Clr turn_label_bg_clr = IS_FREE_STEP_TURN ? clr_green : clr_black;

    render::draw_text("T", panel, p, turn_label_clr, turn_label_bg_clr);

    ++p.x;

    render::draw_text(":", panel, p, clr_gray_drk);

    ++p.x;

    render::draw_text(turn_str, panel, p, clr_white);
}

} //Character_lines
