#include "init.hpp"

#include "UnitTest++.h"

#include <climits>
#include <string>

#include <SDL.h>

#include "config.hpp"
#include "utils.hpp"
#include "render.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "throwing.hpp"
#include "item_factory.hpp"
#include "text_format.hpp"
#include "actor_factory.hpp"
#include "actor_Mon.hpp"
#include "map_Gen.hpp"
#include "converters.hpp"
#include "cmn_Types.hpp"
#include "map_Parsing.hpp"
#include "fov.hpp"
#include "line_Calc.hpp"
#include "save_Handling.hpp"
#include "inventory.hpp"
#include "player_Spells_Handling.hpp"
#include "player_Bon.hpp"
#include "explosion.hpp"
#include "item_Device.hpp"
#include "feature_Rigid.hpp"
#include "feature_Trap.hpp"
#include "drop.hpp"
#include "map_Travel.hpp"

struct Basic_fixture
{
    Basic_fixture()
    {
        init::init_game();
        init::init_session();
        map::player->pos = P(1, 1);
        map::reset_map(); //Because map generation is not run
    }

    ~Basic_fixture()
    {
        init::cleanup_session();
        init::cleanup_game();
    }
};

TEST(Is_Val_In_Range)
{
    //Check in range
    CHECK(utils::is_val_in_range(5,    Range(3,  7)));
    CHECK(utils::is_val_in_range(3,    Range(3,  7)));
    CHECK(utils::is_val_in_range(7,    Range(3,  7)));
    CHECK(utils::is_val_in_range(10,   Range(-5, 12)));
    CHECK(utils::is_val_in_range(-5,   Range(-5, 12)));
    CHECK(utils::is_val_in_range(12,   Range(-5, 12)));

    CHECK(utils::is_val_in_range(0,    Range(-1,  1)));
    CHECK(utils::is_val_in_range(-1,   Range(-1,  1)));
    CHECK(utils::is_val_in_range(1,    Range(-1,  1)));

    CHECK(utils::is_val_in_range(5,    Range(5,  5)));

    //Check NOT in range
    CHECK(!utils::is_val_in_range(2,   Range(3,  7)));
    CHECK(!utils::is_val_in_range(8,   Range(3,  7)));
    CHECK(!utils::is_val_in_range(-1,  Range(3,  7)));

    CHECK(!utils::is_val_in_range(-9,  Range(-5, 12)));
    CHECK(!utils::is_val_in_range(13,  Range(-5, 12)));

    CHECK(!utils::is_val_in_range(0,   Range(1,  2)));

    CHECK(!utils::is_val_in_range(4,   Range(5,  5)));
    CHECK(!utils::is_val_in_range(6,   Range(5,  5)));

    //NOTE: Reversed range settings (e.g. Range(7, 3)) will fail an assert on debug builds.
    //For release builds, this will reverse the result - i.e. is_val_in_range(1, Range(7, 3))
    //will return true.
}

TEST(roll_dice)
{
    int val = rnd::range(100, 200);
    CHECK(val >= 100 && val <= 200);
    val = rnd::range(-1, 1);
    CHECK(val >= -1 && val <= 1);
}

TEST(constrain_val_in_range)
{
    int val = utils::constr_in_range(5, 9, 10);
    CHECK_EQUAL(val, 9);
    val = utils::constr_in_range(5, 11, 10);
    CHECK_EQUAL(val, 10);
    val = utils::constr_in_range(5, 4, 10);
    CHECK_EQUAL(val, 5);

    utils::set_constr_in_range(2, val, 8);
    CHECK_EQUAL(val, 5);
    utils::set_constr_in_range(2, val, 4);
    CHECK_EQUAL(val, 4);
    utils::set_constr_in_range(18, val, 22);
    CHECK_EQUAL(val, 18);

    //Test faulty paramters
    val = utils::constr_in_range(9, 4, 2);   //Min > Max -> return -1
    CHECK_EQUAL(val, -1);
    val = 10;
    utils::set_constr_in_range(20, val, 3);   //Min > Max -> do nothing
    CHECK_EQUAL(val, 10);
}

TEST(calculate_distances)
{
    CHECK_EQUAL(utils::king_dist(P(1, 2), P(2, 3)), 1);
    CHECK_EQUAL(utils::king_dist(P(1, 2), P(2, 4)), 2);
    CHECK_EQUAL(utils::king_dist(P(1, 2), P(1, 2)), 0);
    CHECK_EQUAL(utils::king_dist(P(10, 3), P(1, 4)), 9);
}

TEST(directions)
{
    const int X0 = 20;
    const int Y0 = 20;
    const P from_pos(X0, Y0);
    std::string str = "";
    dir_utils::compass_dir_name(from_pos, P(X0 + 1, Y0), str);
    CHECK_EQUAL("E", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 1, Y0 + 1), str);
    CHECK_EQUAL("SE", str);
    dir_utils::compass_dir_name(from_pos, P(X0    , Y0 + 1), str);
    CHECK_EQUAL("S", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 1, Y0 + 1), str);
    CHECK_EQUAL("SW", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 1, Y0), str);
    CHECK_EQUAL("W", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 1, Y0 - 1), str);
    CHECK_EQUAL("NW", str);
    dir_utils::compass_dir_name(from_pos, P(X0    , Y0 - 1), str);
    CHECK_EQUAL("N", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 1, Y0 - 1), str);
    CHECK_EQUAL("NE", str);

    dir_utils::compass_dir_name(from_pos, P(X0 + 3, Y0 + 1), str);
    CHECK_EQUAL("E", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 2, Y0 + 3), str);
    CHECK_EQUAL("SE", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 1, Y0 + 3), str);
    CHECK_EQUAL("S", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 3, Y0 + 2), str);
    CHECK_EQUAL("SW", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 3, Y0 + 1), str);
    CHECK_EQUAL("W", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 3, Y0 - 2), str);
    CHECK_EQUAL("NW", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 1, Y0 - 3), str);
    CHECK_EQUAL("N", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 3, Y0 - 2), str);
    CHECK_EQUAL("NE", str);

    dir_utils::compass_dir_name(from_pos, P(X0 + 10000, Y0), str);
    CHECK_EQUAL("E", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 10000, Y0 + 10000), str);
    CHECK_EQUAL("SE", str);
    dir_utils::compass_dir_name(from_pos, P(X0        , Y0 + 10000), str);
    CHECK_EQUAL("S", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 10000, Y0 + 10000), str);
    CHECK_EQUAL("SW", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 10000, Y0), str);
    CHECK_EQUAL("W", str);
    dir_utils::compass_dir_name(from_pos, P(X0 - 10000, Y0 - 10000), str);
    CHECK_EQUAL("NW", str);
    dir_utils::compass_dir_name(from_pos, P(X0        , Y0 - 10000), str);
    CHECK_EQUAL("N", str);
    dir_utils::compass_dir_name(from_pos, P(X0 + 10000, Y0 - 10000), str);
    CHECK_EQUAL("NE", str);
}

