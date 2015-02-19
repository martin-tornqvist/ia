#include "init.h"

#include "bot.h"

#include <assert.h>
#include <algorithm>
#include <vector>

#include "properties.h"
#include "actor.h"
#include "feature.h"
#include "input.h"
#include "map.h"
#include "actor_player.h"
#include "actor_factory.h"
#include "attack.h"
#include "feature_door.h"
#include "inventory.h"
#include "actor_mon.h"
#include "map_parsing.h"
#include "utils.h"
#include "game_time.h"
#include "map_travel.h"

using namespace std;

namespace Bot
{

namespace
{

std::vector<Pos> cur_path_;

void find_path_to_stairs()
{
    cur_path_.clear();

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    Pos stair_pos(-1, -1);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const auto cur_id = Map::cells[x][y].rigid->get_id();
            if (cur_id == Feature_id::stairs)
            {
                blocked[x][y] = false;
                stair_pos.set(x, y);
            }
            else if (cur_id == Feature_id::door)
            {
                blocked[x][y] = false;
            }
        }
    }
    assert(stair_pos != Pos(-1, -1));

    Path_find::run(Map::player->pos, stair_pos, blocked, cur_path_);

    assert(!cur_path_.empty());
    assert(cur_path_.front() == stair_pos);
}

bool walk_to_adj_cell(const Pos& p)
{
    assert(Utils::is_pos_adj(Map::player->pos, p, true));

    char key = '0' + int(Dir_utils::get_dir(p - Map::player->pos));

    //Occasionally randomize movement
    if (Rnd::one_in(5)) {key = '0' + Rnd::range(1, 9);}

    Input::handle_map_mode_key_press(Key_data(key));

    return Map::player->pos == p;
}

} //namespace

void init()
{
    cur_path_.clear();
}

void act()
{
    //=======================================================================
    // TESTS
    //=======================================================================
    for (Actor* actor : Game_time::actors_)
    {
#ifdef NDEBUG
        (void)actor;
#else
        assert(Utils::is_pos_inside_map(actor->pos));
#endif
    }
    //=======================================================================

    //Abort?
    if (Input::is_key_held(SDLK_ESCAPE))
    {
        Config::toggle_bot_playing();
    }

    //Check if we are finished with the current run, if so, go back to DLVL 1
    if (Map::dlvl >= DLVL_LAST)
    {
        TRACE << "Starting new run on first dungeon level" << endl;
        Map_travel::init();
        Map::dlvl = 1;
        return;
    }

    Prop_handler& prop_handler = Map::player->get_prop_handler();

    //Keep an allied Mi-go around to help getting out of sticky situations
    bool has_allied_mon = false;

    for (const Actor* const actor : Game_time::actors_)
    {
        if (Map::player->is_leader_of(actor))
        {
            has_allied_mon = true;
            break;
        }
    }
    if (!has_allied_mon)
    {
        Actor_factory::summon(Map::player->pos, {Actor_id::mi_go}, false, Map::player);
    }

    //Occasionally apply RFear (to avoid getting stuck on fear-causing monsters)
    if (Rnd::one_in(7))
    {
        prop_handler.try_apply_prop(new Prop_rFear(Prop_turns::specific, 4), true);
    }

    //Occasionally apply Burning to a random actor (helps to avoid getting stuck)
    if (Rnd::one_in(10))
    {
        const int ELEMENT = Rnd::range(0, Game_time::actors_.size() - 1);
        Actor* const actor = Game_time::actors_[ELEMENT];
        if (actor != Map::player)
        {
            actor->get_prop_handler().try_apply_prop(new Prop_burning(Prop_turns::std), true);
        }
    }

    //Occasionally teleport (to avoid getting stuck)
    if (Rnd::one_in(200))
    {
        Map::player->teleport();
    }

    //Occasionally send a TAB command to attack nearby monsters
    if (Rnd::coin_toss())
    {
        Input::handle_map_mode_key_press(Key_data(SDLK_TAB));
        return;
    }

    //Occasionally apply a random property to exercise the prop code
    if (Rnd::one_in(10))
    {
        vector<Prop_id> prop_bucket;

        for (size_t i = 0; i < size_t(Prop_id::END); ++i)
        {
            if (Prop_data::data[i].allow_test_on_bot)
            {
                prop_bucket.push_back(Prop_id(i));
            }
        }

        Prop_id      prop_id  = prop_bucket[Rnd::range(0, prop_bucket.size() - 1)];
        Prop* const prop    = prop_handler.mk_prop(prop_id, Prop_turns::specific, 5);

        prop_handler.try_apply_prop(prop, true);
    }

    //Handle blocking door
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const Pos p(Map::player->pos + Pos(dx, dy));
            auto* const f = Map::cells[p.x][p.y].rigid;
            if (f->get_id() == Feature_id::door)
            {
                Door* const door = static_cast<Door*>(f);
                door->reveal(false);
                if (door->is_stuck())
                {
                    f->hit(Dmg_type::physical, Dmg_method::kick, Map::player);
                    return;
                }
            }
        }
    }

    //If we are terrified, wait in place
    bool props[size_t(Prop_id::END)];
    Map::player->get_prop_handler().get_prop_ids(props);

    if (props[int(Prop_id::terrified)])
    {
        if (walk_to_adj_cell(Map::player->pos))
        {
            return;
        }
    }

    find_path_to_stairs();

    walk_to_adj_cell(cur_path_.back());
}

} //Bot
