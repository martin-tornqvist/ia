#include "render_inventory.hpp"

#include <string>
#include <iostream>

#include "init.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "render.hpp"
#include "map.hpp"
#include "item.hpp"
#include "text_format.hpp"
#include "menu_input.hpp"

namespace render_inv
{

namespace
{

const int TOP_MORE_Y    = 0;
const int BTM_MORE_Y    = SCREEN_H - 1;
const int INV_Y0        = TOP_MORE_Y + 1;
const int INV_Y1        = BTM_MORE_Y - 1;

void draw_item_symbol(const Item& item, const P& p)
{
    const Clr item_clr = item.clr();

    if (config::is_tiles_mode())
    {
        render::draw_tile(item.tile(), Panel::screen, p, item_clr);
    }
    else //Text mode
    {
        render::draw_glyph(item.glyph(), Panel::screen, p, item_clr);
    }
}

void draw_weight_pct_and_dots(const P item_pos,
                              const size_t ITEM_NAME_LEN,
                              const Item& item,
                              const Clr& item_name_clr,
                              const bool IS_MARKED)
{
    const int WEIGHT_CARRIED_TOT = map::player->inv().total_item_weight();

    int item_weight_pct = 0;

    if (WEIGHT_CARRIED_TOT > 0)
    {
        item_weight_pct = (item.weight() * 100) / WEIGHT_CARRIED_TOT;
    }

    std::string weight_str  = to_str(item_weight_pct) + "%";
    int         weight_x    = DESCR_X0 - 1 - weight_str.size();

    IA_ASSERT(item_weight_pct >= 0 && item_weight_pct <= 100);

    if (item_weight_pct > 0 && item_weight_pct < 100)
    {
        const P weight_pos(weight_x, item_pos.y);

        const Clr weight_clr = IS_MARKED ? clr_white : clr_gray_drk;

        render::draw_text(weight_str,
                          Panel::screen,
                          weight_pos,
                          weight_clr);
    }
    else //"Zero" weight, or 100% of weight - no weight percent should be displayed
    {
        weight_str  = "";
        weight_x    = DESCR_X0 - 1;
    }

    int dots_x = item_pos.x + ITEM_NAME_LEN;
    int dots_w = weight_x - dots_x;

    if (dots_w == 0)
    {
        //Item name fits exactly, just skip drawing dots
        return;
    }

    if (dots_w < 0)
    {
        //Item name does not fit at all, draw a few dots up until the weight percentage
        dots_w              = 3;
        const int DOTS_X1   = weight_x - 1;
        dots_x              = DOTS_X1 - dots_w + 1;
    }

    const std::string   dots_str(dots_w, '.');
    Clr                 dots_clr = IS_MARKED ? clr_white : item_name_clr;

    if (!IS_MARKED)
    {
        dots_clr.r /= 2;
        dots_clr.g /= 2;
        dots_clr.b /= 2;
    }

    render::draw_text(dots_str,
                      Panel::screen,
                      P(dots_x, item_pos.y),
                      dots_clr);
}

void draw_detailed_item_descr(const Item* const item)
{
    std::vector<Str_and_clr> lines;

    if (item)
    {
        const auto base_descr = item->descr();

        if (!base_descr.empty())
        {
            for (const std::string& paragraph : base_descr)
            {
                lines.push_back({paragraph, clr_white_high});
            }
        }

        const bool IS_PLURAL = item->nr_items_ > 1 && item->data().is_stackable;

        const std::string ref_str = IS_PLURAL ? "They are " : "It is ";

        const Item_data_t& d = item->data();

        std::string disturb_str = "";

        const std::string disturb_base_str = ref_str + "a burden on my mind to ";

        if (d.is_ins_raied_while_carried)
        {
            disturb_str = disturb_base_str + "carry";
        }
        else if (d.is_ins_raied_while_equiped)
        {
            if (d.type == Item_type::melee_wpn || d.type == Item_type::ranged_wpn)
            {
                disturb_str = disturb_base_str + "wield.";
            }
            else //Not a wieldable item
            {
                disturb_str = disturb_base_str + "wear.";
            }
        }

        if (!disturb_str.empty())
        {
            disturb_str += " (+" + to_str(INS_FROM_DISTURBING_ITEMS) + "% insanity)";

            lines.push_back({disturb_str, clr_magenta});
        }

        const std::string weight_str = ref_str + item->weight_str() + " to carry.";

        lines.push_back({weight_str, clr_green});

        const int WEIGHT_CARRIED_TOT = map::player->inv().total_item_weight();

        int weight_pct = 0;

        if (WEIGHT_CARRIED_TOT > 0)
        {
            weight_pct = (item->weight() * 100) / WEIGHT_CARRIED_TOT;
        }

        IA_ASSERT(weight_pct >= 0 && weight_pct <= 100);

        if (weight_pct > 0 && weight_pct < 100)
        {
            const std::string pct_str = "(" + to_str(weight_pct) + "% of total carried weight)";

            lines.push_back({pct_str, clr_green});
        }
    }

    //We draw the description box regardless of whether the lines are empty or not,
    //just to clear this area on the screen.
    render::draw_descr_box(lines);
}

} //namespace

const int INV_H = INV_Y1 - INV_Y0 + 1;

void draw_inv(const Menu_browser& browser)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    render::clear_screen();