TEST(format_text)
{
    std::string str = "one two three four";

    std::vector<std::string> formatted_lines;

    int lines_max_w = 100;
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL(str, formatted_lines[0]);
    CHECK_EQUAL(1, int(formatted_lines.size()));

    lines_max_w = 18;
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two three four", formatted_lines[0]);
    CHECK_EQUAL(1, int(formatted_lines.size()));

    lines_max_w = 17;
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two three",    formatted_lines[0]);
    CHECK_EQUAL("four",             formatted_lines[1]);
    CHECK_EQUAL(2, int(formatted_lines.size()));

    lines_max_w = 15;
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two three",    formatted_lines[0]);
    CHECK_EQUAL("four",             formatted_lines[1]);
    CHECK_EQUAL(2, int(formatted_lines.size()));

    lines_max_w = 11;
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two",          formatted_lines[0]);
    CHECK_EQUAL("three four",       formatted_lines[1]);
    CHECK_EQUAL(2, int(formatted_lines.size()));

    lines_max_w    = 4;
    str         = "123456";
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("123456",           formatted_lines[0]);
    CHECK_EQUAL(1, int(formatted_lines.size()));

    lines_max_w    = 4;
    str         = "12 345678";
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("12",               formatted_lines[0]);
    CHECK_EQUAL("345678",           formatted_lines[1]);
    CHECK_EQUAL(2, int(formatted_lines.size()));

    str = "";
    text_format::line_to_lines(str, lines_max_w, formatted_lines);
    CHECK(formatted_lines.empty());
}

TEST_FIXTURE(Basic_fixture, line_calculation)
{
    P origin(0, 0);
    std::vector<P> line;

    line_calc::calc_new_line(origin, P(3, 0), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(1, 0));
    CHECK(line[2] == P(2, 0));
    CHECK(line[3] == P(3, 0));

    line_calc::calc_new_line(origin, P(-3, 0), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(-1, 0));
    CHECK(line[2] == P(-2, 0));
    CHECK(line[3] == P(-3, 0));

    line_calc::calc_new_line(origin, P(0, 3), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(0, 1));
    CHECK(line[2] == P(0, 2));
    CHECK(line[3] == P(0, 3));

    line_calc::calc_new_line(origin, P(0, -3), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(0, -1));
    CHECK(line[2] == P(0, -2));
    CHECK(line[3] == P(0, -3));

    line_calc::calc_new_line(origin, P(3, 3), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(1, 1));
    CHECK(line[2] == P(2, 2));
    CHECK(line[3] == P(3, 3));

    line_calc::calc_new_line(P(9, 9), P(6, 12), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == P(9, 9));
    CHECK(line[1] == P(8, 10));
    CHECK(line[2] == P(7, 11));
    CHECK(line[3] == P(6, 12));

    line_calc::calc_new_line(origin, P(-3, 3), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(-1, 1));
    CHECK(line[2] == P(-2, 2));
    CHECK(line[3] == P(-3, 3));

    line_calc::calc_new_line(origin, P(3, -3), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(1, -1));
    CHECK(line[2] == P(2, -2));
    CHECK(line[3] == P(3, -3));

    line_calc::calc_new_line(origin, P(-3, -3), true, 999, true, line);
    CHECK(line.size() == 4);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(-1, -1));
    CHECK(line[2] == P(-2, -2));
    CHECK(line[3] == P(-3, -3));

    //Test disallowing outside map
    line_calc::calc_new_line(P(1, 0), P(-9, 0), true, 999, false, line);
    CHECK(line.size() == 2);
    CHECK(line[0] == P(1, 0));
    CHECK(line[1] == P(0, 0));

    //Test travel limit parameter
    line_calc::calc_new_line(origin, P(20, 0), true, 2, true, line);
    CHECK(line.size() == 3);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(1, 0));
    CHECK(line[2] == P(2, 0));

    //Test precalculated FOV line offsets
    const std::vector<P>* delta_line = line_calc::fov_delta_line(P(3, 3), FOV_STD_RADI_DB);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(1, 1));
    CHECK(delta_line->at(2) == P(2, 2));
    CHECK(delta_line->at(3) == P(3, 3));

    delta_line = line_calc::fov_delta_line(P(-3, 3), FOV_STD_RADI_DB);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(-1, 1));
    CHECK(delta_line->at(2) == P(-2, 2));
    CHECK(delta_line->at(3) == P(-3, 3));

    delta_line = line_calc::fov_delta_line(P(3, -3), FOV_STD_RADI_DB);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(1, -1));
    CHECK(delta_line->at(2) == P(2, -2));
    CHECK(delta_line->at(3) == P(3, -3));

    delta_line = line_calc::fov_delta_line(P(-3, -3), FOV_STD_RADI_DB);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(-1, -1));
    CHECK(delta_line->at(2) == P(-2, -2));
    CHECK(delta_line->at(3) == P(-3, -3));

    //Check constraints for retrieving FOV offset lines
    //Delta > parameter max distance
    delta_line = line_calc::fov_delta_line(P(3, 0), 2);
    CHECK(!delta_line);
    //Delta > limit of precalculated
    delta_line = line_calc::fov_delta_line(P(50, 0), 999);
    CHECK(!delta_line);
}

