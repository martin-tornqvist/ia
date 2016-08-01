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

const int top_more_y    = 0;
const int btm_more_y    = screen_h - 1;
const int INV_Y0        = top_more_y + 1;
const int INV_Y1        = btm_more_y - 1;

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
                              const size_t item_name_len,
                              const Item& item,
                              const Clr& item_name_clr,
                              const bool is_marked)
{
    const int weight_carried_tot = map::player->inv().total_item_weight();

    int item_weight_pct = 0;

    if (weight_carried_tot > 0)
    {
        item_weight_pct = (item.weight() * 100) / weight_carried_tot;
    }

    std::string weight_str  = to_str(item_weight_pct) + "%";
    int         weight_x    = descr_x0 - 1 - weight_str.size();

    ASSERT(item_weight_pct >= 0 && item_weight_pct <= 100);

    if (item_weight_pct > 0 && item_weight_pct < 100)
    {
        const P weight_pos(weight_x, item_pos.y);

        const Clr weight_clr = is_marked ? clr_white : clr_gray_drk;

        render::draw_text(weight_str,
                          Panel::screen,
                          weight_pos,
                          weight_clr);
    }
    else //"Zero" weight, or 100% of weight - no weight percent should be displayed
    {
        weight_str  = "";
        weight_x    = descr_x0 - 1;
    }

    int dots_x = item_pos.x + item_name_len;
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
    Clr                 dots_clr = is_marked ? clr_white : item_name_clr;

    if (!is_marked)
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
    std::vector<StrAndClr> lines;

    if (item)
    {
        //----------------------------------------------------------------------
        //Base description
        //----------------------------------------------------------------------
        const auto base_descr = item->descr();

        if (!base_descr.empty())
        {
            for (const std::string& paragraph : base_descr)
            {
                lines.push_back(StrAndClr(paragraph, clr_white_high));
            }
        }

        const bool is_plural = item->nr_items_ > 1 && item->data().is_stackable;

        const std::string ref_str = is_plural ? "They are " : "It is ";

        const ItemDataT& d = item->data();

        //----------------------------------------------------------------------
        //Disturbing to carry?
        //----------------------------------------------------------------------
        std::string disturb_str = "";

        const std::string disturb_base_str = ref_str + "a burden on my mind to ";

        if (d.is_ins_raied_while_carried)
        {
            disturb_str = disturb_base_str + "carry";
        }
        else if (d.is_ins_raied_while_equiped)
        {
            if (d.type == ItemType::melee_wpn || d.type == ItemType::ranged_wpn)
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
            disturb_str += " (+" + to_str(ins_from_disturbing_items) + "% insanity)";

            lines.push_back(StrAndClr(disturb_str, clr_magenta));
        }

        //----------------------------------------------------------------------
        //Weight
        //----------------------------------------------------------------------

        const std::string weight_str = ref_str + item->weight_str() + " to carry.";

        lines.push_back(StrAndClr(weight_str, clr_green));

        const int weight_carried_tot = map::player->inv().total_item_weight();

        int weight_pct = 0;

        if (weight_carried_tot > 0)
        {
            weight_pct = (item->weight() * 100) / weight_carried_tot;
        }

        ASSERT(weight_pct >= 0 && weight_pct <= 100);

        if (weight_pct > 0 && weight_pct < 100)
        {
            const std::string pct_str = "(" + to_str(weight_pct) + "% of total carried weight)";

            lines.push_back(StrAndClr(pct_str, clr_green));
        }

        //----------------------------------------------------------------------
        //XP for identifying
        //----------------------------------------------------------------------
        if (!d.is_identified && (d.xp_on_identify > 0))
        {
            const std::string xp_id_str =
                "Identifying grants " + to_str(d.xp_on_identify) + " XP.";

            lines.push_back(StrAndClr(xp_id_str, clr_msg_good));
        }
    }

    //We draw the description box regardless of whether the lines are empty or
    //not, just to clear this area on the screen.
    render::draw_descr_box(lines);
}

} //namespace

const int inv_h = INV_Y1 - INV_Y0 + 1;

