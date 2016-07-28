#include "game_time.hpp"

#include <vector>

#include "init.hpp"
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
#include "map_travel.hpp"
#include "item.hpp"
#include "save_handling.hpp"
#include "msg_log.hpp"

namespace game_time
{

std::vector<Actor*> actors;
std::vector<Mob*>   mobs;

bool is_magic_descend_nxt_std_turn;

namespace
{

std::vector<ActorSpeed> turn_type_vector_;

int     current_turn_type_pos_  = 0;
size_t  current_actor_idx_      = 0;
int     turn_nr_            = 0;

void run_std_turn_events()
{
    if (is_magic_descend_nxt_std_turn)
    {
        render::draw_map_state();

        msg_log::add("I sink downwards!",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        map_travel::go_to_nxt();
        return;
    }

    ++turn_nr_;

    for (auto it = begin(actors); it != end(actors); /* No increment */)
    {
        Actor* const actor = *it;

        //Delete destroyed actors
        if (actor->state() == ActorState::destroyed)
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

            it = actors.erase(it);

            if (current_actor_idx_ >= actors.size())
            {
                current_actor_idx_ = 0;
            }
        }
        else  //Actor is alive or a corpse
        {
            actor->prop_handler().tick(PropTurnMode::std);

            if (!actor->is_player())
            {
                //Count down monster awareness
                Mon* const mon = static_cast<Mon*>(actor);

                if (mon->player_aware_of_me_counter_ > 0)
                {
                    --mon->player_aware_of_me_counter_;
                }
            }

            actor->on_std_turn_common();

            ++it;
        }
    }

    //New turn for rigids
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            map::cells[x][y].rigid->on_new_turn();
        }
    }

    //New turn for mobs (using a copied vector, since mobs may get destroyed)
    const std::vector<Mob*> mobs_cpy = mobs;

    for (auto* f : mobs_cpy)
    {
        f->on_new_turn();
    }

    //Spawn more monsters?
    //(If an unexplored cell is selected, the spawn is canceled)
    if (map::dlvl >= 1 && map::dlvl <= dlvl_last)
    {
        const int spawn_n_turns = 130;

        if (turn_nr_ % spawn_n_turns == 0)
        {
            populate_mon::try_spawn_due_to_time_passed();
        }
    }

    //Run new turn events on all player items
    auto& player_inv = map::player->inv();

    for (Item* const item : player_inv.backpack_)
    {
        item->on_std_turn_in_inv(InvType::backpack);
    }

    for (InvSlot& slot : player_inv.slots_)
    {
        if (slot.item)
        {
            slot.item->on_std_turn_in_inv(InvType::slots);
        }
    }

    snd_emit::reset_nr_snd_msg_printed_current_turn();

    if (map::dlvl > 0)
    {
        audio::try_play_amb(100);
    }
}

void run_atomic_turn_events()
{
    //Stop burning for any actor standing in liquid
    for (auto* const actor : actors)
    {
        const P& p = actor->pos;

        const Rigid* const rigid = map::cells[p.x][p.y].rigid;

        if (rigid->data().matl_type == Matl::fluid)
        {
            actor->prop_handler().end_prop(PropId::burning);
        }
    }

    //NOTE: We add light AFTER ending burning for actors in liquid, since those actors shouldn't
    //add light.
    update_light_map();
}

} //namespace

void init()
{
    current_turn_type_pos_ = current_actor_idx_ = turn_nr_ = 0;

    actors.clear();
    mobs  .clear();

    is_magic_descend_nxt_std_turn = false;
}

