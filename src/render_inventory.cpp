#include "render_inventory.hpp"

#include <string>
#include <iostream>

#include "init.hpp"
#include "actor_player.hpp"
#include "log.hpp"
#include "render.hpp"
#include "map.hpp"
#include "item.hpp"
#include "text_format.hpp"

using namespace std;

namespace
{

void draw_item_symbol(const Item& item, const Pos& p)
{
    const Clr item_clr = item.get_clr();
    if (Config::is_tiles_mode())
    {
        Render::draw_tile(item.get_tile(), Panel::screen, p, item_clr);
    }
    else
    {
        Render::draw_glyph(item.get_glyph(), Panel::screen, p, item_clr);
    }
}

void draw_weight_pct(const int Y, const int ITEM_NAME_X, const size_t ITEM_NAME_LEN,
                   const Item& item, const Clr& item_name_clr, const bool IS_SELECTED)
{
    const int WEIGHT_CARRIED_TOT = Map::player->get_inv().get_total_item_weight();
    const int WEIGHT_PCT         = (item.get_weight() * 100) / WEIGHT_CARRIED_TOT;

    if (WEIGHT_PCT > 0 && WEIGHT_PCT < 100)
    {
        const string  weight_str = to_str(WEIGHT_PCT) + "%";
        const int     WEIGHT_X = DESCR_X0 - weight_str.size() - 1;
        const Pos     weight_pos(WEIGHT_X, Y);
        const Clr     weight_clr = IS_SELECTED ? clr_white : clr_gray_drk;
        Render::draw_text(weight_str, Panel::screen, weight_pos, weight_clr);

        const int DOTS_X  = ITEM_NAME_X + ITEM_NAME_LEN;
        const int DOTS_W  = WEIGHT_X - DOTS_X;
        const string dots_str(DOTS_W, '.');
        Clr dots_clr       = IS_SELECTED ? clr_white : item_name_clr;
        if (!IS_SELECTED) {dots_clr.r /= 2; dots_clr.g /= 2; dots_clr.b /= 2;}
        Render::draw_text(dots_str, Panel::screen, Pos(DOTS_X, Y), dots_clr);
    }
}

void draw_detailed_item_descr(const Item* const item)
{
    if (item)
    {
        vector<Str_and_clr> lines;

        const auto base_descr = item->get_descr();

        if (!base_descr.empty())
        {
            for (const string& paragraph : base_descr)
            {
                lines.push_back({paragraph, clr_white_high});
            }
        }

        const bool  IS_PLURAL = item->nr_items_ > 1 && item->get_data().is_stackable;
        const string weight_str =
            (IS_PLURAL ? "They are " : "It is ") + item->get_weight_str() + " to carry.";

        lines.push_back({weight_str, clr_green});

        const int WEIGHT_CARRIED_TOT = Map::player->get_inv().get_total_item_weight();
        const int WEIGHT_PCT         = (item->get_weight() * 100) / WEIGHT_CARRIED_TOT;

        if (WEIGHT_PCT > 0 && WEIGHT_PCT < 100)
        {
            const string pct_str = "(" + to_str(WEIGHT_PCT) + "% of total carried weight)";
            lines.push_back({pct_str, clr_green});
        }

        Render::draw_descr_box(lines);
    }
}

} //Namespace