TEST_FIXTURE(Basic_fixture, fov)
{
    bool blocked[MAP_W][MAP_H];

    utils::reset_array(blocked, false);   //Nothing blocking sight

    const int X = MAP_W_HALF;
    const int Y = MAP_H_HALF;

    map::player->pos = P(X, Y);

    Los_result fov[MAP_W][MAP_H];

    fov::run(map::player->pos, blocked, fov);

    const int R = FOV_STD_RADI_INT;

    //Not blocked
    CHECK(!fov[X    ][Y    ].is_blocked_hard);
    CHECK(!fov[X + 1][Y    ].is_blocked_hard);
    CHECK(!fov[X - 1][Y    ].is_blocked_hard);
    CHECK(!fov[X    ][Y + 1].is_blocked_hard);
    CHECK(!fov[X    ][Y - 1].is_blocked_hard);
    CHECK(!fov[X + 2][Y + 2].is_blocked_hard);
    CHECK(!fov[X - 2][Y + 2].is_blocked_hard);
    CHECK(!fov[X + 2][Y - 2].is_blocked_hard);
    CHECK(!fov[X - 2][Y - 2].is_blocked_hard);
    CHECK(!fov[X + R][Y    ].is_blocked_hard);
    CHECK(!fov[X - R][Y    ].is_blocked_hard);
    CHECK(!fov[X    ][Y + R].is_blocked_hard);
    CHECK(!fov[X    ][Y - R].is_blocked_hard);

    //Blocked (not within FOV range)
    CHECK(fov[X + R + 1][Y        ].is_blocked_hard);
    CHECK(fov[X - R - 1][Y        ].is_blocked_hard);
    CHECK(fov[X        ][Y + R + 1].is_blocked_hard);
    CHECK(fov[X        ][Y - R - 1].is_blocked_hard);

    //Corners
    CHECK(fov[X + R][Y - R].is_blocked_hard);
    CHECK(fov[X - R][Y - R].is_blocked_hard);
    CHECK(fov[X + R][Y + R].is_blocked_hard);
    CHECK(fov[X - R][Y + R].is_blocked_hard);

    CHECK(fov[X + R - 1][Y - R + 1].is_blocked_hard);
    CHECK(fov[X - R + 1][Y - R + 1].is_blocked_hard);
    CHECK(fov[X + R - 1][Y + R - 1].is_blocked_hard);
    CHECK(fov[X - R + 1][Y + R - 1].is_blocked_hard);
}

TEST_FIXTURE(Basic_fixture, light_map)
{
    //Put walls on the edge of the map, and floor in all other cells, and make all cells dark
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const P p(x, y);

            Cell& cell = map::cells[p.x][p.y];

            cell.is_dark    = true;
            cell.is_lit     = false;

            if (utils::is_pos_inside_map(p, false))
            {
                map::put(new Floor(p));
            }
            else //Is on edge of map
            {
                map::put(new Wall(p));
            }
        }
    }

    map::player->pos.set(40, 12);

    const P burn_pos(40, 10);

    Rigid* const burn_rigid = map::cells[burn_pos.x][burn_pos.y].rigid;

    while (burn_rigid->burn_state() != Burn_state::burning)
    {
        burn_rigid->hit(Dmg_type::fire, Dmg_method::elemental);
    }

    game_time::update_light_map();

    map::player->update_fov();

    //Check that the cells around the burning floor is lit
    CHECK(map::cells[burn_pos.x    ][burn_pos.y    ].is_lit);
    CHECK(map::cells[burn_pos.x    ][burn_pos.y + 1].is_lit);
    CHECK(map::cells[burn_pos.x    ][burn_pos.y - 1].is_lit);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y    ].is_lit);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y + 1].is_lit);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y - 1].is_lit);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y    ].is_lit);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y + 1].is_lit);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y - 1].is_lit);

    //The cells around the burning floor should still be "dark" (not affected by light)
    CHECK(map::cells[burn_pos.x    ][burn_pos.y    ].is_dark);
    CHECK(map::cells[burn_pos.x    ][burn_pos.y + 1].is_dark);
    CHECK(map::cells[burn_pos.x    ][burn_pos.y - 1].is_dark);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y    ].is_dark);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y + 1].is_dark);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y - 1].is_dark);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y    ].is_dark);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y + 1].is_dark);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y - 1].is_dark);
}

TEST_FIXTURE(Basic_fixture, throw_items)
{
    //-----------------------------------------------------------------
    // Throwing a throwing knife at a wall should make it land
    // in front of the wall - i.e. the cell it travelled through
    // before encountering the wall.
    //
    // . <- (5, 7)
    // # <- If aiming at wall here... (5, 8)
    // . <- ...throwing knife should finally land here (5, 9)
    // @ <- Player position (5, 10).
    //-----------------------------------------------------------------

    map::put(new Floor(P(5, 7)));
    map::put(new Floor(P(5, 9)));
    map::put(new Floor(P(5, 10)));
    map::player->pos = P(5, 10);
    P tgt(5, 8);
    Item* item = item_factory::mk(Item_id::thr_knife);
    throwing::throw_item(*(map::player), tgt, *item);
    CHECK(map::cells[5][9].item);
}

TEST_FIXTURE(Basic_fixture, explosions)
{
    const int X0 = 5;
    const int Y0 = 7;

    map::put(new Floor(P(X0, Y0)));

    //Check wall destruction
    for (int i = 0; i < 2; ++i)
    {
        explosion::run(P(X0, Y0), Expl_type::expl);

        //Cells around the center, at a distance of 1, should be destroyed
        int r = 1;
        CHECK(map::cells[X0 + r][Y0    ].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0 - r][Y0    ].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0    ][Y0 + r].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0    ][Y0 - r].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0 + r][Y0 + r].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0 + r][Y0 - r].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0 - r][Y0 + r].rigid->id() != Feature_id::wall);
        CHECK(map::cells[X0 - r][Y0 - r].rigid->id() != Feature_id::wall);

        //Cells around the center, at a distance of 2, should NOT be destroyed
        r = 2;
        CHECK(map::cells[X0 + r][Y0    ].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0 - r][Y0    ].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0    ][Y0 + r].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0    ][Y0 - r].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0 + r][Y0 + r].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0 + r][Y0 - r].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0 - r][Y0 + r].rigid->id() == Feature_id::wall);
        CHECK(map::cells[X0 - r][Y0 - r].rigid->id() == Feature_id::wall);
    }

    //Check damage to actors
    Actor* a1 = actor_factory::mk(Actor_id::rat, P(X0 + 1, Y0));
    explosion::run(P(X0, Y0), Expl_type::expl);
    CHECK_EQUAL(int(Actor_state::destroyed), int(a1->state()));

    //Check that corpses can be destroyed, and do not block living actors
    const int NR_CORPSES = 3;
    Actor* corpses[NR_CORPSES];

    for (int i = 0; i < NR_CORPSES; ++i)
    {
        corpses[i] = actor_factory::mk(Actor_id::rat, P(X0 + 1, Y0));
        corpses[i]->die(false, false, false);
    }

    a1 = actor_factory::mk(Actor_id::rat, P(X0 + 1, Y0));
    explosion::run(P(X0, Y0), Expl_type::expl);

    for (int i = 0; i < NR_CORPSES; ++i)
    {
        CHECK_EQUAL(int(Actor_state::destroyed), int(corpses[i]->state()));
    }

    CHECK_EQUAL(int(Actor_state::destroyed), int(a1->state()));

    //Check explosion applying Burning to living and dead actors
    a1        = actor_factory::mk(Actor_id::rat, P(X0 - 1, Y0));
    Actor* a2 = actor_factory::mk(Actor_id::rat, P(X0 + 1, Y0));

    for (int i = 0; i < NR_CORPSES; ++i)
    {
        corpses[i] = actor_factory::mk(Actor_id::rat, P(X0 + 1, Y0));
        corpses[i]->die(false, false, false);
    }

    explosion::run(P(X0, Y0), Expl_type::apply_prop, Expl_src::misc,
                   Emit_expl_snd::no, 0, new Prop_burning(Prop_turns::std));
    CHECK(a1->prop_handler().prop(Prop_id::burning));
    CHECK(a2->prop_handler().prop(Prop_id::burning));

    CHECK(a1->has_prop(Prop_id::burning));
    CHECK(a2->has_prop(Prop_id::burning));

    for (int i = 0; i < NR_CORPSES; ++i)
    {
        Prop_handler& prop_hlr = corpses[i]->prop_handler();
        CHECK(prop_hlr.prop(Prop_id::burning));
        CHECK(prop_hlr.has_prop(Prop_id::burning));
    }

    //Check that the explosion can handle the map edge (e.g. that it does not
    //destroy the edge wall, or go outside the map - possibly causing a crash)

    //North-west edge
    int x = 1;
    int y = 1;
    map::put(new Floor(P(x, y)));
    explosion::run(P(x, y), Expl_type::expl);
    CHECK(map::cells[x + 1][y    ].rigid->id() != Feature_id::wall);
    CHECK(map::cells[x    ][y + 1].rigid->id() != Feature_id::wall);
    CHECK(map::cells[x - 1][y    ].rigid->id() == Feature_id::wall);
    CHECK(map::cells[x    ][y - 1].rigid->id() == Feature_id::wall);

    //South-east edge
    x = MAP_W - 2;
    y = MAP_H - 2;
    map::put(new Floor(P(x, y)));
    explosion::run(P(x, y), Expl_type::expl);
    CHECK(map::cells[x - 1][y    ].rigid->id() != Feature_id::wall);
    CHECK(map::cells[x    ][y - 1].rigid->id() != Feature_id::wall);
    CHECK(map::cells[x + 1][y    ].rigid->id() == Feature_id::wall);
    CHECK(map::cells[x    ][y + 1].rigid->id() == Feature_id::wall);
}

