#include "inventory_handling.hpp"

#include "init.hpp"

#include <vector>
#include <sstream>
#include <iomanip>

#include "item_scroll.hpp"
#include "actor_player.hpp"
#include "item_potion.hpp"
#include "msg_log.hpp"
#include "browser.hpp"
#include "io.hpp"
#include "drop.hpp"
#include "query.hpp"
#include "item_factory.hpp"
#include "audio.hpp"
#include "map.hpp"
#include "text_format.hpp"
#include "marker.hpp"

// TODO: All this should probably be moved to the inventory screen base class
namespace
{

// Index can mean Slot index or Backpack Index (both start from zero)
bool run_drop_query(const InvType inv_type, const size_t idx)
{
    TRACE_FUNC_BEGIN;

    Inventory& inv = map::player->inv();

    Item* item = nullptr;

    if (inv_type == InvType::slots)
    {
        ASSERT(idx < (int)SlotId::END);

        item = inv.slots_[idx].item;
    }
    else // Backpack
    {
        ASSERT(idx < inv.backpack_.size());

        item = inv.backpack_[idx];
    }

    if (!item)
    {
        TRACE_FUNC_END;

        return false;
    }

    const ItemData& data = item->data();

    msg_log::clear();

    if (data.is_stackable && item->nr_items_ > 1)
    {
        TRACE << "Item is stackable and more than one" << std::endl;

        io::clear_screen();

        states::draw();

        const std::string nr_str = "1-" + std::to_string(item->nr_items_);

        const std::string drop_str = "Drop how many (" + nr_str + ")?:";

        io::draw_text(drop_str,
                      Panel::screen,
                      P(0, 0),
                      colors::light_white());

        io::update_screen();

        const P nr_query_pos(drop_str.size() + 1, 0);

        const int max_digits = 3;
        const P done_inf_pos = nr_query_pos + P(max_digits + 2, 0);

        io::draw_text("[enter] to drop" + cancel_info_str,
                      Panel::screen,
                      done_inf_pos,
                      colors::light_white());

        const int nr_to_drop =
            query::number(nr_query_pos,
                          colors::light_white(),
                          0, 3,
                          item->nr_items_,
                          false);

        if (nr_to_drop <= 0)
        {
            TRACE << "Nr to drop <= 0, nothing to be done" << std::endl;

            TRACE_FUNC_END;

            return false;
        }
        else // Number to drop is at least one
        {
            item_drop::drop_item_from_inv(
                *map::player,
                inv_type,
                idx,
                nr_to_drop);

            TRACE_FUNC_END;

            return true;
        }
    }
    else // Not a stack
    {
        TRACE << "Item not stackable, or only one item" << std::endl;

        item_drop::drop_item_from_inv(
            *map::player,
            inv_type,
            idx);

        TRACE_FUNC_END;

        return true;
    }

    TRACE_FUNC_END;

    return false;
}

} // namespace

// -----------------------------------------------------------------------------
// Abstract inventory screen state
// -----------------------------------------------------------------------------
InvState::InvState() :
    State       (),
    browser_    (),
    top_more_y_ (0),
    btm_more_y_ (screen_h - 1),
    inv_y0_     (top_more_y_ + 1),
    inv_y1_     (btm_more_y_ - 1),
    inv_h_      (inv_y1_ - inv_y0_ + 1)
{

}

StateId InvState::id()
{
    return StateId::inventory;
}