namespace render_inventory
{

void draw_browse_inv(const Menu_browser& browser)
{

    Render::clear_screen();

    const int     BROWSER_Y   = browser.get_y();
    const auto&   inv         = Map::player->get_inv();
    const size_t  NR_SLOTS    = size_t(Slot_id::END);

    const bool    IS_IN_EQP   = BROWSER_Y < int(NR_SLOTS);
    const size_t  INV_ELEMENT = IS_IN_EQP ? 0 : (size_t(BROWSER_Y) - NR_SLOTS);

    const auto* const item    = IS_IN_EQP ?
                                inv.slots_[BROWSER_Y].item :
                                inv.general_[INV_ELEMENT];

    const string query_eq_str   = item ? "unequip" : "equip";
    const string query_base_str = "[enter] to " + (IS_IN_EQP ? query_eq_str : "apply item");

    const string query_drop_str = item ? " [shift+enter] to drop" : "";

    string str                = query_base_str + query_drop_str + " [space/esc] to exit";

    Render::draw_text(str, Panel::screen, Pos(0, 0), clr_white_high);

    Pos p(1, EQP_Y0);

    const Panel panel = Panel::screen;

    for (size_t i = 0; i < NR_SLOTS; ++i)
    {
        const bool IS_CUR_POS = IS_IN_EQP && BROWSER_Y == int(i);
        const Inv_slot& slot   = inv.slots_[i];
        const string slot_name = slot.name;

        p.x = 1;

        Render::draw_text(slot_name, panel, p, IS_CUR_POS ? clr_white_high : clr_menu_drk);

        p.x += 9; //Offset to leave room for slot label

        const auto* const cur_item = slot.item;

        if (cur_item)
        {
            draw_item_symbol(*cur_item, p);
            p.x += 2;

            const Clr clr = IS_CUR_POS ? clr_white_high : cur_item->get_interface_clr();

            const Item_data_t& d    = cur_item->get_data();
            Item_ref_att_inf att_inf  = Item_ref_att_inf::none;
            if (slot.id == Slot_id::wielded || slot.id == Slot_id::wielded_alt)
            {
                //Thrown weapons are forced to show melee info instead
                att_inf = d.main_att_mode == Main_att_mode::thrown ? Item_ref_att_inf::melee :
                         Item_ref_att_inf::wpn_context;
            }
            else if (slot.id == Slot_id::thrown)
            {
                att_inf = Item_ref_att_inf::thrown;
            }

            Item_ref_type ref_type = Item_ref_type::plain;

            if (slot.id == Slot_id::thrown) {ref_type = Item_ref_type::plural;}

            string item_name = cur_item->get_name(ref_type, Item_ref_inf::yes, att_inf);

            Text_format::first_to_upper(item_name);

            Render::draw_text(item_name, panel, p, clr);

            draw_weight_pct(p.y, p.x, item_name.size(), *cur_item, clr, IS_CUR_POS);
        }
        else
        {
            p.x += 2;
            Render::draw_text("<empty>", panel, p, IS_CUR_POS ? clr_white_high : clr_menu_drk);
        }

        ++p.y;
    }


    const size_t  NR_INV_ITEMS  = inv.general_.size();

    size_t inv_top_idx = 0;

    if (!IS_IN_EQP && NR_INV_ITEMS > 0)
    {

        auto is_browser_pos_on_scr = [&](const bool IS_FIRST_SCR)
        {
            const int MORE_LABEL_H = IS_FIRST_SCR ? 1 : 2;
            return int(INV_ELEMENT) < (int(inv_top_idx + INV_H) - MORE_LABEL_H);
        };

        if (int(NR_INV_ITEMS) > INV_H && !is_browser_pos_on_scr(true))
        {

            inv_top_idx = INV_H - 1;

            while (true)
            {
                //Check if this is the bottom screen
                if (int(NR_INV_ITEMS - inv_top_idx) + 1 <= INV_H) {break;}

                //Check if current browser pos is currently on screen
                if (is_browser_pos_on_scr(false)) {break;}

                inv_top_idx += INV_H - 2;
            }
        }
    }

    const int INV_X0  = 1;

    p = Pos(INV_X0, INV_Y0);

    const int INV_ITEM_NAME_X = INV_X0 + 2;

    for (size_t i = inv_top_idx; i < NR_INV_ITEMS; ++i)
    {
        const bool IS_CUR_POS = !IS_IN_EQP && INV_ELEMENT == i;
        Item* const cur_item   = inv.general_[i];

        const Clr clr = IS_CUR_POS ? clr_white_high : cur_item->get_interface_clr();

        if (i == inv_top_idx && inv_top_idx > 0)
        {
            p.x = INV_ITEM_NAME_X;
            Render::draw_text("(more)", panel, p, clr_black, clr_gray);
            ++p.y;
        }

        p.x = INV_X0;

        draw_item_symbol(*cur_item, p);

        p.x = INV_ITEM_NAME_X;

        string item_name = cur_item->get_name(Item_ref_type::plural, Item_ref_inf::yes,
                                           Item_ref_att_inf::wpn_context);

        Text_format::first_to_upper(item_name);

        Render::draw_text(item_name, panel, p, clr);

        draw_weight_pct(p.y, INV_ITEM_NAME_X, item_name.size(), *cur_item, clr, IS_CUR_POS);

        ++p.y;

        if (p.y == INV_Y1 && ((i + 1) < (NR_INV_ITEMS - 1)))
        {
            Render::draw_text("(more)", panel, p, clr_black, clr_gray);
            break;
        }
    }

//  Render::draw_popup_box(eqp_rect, panel, clr_popup_box, false);

    const Rect eqp_rect(0, EQP_Y0 - 1, DESCR_X0 - 1, EQP_Y1 + 1);
    const Rect inv_rect(0, INV_Y0 - 1, DESCR_X0 - 1, INV_Y1 + 1);

    Render::draw_popup_box(eqp_rect, panel, clr_popup_box, false);
    Render::draw_popup_box(inv_rect, panel, clr_popup_box, false);

    if (Config::is_tiles_mode())
    {
        Render::draw_tile(Tile_id::popup_ver_r, panel, inv_rect.p0, clr_popup_box);
        Render::draw_tile(Tile_id::popup_ver_l, panel, Pos(inv_rect.p1.x, inv_rect.p0.y),
                         clr_popup_box);
    }

    draw_detailed_item_descr(item);

    Render::update_screen();
}

void draw_equip(const Menu_browser& browser, const Slot_id slot_id_to_equip,
               const vector<size_t>& gen_inv_indexes)
{
    assert(slot_id_to_equip != Slot_id::END);

    Pos p(0, 0);

    const int NR_ITEMS = browser.get_nr_of_items_in_first_list();
    Render::cover_area(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

    const bool HAS_ITEM = !gen_inv_indexes.empty();

    string str = "";
    switch (slot_id_to_equip)
    {
    case Slot_id::wielded:
        str = HAS_ITEM ?
              "Wield which item?" :
              "I carry no weapon to wield.";
        break;

    case Slot_id::wielded_alt:
        str = HAS_ITEM ?
              "Prepare which weapon?" :
              "I carry no weapon to wield.";
        break;

    case Slot_id::thrown:
        str = HAS_ITEM ?
              "Use which item as thrown weapon?" :
              "I carry no weapon to throw." ;
        break;

    case Slot_id::body:
        str = HAS_ITEM ?
              "Wear which armor?" :
              "I carry no armor.";
        break;

    case Slot_id::head:
        str = HAS_ITEM ?
              "Wear what on head?" :
              "I carry no headwear.";
        break;

    case Slot_id::neck:
        str = HAS_ITEM ?
              "Wear what around the neck?" :
              "I carry nothing to wear around the neck.";
        break;

    case Slot_id::ring1:
    case Slot_id::ring2:
        str = HAS_ITEM ?
              "Wear what ring?" :
              "I carry no ring.";
        break;

    case Slot_id::END: {}
        break;
    }

    if (HAS_ITEM)
    {
        str += " [shift+enter] to drop";
    }

    str += cancel_info_str;

    Render::draw_text(str, Panel::screen, p, clr_white_high);

    ++p.y;

    Inventory& inv = Map::player->get_inv();

    const int NR_INDEXES = gen_inv_indexes.size();

    for (int i = 0; i < NR_INDEXES; ++i)
    {
        const bool IS_CUR_POS = browser.get_pos().y == int(i);
        p.x = 0;

        Item* const item = inv.general_[gen_inv_indexes[i]];

        draw_item_symbol(*item, p);
        p.x += 2;

        const Clr item_interf_clr = IS_CUR_POS ? clr_white_high : item->get_interface_clr();

        const Item_data_t& d    = item->get_data();
        Item_ref_att_inf att_inf  = Item_ref_att_inf::none;
        if (slot_id_to_equip == Slot_id::wielded || slot_id_to_equip == Slot_id::wielded_alt)
        {
            //Thrown weapons are forced to show melee info instead
            att_inf = d.main_att_mode == Main_att_mode::thrown ? Item_ref_att_inf::melee :
                     Item_ref_att_inf::wpn_context;
        }
        else if (slot_id_to_equip == Slot_id::thrown)
        {
            att_inf = Item_ref_att_inf::thrown;
        }

        str = item->get_name(Item_ref_type::plural, Item_ref_inf::yes, att_inf);

        Text_format::first_to_upper(str);

        Render::draw_text(str, Panel::screen, p, item_interf_clr);
        ++p.y;
    }

    Render::update_screen();
}

} //Render_inventory