    const int           BROWSER_Y           = browser.y();
    const auto&         inv                 = map::player->inv();
    const size_t        NR_SLOTS            = size_t(Slot_id::END);
    const bool          IS_ANY_SLOT_MARKED  = BROWSER_Y < int(NR_SLOTS);
    const Panel         panel               = Panel::screen;

    const auto* const item_marked = IS_ANY_SLOT_MARKED ?
                                    inv.slots_[BROWSER_Y].item :
                                    inv.backpack_[size_t(BROWSER_Y) - NR_SLOTS];

    render::draw_text_center("Browsing inventory" + drop_info_str,
                             panel,
                             P(SCREEN_W / 2, 0),
                             clr_brown_gray);

    const Range idx_range_shown = browser.range_shown();

    P p(0, INV_Y0);

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        p.x = 0;

        const bool IS_IDX_MARKED = BROWSER_Y == i;

        Clr clr = IS_IDX_MARKED ? clr_white_high : clr_menu_drk;

        render::draw_text(key_str, panel, p, clr);

        ++key_str[0];

        p.x += 3;

        if (i < int(NR_SLOTS))
        {
            //This index is a slot
            const Inv_slot&     slot        = inv.slots_[i];
            const std::string   slot_name   = slot.name;

            render::draw_text(slot_name, panel, p, clr);

            p.x += 9; //Offset to leave room for slot label

            const Item* const item = slot.item;

            if (item)
            {
                //An item is equipped here
                draw_item_symbol(*item, p);
                p.x += 2;

                const Item_data_t&  d       = item->data();
                Item_ref_att_inf    att_inf = Item_ref_att_inf::none;

                if (slot.id == Slot_id::wpn || slot.id == Slot_id::wpn_alt)
                {
                    //Thrown weapons are forced to show melee info instead
                    att_inf = d.main_att_mode == Att_mode::thrown ?
                              Item_ref_att_inf::melee : Item_ref_att_inf::wpn_context;
                }
                else if (slot.id == Slot_id::thrown)
                {
                    att_inf = Item_ref_att_inf::thrown;
                }

                Item_ref_type ref_type = Item_ref_type::plain;

                if (slot.id == Slot_id::thrown)
                {
                    ref_type = Item_ref_type::plural;
                }

                std::string item_name = item->name(ref_type,
                                                   Item_ref_inf::yes,
                                                   att_inf);

                IA_ASSERT(!item_name.empty());

                text_format::first_to_upper(item_name);

                Clr clr = IS_IDX_MARKED ?
                          clr_white_high : item->interface_clr();

                render::draw_text(item_name, panel, p, clr);

                draw_weight_pct_and_dots(p,
                                         item_name.size(),
                                         *item,
                                         clr,
                                         IS_IDX_MARKED);
            }
            else //No item in this slot
            {
                p.x += 2;
                render::draw_text("<empty>", panel, p, clr);
            }
        }
        else //This index is in backpack
        {
            const size_t BACKPACK_IDX = i - NR_SLOTS;

            const Item* const item = inv.backpack_[BACKPACK_IDX];

            draw_item_symbol(*item, p);
            p.x += 2;

            std::string item_name = item->name(Item_ref_type::plural,
                                               Item_ref_inf::yes,
                                               Item_ref_att_inf::wpn_context);

            text_format::first_to_upper(item_name);

            clr = IS_IDX_MARKED ?
                  clr_white_high : item->interface_clr();

            render::draw_text(item_name, panel, p, clr);

            draw_weight_pct_and_dots(p,
                                     item_name.size(),
                                     *item,
                                     clr,
                                     IS_IDX_MARKED);
        }