TEST_FIXTURE(Basic_fixture, monster_stuck_in_spider_web)
{
    //-----------------------------------------------------------------
    // Test that-
    // * a monster can get stuck in a spider web,
    // * the monster can get loose, and
    // * the web can get destroyed
    //-----------------------------------------------------------------

    const P pos_l(1, 4);
    const P pos_r(2, 4);

    //Spawn left floor cell
    map::put(new Floor(pos_l));

    //Conditions for finished test
    bool tested_stuck               = false;
    bool tested_loose_web_interact  = false;
    bool tested_loose_web_destroyed = false;

    while (!tested_stuck || !tested_loose_web_interact || !tested_loose_web_destroyed)
    {

        //Spawn right floor cell
        map::put(new Floor(pos_r));

        //Spawn a monster that can get stuck in the web
        Actor* const actor = actor_factory::mk(Actor_id::zombie, pos_l);
        Mon* const mon = static_cast<Mon*>(actor);

        //Create a spider web in the right cell
        const auto  mimicId     = map::cells[pos_r.x][pos_r.x].rigid->id();
        const auto& mimic_data  = feature_data::data(mimicId);
        const auto* const mimic = static_cast<const Rigid*>(mimic_data.mk_obj(pos_r));
        map::put(new Trap(pos_r, mimic, Trap_id::web));

        //Move the monster into the trap, and back again
        mon->aware_counter_ = 20000; // > 0 req. for triggering trap
        mon->pos = pos_l;
        mon->move(Dir::right);
        CHECK(mon->pos == pos_r);
        mon->move(Dir::left);
        mon->move(Dir::left);

        //Check conditions
        if (mon->pos == pos_r)
        {
            tested_stuck = true;
        }
        else if (mon->pos == pos_l)
        {
            const auto featureId = map::cells[pos_r.x][pos_r.y].rigid->id();

            if (featureId == Feature_id::floor)
            {
                tested_loose_web_destroyed = true;
            }
            else
            {
                tested_loose_web_interact = true;
            }
        }

        //Remove the monster
        actor_factory::delete_all_mon();
    }

    //Check that all cases have been triggered (not really necessary, it just
    //verifies that the loop above is correctly written).
    CHECK(tested_stuck);
    CHECK(tested_loose_web_interact);
    CHECK(tested_loose_web_destroyed);
}