void InvState::draw_slot(const SlotId id,
                         const int y,
                         const char key,
                         const bool is_marked,
                         const ItemRefAttInf att_info) const
{
    // Draw key
    const Color color =
        is_marked ?
        colors::light_white() :
        colors::menu_dark();

    P p(0, y);

    std::string key_str = "?) ";

    key_str[0] = key;

    io::draw_text(key_str,
                  Panel::screen,
                  p,
                  color);

    p.x += 3;

    // Draw slot label
    const auto& inv = map::player->inv();

    const InvSlot& slot = inv.slots_[(size_t)id];

    const std::string slot_name = slot.name;

    io::draw_text(slot_name,
                  Panel::screen,
                  p,
                  color);

    p.x += 9; // Offset to leave room for slot label

    // Draw item
    const Item* const item = slot.item;

    if (item)
    {
        // An item is equipped here
        draw_item_symbol(*item, p);

        p.x += 2;

        std::string item_name =
            item->name(ItemRefType::plural,
                       ItemRefInf::yes,
                       att_info);

        ASSERT(!item_name.empty());

        item_name = text_format::first_to_upper(item_name);

        const Color color_item =
            is_marked ?
            colors::light_white() :
            item->interface_color();

        io::draw_text(item_name,
                      Panel::screen,
                      p,
                      color_item);

        draw_weight_pct_and_dots(p,
                                 item_name.size(),
                                 *item,
                                 color_item,
                                 is_marked);
    }
    else // No item in this slot
    {
        p.x += 2;

        io::draw_text("<empty>",
                      Panel::screen,
                      p,
                      color);
    }

    if (is_marked)
    {
        draw_detailed_item_descr(item);
    }
}

void InvState::draw_backpack_item(const size_t backpack_idx,
                                  const int y,
                                  const char key,
                                  const bool is_marked,
                                  const ItemRefAttInf att_info) const
{
    // Draw key
    const Color color =
        is_marked ?
        colors::light_white() :
        colors::menu_dark();

    std::string key_str = "?) ";

    key_str[0] = key;

    P p(0, y);

    io::draw_text(key_str,
                  Panel::screen,
                  p,
                  color);

    p.x += 3;

    // Draw item
    auto& inv = map::player->inv();

    const Item* const item = inv.backpack_[backpack_idx];

    draw_item_symbol(*item, p);

    p.x += 2;

    std::string item_name = item->name(ItemRefType::plural,
                                       ItemRefInf::yes,
                                       att_info);

    item_name = text_format::first_to_upper(item_name);

    const Color color_item =
        is_marked ?
        colors::light_white() :
        item->interface_color();

    io::draw_text(item_name,
                  Panel::screen,
                  p,
                  color_item);

    draw_weight_pct_and_dots(p,
                             item_name.size(),
                             *item,
                             color_item,
                             is_marked);

    if (is_marked)
    {
        draw_detailed_item_descr(item);
    }
}

void InvState::draw_item_symbol(const Item& item, const P& p) const
{
    const Color item_color = item.color();

    if (config::is_tiles_mode())
    {
        io::draw_tile(item.tile(), Panel::screen, p, item_color);
    }
    else // Text mode
    {
        io::draw_character(item.character(), Panel::screen, p, item_color);
    }
}

void InvState::draw_weight_pct_and_dots(const P item_pos,
                                        const size_t item_name_len,
                                        const Item& item,
                                        const Color& item_name_color,
                                        const bool is_marked) const
{
    const int weight_carried_tot =
        map::player->inv().total_item_weight();

    int item_weight_pct = 0;

    if (weight_carried_tot > 0)
    {
        item_weight_pct = (item.weight() * 100) / weight_carried_tot;
    }

    std::string weight_str = std::to_string(item_weight_pct) + "%";
    int weight_x = descr_x0 - 1 - weight_str.size();

    ASSERT(item_weight_pct >= 0 && item_weight_pct <= 100);

    if (item_weight_pct > 0 && item_weight_pct < 100)
    {
        const P weight_pos(weight_x, item_pos.y);

        const Color weight_color =
            is_marked ?
            colors::light_white() :
            colors::menu_dark();

        io::draw_text(weight_str,
                      Panel::screen,
                      weight_pos,
                      weight_color);
    }
    else // Zero weight, or 100% of weight
    {
        // No weight percent is displayed
        weight_str = "";
        weight_x = descr_x0 - 1;
    }

    int dots_x = item_pos.x + item_name_len;
    int dots_w = weight_x - dots_x;

    if (dots_w == 0)
    {
        // Item name fits exactly, just skip drawing dots
        return;
    }

    if (dots_w < 0)
    {
        // Item name does not fit at all, draw dots until the weight percent
        dots_w = 3;
        const int dots_x1 = weight_x - 1;
        dots_x = dots_x1 - dots_w + 1;
    }

    const std::string dots_str(dots_w, '.');

    Color dots_color =
        is_marked ?
        colors::white() :
        item_name_color;

    if (!is_marked)
    {
        dots_color = dots_color.fraction(2.0);
    }

    io::draw_text(dots_str,
                  Panel::screen,
                  P(dots_x, item_pos.y),
                  dots_color);
}

