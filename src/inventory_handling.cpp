#include "inventory_handling.hpp"

#include "init.hpp"

#include <vector>

#include "item_scroll.hpp"
#include "actor_player.hpp"
#include "item_potion.hpp"
#include "menu_browser.hpp"
#include "msg_log.hpp"
#include "render_inventory.hpp"
#include "menu_input_handling.hpp"
#include "render.hpp"
#include "drop.hpp"
#include "query.hpp"
#include "item_factory.hpp"
#include "audio.hpp"
#include "map.hpp"

using namespace std;

namespace inv_handling
{

Inv_scr_id  scr_to_open_after_drop     = Inv_scr_id::END;
Inv_slot*  equip_slot_to_open_after_drop  = nullptr;
int       browser_idx_to_set_after_drop  = 0;

namespace
{

//The values in this vector refer to general inventory elements
vector<size_t> general_items_to_show_;

bool run_drop_screen(const Inv_type inv_type, const size_t ELEMENT)
{
    TRACE_FUNC_BEGIN;

    Inventory& inv  = map::player->inv();
    Item* item      = nullptr;

    if (inv_type == Inv_type::slots)
    {
        assert(ELEMENT != int(Slot_id::END));
        item = inv.slots_[ELEMENT].item;
    }
    else
    {
        assert(ELEMENT < inv.general_.size());
        item = inv.general_[ELEMENT];
    }

    if (!item)
    {
        TRACE_FUNC_END;
        return false;
    }

    const Item_data_t& data = item->data();

    msg_log::clear();

    if (data.is_stackable && item->nr_items_ > 1)
    {
        TRACE << "Item is stackable and more than one" << endl;
        render::draw_map_and_interface(false);
        const string nr_str = "1-" + to_str(item->nr_items_);
        string drop_str = "Drop how many (" + nr_str + ")?:";
        render::draw_text(drop_str, Panel::screen, Pos(0, 0), clr_white_high);
        render::update_screen();
        const Pos nr_query_pos(drop_str.size() + 1, 0);
        const int MAX_DIGITS = 3;
        const Pos done_inf_pos = nr_query_pos + Pos(MAX_DIGITS + 2, 0);
        render::draw_text("[enter] to drop" + cancel_info_str, Panel::screen, done_inf_pos,
                          clr_white_high);
        const int NR_TO_DROP = query::number(nr_query_pos, clr_white_high, 0, 3,
                                             item->nr_items_, false);

        if (NR_TO_DROP <= 0)
        {
            TRACE << "Nr to drop <= 0, nothing to be done" << endl;
            TRACE_FUNC_END;
            return false;
        }
        else
        {
            item_drop::try_drop_item_from_inv(*map::player, inv_type, ELEMENT, NR_TO_DROP);
            TRACE_FUNC_END;
            return true;
        }
    }
    else //Not a stack
    {
        TRACE << "Item not stackable, or only one item" << endl;
        item_drop::try_drop_item_from_inv(*map::player, inv_type, ELEMENT);
        TRACE_FUNC_END;
        return true;
    }

    TRACE_FUNC_END;
    return false;
}

void filter_player_general_equip(const Slot_id slot_to_equip)
{
    assert(slot_to_equip != Slot_id::END);

    const auto& inv     = map::player->inv();
    const auto& general = inv.general_;

    general_items_to_show_.clear();

    for (size_t i = 0; i < general.size(); ++i)
    {
        const auto* const item = general[i];
        const auto&       data = item->data();

        switch (slot_to_equip)
        {
        case Slot_id::wielded:
            if (data.melee.is_melee_wpn || data.ranged.is_ranged_wpn)
            {
                general_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::wielded_alt:
            if (data.melee.is_melee_wpn || data.ranged.is_ranged_wpn)
            {
                general_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::thrown:
            general_items_to_show_.push_back(i);
            break;

        case Slot_id::body:
            if (data.type == Item_type::armor)
            {
                general_items_to_show_.push_back(i);
            }

            break;

        case Slot_id::head:
            if (data.type == Item_type::head_wear)
            {
                general_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::neck:
            if (data.type == Item_type::amulet)
            {
                general_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::ring1:
            if (data.type == Item_type::ring)
            {
                general_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::ring2:
            if (data.type == Item_type::ring)
            {
                general_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::END:
            break;
        }
    }
}

void filter_player_general_show_all()
{
    auto& general = map::player->inv().general_;
    general_items_to_show_.clear();
    const int NR_GEN = general.size();

    for (int i = 0; i < NR_GEN; ++i) {general_items_to_show_.push_back(i);}
}

void swap_items(Item** item1, Item** item2)
{
    Item* buffer  = *item1;
    *item1        = *item2;
    *item2        = buffer;
}

} //namespace

void init()
{
    scr_to_open_after_drop     = Inv_scr_id::END;
    equip_slot_to_open_after_drop  = nullptr;
    browser_idx_to_set_after_drop  = 0;
}

void activate(const size_t GENERAL_ITEMS_ELEMENT)
{
    Inventory& player_inv  = map::player->inv();
    Item* item            = player_inv.general_[GENERAL_ITEMS_ELEMENT];

    if (item->activate(map::player) == Consume_item::yes)
    {
        player_inv.decr_item_in_general(GENERAL_ITEMS_ELEMENT);
    }
}

void run_inv_screen()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    scr_to_open_after_drop = Inv_scr_id::END;
    render::draw_map_and_interface();

    Inventory& inv = map::player->inv();

    inv.sort_general_inventory();

    const int SLOTS_SIZE  = int(Slot_id::END);
    const int INV_H       = render_inventory::INV_H;

    auto mk_browser = [](const Inventory & inventory)
    {
        const int GEN_SIZE        = int(inventory.general_.size());
        const int ELEM_ON_WRAP_UP = GEN_SIZE > INV_H ? (SLOTS_SIZE + INV_H - 2) : -1;
        return Menu_browser(int(Slot_id::END) + GEN_SIZE, 0, ELEM_ON_WRAP_UP);
    };

    Menu_browser browser = mk_browser(inv);

    browser.set_pos(Pos(0, browser_idx_to_set_after_drop));
    browser_idx_to_set_after_drop = 0;
    render_inventory::draw_browse_inv(browser);

    while (true)
    {
        inv.sort_general_inventory();

        const Inv_type inv_type = browser.pos().y < int(Slot_id::END) ?
                                  Inv_type::slots : Inv_type::general;

        const Menu_action action = menu_input_handling::action(browser);

        switch (action)
        {
        case Menu_action::browsed:
        {
            render_inventory::draw_browse_inv(browser);
        } break;

        case Menu_action::selected_shift:
        {
            const int       BROWSER_Y   = browser.pos().y;
            const size_t    ELEMENT     = inv_type == Inv_type::slots ?
                                          BROWSER_Y : (BROWSER_Y - int(Slot_id::END));

            if (run_drop_screen(inv_type, ELEMENT))
            {
                browser.set_good_pos();
                browser_idx_to_set_after_drop  = browser.y();
                scr_to_open_after_drop     = Inv_scr_id::inv;

                TRACE_FUNC_END_VERBOSE;
                return;
            }

            render_inventory::draw_browse_inv(browser);
        } break;

        case Menu_action::selected:
        {
            if (inv_type == Inv_type::slots)
            {
                const size_t ELEMENT  = browser.y();
                Inv_slot& slot         = inv.slots_[ELEMENT];

                if (slot.item)
                {
                    msg_log::clear();

                    const Unequip_allowed unequip_allowed = slot.item->on_unequip();

                    if (unequip_allowed == Unequip_allowed::yes)
                    {
                        inv.move_to_general(slot.id);
                        inv.sort_general_inventory();
                    }

                    switch (Slot_id(ELEMENT))
                    {
                    case Slot_id::wielded:
                    case Slot_id::wielded_alt:
                    case Slot_id::thrown:
                    case Slot_id::head:
                    case Slot_id::neck:
                    case Slot_id::ring1:
                    case Slot_id::ring2:
                        render_inventory::draw_browse_inv(browser);
                        break;

                    case Slot_id::body:
                        scr_to_open_after_drop     = Inv_scr_id::inv;
                        browser_idx_to_set_after_drop  = browser.y();

                        TRACE_FUNC_END_VERBOSE;
                        return;

                    case Slot_id::END:
                        break;
                    }

                    //Create a new browser to adjust for changed inventory size
                    const Pos p = browser.pos();
                    browser     = mk_browser(inv);
                    browser.set_pos(p);
                }
                else //No item in slot
                {
                    if (run_equip_screen(slot))
                    {
                        render::draw_map_and_interface();

                        TRACE_FUNC_END_VERBOSE;
                        return;
                    }
                    else
                    {
                        render_inventory::draw_browse_inv(browser);
                    }
                }
            }
            else //In general inventory
            {
                const size_t ELEMENT = browser.y() - int(Slot_id::END);
                activate(ELEMENT);
                render::draw_map_and_interface();

                TRACE_FUNC_END_VERBOSE;
                return;
            }
        } break;

        case Menu_action::esc:
        case Menu_action::space:
            render::draw_map_and_interface();

            TRACE_FUNC_END_VERBOSE;
            return;
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

bool run_equip_screen(Inv_slot& slot_to_equip)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    scr_to_open_after_drop       = Inv_scr_id::END;
    equip_slot_to_open_after_drop   = &slot_to_equip;
    render::draw_map_and_interface();

    auto& inv = map::player->inv();

    inv.sort_general_inventory();

    filter_player_general_equip(slot_to_equip.id);

    Menu_browser browser(general_items_to_show_.size(), 0);
    browser.set_pos(Pos(0, browser_idx_to_set_after_drop));
    browser_idx_to_set_after_drop = 0;

    audio::play(Sfx_id::backpack);

    render_inventory::draw_equip(browser, slot_to_equip.id, general_items_to_show_);

    while (true)
    {
        const Menu_action action = menu_input_handling::action(browser);

        switch (action)
        {
        case Menu_action::browsed:
        {
            render_inventory::draw_equip(browser, slot_to_equip.id, general_items_to_show_);
        } break;

        case Menu_action::selected:
        {
            if (!general_items_to_show_.empty())
            {
                const int ELEMENT = general_items_to_show_[browser.y()];
                render::draw_map_and_interface();

                inv.equip_general_item(ELEMENT, slot_to_equip.id);

                slot_to_equip.item->on_equip(false);

                game_time::tick();

                browser_idx_to_set_after_drop  = int(slot_to_equip.id);
                scr_to_open_after_drop     = Inv_scr_id::inv;

                TRACE_FUNC_END_VERBOSE;
                return true;
            }
        } break;

        case Menu_action::selected_shift:
        {
            if (run_drop_screen(Inv_type::general, general_items_to_show_[browser.y()]))
            {
                browser.set_good_pos();
                browser_idx_to_set_after_drop  = browser.y();
                scr_to_open_after_drop     = Inv_scr_id::equip;

                TRACE_FUNC_END_VERBOSE;
                return true;
            }

            render_inventory::draw_equip(browser, slot_to_equip.id, general_items_to_show_);
        } break;

        case Menu_action::esc:
        case Menu_action::space: {return false;} break;
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

} //Inv_handling