void draw_inv(const MenuBrowser& browser)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    render::clear_screen();

    const int           browser_y           = browser.y();
    const auto&         inv                 = map::player->inv();
    const size_t        nr_slots            = size_t(SlotId::END);
    const bool          is_any_slot_marked  = browser_y < int(nr_slots);
    const Panel         panel               = Panel::screen;

    const auto* const item_marked = is_any_slot_marked ?
                                    inv.slots_[browser_y].item :
                                    inv.backpack_[(size_t)browser_y - nr_slots];

    render::draw_text_center("Browsing inventory" + drop_info_str,
                             panel,
                             P(screen_w / 2, 0),
                             clr_brown_gray);

    const Range idx_range_shown = browser.range_shown();

    P p(0, INV_Y0);

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        p.x = 0;

        const bool is_idx_marked = browser_y == i;

        Clr clr = is_idx_marked ? clr_white_high : clr_menu_drk;

        render::draw_text(key_str,
                          panel,
                          p,
                          clr);

        ++key_str[0];

        p.x += 3;

        if (i < int(nr_slots))
        {
            //This index is a slot
            const InvSlot&     slot        = inv.slots_[i];
            const std::string   slot_name   = slot.name;

            render::draw_text(slot_name, panel, p, clr);

            p.x += 9; //Offset to leave room for slot label

            const Item* const item = slot.item;

            if (item)
            {
                //An item is equipped here
                draw_item_symbol(*item, p);
                p.x += 2;

                const ItemDataT&  d       = item->data();
                ItemRefAttInf    att_inf = ItemRefAttInf::none;

                if (slot.id == SlotId::wpn || slot.id == SlotId::wpn_alt)
                {
                    //Thrown weapons are forced to show melee info instead
                    att_inf = d.main_att_mode == AttMode::thrown ?
                              ItemRefAttInf::melee : ItemRefAttInf::wpn_context;
                }
                else if (slot.id == SlotId::thrown)
                {
                    att_inf = ItemRefAttInf::thrown;
                }

                ItemRefType ref_type = ItemRefType::plain;

                if (slot.id == SlotId::thrown)
                {
                    ref_type = ItemRefType::plural;
                }

                std::string item_name = item->name(ref_type,
                                                   ItemRefInf::yes,
                                                   att_inf);

                ASSERT(!item_name.empty());

                text_format::first_to_upper(item_name);

                Clr clr = is_idx_marked ?
                          clr_white_high : item->interface_clr();

                render::draw_text(item_name, panel, p, clr);

                draw_weight_pct_and_dots(p,
                                         item_name.size(),
                                         *item,
                                         clr,
                                         is_idx_marked);
            }
            else //No item in this slot
            {
                p.x += 2;
                render::draw_text("<empty>", panel, p, clr);
            }
        }
        else //This index is in backpack
        {
            const size_t backpack_idx = i - nr_slots;

            const Item* const item = inv.backpack_[backpack_idx];

            draw_item_symbol(*item, p);
            p.x += 2;

            std::string item_name = item->name(ItemRefType::plural,
                                               ItemRefInf::yes,
                                               ItemRefAttInf::wpn_context);

            text_format::first_to_upper(item_name);

            clr = is_idx_marked ?
                  clr_white_high : item->interface_clr();

            render::draw_text(item_name, panel, p, clr);

            draw_weight_pct_and_dots(p,
                                     item_name.size(),
                                     *item,
                                     clr,
                                     is_idx_marked);
        }

        ++p.y;
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          panel,
                          P(0, top_more_y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          panel,
                          P(0, btm_more_y),
                          clr_white_high);
    }

    draw_detailed_item_descr(item_marked);

    render::update_screen();

    TRACE_FUNC_END_VERBOSE;
}

void draw_apply(const MenuBrowser& browser, const std::vector<size_t>& gen_inv_indexes)
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

    const int           browser_y           = browser.y();
    const auto&         inv                 = map::player->inv();
    const size_t        backpack_idx_marked = gen_inv_indexes[size_t(browser_y)];
    const auto* const   item_marked         = inv.backpack_[backpack_idx_marked];

    render::draw_text_center("Apply which item?" + drop_info_str,
                             panel,
                             P(screen_w / 2, 0),
                             clr_brown_gray);

    const Range idx_range_shown = browser.range_shown();

    P p(0, INV_Y0);

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        p.x = 0;

        const bool is_idx_marked = browser_y == i;

        Clr clr = is_idx_marked ? clr_white_high : clr_menu_drk;

        render::draw_text(key_str, panel, p, clr);

        ++key_str[0];

        p.x += 3;

        const size_t backpack_idx = gen_inv_indexes[i];

        const Item* const item = inv.backpack_[backpack_idx];

        draw_item_symbol(*item, p);
        p.x += 2;

        std::string item_name = item->name(ItemRefType::plural,
                                           ItemRefInf::yes,
                                           ItemRefAttInf::wpn_context);

        ASSERT(!item_name.empty());

        text_format::first_to_upper(item_name);

        clr = is_idx_marked ? clr_white_high : item->interface_clr();

        render::draw_text(item_name, panel, p, clr);

        const size_t item_name_len = item_name.size();

        draw_weight_pct_and_dots(p, item_name_len, *item, clr, is_idx_marked);

        ++p.y;
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          panel,
                          P(0, top_more_y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          panel,
                          P(0, btm_more_y),
                          clr_white_high);
    }

    draw_detailed_item_descr(item_marked);

    render::update_screen();

    TRACE_FUNC_END_VERBOSE;
}

