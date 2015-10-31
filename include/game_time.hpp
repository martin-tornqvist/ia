#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <vector>

#include "feature.hpp"
#include "actor_data.hpp"

class Mob;

enum class Turn_type
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

void tick(const bool IS_FREE_TURN = false);

int turn();

Actor* cur_actor();

void mobs_at_pos(const P& pos, std::vector<Mob*>& vector_ref);

void add_mob(Mob* const f);

void erase_mob(Mob* const f, const bool DESTROY_OBJECT);

void erase_all_mobs();

void reset_turn_type_and_actor_counters();

void update_light_map();

} //game_time

#endif