TEST_FIXTURE(Basic_fixture, inventory_handling)
{
    const P p(10, 10);
    map::put(new Floor(p));
    map::player->pos = p;

    Inventory&  inv         = map::player->inv();
    Inv_slot&   body_slot   = inv.slots_[size_t(Slot_id::body)];

    delete body_slot.item;
    body_slot.item = nullptr;

    Prop_handler& prop_hlr = map::player->prop_handler();

    //Check that no props are enabled
    for (size_t i = 0; i < size_t(Prop_id::END); ++i)
    {
        CHECK(!prop_hlr.has_prop(Prop_id(i)));
        CHECK(!prop_hlr.prop(Prop_id(i)));
    }

    //Wear asbesthos suit
    Item* item = item_factory::mk(Item_id::armor_asb_suit);

    inv.put_in_slot(Slot_id::body, item);

    //Check that the props are applied
    size_t nr_props = 0;

    for (size_t i = 0; i < size_t(Prop_id::END); ++i)
    {
        if (prop_hlr.has_prop(Prop_id(i)))
        {
            CHECK(prop_hlr.prop(Prop_id(i)));
            ++nr_props;
        }
    }

    CHECK_EQUAL(4, int(nr_props));

    CHECK(prop_hlr.prop(Prop_id::rFire));
    CHECK(prop_hlr.prop(Prop_id::rElec));
    CHECK(prop_hlr.prop(Prop_id::rAcid));
    CHECK(prop_hlr.prop(Prop_id::rBreath));

    CHECK(prop_hlr.has_prop(Prop_id::rFire));
    CHECK(prop_hlr.has_prop(Prop_id::rElec));
    CHECK(prop_hlr.has_prop(Prop_id::rAcid));
    CHECK(prop_hlr.has_prop(Prop_id::rBreath));

    //Take off asbeshos suit
    inv.try_unequip_slot(Slot_id::body);

    CHECK(inv.backpack_idx(Item_id::armor_asb_suit) != -1);

    //Check that the properties are cleared
    for (int i = 0; i < int(Prop_id::END); ++i)
    {
        CHECK(!prop_hlr.has_prop(Prop_id(i)));
        CHECK(!prop_hlr.prop(Prop_id(i)));
    }

    //Wear the asbeshos suit again
    inv.equip_backpack_item(inv.backpack_idx(Item_id::armor_asb_suit), Slot_id::body);

    game_time::tick();

    //Check that the props are applied
    nr_props = 0;

    for (int i = 0; i < int(Prop_id::END); ++i)
    {
        if (prop_hlr.has_prop(Prop_id(i)))
        {
            CHECK(prop_hlr.prop(Prop_id(i)));
            ++nr_props;
        }
    }

    CHECK_EQUAL(4, int(nr_props));

    CHECK(prop_hlr.prop(Prop_id::rFire));
    CHECK(prop_hlr.prop(Prop_id::rElec));
    CHECK(prop_hlr.prop(Prop_id::rAcid));
    CHECK(prop_hlr.prop(Prop_id::rBreath));

    CHECK(prop_hlr.has_prop(Prop_id::rFire));
    CHECK(prop_hlr.has_prop(Prop_id::rElec));
    CHECK(prop_hlr.has_prop(Prop_id::rAcid));
    CHECK(prop_hlr.has_prop(Prop_id::rBreath));

    //Drop the asbeshos suit on the ground
    item_drop::try_drop_item_from_inv(*map::player, Inv_type::slots, int(Slot_id::body), 1);

    //Check that no item exists in body slot
    CHECK(!body_slot.item);

    //Check that the item is on the ground
    Cell& cell = map::cells[p.x][p.y];
    CHECK(cell.item);

    //Check that the properties are cleared
    for (int i = 0; i < int(Prop_id::END); ++i)
    {
        CHECK(!prop_hlr.has_prop(Prop_id(i)));
        CHECK(!prop_hlr.prop(Prop_id(i)));
    }

    //Wear the same dropped asbesthos suit again
    inv.put_in_slot(Slot_id::body, cell.item);

    cell.item = nullptr;

    //Check that the props are applied
    nr_props = 0;

    for (int i = 0; i < int(Prop_id::END); ++i)
    {
        if (prop_hlr.has_prop(Prop_id(i)))
        {
            CHECK(prop_hlr.prop(Prop_id(i)));
            ++nr_props;
        }
    }

    CHECK_EQUAL(4, int(nr_props));

    CHECK(prop_hlr.prop(Prop_id::rFire));
    CHECK(prop_hlr.prop(Prop_id::rElec));
    CHECK(prop_hlr.prop(Prop_id::rAcid));
    CHECK(prop_hlr.prop(Prop_id::rBreath));

    CHECK(prop_hlr.has_prop(Prop_id::rFire));
    CHECK(prop_hlr.has_prop(Prop_id::rElec));
    CHECK(prop_hlr.has_prop(Prop_id::rAcid));
    CHECK(prop_hlr.has_prop(Prop_id::rBreath));

    //Destroy the asbesthos suit
    map::player->restore_hp(99999, true /* Restoring above max */);

    for (int i = 0; i < 10; ++i)
    {
        explosion::run(map::player->pos, Expl_type::expl);
    }

    //Check that the asbesthos suit is destroyed
    CHECK(!body_slot.item);

    //Check that the properties are cleared
    for (int i = 0; i < int(Prop_id::END); ++i)
    {
        CHECK(!prop_hlr.has_prop(Prop_id(i)));
        CHECK(!prop_hlr.prop(Prop_id(i)));
    }
}

TEST_FIXTURE(Basic_fixture, saving_game)
{
    //Item data
    item_data::data[int(Item_id::scroll_telep)].is_tried = true;
    item_data::data[int(Item_id::scroll_opening)].is_identified = true;

    //Bonus
    player_bon::pick_bg(Bg::rogue);
    player_bon::traits[size_t(Trait::healer)] = true;

    //Player inventory
    Inventory& inv = map::player->inv();

    //First, remove all present items to get a clean state
    std::vector<Item*>& gen = inv.backpack_;

    for (Item* item : gen)
    {
        delete item;
    }

    gen.clear();

    for (size_t i = 0; i < size_t(Slot_id::END); ++i)
    {
        auto& slot = inv.slots_[i];

        if (slot.item)
        {
            delete slot.item;
            slot.item = nullptr;
        }
    }

    //Put new items
    Item* item = item_factory::mk(Item_id::mi_go_gun);
    inv.put_in_slot(Slot_id::wpn, item);

    //Wear asbestos suit to test properties from wearing items
    item = item_factory::mk(Item_id::armor_asb_suit);
    inv.put_in_slot(Slot_id::body, item);

    item = item_factory::mk(Item_id::pistol_clip);
    static_cast<Ammo_clip*>(item)->ammo_ = 1;
    inv.put_in_backpack(item);

    item = item_factory::mk(Item_id::pistol_clip);
    static_cast<Ammo_clip*>(item)->ammo_ = 2;
    inv.put_in_backpack(item);

    item = item_factory::mk(Item_id::pistol_clip);
    static_cast<Ammo_clip*>(item)->ammo_ = 3;
    inv.put_in_backpack(item);

    item = item_factory::mk(Item_id::pistol_clip);
    static_cast<Ammo_clip*>(item)->ammo_ = 3;
    inv.put_in_backpack(item);

    item = item_factory::mk(Item_id::device_blaster);
    static_cast<Strange_device*>(item)->condition_ = Condition::shoddy;
    inv.put_in_backpack(item);

    item = item_factory::mk(Item_id::lantern);

    Device_lantern* lantern = static_cast<Device_lantern*>(item);

    lantern->nr_turns_left_         = 789;
    lantern->nr_flicker_turns_left_ = 456;
    lantern->working_state_         = Lantern_working_state::flicker;
    lantern->is_activated_          = true;

    inv.put_in_backpack(item);

    //Player
    Actor_data_t& def = map::player->data();

    def.name_a = def.name_the = "TEST PLAYER";

    map::player->change_max_hp(5, Verbosity::silent);

    //map
    map::dlvl = 7;

    //Actor data
    actor_data::data[size_t(Actor_id::END) - 1].nr_kills = 123;

    //Learned spells
    player_spells_handling::learn_spell_if_not_known(Spell_id::bless);
    player_spells_handling::learn_spell_if_not_known(Spell_id::aza_wrath);

    //Applied properties
    Prop_handler& prop_hlr = map::player->prop_handler();
    prop_hlr.try_add_prop(new Prop_rSleep(Prop_turns::specific, 3));
    prop_hlr.try_add_prop(new Prop_diseased(Prop_turns::indefinite));
    prop_hlr.try_add_prop(new Prop_blessed(Prop_turns::std));

    //Check a a few of the props applied
    Prop* prop = prop_hlr.prop(Prop_id::diseased);
    CHECK(prop);

    prop = prop_hlr.prop(Prop_id::blessed);
    CHECK(prop);

    //Check a prop that was NOT applied
    prop = prop_hlr.prop(Prop_id::confused);
    CHECK(!prop);

    //map sequence
    map_travel::map_list[5] = {Map_type::rats_in_the_walls, Is_main_dungeon::yes};
    map_travel::map_list[7] = {Map_type::leng,              Is_main_dungeon::no};

    save_handling::save_game();
    CHECK(save_handling::is_save_available());
}