void InvState::draw_detailed_item_descr(const Item* const item) const
{
    std::vector<ColoredString> lines;

    if (item)
    {
        // ---------------------------------------------------------------------
        // Base description
        // ---------------------------------------------------------------------
        const auto base_descr = item->descr();

        if (!base_descr.empty())
        {
            for (const std::string& paragraph : base_descr)
            {
                lines.push_back(
                    ColoredString(paragraph, colors::light_white()));
            }
        }

        const bool is_plural =
            item->nr_items_ > 1 &&
            item->data().is_stackable;

        const std::string ref_str =
            is_plural ?
            "They are " : "It is ";

        const ItemData& d = item->data();

        // ---------------------------------------------------------------------
        // Damage dice?
        // ---------------------------------------------------------------------
        if (d.allow_display_dmg)
        {
            const std::string dmg_str =
                item->dmg_str(ItemRefAttInf::wpn_main_att_mode,
                              ItemRefDmg::dice);

            const std::string dmg_str_avg =
                item->dmg_str(ItemRefAttInf::wpn_main_att_mode,
                              ItemRefDmg::average);

            if (!dmg_str.empty() && !dmg_str_avg.empty())
            {
                lines.push_back(
                    ColoredString(
                        "Damage: " + dmg_str + " (average " + dmg_str_avg + ")",
                        colors::light_white()));
            }
        }

        // ---------------------------------------------------------------------
        // Can be used for breaking doors or destroying corpses?
        // ---------------------------------------------------------------------
        const bool can_att_rigid = d.melee.att_rigid;
        const bool can_att_corpse = d.melee.att_corpse;

        std::string att_obj_str = "";

        if (can_att_rigid || can_att_corpse)
        {
            att_obj_str = "Can be used for ";
        }

        if (can_att_rigid)
        {
            att_obj_str += "breaching doors";
        }

        if (can_att_corpse)
        {
            if (can_att_rigid)
            {
                att_obj_str += " and ";
            }

            att_obj_str += "destroying corpses";
        }

        if (can_att_rigid || can_att_corpse)
        {
            att_obj_str +=
                " more effectively (bonus is based on attack damage).";

            lines.push_back(ColoredString(att_obj_str, colors::light_white()));
        }

        // ---------------------------------------------------------------------
        // Disturbing to carry?
        // ---------------------------------------------------------------------
        std::string disturb_str = "";

        const std::string disturb_base_str =
            ref_str + "a burden on my mind to ";

        if (d.is_carry_shocking)
        {
            disturb_str = disturb_base_str + "carry";
        }
        else if (d.is_equiped_shocking)
        {
            if (d.type == ItemType::melee_wpn ||
                d.type == ItemType::ranged_wpn)
            {
                disturb_str = disturb_base_str + "wield.";
            }
            else // Not a wieldable item
            {
                disturb_str = disturb_base_str + "wear.";
            }
        }

        if (!disturb_str.empty())
        {
            std::stringstream shock_value_stream;

            shock_value_stream << std::setprecision(2)
                               << shock_from_disturbing_items;

            disturb_str +=
                " (+" +
                shock_value_stream.str() +
                "% shock taken per turn - before resistances are applied)";

            lines.push_back(ColoredString(disturb_str, colors::magenta()));
        }

        // ---------------------------------------------------------------------
        // Weight
        // ---------------------------------------------------------------------
        const std::string weight_str =
            ref_str + item->weight_str() + " to carry.";

        lines.push_back(ColoredString(weight_str, colors::green()));

        const int weight_carried_tot =
            map::player->inv().total_item_weight();

        int weight_pct = 0;

        if (weight_carried_tot > 0)
        {
            weight_pct = (item->weight() * 100) / weight_carried_tot;
        }

        ASSERT(weight_pct >= 0 && weight_pct <= 100);

        if (weight_pct > 0 && weight_pct < 100)
        {
            const std::string pct_str =
                "(" + std::to_string(weight_pct) + "% of total carried weight)";

            lines.push_back(ColoredString(pct_str, colors::green()));
        }
    }

    // We draw the description box regardless of whether the lines are empty or
    // not, just to clear this area on the screen.
    io::draw_descr_box(lines);
}

