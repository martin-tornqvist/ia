#include "map_travel.hpp"

#include "init.hpp"

#include <list>

#ifndef NDEBUG
#include <chrono>
#endif // NDEBUG

#include "map.hpp"
#include "map_gen.hpp"
#include "populate_items.hpp"
#include "render.hpp"
#include "msg_log.hpp"
#include "feature_rigid.hpp"
#include "utils.hpp"

using namespace std;

namespace map_travel
{

vector<map_data> map_list;

namespace
{

void mk_lvl(const Map_type& map_type)
{
    TRACE_FUNC_BEGIN;

    bool is_lvl_built = false;

#ifndef NDEBUG
    int   nr_attempts  = 0;
    auto  start_time   = chrono::steady_clock::now();
#endif

    //TODO: When the map is invalid, any unique items spawned are lost forever.
    //Currently, the only effect of this should be that slightly fewever unique items
    //are found by the player.
    //It is bad design and should be fixed (but "good enough" for v17.0).

    while (!is_lvl_built)
    {
#ifndef NDEBUG
        ++nr_attempts;
#endif
        switch (map_type)
        {
        case Map_type::intro:          is_lvl_built = map_gen::mk_intro_lvl();          break;
        case Map_type::std:            is_lvl_built = map_gen::mk_std_lvl();            break;
        case Map_type::egypt:          is_lvl_built = map_gen::mk_egypt_lvl();          break;
        case Map_type::leng:           is_lvl_built = map_gen::mk_leng_lvl();           break;
        case Map_type::rats_in_the_walls: is_lvl_built = map_gen::mk_rats_in_the_walls_lvl(); break;
        case Map_type::trapezohedron:  is_lvl_built = map_gen::mk_trapezohedron_lvl();  break;
        case Map_type::boss:           is_lvl_built = map_gen::mk_boss_lvl();           break;
        }
    }

#ifndef NDEBUG
    auto diff_time = chrono::steady_clock::now() - start_time;

    TRACE << "map built after   " << nr_attempts << " attempt(s). " << endl
          << "Total time taken: "
          << chrono::duration <double, milli> (diff_time).count() << " ms" << endl;
#endif

    TRACE_FUNC_END;
}

} //namespace

void init()
{
    //Forest + dungeon + boss + trapezohedron
    const size_t NR_LVL_TOT = DLVL_LAST + 3;

    map_list = vector<map_data>(NR_LVL_TOT, {Map_type::std, Is_main_dungeon::yes});

    //Forest intro level
    map_list[0] = {Map_type::intro, Is_main_dungeon::yes};

    //Occasionally set rats-in-the-walls level as intro to first late game level
    if (rnd::one_in(3))
    {
        map_list[DLVL_FIRST_LATE_GAME - 1] =
        {Map_type::rats_in_the_walls, Is_main_dungeon::yes};
    }

    //"Pharaoh chamber" is the first late game level
    map_list[DLVL_FIRST_LATE_GAME] = {Map_type::egypt, Is_main_dungeon::yes};

    map_list[DLVL_LAST + 1] = {Map_type::boss,           Is_main_dungeon::yes};
    map_list[DLVL_LAST + 2] = {Map_type::trapezohedron,  Is_main_dungeon::yes};
}

void store_to_save_lines(std::vector<std::string>& lines)
{
    lines.push_back(to_str(map_list.size()));

    for (const auto& map_data : map_list)
    {
        lines.push_back(to_str(int(map_data.type)));
        lines.push_back(map_data.is_main_dungeon == Is_main_dungeon::yes ? "1" : "0");
    }
}

void setup_from_save_lines(std::vector<std::string>& lines)
{
    const int NR_MAPS = to_int(lines.front());
    lines.erase(begin(lines));

    map_list.resize(size_t(NR_MAPS));

    for (auto& map_data : map_list)
    {
        map_data.type = Map_type(to_int(lines.front()));
        lines.erase(begin(lines));

        map_data.is_main_dungeon = lines.front() == "1" ?
                                   Is_main_dungeon::yes : Is_main_dungeon::no;
        lines.erase(begin(lines));
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

    map::player->restore_shock(999, true);

    map::player->tgt_ = nullptr;
    game_time::update_light_map();
    map::player->update_fov();
    map::player->update_clr();
    render::draw_map_and_interface();

    if (map_data.is_main_dungeon == Is_main_dungeon::yes && map::dlvl == DLVL_LAST - 1)
    {
        msg_log::add("An ominous voice thunders in my ears.", clr_white, false, true);
        audio::play(Sfx_id::boss_voice2);
    }

    audio::try_play_amb(1);

    if (map::player->phobias[int(Phobia::deep_places)])
    {
        msg_log::add("I am plagued by my phobia of deep places!");
        map::player->get_prop_handler().try_apply_prop(new Prop_terrified(Prop_turns::std));
        return;
    }

    TRACE_FUNC_END;
}

Map_type get_map_type()
{
    return map_list.front().type;
}

} //map_travel