TEST_FIXTURE(Basic_fixture, loading_game)
{
    CHECK(save_handling::is_save_available());

    const int PLAYER_MAX_HP_BEFORE_LOAD = map::player->hp_max(true);

    save_handling::load_game();

    //Item data
    CHECK_EQUAL(true,  item_data::data[int(Item_id::scroll_telep)].is_tried);
    CHECK_EQUAL(false, item_data::data[int(Item_id::scroll_telep)].is_identified);
    CHECK_EQUAL(true,  item_data::data[int(Item_id::scroll_opening)].is_identified);
    CHECK_EQUAL(false, item_data::data[int(Item_id::scroll_opening)].is_tried);
    CHECK_EQUAL(false, item_data::data[int(Item_id::scroll_det_mon)].is_tried);
    CHECK_EQUAL(false, item_data::data[int(Item_id::scroll_det_mon)].is_identified);

    //Bonus
    CHECK_EQUAL(int(Bg::rogue), int(player_bon::bg()));
    CHECK(player_bon::traits[size_t(Trait::healer)]);
    CHECK(!player_bon::traits[size_t(Trait::fast_shooter)]);

    //Player inventory
    Inventory& inv  = map::player->inv();
    auto& genInv    = inv.backpack_;

    CHECK_EQUAL(6, int(genInv.size()));
    CHECK_EQUAL(int(Item_id::mi_go_gun), int(inv.item_in_slot(Slot_id::wpn)->data().id));
    CHECK_EQUAL(int(Item_id::armor_asb_suit), int(inv.item_in_slot(Slot_id::body)->data().id));

    int nr_clip_with_1 = 0;
    int nr_clip_with_2 = 0;
    int nr_clip_with_3 = 0;
    bool is_sentry_device_found = false;
    bool is_lantern_found       = false;

    for (Item* item : genInv)
    {
        Item_id id = item->id();

        if (id == Item_id::pistol_clip)
        {
            switch (static_cast<Ammo_clip*>(item)->ammo_)
            {
            case 1:
                ++nr_clip_with_1;
                break;

            case 2:
                ++nr_clip_with_2;
                break;

            case 3:
                ++nr_clip_with_3;
                break;

            default:
                break;
            }
        }
        else if (id == Item_id::device_blaster)
        {
            is_sentry_device_found = true;
            CHECK_EQUAL(int(Condition::shoddy),
                        int(static_cast<Strange_device*>(item)->condition_));
        }
        else if (id == Item_id::lantern)
        {
            is_lantern_found = true;
            Device_lantern* lantern = static_cast<Device_lantern*>(item);
            CHECK_EQUAL(789, lantern->nr_turns_left_);
            CHECK_EQUAL(456, lantern->nr_flicker_turns_left_);
            CHECK_EQUAL(int(Lantern_working_state::flicker), int(lantern->working_state_));
            CHECK(lantern->is_activated_);
        }
    }

    CHECK_EQUAL(1, nr_clip_with_1);
    CHECK_EQUAL(1, nr_clip_with_2);
    CHECK_EQUAL(2, nr_clip_with_3);
    CHECK(is_sentry_device_found);
    CHECK(is_lantern_found);

    //Player
    Actor_data_t& def = map::player->data();

    CHECK_EQUAL("TEST PLAYER", def.name_a);
    CHECK_EQUAL("TEST PLAYER", def.name_the);

    //Check max HP (affected by disease)
    CHECK_EQUAL((PLAYER_MAX_HP_BEFORE_LOAD + 5) / 2, map::player->hp_max(true));

    //map
    CHECK_EQUAL(7, map::dlvl);

    //Actor data
    CHECK_EQUAL(123, actor_data::data[int(Actor_id::END) - 1].nr_kills);

    //Learned spells
    CHECK(player_spells_handling::is_spell_learned(Spell_id::bless));
    CHECK(player_spells_handling::is_spell_learned(Spell_id::aza_wrath));
    CHECK_EQUAL(false, player_spells_handling::is_spell_learned(Spell_id::mayhem));

    //Properties
    Prop_handler& prop_hlr = map::player->prop_handler();
    Prop* prop = prop_hlr.prop(Prop_id::diseased);
    CHECK(prop);
    CHECK(prop_hlr.has_prop(Prop_id::diseased));
    CHECK_EQUAL(-1, prop->nr_turns_left());

    //Check currrent HP (should not be affected)
    CHECK_EQUAL(map::player->data().hp, map::player->hp());

    prop = prop_hlr.prop(Prop_id::rSleep);
    CHECK(prop);
    CHECK(prop_hlr.has_prop(Prop_id::rSleep));
    CHECK_EQUAL(3, prop->nr_turns_left());

    prop = prop_hlr.prop(Prop_id::blessed);
    CHECK(prop);
    CHECK(prop_hlr.has_prop(Prop_id::blessed));
    CHECK(prop->nr_turns_left() > 0);

    //Properties from worn item
    prop = prop_hlr.prop(Prop_id::rAcid);
    CHECK(prop);
    CHECK(prop->nr_turns_left() == -1);
    prop = prop_hlr.prop(Prop_id::rFire);
    CHECK(prop);
    CHECK(prop->nr_turns_left() == -1);

    //map sequence
    auto mapData = map_travel::map_list[3];
    CHECK(mapData.type              == Map_type::std);
    CHECK(mapData.is_main_dungeon   == Is_main_dungeon::yes);

    mapData = map_travel::map_list[5];
    CHECK(mapData.type              == Map_type::rats_in_the_walls);
    CHECK(mapData.is_main_dungeon   == Is_main_dungeon::yes);

    mapData = map_travel::map_list[7];
    CHECK(mapData.type              == Map_type::leng);
    CHECK(mapData.is_main_dungeon   == Is_main_dungeon::no);

    //Game time
    CHECK_EQUAL(0, game_time::turn());
}

