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

Inv_scr     scr_to_open_on_new_turn          = Inv_scr::END;
Inv_slot*   equip_slot_to_open_on_new_turn   = nullptr;
int         browser_idx_to_set_on_new_turn   = 0;

namespace
{

//The values in this vector refer to backpack inventory elements
vector<size_t> backpack_items_to_show_;

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
        assert(ELEMENT < inv.backpack_.size());
        item = inv.backpack_[ELEMENT];
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

void filter_player_backpack_equip(const Slot_id slot_to_equip)
{
    assert(slot_to_equip != Slot_id::END);

    const auto& inv     = map::player->inv();
    const auto& backpack = inv.backpack_;

    backpack_items_to_show_.clear();

    for (size_t i = 0; i < backpack.size(); ++i)
    {
        const auto* const item = backpack[i];
        const auto&       data = item->data();

        switch (slot_to_equip)
        {
        case Slot_id::wielded:
            if (data.melee.is_melee_wpn || data.ranged.is_ranged_wpn)
            {
                backpack_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::wielded_alt:
            if (data.melee.is_melee_wpn || data.ranged.is_ranged_wpn)
            {
                backpack_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::thrown:
            backpack_items_to_show_.push_back(i);
            break;

        case Slot_id::body:
            if (data.type == Item_type::armor)
            {
                backpack_items_to_show_.push_back(i);
            }

            break;

        case Slot_id::head:
            if (data.type == Item_type::head_wear)
            {
                backpack_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::neck:
            if (data.type == Item_type::amulet)
            {
                backpack_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::ring1:
            if (data.type == Item_type::ring)
            {
                backpack_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::ring2:
            if (data.type == Item_type::ring)
            {
                backpack_items_to_show_.push_back(i);
            }
            break;

        case Slot_id::END:
            break;
        }
    }
}

void filter_player_backpack_show_all()
{
    auto& backpack = map::player->inv().backpack_;
    backpack_items_to_show_.clear();
    const int NR_GEN = backpack.size();

    for (int i = 0; i < NR_GEN; ++i) {backpack_items_to_show_.push_back(i);}
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
    scr_to_open_on_new_turn         = Inv_scr::END;
    equip_slot_to_open_on_new_turn  = nullptr;
    browser_idx_to_set_on_new_turn  = 0;
}

void activate(const size_t GENERAL_ITEMS_ELEMENT)
{
    Inventory&  player_inv  = map::player->inv();
    Item*       item        = player_inv.backpack_[GENERAL_ITEMS_ELEMENT];

    if (item->activate(map::player) == Consume_item::yes)
    {
        player_inv.decr_item_in_backpack(GENERAL_ITEMS_ELEMENT);
    }
}

void run_inv_screen()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    scr_to_open_on_new_turn = Inv_scr::END;
    render::draw_map_and_interface();

    Inventory& inv = map::player->inv();

    inv.sort_backpack();

    const int SLOTS_SIZE  = int(Slot_id::END);
    const int INV_H       = render_inventory::INV_H;

    auto mk_browser = [](const Inventory & inventory)
    {
        const int GEN_SIZE        = int(inventory.backpack_.size());
        const int ELEM_ON_WRAP_UP = GEN_SIZE > INV_H ? (SLOTS_SIZE + INV_H - 2) : -1;
        return Menu_browser(int(Slot_id::END) + GEN_SIZE, 0, ELEM_ON_WRAP_UP);
    };

    Menu_browser browser = mk_browser(inv);

    browser.set_pos(Pos(0, browser_idx_to_set_on_new_turn));
    browser_idx_to_set_on_new_turn = 0;
    render_inventory::draw_browse_inv(browser);

    while (true)
    {
        inv.sort_backpack();

        const Inv_type inv_type = browser.pos().y < int(Slot_id::END) ?
                                  Inv_type::slots : Inv_type::backpack;

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
                browser_idx_to_set_on_new_turn  = browser.y();
                scr_to_open_on_new_turn         = Inv_scr::inv;

                TRACE_FUNC_END_VERBOSE;
                return;
            }

            render_inventory::draw_browse_inv(browser);
        } break;

        case Menu_action::selected:
        {
            if (inv_type == Inv_type::slots)
            {
                const size_t ELEMENT    = browser.y();
                Inv_slot& slot          = inv.slots_[ELEMENT];

                if (slot.item)
                {
                    msg_log::clear();

                    const Unequip_allowed unequip_allowed = inv.try_unequip_slot(slot.id);

                    if (unequip_allowed == Unequip_allowed::yes)
                    {
                        game_time::tick();
                    }

                    scr_to_open_on_new_turn         = Inv_scr::inv;
                    browser_idx_to_set_on_new_turn  = browser.y();

                    TRACE_FUNC_END_VERBOSE;
                    return;
                }
                else //No item in slot
                {
                    if (run_equip_screen(slot))
                    {
                        render::draw_map_and_interface();

                        TRACE_FUNC_END_VERBOSE;
                        return;
                    }
                    else //No item equipped
                    {
                        render_inventory::draw_browse_inv(browser);
                    }
                }
            }
            else //In backpack inventory
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

    scr_to_open_on_new_turn          = Inv_scr::END;
    equip_slot_to_open_on_new_turn   = &slot_to_equip;
    render::draw_map_and_interface();

    auto& inv = map::player->inv();

    inv.sort_backpack();

    filter_player_backpack_equip(slot_to_equip.id);

    Menu_browser browser(backpack_items_to_show_.size(), 0);
    browser.set_pos(Pos(0, browser_idx_to_set_on_new_turn));
    browser_idx_to_set_on_new_turn = 0;

    audio::play(Sfx_id::backpack);

    render_inventory::draw_equip(browser, slot_to_equip.id, backpack_items_to_show_);

    while (true)
    {
        const Menu_action action = menu_input_handling::action(browser);

        switch (action)
        {
        case Menu_action::browsed:
        {
            render_inventory::draw_equip(browser, slot_to_equip.id, backpack_items_to_show_);
        }
        break;

        case Menu_action::selected:
        {
            if (!backpack_items_to_show_.empty())
            {
                const int ELEMENT = backpack_items_to_show_[browser.y()];
                render::draw_map_and_interface();

                inv.equip_backpack_item(ELEMENT, slot_to_equip.id); //Calls the items equip hook

                game_time::tick();

                browser_idx_to_set_on_new_turn   = int(slot_to_equip.id);
                scr_to_open_on_new_turn          = Inv_scr::inv;

                TRACE_FUNC_END_VERBOSE;
                return true;
            }
        }
        break;

        case Menu_action::selected_shift:
        {
            if (run_drop_screen(Inv_type::backpack, backpack_items_to_show_[browser.y()]))
            {
                browser.set_good_pos();
                browser_idx_to_set_on_new_turn   = browser.y();
                scr_to_open_on_new_turn          = Inv_scr::equip;

                TRACE_FUNC_END_VERBOSE;
                return true;
            }

            render_inventory::draw_equip(browser, slot_to_equip.id, backpack_items_to_show_);
        }
        break;

        case Menu_action::esc:
        case Menu_action::space:
            return false;
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

} //Inv_handling