        ++p.y;
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          panel,
                          P(0, TOP_MORE_Y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          panel,
                          P(0, BTM_MORE_Y),
                          clr_white_high);
    }

    draw_detailed_item_descr(item_marked);

    render::update_screen();

    TRACE_FUNC_END_VERBOSE;
}

void draw_apply(const Menu_browser& browser, const std::vector<size_t>& gen_inv_indexes)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const Panel panel = Panel::screen;

    if (gen_inv_indexes.empty())
    {
        render::draw_text("I carry nothing to apply." + cancel_info_str,
                          panel,
                          P(0, 0),
                          clr_white_high);

        render::update_screen();

        return;
    }

    render::clear_screen();

    const int           BROWSER_Y           = browser.y();
    const auto&         inv                 = map::player->inv();
    const size_t        BACKPACK_IDX_MARKED = gen_inv_indexes[size_t(BROWSER_Y)];
    const auto* const   item_marked         = inv.backpack_[BACKPACK_IDX_MARKED];

    render::draw_text_center("Apply which item?" + drop_info_str,
                             panel,
                             P(SCREEN_W / 2, 0),
                             clr_brown_gray);

    const Range idx_range_shown = browser.range_shown();

    P p(0, INV_Y0);

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        p.x = 0;

        const bool IS_IDX_MARKED = BROWSER_Y == i;

        Clr clr = IS_IDX_MARKED ? clr_white_high : clr_menu_drk;

        render::draw_text(key_str, panel, p, clr);

        ++key_str[0];

        p.x += 3;

        const size_t BACKPACK_IDX = gen_inv_indexes[i];

        const Item* const item = inv.backpack_[BACKPACK_IDX];

        draw_item_symbol(*item, p);
        p.x += 2;

        std::string item_name = item->name(Item_ref_type::plural,
                                           Item_ref_inf::yes,
                                           Item_ref_att_inf::wpn_context);

        IA_ASSERT(!item_name.empty());

        text_format::first_to_upper(item_name);

        clr = IS_IDX_MARKED ? clr_white_high : item->interface_clr();

        render::draw_text(item_name, panel, p, clr);

        const size_t ITEM_NAME_LEN = item_name.size();

        draw_weight_pct_and_dots(p, ITEM_NAME_LEN, *item, clr, IS_IDX_MARKED);

        ++p.y;
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          panel,
                          P(0, TOP_MORE_Y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          panel,
                          P(0, BTM_MORE_Y),
                          clr_white_high);
    }

    draw_detailed_item_descr(item_marked);

    render::update_screen();

    TRACE_FUNC_END_VERBOSE;
}