TEST_FIXTURE(Basic_fixture, flood_filling)
{
    bool b[MAP_W][MAP_H];
    utils::reset_array(b, false);

    for (int y = 0; y < MAP_H; ++y)
    {
        b[0][y] = b[MAP_W - 1][y] = false;
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        b[x][0] = b[x][MAP_H - 1] = false;
    }

    int flood[MAP_W][MAP_H];
    flood_fill::run(P(20, 10), b, flood, 999, P(-1, -1), true);

    CHECK_EQUAL(0, flood[20][10]);
    CHECK_EQUAL(1, flood[19][10]);
    CHECK_EQUAL(1, flood[21][10]);
    CHECK_EQUAL(1, flood[20][11]);
    CHECK_EQUAL(1, flood[21][11]);
    CHECK_EQUAL(4, flood[24][12]);
    CHECK_EQUAL(4, flood[24][14]);
    CHECK_EQUAL(5, flood[24][15]);
    CHECK_EQUAL(0, flood[0][0]);
    CHECK_EQUAL(0, flood[MAP_W - 1][MAP_H - 1]);
}

TEST_FIXTURE(Basic_fixture, path_finding)
{
    std::vector<P> path;
    bool b[MAP_W][MAP_H];
    utils::reset_array(b, false);

    for (int y = 0; y < MAP_H; ++y)
    {
        b[0][y] = b[MAP_W - 1][y] = false;
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        b[x][0] = b[x][MAP_H - 1] = false;
    }

    path_find::run(P(20, 10), P(25, 10), b, path);

    CHECK(!path.empty());
    CHECK(path.back() != P(20, 10));
    CHECK_EQUAL(25, path.front().x);
    CHECK_EQUAL(10, path.front().y);
    CHECK_EQUAL(5, int(path.size()));

    path_find::run(P(20, 10), P(5, 3), b, path);

    CHECK(!path.empty());
    CHECK(path.back() != P(20, 10));
    CHECK_EQUAL(5, path.front().x);
    CHECK_EQUAL(3, path.front().y);
    CHECK_EQUAL(15, int(path.size()));

    b[10][5] = true;

    path_find::run(P(7, 5), P(20, 5), b, path);

    CHECK(!path.empty());
    CHECK(path.back() != P(7, 5));
    CHECK_EQUAL(20, path.front().x);
    CHECK_EQUAL(5, path.front().y);
    CHECK_EQUAL(13, int(path.size()));
    CHECK(find(begin(path), end(path), P(10, 5)) == end(path));

    b[19][4] = b[19][5] =  b[19][6] = true;

    path_find::run(P(7, 5), P(20, 5), b, path);

    CHECK(!path.empty());
    CHECK(path.back() != P(7, 5));
    CHECK_EQUAL(20, path.front().x);
    CHECK_EQUAL(5, path.front().y);
    CHECK_EQUAL(14, int(path.size()));
    CHECK(find(begin(path), end(path), P(19, 4)) == end(path));
    CHECK(find(begin(path), end(path), P(19, 5)) == end(path));
    CHECK(find(begin(path), end(path), P(19, 6)) == end(path));

    path_find::run(P(40, 10), P(43, 15), b, path, false);

    CHECK(!path.empty());
    CHECK(path.back() != P(40, 10));
    CHECK_EQUAL(43, path.front().x);
    CHECK_EQUAL(15, path.front().y);
    CHECK_EQUAL(8, int(path.size()));

    b[41][10] = b[40][11]  = true;

    path_find::run(P(40, 10), P(43, 15), b, path, false);

    CHECK(!path.empty());
    CHECK(path.back() != P(40, 10));
    CHECK_EQUAL(43, path.front().x);
    CHECK_EQUAL(15, path.front().y);
    CHECK_EQUAL(10, int(path.size()));
}

TEST_FIXTURE(Basic_fixture, map_parse_expand_one)
{
    bool in[MAP_W][MAP_H];
    utils::reset_array(in, false);

    in[10][5] = true;

    bool out[MAP_W][MAP_H];
    map_parse::expand(in, out);

    CHECK(!out[8][5]);
    CHECK(out[9][5]);
    CHECK(out[10][5]);
    CHECK(out[11][5]);
    CHECK(!out[12][5]);

    CHECK(!out[8][4]);
    CHECK(out[9][4]);
    CHECK(out[10][4]);
    CHECK(out[11][4]);
    CHECK(!out[12][4]);

    in[14][5] = true;
    map_parse::expand(in, out);

    CHECK(out[10][5]);
    CHECK(out[11][5]);
    CHECK(!out[12][5]);
    CHECK(out[13][5]);
    CHECK(out[14][5]);

    in[12][5] = true;
    map_parse::expand(in, out);
    CHECK(out[12][4]);
    CHECK(out[12][5]);
    CHECK(out[12][6]);

    //Check that old values are cleared
    utils::reset_array(in, false);
    in[40][10] = true;
    map_parse::expand(in, out);
    CHECK(out[39][10]);
    CHECK(out[40][10]);
    CHECK(out[41][10]);
    CHECK(!out[10][5]);
    CHECK(!out[12][5]);
    CHECK(!out[14][5]);
}