void cleanup()
{
    for (Actor* a : actors)
    {
        delete a;
    }

    actors.clear();

    for (auto* f : mobs)
    {
        delete f;
    }

    mobs.clear();

    is_magic_descend_nxt_std_turn = false;
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

void mobs_at_pos(const P& p, std::vector<Mob*>& vector_ref)
{
    vector_ref.clear();

    for (auto* m : mobs)
    {
        if (m->pos() == p)
        {
            vector_ref.push_back(m);
        }
    }
}

void add_mob(Mob* const f)
{
    mobs.push_back(f);
}

void erase_mob(Mob* const f, const bool destroy_object)
{
    for (auto it = mobs.begin(); it != mobs.end(); ++it)
    {
        if (*it == f)
        {
            if (destroy_object)
            {
                delete f;
            }

            mobs.erase(it);
            return;
        }
    }
}

void erase_all_mobs()
{
    for (auto* m : mobs)
    {
        delete m;
    }

    mobs.clear();
}

void add_actor(Actor* actor)
{
    //Sanity checks
    ASSERT(map::is_pos_inside_map(actor->pos));

#ifndef NDEBUG
    for (Actor* const old_actor : actors)
    {
        //Never insert the same actor twice
        ASSERT(actor != old_actor);

        //Never insert an actor on the same position as an existing living actor
        //NOTE: Actors could be placed dead, e.g. Zuul can do this (immediately spawns a priest),
        //so we check if BOTH actors are alive first before we panic.
        if (actor->is_alive() && old_actor->is_alive())
        {
            const P& new_actor_p = actor->pos;
            const P& old_actor_p = old_actor->pos;

            ASSERT(new_actor_p != old_actor_p);
        }
    }
#endif // NDEBUG

    actors.push_back(actor);
}

void reset_turn_type_and_actor_counters()
{
    current_turn_type_pos_ = current_actor_idx_ = 0;
}

//For every turn type step, run through all actors and let those who can act during this
//type of turn act. When all actors who can act on this phase have acted, and if this is
//a normal speed phase - consider it a standard turn (update properties, update features,
//spawn more monsters etc.)
void tick(const PassTime pass_time)
{
    run_atomic_turn_events();

    auto* actor = current_actor();

    actor->on_actor_turn();

    //Tick properties running on actor turns
    actor->prop_handler().tick(PropTurnMode::actor);

    //Should time move forward?
    if (pass_time == PassTime::yes)
    {
        bool can_act = false;

        while (!can_act)
        {
            auto current_turn_type = TurnType(current_turn_type_pos_);

            ++current_actor_idx_;

            if (current_actor_idx_ >= actors.size())
            {
                current_actor_idx_ = 0;

                ++current_turn_type_pos_;

                if (current_turn_type_pos_ == int(TurnType::END))
                {
                    current_turn_type_pos_ = 0;
                }

                //Every turn type except "fast" and "fastest" are standard turns
                //(i.e. we increment the turn counter, and run standard turn events)
                if (current_turn_type != TurnType::fast && current_turn_type != TurnType::fastest)
                {
                    run_std_turn_events();
                }
            }

            const auto speed = current_actor()->speed();

            switch (speed)
            {
            case ActorSpeed::sluggish:
                can_act = (current_turn_type == TurnType::slow ||
                           current_turn_type == TurnType::normal2)
                          && rnd::fraction(2, 3);
                break;

            case ActorSpeed::slow:
                can_act = current_turn_type == TurnType::slow ||
                          current_turn_type == TurnType::normal2;
                break;

            case ActorSpeed::normal:
                can_act = current_turn_type != TurnType::fast &&
                          current_turn_type != TurnType::fastest;
                break;

            case ActorSpeed::fast:
                can_act = current_turn_type != TurnType::fastest;
                break;

            case ActorSpeed::fastest:
                can_act = true;
                break;

            case ActorSpeed::END:
                ASSERT(false);
                break;
            }
        }
    }
}

void update_light_map()
{
    bool light[map_w][map_h];

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            map::cells[x][y].is_lit = light[x][y] = false;
        }
    }

    //Do not add light on Leng
    if (map_travel::map_type() == MapType::leng)
    {
        return;
    }

    for (const auto* const a : actors)
    {
        a->add_light(light);
    }

    for (const auto* const m : mobs)
    {
        m->add_light(light);
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            map::cells[x][y].rigid->add_light(light);
        }
    }

    //Copy the temp values to the real light map
    //NOTE: This must be done separately - it cannot be done in the map loop above
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            map::cells[x][y].is_lit = light[x][y];
        }
    }
}

Actor* current_actor()
{
    Actor* const actor = actors[current_actor_idx_];

    //Sanity check actor retrieved
    ASSERT(map::is_pos_inside_map(actor->pos));
    return actor;
}

} //game_time
