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

extern std::vector<Actor*>       actors_;
extern std::vector<Mob*>  mobs_;

void init();
void cleanup();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

void add_actor(Actor* actor);

void tick(const bool IS_FREE_TURN = false);

int get_turn();

Actor* get_cur_actor();

void erase_actor_in_element(const size_t i);

void get_mobs_at_pos(const Pos& pos, std::vector<Mob*>& vector_ref);

void add_mob(Mob* const f);

void erase_mob(Mob* const f, const bool DESTROY_OBJECT);

void erase_all_mobs();

void reset_turn_type_and_actor_counters();

void update_light_map();

} //game_time

#endif
