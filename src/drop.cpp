#include "drop.hpp"

#include <algorithm>
#include <string>

#include "init.hpp"
#include "game_time.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "render.hpp"


namespace item_drop
{

void drop_all_characters_items(Actor& actor)
{
    actor.inv().drop_all_non_intrinsic(actor.pos);
}

void try_drop_item_from_inv(Actor& actor,
                            const InvType inv_type,
                            const size_t idx,
                            const int nr_items_to_drop)
{
    Inventory&  inv             = actor.inv();
    Item*       item_to_drop    = nullptr;

    if (inv_type == InvType::slots)
    {
        ASSERT(idx != size_t(SlotId::END));
        item_to_drop = inv.slots_[idx].item;
    }
    else //Backpack item
    {
        ASSERT(idx < inv.backpack_.size());
        item_to_drop = inv.backpack_[idx];
    }

    if (item_to_drop)
    {
        const ItemDataT&  data = item_to_drop->data();

        const bool  is_stackable            = data.is_stackable;
        const int   nr_items_before_drop    = item_to_drop->nr_items_;

        const bool  is_whole_stack_dropped  = !is_stackable             ||
                                              nr_items_to_drop == -1    ||
                                              (nr_items_to_drop >= nr_items_before_drop);

        std::string item_ref = "";

        if (inv_type == InvType::slots && is_whole_stack_dropped)
        {
            if (item_to_drop->on_unequip() == UnequipAllowed::no)
            {
                return;
            }
        }

        if (is_whole_stack_dropped)
        {
            item_ref = item_to_drop->name(ItemRefType::plural);
            inv.remove_without_destroying(inv_type, idx);

            drop_item_on_map(actor.pos, *item_to_drop);

            item_to_drop->on_removed_from_inv();
        }
        else //Only some items are dropped
        {
            Item* item_to_keep      = item_to_drop;
            item_to_drop            = item_factory::copy_item(*item_to_keep);
            item_to_drop->nr_items_ = nr_items_to_drop;
            item_ref                = item_to_drop->name(ItemRefType::plural);
            item_to_keep->nr_items_ = nr_items_before_drop - nr_items_to_drop;

            drop_item_on_map(actor.pos, *item_to_drop);

            item_to_drop->on_removed_from_inv();
        }

        //Messages
        if (&actor == map::player)
        {
            msg_log::clear();
            render::draw_map_state();
            msg_log::add("I drop " + item_ref + ".",
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
        }
        else //Monster is dropping item
        {
            if (map::player->can_see_actor(actor))
            {
                msg_log::add(actor.name_the() + " drops " + item_ref + ".");
            }
        }

        //End turn
        game_time::tick();
    }
}

Item* drop_item_on_map(const P& intended_pos, Item& item)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    ASSERT(map::is_pos_inside_map(intended_pos, false));

    //If target cell is bottomless, just destroy the item
    const auto* const tgt_rigid =
        map::cells[intended_pos.x][intended_pos.y].rigid;

    if (tgt_rigid->is_bottomless())
    {
        delete &item;

        TRACE_FUNC_END_VERBOSE;
        return nullptr;
    }

    //Make a vector of all cells on map with no blocking feature
    bool free_cell_array[map_w][map_h];

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Rigid* const f = map::cells[x][y].rigid;

            free_cell_array[x][y] = f->can_have_item() && !f->is_bottomless();
        }
    }

    std::vector<P> free_cells;
    to_vec(free_cell_array, true, free_cells);

    if (free_cells.empty())
    {
        //No cells found were items could be placed - too bad!
        delete &item;

        TRACE_FUNC_END_VERBOSE;
        return nullptr;
    }

    //Sort the vector according to distance to origin
    IsCloserToPos is_closer_to_origin(intended_pos);

    sort(begin(free_cells), end(free_cells), is_closer_to_origin);

    const bool is_stackable_type = item.data().is_stackable;

    int dist_searched_stackable = -1;

    //If the item is stackable, and there is a cell A in which the item can be
    //stacked, and a cell B which is empty, and A and B are of equal distance
    //to the origin, then we ALWAYS prefer cell A.
    //In other words, try to drop as near as possible, but prefer stacking.
    for (auto outer_it = begin(free_cells); outer_it != end(free_cells); ++outer_it)
    {
        const P& p = *outer_it;

        if (is_stackable_type)
        {
            const int dist = king_dist(intended_pos, p);

            ASSERT(dist >= dist_searched_stackable);

            //Have we searched at this distance before?
            if (dist > dist_searched_stackable)
            {
                //Search each cell which have equal distance to the current distance
                for (auto stack_it = outer_it; stack_it != end(free_cells); ++stack_it)
                {
                    const P& stack_p = *stack_it;

                    const int stack_dist = king_dist(intended_pos, stack_p);

                    ASSERT(stack_dist >= dist);

                    if (stack_dist > dist)
                    {
                        break;
                    }

                    Item* item_on_floor = map::cells[stack_p.x][stack_p.y].item;

                    if (
                        item_on_floor &&
                        item_on_floor->data().id == item.data().id)
                    {
                        TRACE_VERBOSE << "Stacking item with item on floor" << std::endl;

                        item.nr_items_ += item_on_floor->nr_items_;

                        delete item_on_floor;

                        map::cells[stack_p.x][stack_p.y].item = &item;

                        TRACE_FUNC_END_VERBOSE;
                        return &item;
                    }
                } //Stack position loop

                dist_searched_stackable = dist;
            }
        }

        // Item has not been stacked at this distance

        if (!map::cells[p.x][p.y].item)
        {
            // Alright, this cell is empty, let's put the item here
            map::cells[p.x][p.y].item = &item;

            return &item;
        }
    } //Free cells loop

    //All free cells occupied by other items (and stacking failed) - too bad!
    delete &item;

    TRACE_FUNC_END_VERBOSE;
    return nullptr;
}

} //item_drop