void draw_equip(const Menu_browser& browser,
                const Slot_id slot_id_to_equip,
                const std::vector<size_t>& gen_inv_indexes)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    IA_ASSERT(slot_id_to_equip != Slot_id::END);

    const bool HAS_ITEM = !gen_inv_indexes.empty();

    const Panel panel = Panel::screen;

    std::string heading = "";

    switch (slot_id_to_equip)
    {
    case Slot_id::wpn:
        heading = HAS_ITEM ?
                  "Wield which item?" :
                  "I carry no weapon to wield.";
        break;

    case Slot_id::wpn_alt:
        heading = HAS_ITEM ?
                  "Prepare which weapon?" :
                  "I carry no weapon to wield.";
        break;

    case Slot_id::thrown:
        heading = HAS_ITEM ?
                  "Use which item as thrown weapon?" :
                  "I carry no weapon to throw.";
        break;

    case Slot_id::body:
        heading = HAS_ITEM ?
                  "Wear which armor?" :
                  "I carry no armor.";
        break;

    case Slot_id::head:
        heading = HAS_ITEM ?
                  "Wear what on head?" :
                  "I carry no headwear.";
        break;

    case Slot_id::neck:
        heading = HAS_ITEM ?
                  "Wear what around the neck?" :
                  "I carry nothing to wear around the neck.";
        break;

    case Slot_id::END:
        break;
    }

    if (HAS_ITEM)
    {
        render::clear_screen();

        render::draw_text_center(heading + drop_info_str,
                                 panel,
                                 P(SCREEN_W / 2, 0),
                                 clr_brown_gray);
    }
    else
    {
        render::draw_text(heading + cancel_info_str,
                          panel,
                          P(0, 0),
                          clr_white_high);
    }

    if (HAS_ITEM)
    {
        const int           BROWSER_Y           = browser.y();
        const auto&         inv                 = map::player->inv();
        const size_t        BACKPACK_IDX_MARKED = gen_inv_indexes[size_t(BROWSER_Y)];
        const auto* const   item_marked         = inv.backpack_[BACKPACK_IDX_MARKED];

        const Range idx_range_shown = browser.range_shown();

        P p(0, INV_Y0);

        std::string key_str = "a) ";

        for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
        {
            p.x = 0;

            const bool IS_IDX_MARKED = BROWSER_Y == i;

            Clr clr = IS_IDX_MARKED ? clr_white_high : clr_menu_drk;

            render::draw_text(key_str, panel, p, clr);

            ++key_str[0];

            p.x += 3;

            const size_t BACKPACK_IDX = gen_inv_indexes[i];

            Item* const item = inv.backpack_[BACKPACK_IDX];

            draw_item_symbol(*item, p);
            p.x += 2;

            const Item_data_t&  d       = item->data();
            Item_ref_att_inf    att_inf = Item_ref_att_inf::none;

            if (slot_id_to_equip == Slot_id::wpn || slot_id_to_equip == Slot_id::wpn_alt)
            {
                //Thrown weapons are forced to show melee info instead
                att_inf = d.main_att_mode == Att_mode::thrown ?
                          Item_ref_att_inf::melee : Item_ref_att_inf::wpn_context;
            }
            else if (slot_id_to_equip == Slot_id::thrown)
            {
                att_inf = Item_ref_att_inf::thrown;
            }

            std::string item_name = item->name(Item_ref_type::plural,
                                               Item_ref_inf::yes,
                                               att_inf);

            IA_ASSERT(!item_name.empty());

            text_format::first_to_upper(item_name);

            clr = IS_IDX_MARKED ? clr_white_high : item->interface_clr();

            render::draw_text(item_name, panel, p, clr);

            const size_t ITEM_NAME_LEN = item_name.size();

            draw_weight_pct_and_dots(p,
                                     ITEM_NAME_LEN,
                                     *item,
                                     clr,
                                     IS_IDX_MARKED);

            ++p.y;
        }

        draw_detailed_item_descr(item_marked);
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          panel,
                          P(0, TOP_MORE_Y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          panel,
                          P(0, BTM_MORE_Y),
                          clr_white_high);
    }

    render::update_screen();

    TRACE_FUNC_END_VERBOSE;
}

} //render_inv
