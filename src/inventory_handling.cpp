#include "inventory_handling.h"

#include "init.h"

#include <vector>

#include "item_scroll.h"
#include "actor_player.h"
#include "item_potion.h"
#include "menu_browser.h"
#include "log.h"
#include "render_inventory.h"
#include "menu_input_handling.h"
#include "render.h"
#include "drop.h"
#include "query.h"
#include "item_factory.h"
#include "audio.h"
#include "map.h"

using namespace std;

namespace Inv_handling
{

Inv_scr_id  screen_to_open_after_drop     = Inv_scr_id::END;
Inv_slot*  equip_slot_to_open_after_drop  = nullptr;
int       browser_idx_to_set_after_drop  = 0;

namespace
{

//The values in this vector refer to general inventory elements
vector<size_t> general_items_to_show_;

bool run_drop_screen(const Inv_type inv_type, const size_t ELEMENT)
{
    TRACE_FUNC_BEGIN;

    Inventory& inv  = Map::player->get_inv();
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

    const Item_data_t& data = item->get_data();

    Log::clear_log();

    if (data.is_stackable && item->nr_items_ > 1)
    {
        TRACE << "Item is stackable and more than one" << endl;
        Render::draw_map_and_interface(false);
        const string nr_str = "1-" + to_str(item->nr_items_);
        string drop_str = "Drop how many (" + nr_str + ")?:";
        Render::draw_text(drop_str, Panel::screen, Pos(0, 0), clr_white_high);
        Render::update_screen();
        const Pos nr_query_pos(drop_str.size() + 1, 0);
        const int MAX_DIGITS = 3;
        const Pos done_inf_pos = nr_query_pos + Pos(MAX_DIGITS + 2, 0);
        Render::draw_text("[enter] to drop" + cancel_info_str, Panel::screen, done_inf_pos,
                         clr_white_high);
        const int NR_TO_DROP = Query::number(nr_query_pos, clr_white_high, 0, 3,
                                             item->nr_items_, false);
        if (NR_TO_DROP <= 0)
        {
            TRACE << "Nr to drop <= 0, nothing to be done" << endl;
            TRACE_FUNC_END;
            return false;
        }
        else
        {
            Item_drop::try_drop_item_from_inv(*Map::player, inv_type, ELEMENT, NR_TO_DROP);
            TRACE_FUNC_END;
            return true;
        }
    }
    else //Not a stack
    {
        TRACE << "Item not stackable, or only one item" << endl;
        Item_drop::try_drop_item_from_inv(*Map::player, inv_type, ELEMENT);
        TRACE_FUNC_END;
        return true;
    }
    TRACE_FUNC_END;
    return false;
}

void filter_player_general_equip(const Slot_id slot_to_equip)
{
    assert(slot_to_equip != Slot_id::END);

    const auto& inv     = Map::player->get_inv();
    const auto& general = inv.general_;

    general_items_to_show_.clear();

    for (size_t i = 0; i < general.size(); ++i)
    {
        const auto* const item = general[i];
        const auto&       data = item->get_data();

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
            if (data.ranged.is_throwing_wpn)
            {
                general_items_to_show_.push_back(i);
            }
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

        case Slot_id::END: {}
            break;
        }
    }
}

void filter_player_general_show_all()
{
    auto& general = Map::player->get_inv().general_;
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
    screen_to_open_after_drop     = Inv_scr_id::END;
    equip_slot_to_open_after_drop  = nullptr;
    browser_idx_to_set_after_drop  = 0;
}

void activate(const size_t GENERAL_ITEMS_ELEMENT)
{
    Inventory& player_inv  = Map::player->get_inv();
    Item* item            = player_inv.general_[GENERAL_ITEMS_ELEMENT];

    if (item->activate(Map::player) == Consume_item::yes)
    {
        player_inv.decr_item_in_general(GENERAL_ITEMS_ELEMENT);
    }
}

void run_inv_screen()
{
    screen_to_open_after_drop = Inv_scr_id::END;
    Render::draw_map_and_interface();

    Inventory& inv = Map::player->get_inv();

    inv.sort_general_inventory();

    const int SLOTS_SIZE  = int(Slot_id::END);
    const int INV_H       = Render_inventory::INV_H;

    auto get_browser = [](const Inventory & inventory)
    {
        const int GEN_SIZE        = int(inventory.general_.size());
        const int ELEM_ON_WRAP_UP = GEN_SIZE > INV_H ? (SLOTS_SIZE + INV_H - 2) : -1;
        return Menu_browser(int(Slot_id::END) + GEN_SIZE, 0, ELEM_ON_WRAP_UP);
    };

    Menu_browser browser = get_browser(inv);

    browser.set_pos(Pos(0, browser_idx_to_set_after_drop));
    browser_idx_to_set_after_drop = 0;
    Render_inventory::draw_browse_inv(browser);

    while (true)
    {
        inv.sort_general_inventory();

        const Inv_type inv_type = browser.get_pos().y < int(Slot_id::END) ?
                                Inv_type::slots : Inv_type::general;

        const Menu_action action = Menu_input_handling::get_action(browser);
        switch (action)
        {
        case Menu_action::browsed:
        {
            Render_inventory::draw_browse_inv(browser);
        } break;

        case Menu_action::selected_shift:
        {
            const int BROWSER_Y = browser.get_pos().y;
            const size_t ELEMENT =
                inv_type == Inv_type::slots ? BROWSER_Y : (BROWSER_Y - int(Slot_id::END));
            if (run_drop_screen(inv_type, ELEMENT))
            {
                browser.set_good_pos();
                browser_idx_to_set_after_drop  = browser.get_y();
                screen_to_open_after_drop     = Inv_scr_id::inv;
                return;
            }
            Render_inventory::draw_browse_inv(browser);
        } break;

        case Menu_action::selected:
        {
            if (inv_type == Inv_type::slots)
            {
                const size_t ELEMENT  = browser.get_y();
                Inv_slot& slot         = inv.slots_[ELEMENT];

                if (slot.item)
                {
                    Log::clear_log();

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
                        Render_inventory::draw_browse_inv(browser);
                        break;

                    case Slot_id::body:
                        screen_to_open_after_drop     = Inv_scr_id::inv;
                        browser_idx_to_set_after_drop  = browser.get_y();
                        return;

                    case Slot_id::END: {}
                        break;
                    }

                    //Create a new browser to adjust for changed inventory size
                    const Pos p = browser.get_pos();
                    browser     = get_browser(inv);
                    browser.set_pos(p);
                }
                else //No item in slot
                {
                    if (run_equip_screen(slot))
                    {
                        Render::draw_map_and_interface();
                        return;
                    }
                    else
                    {
                        Render_inventory::draw_browse_inv(browser);
                    }
                }
            }
            else //In general inventory
            {
                const size_t ELEMENT = browser.get_y() - int(Slot_id::END);
                activate(ELEMENT);
                Render::draw_map_and_interface();
                return;
            }
        } break;

        case Menu_action::esc:
        case Menu_action::space:
        {
            Render::draw_map_and_interface();
            return;
        } break;
        }
    }
}

