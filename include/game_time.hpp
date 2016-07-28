#ifndef GAME_TIME_HPP
#define GAME_TIME_HPP

#include <vector>

#include "feature.hpp"
#include "actor_data.hpp"

class Mob;

enum class TurnType
{
    fast,
    normal1,
    slow,
    fastest,
    normal2,
    END
};

namespace game_time
{

extern std::vector<Actor*> actors;
extern std::vector<Mob*> mobs;

extern bool is_magic_descend_nxt_std_turn;

void init();
void cleanup();

void save();
void load();

void add_actor(Actor* actor);

void tick(const PassTime pass_time = PassTime::yes);

int turn();

Actor* current_actor();

void mobs_at_pos(const P& pos, std::vector<Mob*>& vector_ref);

void add_mob(Mob* const f);

void erase_mob(Mob* const f, const bool destroy_object);

void erase_all_mobs();

void reset_turn_type_and_actor_counters();

void update_light_map();

} //game_time

#endif