TEST_FIXTURE(Basic_fixture, find_room_corr_entries)
{
    //------------------------------------------------ Square, normal sized room
    Rect roomRect(20, 5, 30, 10);

    Room* room = room_factory::mk(Room_type::plain, roomRect);

    for (int y = roomRect.p0.y; y <= roomRect.p1.y; ++y)
    {
        for (int x = roomRect.p0.x; x <= roomRect.p1.x; ++x)
        {
            map::put(new Floor(P(x, y)));
            map::room_map[x][y] = room;
        }
    }

    std::vector<P> entry_list;
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    bool entry_map[MAP_W][MAP_H];
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    CHECK(!entry_map[19][4]);
    CHECK(entry_map[19][5]);
    CHECK(entry_map[20][4]);
    CHECK(!entry_map[20][5]);
    CHECK(entry_map[21][4]);
    CHECK(entry_map[25][4]);
    CHECK(!entry_map[25][8]);
    CHECK(entry_map[29][4]);
    CHECK(entry_map[30][4]);
    CHECK(!entry_map[31][4]);

    //Check that a cell in the middle of the room is not an entry, even if it's not
    //belonging to the room
    map::room_map[25][7] = nullptr;
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    CHECK(!entry_map[25][7]);

    //The cell should also not be an entry if it's a wall and belonging to the room
    map::room_map[25][7] = room;
    map::put(new Wall(P(25, 7)));
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    CHECK(!entry_map[25][7]);

    //The cell should also not be an entry if it's a wall and not belonging to the room
    map::room_map[25][7] = nullptr;
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    CHECK(!entry_map[25][7]);

    //Check that the room can share an antry point with a nearby room
    roomRect = Rect(10, 5, 18, 10);

    Room* nearby_room = room_factory::mk(Room_type::plain, roomRect);

    for (int y = roomRect.p0.y; y <= roomRect.p1.y; ++y)
    {
        for (int x = roomRect.p0.x; x <= roomRect.p1.x; ++x)
        {
            map::put(new Floor(P(x, y)));
            map::room_map[x][y] = nearby_room;
        }
    }

    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    std::vector<P> entry_list_nearby_room;
    map_gen_utils::valid_room_corr_entries(*nearby_room, entry_list_nearby_room);
    bool entry_map_near_room[MAP_W][MAP_H];
    utils::mk_bool_map_from_vector(entry_list_nearby_room, entry_map_near_room);

    for (int y = 5; y <= 10; ++y)
    {
        CHECK(entry_map[19][y]);
        CHECK(entry_map_near_room[19][y]);
    }

    delete nearby_room;

    //------------------------------------------------ Room with only one cell
    delete room;
    room = room_factory::mk(Room_type::plain, {60, 10, 60, 10});
    map::put(new Floor(P(60, 10)));
    map::room_map[60][10] = room;
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    // 59 60 61
    // #  #  # 9
    // #  .  # 10
    // #  #  # 11
    CHECK(!entry_map[59][9]);
    CHECK(entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK(entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK(entry_map[61][10]);
    CHECK(!entry_map[59][11]);
    CHECK(entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    //Add an adjacent floor above the room
    // 59 60 61
    // #  .  # 9
    // #  .  # 10
    // #  #  # 11
    map::put(new Floor(P(60, 9)));
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    CHECK(!entry_map[59][9]);
    CHECK(!entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK(entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK(entry_map[61][10]);
    CHECK(!entry_map[59][11]);
    CHECK(entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    //Mark the adjacent floor as a room and check again
    Room* adj_room = room_factory::mk(Room_type::plain, {60, 9, 60, 9});
    map::room_map[60][9] = adj_room;
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    delete adj_room;

    CHECK(!entry_map[59][9]);
    CHECK(!entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK(entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK(entry_map[61][10]);
    CHECK(!entry_map[59][11]);
    CHECK(entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    //Make the room wider, entries should not be placed next to adjacent floor
    // 58 59 60 61
    // #  #  .  # 9
    // #  .  .  # 10
    // #  #  #  # 11
    room->r_.p0.x = 59;
    map::put(new Floor(P(59, 10)));
    map::room_map[59][10] = room;
    map_gen_utils::valid_room_corr_entries(*room, entry_list);
    utils::mk_bool_map_from_vector(entry_list, entry_map);

    CHECK(!entry_map[58][9]);
    CHECK(entry_map[59][9]);
    CHECK(!entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK(entry_map[58][10]);
    CHECK(!entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK(entry_map[61][10]);
    CHECK(!entry_map[58][11]);
    CHECK(entry_map[59][11]);
    CHECK(entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    //Remove the adjacent room, and check that the blocked entries are now placed
    //TODO

    delete room;
}

TEST_FIXTURE(Basic_fixture, connect_rooms_with_corridor)
{
    Rect room_area_1(P(1, 1), P(10, 10));
    Rect room_area_2(P(15, 4), P(23, 14));

    Room* room0 = room_factory::mk(Room_type::plain, room_area_1);
    Room* room1 = room_factory::mk(Room_type::plain, room_area_2);

    for (int x = room_area_1.p0.x; x <= room_area_1.p1.x; ++x)
    {
        for (int y = room_area_1.p0.y; y <= room_area_1.p1.y; ++y)
        {
            map::put(new Floor(P(x, y)));
            map::room_map[x][y] = room0;
        }
    }

    for (int x = room_area_2.p0.x; x <= room_area_2.p1.x; ++x)
    {
        for (int y = room_area_2.p0.y; y <= room_area_2.p1.y; ++y)
        {
            map::put(new Floor(P(x, y)));
            map::room_map[x][y] = room1;
        }
    }

    map_gen_utils::mk_path_find_cor(*room0, *room1);

    int flood[MAP_W][MAP_H];
    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);
    flood_fill::run(5, blocked, flood, INT_MAX, -1, true);
    CHECK(flood[20][10] > 0);

    delete room0;
    delete room1;
}

TEST_FIXTURE(Basic_fixture, map_parse_cells_within_dist_of_others)
{
    bool in[MAP_W][MAP_H];
    bool out[MAP_W][MAP_H];

    utils::reset_array(in, false);  //Make sure all values are 0

    in[20][10] = true;

    map_parse::cells_within_dist_of_others(in, out, Range(0, 1));
    CHECK_EQUAL(false, out[18][10]);
    CHECK_EQUAL(true,  out[19][10]);
    CHECK_EQUAL(false, out[20][ 8]);
    CHECK_EQUAL(true,  out[20][ 9]);
    CHECK_EQUAL(true,  out[20][10]);
    CHECK_EQUAL(true,  out[20][11]);
    CHECK_EQUAL(true,  out[21][11]);

    map_parse::cells_within_dist_of_others(in, out, Range(1, 1));
    CHECK_EQUAL(true,  out[19][10]);
    CHECK_EQUAL(false, out[20][10]);
    CHECK_EQUAL(true,  out[21][11]);

    map_parse::cells_within_dist_of_others(in, out, Range(1, 5));
    CHECK_EQUAL(true,  out[23][10]);
    CHECK_EQUAL(true,  out[24][10]);
    CHECK_EQUAL(true,  out[25][10]);
    CHECK_EQUAL(true,  out[25][ 9]);
    CHECK_EQUAL(true,  out[25][11]);
    CHECK_EQUAL(false, out[26][10]);
    CHECK_EQUAL(true,  out[16][10]);
    CHECK_EQUAL(true,  out[15][10]);
    CHECK_EQUAL(true,  out[15][ 9]);
    CHECK_EQUAL(true,  out[15][11]);
    CHECK_EQUAL(false, out[14][10]);

    in[23][10] = true;

    map_parse::cells_within_dist_of_others(in, out, Range(1, 1));
    CHECK_EQUAL(false, out[18][10]);
    CHECK_EQUAL(true,  out[19][10]);
    CHECK_EQUAL(false, out[20][10]);
    CHECK_EQUAL(true,  out[21][10]);
    CHECK_EQUAL(true,  out[22][10]);
    CHECK_EQUAL(false, out[23][10]);
    CHECK_EQUAL(true,  out[24][10]);
    CHECK_EQUAL(false, out[25][10]);
}

//-----------------------------------------------------------------------------
// Some code exercise - Ichi! Ni! San!
//-----------------------------------------------------------------------------
//TEST_FIXTURE(Basic_fixture, mapGenStd)
//{
//    for (int i = 0; i < 100; ++i)
//    {
//        map_gen::mk_std_lvl();
//    }
//}

#ifdef _WIN32
#undef main
#endif
int main()
{
    UnitTest::RunAllTests();
    return 0;
}
