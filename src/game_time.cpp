#include "game_time.hpp"

#include <vector>
#include <cassert>

#include "cmn_types.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "populate_monsters.hpp"
#include "input.hpp"
#include "inventory.hpp"
#include "inventory_handling.hpp"
#include "player_bon.hpp"
#include "audio.hpp"
#include "map_parsing.hpp"
#include "render.hpp"
#include "utils.hpp"
#include "map_travel.hpp"
#include "item.hpp"
#include "save_handling.hpp"

namespace game_time
{

std::vector<Actor*> actors_;
std::vector<Mob*>   mobs_;

namespace
{

std::vector<Actor_speed> turn_type_vector_;

int     cur_turn_type_pos_  = 0;
size_t  cur_actor_index_    = 0;
int     turn_nr_            = 0;

bool is_spi_regen_this_turn(const int REGEN_N_TURNS)
{
    assert(REGEN_N_TURNS != 0);
    return turn_nr_ == (turn_nr_ / REGEN_N_TURNS) * REGEN_N_TURNS;
}

void run_std_turn_events()
{
    ++turn_nr_;

    for (size_t i = 0; i < actors_.size(); ++i)
    {
        Actor* const actor = actors_[i];

        //Delete destroyed actors
        if (actor->state() == Actor_state::destroyed)
        {
            //Do not delete player if player died, just return
            if (actor == map::player)
            {
                return;
            }

            if (map::player->tgt_ == actor)
            {
                map::player->tgt_ = nullptr;
            }

            delete actor;

            actors_.erase(actors_.begin() + i);
            --i;

            if (cur_actor_index_ >= actors_.size())
            {
                cur_actor_index_ = 0;
            }
        }
        else  //Actor is alive or is a corpse
        {
            actor->prop_handler().tick(Prop_turn_mode::std);

            if (!actor->is_player())
            {
                Mon* const mon = static_cast<Mon*>(actor);

                if (mon->player_aware_of_me_counter_ > 0)
                {
                    --(mon->player_aware_of_me_counter_);
                }
            }

            //Do light damage if actor in lit cell
            const Pos& pos = actor->pos;

            if (map::cells[pos.x][pos.y].is_lit)
            {
                actor->hit(1, Dmg_type::light);
            }

            if (actor->is_alive())
            {
                //Regen Spi

                int regen_spi_n_turns = 15;

                if (actor->is_player())
                {
                    if (player_bon::traits[size_t(Trait::stout_spirit)])
                    {
                        regen_spi_n_turns -= 3;
                    }

                    if (player_bon::traits[size_t(Trait::strong_spirit)])
                    {
                        regen_spi_n_turns -= 3;
                    }

                    if (player_bon::traits[size_t(Trait::mighty_spirit)])
                    {
                        regen_spi_n_turns -= 3;
                    }
                }
                else //Is monster
                {
                    //Monsters regen spirit very quickly, so spell casters don't suddenly get
                    //completely handicapped
                    regen_spi_n_turns = 2;
                }

                if (is_spi_regen_this_turn(regen_spi_n_turns))
                {
                    actor->restore_spi(1, false, Verbosity::silent);
                }

                actor->on_std_turn();
            }
        }
    }

    //New turn for rigids
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            map::cells[x][y].rigid->on_new_turn();
        }
    }

    //New turn for mobs (using a copied vector, since mobs may get destroyed)
    const std::vector<Mob*> mobs_cpy = mobs_;

    for (auto* f : mobs_cpy)
    {
        f->on_new_turn();
    }

    //Spawn more monsters?
    //(If an unexplored cell is selected, the spawn is canceled)
    if (map::dlvl >= 1 && map::dlvl <= DLVL_LAST)
    {
        const int SPAWN_N_TURNS = 130;

        if (turn_nr_ % SPAWN_N_TURNS == 0)
        {
            populate_mon::try_spawn_due_to_time_passed();
        }
    }

    //Run new turn events on all player items
    auto& player_inv = map::player->inv();

    for (Item* const item : player_inv.backpack_)
    {
        item->on_std_turn_in_inv(Inv_type::backpack);
    }

    for (Inv_slot& slot : player_inv.slots_)
    {
        if (slot.item)
        {
            slot.item->on_std_turn_in_inv(Inv_type::slots);
        }
    }

    snd_emit::reset_nr_snd_msg_printed_cur_turn();

    if (map::dlvl > 0)
    {
        audio::try_play_amb(100);
    }
}

void run_atomic_turn_events()
{
    update_light_map();
}

} //namespace

void init()
{
    cur_turn_type_pos_ = cur_actor_index_ = turn_nr_ = 0;
    actors_.clear();
    mobs_  .clear();
}

void cleanup()
{
    for (Actor* a : actors_)
    {
        delete a;
    }

    actors_.clear();

    for (auto* f : mobs_)
    {
        delete f;
    }

    mobs_.clear();
}

void save()
{
    save_handling::put_int(turn_nr_);
}

