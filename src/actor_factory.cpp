#include "actor_factory.hpp"

#include <algorithm>
#include <assert.h>

#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "render.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"
#include "actor.hpp"
#include "feature_rigid.hpp"

using namespace std;

namespace actor_factory
{

namespace
{

Actor* mk_actor_from_id(const Actor_id id)
{
    switch (id)
    {
    case Actor_id::player:
        return new Player();

    case Actor_id::zombie:
        return new Zombie_claw();

    case Actor_id::zombie_axe:
        return new Zombie_axe();

    case Actor_id::bloated_zombie:
        return new Bloated_zombie();

    case Actor_id::major_clapham_lee:
        return new Major_clapham_lee();

    case Actor_id::dean_halsey:
        return new Dean_halsey();

    case Actor_id::rat:
        return new Rat();

    case Actor_id::rat_thing:
        return new Rat_thing();

    case Actor_id::brown_jenkin:
        return new Brown_jenkin();

    case Actor_id::green_spider:
        return new Green_spider();

    case Actor_id::red_spider:
        return new Red_spider();

    case Actor_id::white_spider:
        return new White_spider();

    case Actor_id::shadow_spider:
        return new Shadow_spider();

    case Actor_id::leng_spider:
        return new Leng_spider();

    case Actor_id::fire_hound:
        return new Fire_hound();

    case Actor_id::frost_hound:
        return new Frost_hound();

    case Actor_id::zuul:
        return new Zuul();

    case Actor_id::ghost:
        return new Ghost();

    case Actor_id::wraith:
        return new Wraith();

    case Actor_id::phantasm:
        return new Phantasm();

    case Actor_id::giant_bat:
        return new Giant_bat();

    case Actor_id::cultist:
        return new Cultist();

    case Actor_id::cultist_electric:
        return new Cultist_electric();

    case Actor_id::cultist_spike_gun:
        return new Cultist_spike_gun();

    case Actor_id::cultist_priest:
        return new Cultist_priest();

    case Actor_id::keziah_mason:
        return new Keziah_mason();

    case Actor_id::leng_elder:
        return new Leng_elder();

    case Actor_id::wolf:
        return new Wolf();

    case Actor_id::flying_polyp:
        return new Flying_polyp();

    case Actor_id::greater_polyp:
        return new Greater_polyp();

    case Actor_id::mi_go:
        return new Mi_go();

    case Actor_id::mi_go_commander:
        return new Mi_go_commander();

    case Actor_id::ghoul:
        return new Ghoul();

    case Actor_id::shadow:
        return new Shadow();

    case Actor_id::byakhee:
        return new Byakhee();

    case Actor_id::giant_mantis:
        return new Giant_mantis();

    case Actor_id::locust:
        return new Giant_locust();

    case Actor_id::mummy:
        return new Mummy();

    case Actor_id::croc_head_mummy:
        return new Mummy_croc_head();

    case Actor_id::khephren:
        return new Khephren();

    case Actor_id::nitokris:
        return new Mummy_unique();

    case Actor_id::deep_one:
        return new Deep_one();

    case Actor_id::ape:
        return new Ape();

    case Actor_id::worm_mass:
        return new Worm_mass();

    case Actor_id::dust_vortex:
        return new Dust_vortex();

    case Actor_id::fire_vortex:
        return new Fire_vortex();

    case Actor_id::frost_vortex:
        return new Frost_vortex();

    case Actor_id::ooze_black:
        return new Ooze_black();

    case Actor_id::color_oo_space:
        return new Color_oo_space();

    case Actor_id::ooze_clear:
        return new Ooze_clear();

    case Actor_id::ooze_putrid:
        return new Ooze_putrid();

    case Actor_id::ooze_poison:
        return new Ooze_poison();

    case Actor_id::chthonian:
        return new Chthonian();

    case Actor_id::death_fiend:
        return new Death_fiend();

    case Actor_id::hunting_horror:
        return new Hunting_horror();

    case Actor_id::sentry_drone:
        return new Sentry_drone();

    case Actor_id::mold:
        return new Mold();

    case Actor_id::gas_spore:
        return new Gas_spore();

    case Actor_id::the_high_priest:
        return new The_high_priest();

    case Actor_id::the_high_priest_cpy:
        return new The_high_priest_cpy();

    case Actor_id::END: {} break;
    }

    return nullptr;
}

} //namespace

Actor* mk(const Actor_id id, const Pos& pos)
{
    assert(
        !map::cells[pos.x][pos.y].rigid ||
        map::cells[pos.x][pos.y].rigid->id() != Feature_id::stairs);

    Actor* const actor = mk_actor_from_id(id);

    actor->place(pos, actor_data::data[int(id)]);

    auto& data = actor->data();

    if (data.nr_left_allowed_to_spawn > 0)
    {
        data.nr_left_allowed_to_spawn--;
    }

    game_time::add_actor(actor);

    return actor;
}

void delete_all_mon()
{
    vector<Actor*>& actors = game_time::actors_;

    for (size_t i = 0; i < actors.size(); ++i)
    {
        if (actors[i] != map::player)
        {
            game_time::erase_actor_in_element(i);
            --i;
        }
    }
}

void summon(const Pos& origin, const vector<Actor_id>& monster_ids, const bool MAKE_MONSTERS_AWARE,
            Actor* const actor_to_set_as_leader, vector<Mon*>* monsters_ret)
{
    if (monsters_ret) {monsters_ret->clear();}

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(true), blocked);

    vector<Pos> free_cells;
    utils::mk_vector_from_bool_map(false, blocked, free_cells);

    sort(begin(free_cells), end(free_cells), Is_closer_to_pos(origin));

    const size_t    NR_FREE_CELLS   = free_cells.size();
    const size_t    NR_MONSTER_IDS  = monster_ids.size();
    const int       NR_TO_SPAWN     = min(NR_FREE_CELLS, NR_MONSTER_IDS);

    vector<Pos> positions_to_animate;

    for (int i = 0; i < NR_TO_SPAWN; ++i)
    {
        const Pos&    pos   = free_cells[i];
        const Actor_id id    = monster_ids[i];
        Actor* const  actor = mk(id, pos);
        Mon* const    mon   = static_cast<Mon*>(actor);

        assert(utils::is_pos_inside_map(pos, false));

        if (monsters_ret)
        {
            monsters_ret->push_back(mon);
        }

        if (actor_to_set_as_leader)
        {
            mon->leader_ = actor_to_set_as_leader;
        }

        if (MAKE_MONSTERS_AWARE)
        {
            mon->aware_counter_ = mon->data().nr_turns_aware;
        }

        if (map::player->can_see_actor(*actor, nullptr))
        {
            positions_to_animate.push_back(pos);
        }
    }

    render::draw_blast_at_seen_cells(positions_to_animate, clr_magenta);
}

} //Actor_factory