void InvState::activate(const size_t backpack_idx)
{
    Inventory& player_inv = map::player->inv();
    Item* item = player_inv.backpack_[backpack_idx];

    auto result = item->activate(map::player);

    if (result == ConsumeItem::yes)
    {
        player_inv.decr_item_in_backpack(backpack_idx);
    }
}

// -----------------------------------------------------------------------------
// Inventory browsing state
// -----------------------------------------------------------------------------
void BrowseInv::on_start()
{
    Inventory& inv = map::player->inv();

    inv.sort_backpack();

    const int list_size =
        (int)SlotId::END +
        (int)inv.backpack_.size();

    browser_.reset(list_size, inv_h_);

    inv.sort_backpack();

    audio::play(SfxId::backpack);
}

void BrowseInv::on_resume()
{
    // If we have been away from this state (e.g. equipping a slot), then the
    // inventory may have been affected by the other state(s), so we need to
    // reset the browser.

    Inventory& inv = map::player->inv();

    const int list_size =
        (int)SlotId::END +
        (int)inv.backpack_.size();

    // (The browser will also set the y pos to the nearest valid point)
    browser_.reset(list_size, inv_h_);
}

void BrowseInv::draw()
{
    // Only draw this state if it's the current state, and no "exit" command
    // received from another state - so that messages can be drawn over the map
    if (!states::is_current_state(*this) ||
        received_exit_cmd_)
    {
        return;
    }

    io::clear_screen();

    const int browser_y = browser_.y();

    const size_t nr_slots = (size_t)SlotId::END;

    io::draw_text_center("Browsing inventory" + drop_info_str,
                         Panel::screen,
                         P(screen_w / 2, 0),
                         colors::title());

    const Range idx_range_shown = browser_.range_shown();

    int y = inv_y0_;

    char key = 'a';

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const bool is_marked = browser_y == i;

        if (i < (int)nr_slots)
        {
            draw_slot((SlotId)i,
                      y,
                      key,
                      is_marked,
                      ItemRefAttInf::wpn_main_att_mode);
        }
        else // This index is in backpack
        {
            const size_t backpack_idx = i - nr_slots;

            draw_backpack_item(backpack_idx,
                               y,
                               key,
                               is_marked,
                               ItemRefAttInf::wpn_main_att_mode);
        }

        ++key;

        ++y;
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      Panel::screen,
                      P(0, top_more_y_),
                      colors::light_white());
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      Panel::screen,
                      P(0, btm_more_y_),
                      colors::light_white());
    }
}