bool run_equip_screen(Inv_slot& slot_to_equip)
{
    screen_to_open_after_drop     = Inv_scr_id::END;
    equip_slot_to_open_after_drop  = &slot_to_equip;
    Render::draw_map_and_interface();

    auto& inv = Map::player->get_inv();

    inv.sort_general_inventory();

    filter_player_general_equip(slot_to_equip.id);

    Menu_browser browser(general_items_to_show_.size(), 0);
    browser.set_pos(Pos(0, browser_idx_to_set_after_drop));
    browser_idx_to_set_after_drop = 0;

    Audio::play(Sfx_id::backpack);

    Render_inventory::draw_equip(browser, slot_to_equip.id, general_items_to_show_);

    while (true)
    {
        const Menu_action action = Menu_input_handling::get_action(browser);
        switch (action)
        {
        case Menu_action::browsed:
        {
            Render_inventory::draw_equip(browser, slot_to_equip.id, general_items_to_show_);
        } break;

        case Menu_action::selected:
        {
            if (!general_items_to_show_.empty())
            {
                const int ELEMENT = general_items_to_show_[browser.get_y()];
                Render::draw_map_and_interface();

                inv.equip_general_item(ELEMENT, slot_to_equip.id);

                slot_to_equip.item->on_equip(false);

                Game_time::tick();

                browser_idx_to_set_after_drop  = int(slot_to_equip.id);
                screen_to_open_after_drop     = Inv_scr_id::inv;
                return true;
            }
        } break;

        case Menu_action::selected_shift:
        {
            if (run_drop_screen(Inv_type::general, general_items_to_show_[browser.get_y()]))
            {
                browser.set_good_pos();
                browser_idx_to_set_after_drop  = browser.get_y();
                screen_to_open_after_drop     = Inv_scr_id::equip;
                return true;
            }
            Render_inventory::draw_equip(browser, slot_to_equip.id, general_items_to_show_);
        } break;

        case Menu_action::esc:
        case Menu_action::space: {return false;} break;
        }
    }
}

} //Inv_handling