void load()
{
    turn_nr_ = save_handling::get_int();
}

int turn()
{
    return turn_nr_;
}

void mobs_at_pos(const Pos& p, std::vector<Mob*>& vector_ref)
{
    vector_ref.clear();

    for (auto* m : mobs_)
    {
        if (m->pos() == p)
        {
            vector_ref.push_back(m);
        }
    }
}

void add_mob(Mob* const f)
{
    mobs_.push_back(f);
}

void erase_mob(Mob* const f, const bool DESTROY_OBJECT)
{
    for (auto it = mobs_.begin(); it != mobs_.end(); ++it)
    {
        if (*it == f)
        {
            if (DESTROY_OBJECT)
            {
                delete f;
            }

            mobs_.erase(it);
            return;
        }
    }
}

void erase_all_mobs()
{
    for (auto* m : mobs_)
    {
        delete m;
    }

    mobs_.clear();
}

void erase_actor_in_element(const size_t i)
{
    if (!actors_.empty())
    {
        delete actors_[i];
        actors_.erase(actors_.begin() + i);
    }
}

void add_actor(Actor* actor)
{
    //Sanity check actor inserted
    assert(utils::is_pos_inside_map(actor->pos));
    actors_.push_back(actor);
}

void reset_turn_type_and_actor_counters()
{
    cur_turn_type_pos_ = cur_actor_index_ = 0;
}

//For every turn type step, run through all actors and let those who can act during this
//type of turn act. When all actors who can act on this phase have acted, and if this is
//a normal speed phase - consider it a standard turn (update properties, update features,
//spawn more monsters etc.)
void tick(const bool IS_FREE_TURN)
{
    run_atomic_turn_events();

    auto* actor = cur_actor();

    if (actor == map::player)
    {
        map::player->update_fov();
        render::draw_map_and_interface();
        map::cpy_render_array_to_visual_memory();

        //Run new turn events on all player items
        auto& inv = map::player->inv();

        for (Item* const item : inv.backpack_)
        {
            item->on_actor_turn_in_inv(Inv_type::backpack);
        }

        for (Inv_slot& slot : inv.slots_)
        {
            if (slot.item)
            {
                slot.item->on_actor_turn_in_inv(Inv_type::slots);
            }
        }
    }
    else //Actor is monster
    {
        auto* mon = static_cast<Mon*>(actor);

        if (mon->aware_counter_ > 0)
        {
            --mon->aware_counter_;
        }
    }

    //Tick properties running on actor turns
    actor->prop_handler().tick(Prop_turn_mode::actor);

    if (!IS_FREE_TURN)
    {
        bool can_act = false;

        while (!can_act)
        {
            auto cur_turn_type = Turn_type(cur_turn_type_pos_);

            ++cur_actor_index_;

            if (cur_actor_index_ >= actors_.size())
            {
                cur_actor_index_ = 0;

                ++cur_turn_type_pos_;

                if (cur_turn_type_pos_ == int(Turn_type::END))
                {
                    cur_turn_type_pos_ = 0;
                }

                if (cur_turn_type != Turn_type::fast && cur_turn_type != Turn_type::fastest)
                {
                    run_std_turn_events();
                }
            }

            const auto speed = cur_actor()->speed();

            switch (speed)
            {
            case Actor_speed::sluggish:
                can_act = (cur_turn_type == Turn_type::slow ||
                           cur_turn_type == Turn_type::normal2)
                          && rnd::fraction(2, 3);
                break;

            case Actor_speed::slow:
                can_act = cur_turn_type == Turn_type::slow ||
                          cur_turn_type == Turn_type::normal2;
                break;

            case Actor_speed::normal:
                can_act = cur_turn_type != Turn_type::fast &&
                          cur_turn_type != Turn_type::fastest;
                break;

            case Actor_speed::fast:
                can_act = cur_turn_type != Turn_type::fastest;
                break;

            case Actor_speed::fastest:
                can_act = true;
                break;

            case Actor_speed::END:
                assert(false);
                break;
            }
        }
    }
}

void update_light_map()
{
    bool light_tmp[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            map::cells[x][y].is_lit = light_tmp[x][y] = false;
        }
    }

    //Do not add light on Leng
    if (map_travel::map_type() == Map_type::leng)
    {
        return;
    }

    for (const auto* const a : actors_)  {a->add_light(light_tmp);}

    for (const auto* const m : mobs_)    {m->add_light(light_tmp);}

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            auto* rigid = map::cells[x][y].rigid;

            if (rigid->burn_state() == Burn_state::burning)
            {
                light_tmp[x][y] = true;
            }

            rigid->add_light(light_tmp);

            //Copy the temp values to the real light map.
            map::cells[x][y].is_lit = light_tmp[x][y];
        }
    }
}

Actor* cur_actor()
{
    Actor* const actor = actors_[cur_actor_index_];

    //Sanity check actor retrieved
    assert(utils::is_pos_inside_map(actor->pos));
    return actor;
}

} //game_time