void draw_equip(const MenuBrowser& browser,
                const SlotId slot_id_to_equip,
                const std::vector<size_t>& gen_inv_indexes)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    ASSERT(slot_id_to_equip != SlotId::END);

    const bool has_item = !gen_inv_indexes.empty();

    const Panel panel = Panel::screen;

    std::string heading = "";

    switch (slot_id_to_equip)
    {
    case SlotId::wpn:
        heading = has_item ?
                  "Wield which item?" :
                  "I carry no weapon to wield.";
        break;

    case SlotId::wpn_alt:
        heading = has_item ?
                  "Prepare which weapon?" :
                  "I carry no weapon to wield.";
        break;

    case SlotId::thrown:
        heading = has_item ?
                  "Use which item as thrown weapon?" :
                  "I carry no weapon to throw.";
        break;

    case SlotId::body:
        heading = has_item ?
                  "Wear which armor?" :
                  "I carry no armor.";
        break;

    case SlotId::head:
        heading = has_item ?
                  "Wear what on head?" :
                  "I carry no headwear.";
        break;

    case SlotId::neck:
        heading = has_item ?
                  "Wear what around the neck?" :
                  "I carry nothing to wear around the neck.";
        break;

    case SlotId::END:
        break;
    }

    if (has_item)
    {
        render::clear_screen();

        render::draw_text_center(heading + drop_info_str,
                                 panel,
                                 P(screen_w / 2, 0),
                                 clr_brown_gray);
    }
    else
    {
        render::draw_text(heading + cancel_info_str,
                          panel,
                          P(0, 0),
                          clr_white_high);
    }

    if (has_item)
    {
        const int           browser_y           = browser.y();
        const auto&         inv                 = map::player->inv();
        const size_t        backpack_idx_marked = gen_inv_indexes[size_t(browser_y)];
        const auto* const   item_marked         = inv.backpack_[backpack_idx_marked];

        const Range idx_range_shown = browser.range_shown();

        P p(0, INV_Y0);

        std::string key_str = "a) ";

        for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
        {
            p.x = 0;

            const bool is_idx_marked = browser_y == i;

            Clr clr = is_idx_marked ? clr_white_high : clr_menu_drk;

            render::draw_text(key_str, panel, p, clr);

            ++key_str[0];

            p.x += 3;

            const size_t backpack_idx = gen_inv_indexes[i];

            Item* const item = inv.backpack_[backpack_idx];

            draw_item_symbol(*item, p);
            p.x += 2;

            const ItemDataT&  d       = item->data();
            ItemRefAttInf    att_inf = ItemRefAttInf::none;

            if (slot_id_to_equip == SlotId::wpn || slot_id_to_equip == SlotId::wpn_alt)
            {
                //Thrown weapons are forced to show melee info instead
                att_inf = d.main_att_mode == AttMode::thrown ?
                          ItemRefAttInf::melee : ItemRefAttInf::wpn_context;
            }
            else if (slot_id_to_equip == SlotId::thrown)
            {
                att_inf = ItemRefAttInf::thrown;
            }

            std::string item_name = item->name(ItemRefType::plural,
                                               ItemRefInf::yes,
                                               att_inf);

            ASSERT(!item_name.empty());

            text_format::first_to_upper(item_name);

            clr = is_idx_marked ? clr_white_high : item->interface_clr();

            render::draw_text(item_name, panel, p, clr);

            const size_t item_name_len = item_name.size();

            draw_weight_pct_and_dots(p,
                                     item_name_len,
                                     *item,
                                     clr,
                                     is_idx_marked);

            ++p.y;
        }

        draw_detailed_item_descr(item_marked);
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          panel,
                          P(0, top_more_y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          panel,
                          P(0, btm_more_y),
                          clr_white_high);
    }

    render::update_screen();

    TRACE_FUNC_END_VERBOSE;
}

} //render_inv