void BrowseInv::update()
{
    // Have we been told by another state that we should exit?
    if (received_exit_cmd_)
    {
        states::pop();

        return;
    }

    Inventory& inv = map::player->inv();

    auto inv_type = [&]()
    {
        const InvType inv_type_marked =
            (browser_.y() < (int)SlotId::END) ?
            InvType::slots :
            InvType::backpack;

        return inv_type_marked;
    };

    const auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    {
        const auto inv_type_marked = inv_type();

        if (inv_type_marked == InvType::slots)
        {
            const size_t browser_y = browser_.y();
            InvSlot& slot = inv.slots_[browser_y];

            if (slot.item)
            {
                // Exit screen
                states::pop();

                msg_log::clear();

                if (slot.id == SlotId::body)
                {
                    // Start taking off the armor
                    map::player->nr_turns_until_handle_armor_done_ =
                        nr_turns_to_handle_armor;
                }
                else // Not the body slot
                {
                    // Remove the item immediately
                    inv.unequip_slot(slot.id);
                }

                game_time::tick();

                return;
            }
            else // No item in slot
            {
                std::unique_ptr<State> equip(new Equip(slot, *this));

                states::push(std::move(equip));
            }
        }
        else // In backpack inventory
        {
            const size_t browser_y =
                browser_.y() - (int)SlotId::END;

            // Exit screen
            states::pop();

            activate(browser_y);

            return;
        }
    }
    break;

    case MenuAction::selected_shift:
    {
        const int browser_y = browser_.y();

        const auto inv_type_marked = inv_type();

        const size_t idx =
            (inv_type_marked == InvType::slots) ?
            browser_y :
            (browser_y - (int)SlotId::END);

        // Exit screen
        states::pop();

        if ((inv_type_marked == InvType::slots) &&
            (idx == (size_t)SlotId::body))
        {
            // Start dropping the armor
            map::player->nr_turns_until_handle_armor_done_ =
                nr_turns_to_handle_armor;

            map::player->is_dropping_armor_from_body_slot_ = true;

            game_time::tick();
        }
        else // Not dropping from body slot
        {
            // Drop the item immediately
            const bool did_drop = run_drop_query(inv_type_marked, idx);

            if (did_drop)
            {
                game_time::tick();
            }
        }

        return;
    }
    break;

    case MenuAction::esc:
    case MenuAction::space:
    {
        // Exit screen
        states::pop();

        return;
    }
    break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Apply item state
// -----------------------------------------------------------------------------
void Apply::on_start()
{
    auto& inv = map::player->inv();

    inv.sort_backpack();

    auto& backpack = inv.backpack_;

    filtered_backpack_indexes_.clear();

    filtered_backpack_indexes_.reserve(backpack.size());

    for (size_t i = 0; i < backpack.size(); ++i)
    {
        const Item* const item = backpack[i];

        const ItemData& d = item->data();

        if (d.has_std_activate)
        {
            filtered_backpack_indexes_.push_back(i);
        }
    }

    if (filtered_backpack_indexes_.empty())
    {
        // Exit screen
        states::pop();

        msg_log::add("I carry nothing to apply.");

        return;
    }

    browser_.reset(filtered_backpack_indexes_.size(),
                   inv_h_);

    audio::play(SfxId::backpack);
}

void Apply::draw()
{
    // Only draw this state if it's the current state - so that messages can be
    // drawn over the map
    if (!states::is_current_state(*this))
    {
        return;
    }

    io::clear_screen();

    const Panel panel = Panel::screen;

    const int browser_y = browser_.y();

    io::draw_text_center("Apply which item?" + drop_info_str,
                         panel,
                         P(screen_w / 2, 0),
                         colors::title());

    const Range idx_range_shown = browser_.range_shown();

    int y = inv_y0_;

    char key = 'a';

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const bool is_marked = browser_y == i;

        const size_t backpack_idx = filtered_backpack_indexes_[i];

        draw_backpack_item(backpack_idx,
                           y,
                           key,
                           is_marked,
                           ItemRefAttInf::wpn_main_att_mode);

        ++key;

        ++y;
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      panel,
                      P(0, top_more_y_),
                      colors::light_white());
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      panel,
                      P(0, btm_more_y_),
                      colors::light_white());
    }
}

