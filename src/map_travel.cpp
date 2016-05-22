#include "map_travel.hpp"

#include "init.hpp"

#include <list>

#ifndef NDEBUG
#include <chrono>
#endif // NDEBUG

#include "map.hpp"
#include "mapgen.hpp"
#include "populate_items.hpp"
#include "render.hpp"
#include "msg_log.hpp"
#include "feature_rigid.hpp"
#include "save_handling.hpp"

#include "sdl_wrapper.hpp" // *** Temporary ***

namespace map_travel
{

std::vector<map_data> map_list;

namespace
{

void mk_lvl(const Map_type& map_type)
{
    TRACE_FUNC_BEGIN;

    bool map_ok = false;

#ifndef NDEBUG
    int   nr_attempts  = 0;
    auto  start_time   = std::chrono::steady_clock::now();
#endif

    //TODO: When the map is invalid, any unique items spawned are lost forever.
    //Currently, the only effect of this should be that slightly fewever unique items
    //are found by the player.
    //It is bad design and should be fixed (but "good enough" for v17.0).

    while (!map_ok)
    {
#ifndef NDEBUG
        ++nr_attempts;
#endif

        switch (map_type)
        {
        case Map_type::intro:
            map_ok = mapgen::mk_intro_lvl();
            break;

        case Map_type::std:
            map_ok = mapgen::mk_std_lvl();
            break;

        case Map_type::egypt:
            map_ok = mapgen::mk_egypt_lvl();
            break;

        case Map_type::leng:
            map_ok = mapgen::mk_leng_lvl();
            break;

        case Map_type::rats_in_the_walls:
            map_ok = mapgen::mk_rats_in_the_walls_lvl();
            break;

        case Map_type::trapez:
            map_ok = mapgen::mk_trapez_lvl();
            break;

        case Map_type::boss:
            map_ok = mapgen::mk_boss_lvl();
            break;
        }
    }

#ifndef NDEBUG
    auto diff_time = std::chrono::steady_clock::now() - start_time;

    TRACE << "map built after   " << nr_attempts << " attempt(s). " << std::endl
          << "Total time taken: "
          << std::chrono::duration<double, std::milli>(diff_time).count() << " ms" << std::endl;
#endif

    TRACE_FUNC_END;
}

} //namespace

void init()
{
    //Forest + dungeon + boss + trapezohedron
    const size_t NR_LVL_TOT = DLVL_LAST + 3;

    map_list = std::vector<map_data>(NR_LVL_TOT, {Map_type::std, Is_main_dungeon::yes});

    //Forest intro level
    map_list[0] = {Map_type::intro, Is_main_dungeon::yes};

    //Occasionally set rats-in-the-walls level as intro to first late game level
    if (rnd::one_in(3))
    {
        map_list[DLVL_FIRST_LATE_GAME - 1] =
        {
            Map_type::rats_in_the_walls,
            Is_main_dungeon::yes
        };
    }

    //"Pharaoh chamber" is the first late game level
    map_list[DLVL_FIRST_LATE_GAME] = {Map_type::egypt,  Is_main_dungeon::yes};

    map_list[DLVL_LAST + 1] = {Map_type::boss,          Is_main_dungeon::yes};
    map_list[DLVL_LAST + 2] = {Map_type::trapez,        Is_main_dungeon::yes};
}

void save()
{
    save_handling::put_int(map_list.size());

    for (const auto& map_data : map_list)
    {
        save_handling::put_int(int(map_data.type));
        save_handling::put_int(int(map_data.is_main_dungeon));
    }
}

void load()
{
    const int NR_MAPS = save_handling::get_int();

    map_list.resize(size_t(NR_MAPS));

    for (auto& map_data : map_list)
    {
        map_data.type               = Map_type(save_handling::get_int());
        map_data.is_main_dungeon    = Is_main_dungeon(save_handling::get_int());
    }
}

void go_to_nxt()
{
    TRACE_FUNC_BEGIN;

    map_list.erase(map_list.begin());

    const auto& map_data = map_list.front();

    if (map_data.is_main_dungeon == Is_main_dungeon::yes)
    {
        ++map::dlvl;
    }

    mk_lvl(map_data.type);

    map::player->prop_handler().end_prop(Prop_id::descend, false);

    game_time::is_magic_descend_nxt_std_turn = false;

    map::player->restore_shock(999, true);

    map::player->tgt_ = nullptr;
    game_time::update_light_map();
    map::player->update_fov();
    map::player->update_clr();
    render::draw_map_and_interface();

    if (map_data.is_main_dungeon == Is_main_dungeon::yes && map::dlvl == DLVL_LAST - 1)
    {
        msg_log::add("An ominous voice thunders in my ears.",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        audio::play(Sfx_id::boss_voice2);
    }

    audio::try_play_amb(1);

    if (insanity::has_sympt(Ins_sympt_id::phobia_deep))
    {
        msg_log::add("I am plagued by my phobia of deep places!");

        map::player->prop_handler().try_add(new Prop_terrified(Prop_turns::std));
        return;
    }

    TRACE_FUNC_END;
}

Map_type map_type()
{
    return map_list.front().type;
}

} //map_travel
