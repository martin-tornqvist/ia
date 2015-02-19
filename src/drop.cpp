#include "Drop.h"

#include <algorithm>
#include <string>
#include <assert.h>

#include "Cmn_types.h"
#include "Game_time.h"
#include "Actor_player.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "Item_factory.h"
#include "Map_parsing.h"
#include "Utils.h"
#include "Feature_rigid.h"
#include "Render.h"

using namespace std;

namespace Item_drop
{

void drop_all_characters_items(Actor& actor)
{
    actor.get_inv().drop_all_non_intrinsic(actor.pos);
}

void try_drop_item_from_inv(Actor& actor, const Inv_type inv_type, const size_t IDX,
                        const int NR_ITEMS_TO_DROP)
{
    Inventory& inv    = actor.get_inv();
    Item* item_to_drop  = nullptr;

    if (inv_type == Inv_type::slots)
    {
        assert(IDX != int(Slot_id::END));
        item_to_drop = inv.slots_[IDX].item;
    }
    else //Backpack item
    {
        assert(IDX < inv.general_.size());
        item_to_drop = inv.general_[IDX];
    }

    if (item_to_drop)
    {
        const bool  IS_STACKABLE            = item_to_drop->get_data().is_stackable;
        const int   NR_ITEMS_BEFORE_DROP    = item_to_drop->nr_items_;
        const bool  IS_WHOLE_STACK_DROPPED  = !IS_STACKABLE || NR_ITEMS_TO_DROP == -1 ||
                                              (NR_ITEMS_TO_DROP >= NR_ITEMS_BEFORE_DROP);

        string item_ref = "";

        if (inv_type == Inv_type::slots && IS_WHOLE_STACK_DROPPED)
        {
            if (item_to_drop->on_unequip() == Unequip_allowed::no)
            {
                return;
            }
        }

        if (IS_WHOLE_STACK_DROPPED)
        {
            item_ref = item_to_drop->get_name(Item_ref_type::plural);
            inv.remove_without_destroying(inv_type, IDX);
            drop_item_on_map(actor.pos, *item_to_drop);
        }
        else
        {
            Item* item_to_keep      = item_to_drop;
            item_to_drop            = Item_factory::copy_item(item_to_keep);
            item_to_drop->nr_items_  = NR_ITEMS_TO_DROP;
            item_ref               = item_to_drop->get_name(Item_ref_type::plural);
            item_to_keep->nr_items_  = NR_ITEMS_BEFORE_DROP - NR_ITEMS_TO_DROP;
            drop_item_on_map(actor.pos, *item_to_drop);
        }

        //Messages
        if (&actor == Map::player)
        {
            Log::clear_log();
            Render::draw_map_and_interface();
            Log::add_msg("I drop " + item_ref + ".", clr_white, false, true);
        }
        else
        {
            bool blocked[MAP_W][MAP_H];
            Map_parse::run(Cell_check::Blocks_los(), blocked);
            if (Map::player->can_see_actor(actor, blocked))
            {
                Log::add_msg(actor.get_name_the() + " drops " + item_ref + ".");
            }
        }

        //End turn
        Game_time::tick();
    }
}

//TODO: This function is really weirdly written, and seems to even be doing
//wrong things. It should be refactored.
Item* drop_item_on_map(const Pos& intended_pos, Item& item)
{
    //If target cell is bottomless, just destroy the item
    const auto* const tgt_rigid = Map::cells[intended_pos.x][intended_pos.y].rigid;
    if (tgt_rigid->is_bottomless())
    {
        delete &item;
        return nullptr;
    }

    //Make a vector of all cells on map with no blocking feature
    bool free_cell_array[MAP_W][MAP_H];
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Rigid* const f = Map::cells[x][y].rigid;
            free_cell_array[x][y] = f->can_have_item() && !f->is_bottomless();
        }
    }
    vector<Pos> free_cells;
    Utils::mk_vector_from_bool_map(true, free_cell_array, free_cells);

    //Sort the vector according to distance to origin
    Is_closer_to_pos is_closer_to_origin(intended_pos);
    sort(begin(free_cells), end(free_cells), is_closer_to_origin);

    Pos cur_pos;
    Pos stack_pos;
    const bool IS_STACKABLE_TYPE = item.get_data().is_stackable;

    size_t          ii          = 0;
    const size_t    VEC_SIZE    = free_cells.size();

    for (size_t i = 0; i < VEC_SIZE; ++i)
    {
        //First look in all cells that has distance to origin equal to cell i to try and
        //merge the item if it stacks
        if (IS_STACKABLE_TYPE)
        {
            //While ii cell is not further away than i cell
            while (!is_closer_to_origin(free_cells[i], free_cells[ii]))
            {
                stack_pos = free_cells[ii];
                Item* item_found_on_floor = Map::cells[stack_pos.x][stack_pos.y].item;
                if (item_found_on_floor)
                {
                    if (item_found_on_floor->get_data().id == item.get_data().id)
                    {
                        item.nr_items_ += item_found_on_floor->nr_items_;
                        delete item_found_on_floor;
                        Map::cells[stack_pos.x][stack_pos.y].item = &item;
                        return &item;
                    }
                }
                ++ii;
            }
        }

        cur_pos = free_cells[i];
        if (!Map::cells[cur_pos.x][cur_pos.y].item)
        {
            Map::cells[cur_pos.x][cur_pos.y].item = &item;

            //TODO: Won't this cause nullptr to be returned?
            //Shouldn't a pointer to the item be returned?
            break;
        }

        if (i == VEC_SIZE - 1)
        {
            delete &item;
            return nullptr;
        }
    }
    return nullptr;
}

} //Item_drop