void Apply::update()
{
    auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    {
        if (!filtered_backpack_indexes_.empty())
        {
            const size_t idx =
                filtered_backpack_indexes_[browser_.y()];

            // Exit screen
            states::pop();

            activate(idx);

            return;
        }
    }
    break;

    case MenuAction::selected_shift:
    {
        if (!filtered_backpack_indexes_.empty())
        {
            const size_t idx =
                filtered_backpack_indexes_[browser_.y()];

            // Exit screen
            states::pop();

            // Drop the item immediately
            const bool did_drop = run_drop_query(InvType::backpack, idx);

            if (did_drop)
            {
                game_time::tick();
            }

            return;
        }
    }
    break;

    case MenuAction::esc:
    case MenuAction::space:
    {
        // Exit screen
        states::pop();
        return;
    }
    break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Equip state
// -----------------------------------------------------------------------------
void Equip::on_start()
{
    auto& inv = map::player->inv();

    inv.sort_backpack();

    // Filter backpack
    const auto& backpack = inv.backpack_;

    filtered_backpack_indexes_.clear();

    for (size_t i = 0; i < backpack.size(); ++i)
    {
        const auto* const item = backpack[i];
        const auto& data = item->data();

        switch (slot_to_equip_.id)
        {
        case SlotId::wpn:
            if (data.melee.is_melee_wpn || data.ranged.is_ranged_wpn)
            {
                filtered_backpack_indexes_.push_back(i);
            }
            break;

        case SlotId::wpn_alt:
            if (data.melee.is_melee_wpn || data.ranged.is_ranged_wpn)
            {
                filtered_backpack_indexes_.push_back(i);
            }
            break;

        case SlotId::body:
            if (data.type == ItemType::armor)
            {
                filtered_backpack_indexes_.push_back(i);
            }
            break;

        case SlotId::head:
            if (data.type == ItemType::head_wear)
            {
                filtered_backpack_indexes_.push_back(i);
            }
            break;

        case SlotId::END:
            break;
        }
    }

    browser_.reset(filtered_backpack_indexes_.size(), inv_h_);

    browser_.set_y(0);
}

void Equip::draw()
{
    const bool has_item = !filtered_backpack_indexes_.empty();
    const Panel panel = Panel::screen;
    std::string heading = "";

    switch (slot_to_equip_.id)
    {
    case SlotId::wpn:
        heading =
            has_item ?
            "Wield which item?" :
            "I carry no weapon to wield.";
        break;

    case SlotId::wpn_alt:
        heading =
            has_item ?
            "Prepare which weapon?" :
            "I carry no weapon to wield.";
        break;

    case SlotId::body:
        heading =
            has_item ?
            "Wear which armor?" :
            "I carry no armor.";
        break;

    case SlotId::head:
        heading =
            has_item ?
            "Wear what on head?" :
            "I carry no headwear.";
        break;

    case SlotId::END:
        break;
    }

    if (!has_item)
    {
        io::draw_text(heading + any_key_info_str,
                      panel,
                      P(0, 0),
                      colors::light_white());

        return;
    }

    // An item is available

    io::draw_text_center(heading + drop_info_str,
                         panel,
                         P(screen_w / 2, 0),
                         colors::title());

    auto& inv = map::player->inv();

    const int browser_y = browser_.y();

    const Range idx_range_shown = browser_.range_shown();

    int y =inv_y0_;

    char key = 'a';

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const bool is_marked = browser_y == i;

        const size_t backpack_idx =
            filtered_backpack_indexes_[i];

        Item* const item = inv.backpack_[backpack_idx];

        const ItemData& d = item->data();

        ItemRefAttInf att_inf = ItemRefAttInf::none;

        if ((slot_to_equip_.id == SlotId::wpn) ||
            (slot_to_equip_.id == SlotId::wpn_alt))
        {
            // Thrown weapons are forced to show melee info instead
            att_inf =
                (d.main_att_mode == AttMode::thrown) ?
                ItemRefAttInf::melee :
                ItemRefAttInf::wpn_main_att_mode;
        }

        draw_backpack_item(backpack_idx,
                           y,
                           key,
                           is_marked,
                           att_inf);

        ++key;

        ++y;
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      panel,
                      P(0, top_more_y_),
                      colors::light_white());
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      panel,
                      P(0, btm_more_y_),
                      colors::light_white());
    }
}

