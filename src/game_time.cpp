#include "game_time.hpp"

#include <vector>

#include "init.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "populate_monsters.hpp"
#include "inventory.hpp"
#include "inventory_handling.hpp"
#include "player_bon.hpp"
#include "audio.hpp"
#include "map_parsing.hpp"
#include "io.hpp"
#include "map_travel.hpp"
#include "item.hpp"
#include "saving.hpp"
#include "msg_log.hpp"

namespace game_time
{

std::vector<Actor*> actors;
std::vector<Mob*>   mobs;

bool is_magic_descend_nxt_std_turn;

namespace
{

std::vector<ActorSpeed> turn_type_vector_;

const int ticks_per_turn_ = 20;

int current_turn_type_pos_ = 0;

size_t current_actor_idx_ = 0;

int turn_nr_ = 0;

int std_turn_delay_ = ticks_per_turn_;

void run_std_turn_events()
{
    if (is_magic_descend_nxt_std_turn)
    {
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

        // Delete destroyed actors
        if (actor->state() == ActorState::destroyed)
        {
            // Do not delete player if player died, just return
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
        else  // Actor is alive or a corpse
        {
            if (!actor->is_player())
            {
                // Count down monster awareness
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

    // Allow already burning features to damage stuff, spread fire, etc
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            auto& r = *map::cells[x][y].rigid;

            if (r.burn_state_ == BurnState::burning)
            {
                r.started_burning_this_turn_ = false;
            }
        }
    }

    // New turn for rigids
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            map::cells[x][y].rigid->on_new_turn();
        }
    }

    // New turn for mobs (using a copied vector, since mobs may get destroyed)
    const std::vector<Mob*> mobs_cpy = mobs;

    for (auto* f : mobs_cpy)
    {
        f->on_new_turn();
    }

    // Spawn more monsters?
    const auto map_data = map_travel::current_map_data();

    if (map_data.allow_spawn_mon_over_time == AllowSpawnMonOverTime::yes)
    {
        const int spawn_n_turns = 275;

        if (turn_nr_ % spawn_n_turns == 0)
        {
            // NOTE: If an unexplored cell is selected, the spawn is canceled
            populate_mon::try_spawn_due_to_time_passed();
        }
    }

    // Run new turn events on all player items
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

    if ((map::dlvl > 0) && !map::player->has_prop(PropId::deaf))
    {
        const int play_one_in_n = 250;

        audio::try_play_amb(play_one_in_n);
    }
}

void run_atomic_turn_events()
{
    // Stop burning for any actor standing in liquid
    for (auto* const actor : actors)
    {
        const P& p = actor->pos;

        const Rigid* const rigid = map::cells[p.x][p.y].rigid;

        if (rigid->data().matl_type == Matl::fluid)
        {
            actor->prop_handler().end_prop(PropId::burning);
        }
    }

    // NOTE: We add light AFTER ending burning for actors in liquid, since those
    //       actors shouldn't add light.
    update_light_map();
}

} // namespace

void init()
{
    current_turn_type_pos_ = 0;
    current_actor_idx_ = 0;
    turn_nr_ = 0;
    std_turn_delay_ = ticks_per_turn_;

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
    saving::put_int(turn_nr_);
}

void load()
{
    turn_nr_ = saving::get_int();
}

int turn_nr()
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

    ASSERT(false);
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
    // Sanity checks
    ASSERT(map::is_pos_inside_map(actor->pos));

#ifndef NDEBUG
    for (Actor* const old_actor : actors)
    {
        // Never insert the same actor twice
        ASSERT(actor != old_actor);

        // Never insert an actor at the same position as another living actor

        //
        // NOTE: Actors could be placed dead, e.g. Zuul can do this (immediately
        //       spawns a priest) so we check if BOTH actors are alive first
        //       before we panic.
        //
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

void tick(const int speed_pct_diff)
{
    auto* actor = current_actor();

    {
        const int actor_speed_pct = actor->speed_pct();

        const int speed_pct = std::max(1, actor_speed_pct + speed_pct_diff);

        int delay_to_set = (ticks_per_turn_ * 100) / speed_pct;

        // Make sure the delay is at least 1, to never give an actor infinite
        // number of actions
        delay_to_set = std::max(1, delay_to_set);

        actor->delay_ = delay_to_set;
    }

    actor->prop_handler().on_turn_end();

    // Find next actor who can act
    while (true)
    {
        if (actors.empty())
        {
            return;
        }

        ++current_actor_idx_;

        if (current_actor_idx_ == actors.size())
        {
            // New standard turn?
            if (std_turn_delay_ == 0)
            {
                // Increment the turn counter, and run standard turn events

                //
                // NOTE: This will prune destroyed actors, which will decrease
                //       the actor vector size.
                //
                run_std_turn_events();

                std_turn_delay_ = ticks_per_turn_;
            }
            else
            {
                --std_turn_delay_;
            }

            current_actor_idx_ = 0;
        }

        actor = current_actor();

        ASSERT(actor->delay_ >= 0);

        if (actor->delay_ == 0)
        {
            // Actor is ready to go
            break;
        }

        // Actor is still waiting
        --actor->delay_;
    }

    run_atomic_turn_events();

    current_actor()->prop_handler().on_turn_begin();

    current_actor()->on_actor_turn();
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

    // Do not add light on Leng
    // TODO: This is a hard coded hack, specify if the map should be lit in the
    //       data instead.
    if (map_travel::current_map_data().type == MapType::leng)
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

    // Copy the temp values to the real light map

    // NOTE: Must be done separately - it cannot be done in the map loop above
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
    ASSERT(current_actor_idx_ < actors.size());

    Actor* const actor = actors[current_actor_idx_];

    ASSERT(map::is_pos_inside_map(actor->pos));

    return actor;
}

} // game_time