void Equip::update()
{
    const auto input = io::get(false);

    if (filtered_backpack_indexes_.empty() ||
        (input.key == SDLK_SPACE) ||
        (input.key == SDLK_ESCAPE))
    {
        // Leave screen, and go back to inventory
        states::pop();

        return;
    }

    auto& inv = map::player->inv();

    const MenuAction action =
        browser_.read(input, MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    {
        const size_t idx = filtered_backpack_indexes_[browser_.y()];

        const auto slot_id = slot_to_equip_.id;

        // Tell the inventory browsing screen that it's time to exit

        // TODO: It would be better to pop a specific state ID instead
        browse_inv_state_.exit_screen();

        // Exit screen
        states::pop();

        if (slot_id == SlotId::body)
        {
            if (map::player->has_prop(PropId::burning))
            {
                msg_log::add("Not while burning.");

                return;
            }

            // Start putting on armor
            map::player->nr_turns_until_handle_armor_done_ =
                nr_turns_to_handle_armor;

            map::player->armor_putting_on_backpack_idx_ = idx;
        }
        else // Not the body slot
        {
            // Equip the item immediately
            inv.equip_backpack_item(idx, slot_id);
        }

        game_time::tick();

        return;
    }
    break;

    case MenuAction::selected_shift:
    {
        const size_t idx = filtered_backpack_indexes_[browser_.y()];

        // Tell the inventory browsing screen that it's time to exit
        browse_inv_state_.exit_screen();

        // Exit screen
        states::pop();

        const bool did_drop = run_drop_query(InvType::backpack, idx);

        if (did_drop)
        {
            game_time::tick();
        }

        return;
    }
    break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Select throwing state
// -----------------------------------------------------------------------------
void SelectThrow::on_start()
{
    auto& inv = map::player->inv();

    inv.sort_backpack();

    // Filter slots
    for (InvSlot& slot : inv.slots_)
    {
        const Item* const item = slot.item;

        if (item)
        {
            const ItemData& d = item->data();

            if (d.ranged.is_throwable_wpn)
            {
                slots_.push_back(slot.id);
            }
        }
    }

    // Filter backpack
    const auto& backpack = inv.backpack_;

    filtered_backpack_indexes_.clear();

    for (size_t i = 0; i < backpack.size(); ++i)
    {
        const auto* const item = backpack[i];

        const auto& d = item->data();

        if (d.ranged.is_throwable_wpn)
        {
            filtered_backpack_indexes_.push_back(i);
        }
    }

    const int list_size =
        (int)slots_.size() +
        (int)filtered_backpack_indexes_.size();

    if (list_size == 0)
    {
        // Nothing to throw, exit screen
        states::pop();

        msg_log::add("I carry no throwing weapons.");

        return;
    }

    browser_.reset(list_size, inv_h_);

    audio::play(SfxId::backpack);
}

void SelectThrow::draw()
{
    io::draw_text_center("Throw which item?" + drop_info_str,
                         Panel::log,
                         P(screen_w / 2, 0),
                         colors::title());

    const int browser_y = browser_.y();

    const Range idx_range_shown = browser_.range_shown();

    int y = inv_y0_;

    char key = 'a';

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const bool is_marked = browser_y == i;

        if (i < (int)slots_.size())
        {
            const SlotId slot_id = slots_[i];

            draw_slot(slot_id,
                      y,
                      key,
                      is_marked,
                      ItemRefAttInf::thrown);
        }
        else // This index is in backpack
        {
            const size_t backpack_idx =
                filtered_backpack_indexes_[i - (int)slots_.size()];

            draw_backpack_item(backpack_idx,
                               y,
                               key,
                               is_marked,
                               ItemRefAttInf::thrown);
        }

        ++key;

        ++y;
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      Panel::screen,
                      P(0, top_more_y_),
                      colors::light_white());
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      Panel::screen,
                      P(0, btm_more_y_),
                      colors::light_white());
    }
}

void SelectThrow::update()
{
    auto inv_type = [&]()
    {
        const InvType inv_type_marked =
            (browser_.y() < (int)slots_.size()) ?
            InvType::slots :
            InvType::backpack;

        return inv_type_marked;
    };

    const auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    const auto inv_type_marked = inv_type();

    Inventory& inv = map::player->inv();

    Item* item;

    // Index relative to the current inventory part (slots/backpack)
    size_t relative_inv_idx;

    if (inv_type_marked == InvType::slots)
    {
        const SlotId slot_id_marked = slots_[browser_.y()];

        relative_inv_idx = (size_t)slot_id_marked;

        InvSlot& slot = inv.slots_[relative_inv_idx];

        item = slot.item;
    }
    else // Backpack item selected
    {
        const size_t relative_browser_idx = browser_.y() - (int)slots_.size();

        relative_inv_idx = filtered_backpack_indexes_[relative_browser_idx];

        item = inv.backpack_[relative_inv_idx];
    }

    switch (action)
    {
    case MenuAction::selected:
    {
        map::player->thrown_item_ = item;

        // Exit screen
        states::pop();

        return;
    }
    break;

    case MenuAction::selected_shift:
    {
        // Exit screen
        states::pop();

        const bool did_drop =
            run_drop_query(inv_type_marked, relative_inv_idx);

        if (did_drop)
        {
            game_time::tick();
        }

        return;
    }
    break;

    case MenuAction::esc:
    case MenuAction::space:
    {
        // Exit screen
        states::pop();

        return;
    }
    break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Select identify state
// -----------------------------------------------------------------------------
void SelectIdentify::on_start()
{
    auto& inv = map::player->inv();

    inv.sort_backpack();

    // Filter slots
    for (InvSlot& slot : inv.slots_)
    {
        const Item* const item = slot.item;

        if (item)
        {
            const ItemData& d = item->data();

            if (!d.is_identified)
            {
                slots_.push_back(slot.id);
            }
        }
    }

    // Filter backpack
    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        const Item* const item = inv.backpack_[i];

        if (item->id() != ItemId::potion_insight)
        {
            const ItemData& d = item->data();

            if (!d.is_identified)
            {
                backpack_indexes_.push_back(i);
            }
        }
    }

    const int list_size =
        (int)slots_.size() +
        (int)backpack_indexes_.size();

    if (list_size == 0)
    {
        // Nothing to identify, exit screen
        states::pop();

        msg_log::add("I carry no unknown objects.");

        return;
    }

    browser_.reset(list_size, inv_h_);

    audio::play(SfxId::backpack);
}

void SelectIdentify::draw()
{
    io::clear_screen();

    const int browser_y = browser_.y();

    io::draw_text_center("Identify which item?",
                         Panel::screen,
                         P(screen_w / 2, 0),
                         colors::title());

    const Range idx_range_shown = browser_.range_shown();

    int y = inv_y0_;

    char key = 'a';

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const bool is_marked = browser_y == i;

        if (i < (int)slots_.size())
        {
            const SlotId slot_id = slots_[i];

            draw_slot(slot_id,
                      y,
                      key,
                      is_marked,
                      ItemRefAttInf::wpn_main_att_mode);
        }
        else // This index is in backpack
        {
            const size_t backpack_idx =
                backpack_indexes_[i - (int)slots_.size()];

            draw_backpack_item(backpack_idx,
                               y,
                               key,
                               is_marked,
                               ItemRefAttInf::wpn_main_att_mode);
        }

        ++key;

        ++y;
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      Panel::screen,
                      P(0, top_more_y_),
                      colors::light_white());
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      Panel::screen,
                      P(0, btm_more_y_),
                      colors::light_white());
    }
}

void SelectIdentify::update()
{
    auto inv_type = [&]()
    {
        const InvType inv_type_marked =
            (browser_.y() < (int)slots_.size()) ?
            InvType::slots :
            InvType::backpack;

        return inv_type_marked;
    };

    const auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    {
        const auto inv_type_marked = inv_type();

        Inventory& inv = map::player->inv();

        Item* item_to_identify;

        if (inv_type_marked == InvType::slots)
        {
            const SlotId slot_id_marked = slots_[browser_.y()];

            InvSlot& slot = inv.slots_[(size_t)slot_id_marked];

            item_to_identify = slot.item;
        }
        else // Backpack item selected
        {
            const size_t relative_browser_idx =
                browser_.y() - (int)slots_.size();

            const size_t backpack_idx_marked =
                backpack_indexes_[relative_browser_idx];

            item_to_identify = inv.backpack_[backpack_idx_marked];
        }

        // Exit screen
        states::pop();

        io::clear_screen();

        map::update_vision();

        // Identify item
        item_to_identify->identify(Verbosity::verbose);

        return;
    }
    break;

    default:
        break;

    } // action switch
}
