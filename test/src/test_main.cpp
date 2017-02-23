#include "init.hpp"

#include "UnitTest++.h"

#include <climits>
#include <string>

#include <SDL.h>

#include "rl_utils.hpp"
#include "config.hpp"
#include "io.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "throwing.hpp"
#include "item_factory.hpp"
#include "text_format.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "mapgen.hpp"
#include "map_parsing.hpp"
#include "fov.hpp"
#include "line_calc.hpp"
#include "saving.hpp"
#include "inventory.hpp"
#include "player_spells.hpp"
#include "player_bon.hpp"
#include "explosion.hpp"
#include "item_device.hpp"
#include "feature_rigid.hpp"
#include "feature_trap.hpp"
#include "drop.hpp"
#include "map_travel.hpp"

struct BasicFixture
{
    BasicFixture()
    {
        init::init_game();
        init::init_session();

        map::player->mk_start_items();
        map::player->pos = P(1, 1);

        // Because map generation is not run
        map::reset_map();
    }

    ~BasicFixture()
    {
        init::cleanup_session();
        init::cleanup_game();
    }
};

TEST(2d_array)
{
    Array2<char> a(3, 5);

    a.for_each([](char& v)
    {
        v = 0;
    });

    CHECK(a.dims() == P(3, 5));

    a(0, 0) = 'x';

    CHECK(a(0, 0) == 'x');

    //
    // Rotate clock wise
    //
    a.rotate_cw();

    CHECK(a.dims() == P(5, 3));

    CHECK(a(0, 0) == 0);

    CHECK(a(4, 0) == 'x');

    //
    // Flip vertically
    //
    a.flip_ver();

    CHECK(a(4, 0) == 0);

    CHECK(a(4, 2) == 'x');

    //
    // Rotate counter clock wise
    //
    a.rotate_ccw();

    CHECK(a.dims() == P(3, 5));

    CHECK(a(2, 0) == 'x');

    //
    // Flip horizontally
    //
    a.flip_hor();

    CHECK(a(2, 0) == 0);

    CHECK(a(0, 0) == 'x');
}

TEST(is_val_in_range)
{
    // Check in range
    CHECK(is_val_in_range(5,    Range(3,  7)));
    CHECK(is_val_in_range(3,    Range(3,  7)));
    CHECK(is_val_in_range(7,    Range(3,  7)));
    CHECK(is_val_in_range(10,   Range(-5, 12)));
    CHECK(is_val_in_range(-5,   Range(-5, 12)));
    CHECK(is_val_in_range(12,   Range(-5, 12)));

    CHECK(is_val_in_range(0,    Range(-1,  1)));
    CHECK(is_val_in_range(-1,   Range(-1,  1)));
    CHECK(is_val_in_range(1,    Range(-1,  1)));

    CHECK(is_val_in_range(5,    Range(5,  5)));

    // Check NOT in range
    CHECK(!is_val_in_range(2,   Range(3,  7)));
    CHECK(!is_val_in_range(8,   Range(3,  7)));
    CHECK(!is_val_in_range(-1,  Range(3,  7)));

    CHECK(!is_val_in_range(-9,  Range(-5, 12)));
    CHECK(!is_val_in_range(13,  Range(-5, 12)));

    CHECK(!is_val_in_range(0,   Range(1,  2)));

    CHECK(!is_val_in_range(4,   Range(5,  5)));
    CHECK(!is_val_in_range(6,   Range(5,  5)));

    // NOTE: Reversed ranges will fail an assert on debug builds., e.g.:
    //
    //       Range(7, 3))
    //
    //       For release builds, this will reverse the result - i.e.
    //
    //       is_val_in_range(1, Range(7, 3))
    //
    //       ...will return true.
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
    int val = constr_in_range(5, 9, 10);

    CHECK_EQUAL(val, 9);

    val = constr_in_range(5, 11, 10);

    CHECK_EQUAL(val, 10);

    val = constr_in_range(5, 4, 10);

    CHECK_EQUAL(val, 5);

    set_constr_in_range(2, val, 8);

    CHECK_EQUAL(val, 5);

    set_constr_in_range(2, val, 4);

    CHECK_EQUAL(val, 4);

    set_constr_in_range(18, val, 22);

    CHECK_EQUAL(val, 18);

    // Test faulty paramters
    val = constr_in_range(9, 4, 2);   // Min > Max -> return -1

    CHECK_EQUAL(val, -1);

    val = 10;

    set_constr_in_range(20, val, 3);   // Min > Max -> do nothing

    CHECK_EQUAL(val, 10);
}

TEST(calculate_distances)
{
    CHECK_EQUAL(king_dist(P(1, 2), P(2, 3)), 1);
    CHECK_EQUAL(king_dist(P(1, 2), P(2, 4)), 2);
    CHECK_EQUAL(king_dist(P(1, 2), P(1, 2)), 0);
    CHECK_EQUAL(king_dist(P(10, 3), P(1, 4)), 9);
}

TEST(directions)
{
    const int x0 = 20;
    const int y0 = 20;

    const P from_pos(x0, y0);

    std::string str = "";

    dir_utils::compass_dir_name(from_pos, P(x0 + 1, y0), str);

    CHECK_EQUAL("E", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 1, y0 + 1), str);

    CHECK_EQUAL("SE", str);

    dir_utils::compass_dir_name(from_pos, P(x0, y0 + 1), str);

    CHECK_EQUAL("S", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 1, y0 + 1), str);

    CHECK_EQUAL("SW", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 1, y0), str);

    CHECK_EQUAL("W", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 1, y0 - 1), str);

    CHECK_EQUAL("NW", str);

    dir_utils::compass_dir_name(from_pos, P(x0, y0 - 1), str);

    CHECK_EQUAL("N", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 1, y0 - 1), str);

    CHECK_EQUAL("NE", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 3, y0 + 1), str);

    CHECK_EQUAL("E", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 2, y0 + 3), str);

    CHECK_EQUAL("SE", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 1, y0 + 3), str);

    CHECK_EQUAL("S", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 3, y0 + 2), str);

    CHECK_EQUAL("SW", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 3, y0 + 1), str);

    CHECK_EQUAL("W", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 3, y0 - 2), str);

    CHECK_EQUAL("NW", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 1, y0 - 3), str);

    CHECK_EQUAL("N", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 3, y0 - 2), str);

    CHECK_EQUAL("NE", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 10000, y0), str);

    CHECK_EQUAL("E", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 10000, y0 + 10000), str);

    CHECK_EQUAL("SE", str);

    dir_utils::compass_dir_name(from_pos, P(x0, y0 + 10000), str);

    CHECK_EQUAL("S", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 10000, y0 + 10000), str);

    CHECK_EQUAL("SW", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 10000, y0), str);

    CHECK_EQUAL("W", str);

    dir_utils::compass_dir_name(from_pos, P(x0 - 10000, y0 - 10000), str);

    CHECK_EQUAL("NW", str);

    dir_utils::compass_dir_name(from_pos, P(x0, y0 - 10000), str);

    CHECK_EQUAL("N", str);

    dir_utils::compass_dir_name(from_pos, P(x0 + 10000, y0 - 10000), str);

    CHECK_EQUAL("NE", str);
}

TEST(format_text)
{
    std::string str = "one two three four";

    std::vector<std::string> formatted_lines;

    int lines_max_w = 100;
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL(str, formatted_lines[0]);
    CHECK_EQUAL(1, (int)formatted_lines.size());

    lines_max_w = 18;
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two three four", formatted_lines[0]);
    CHECK_EQUAL(1, (int)formatted_lines.size());

    lines_max_w = 17;
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two three",    formatted_lines[0]);
    CHECK_EQUAL("four",             formatted_lines[1]);
    CHECK_EQUAL(2, (int)formatted_lines.size());

    lines_max_w = 15;
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two three",    formatted_lines[0]);
    CHECK_EQUAL("four",             formatted_lines[1]);
    CHECK_EQUAL(2, (int)formatted_lines.size());

    lines_max_w = 11;
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("one two",          formatted_lines[0]);
    CHECK_EQUAL("three four",       formatted_lines[1]);
    CHECK_EQUAL(2, (int)formatted_lines.size());

    lines_max_w    = 4;
    str         = "123456";
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("123456",           formatted_lines[0]);
    CHECK_EQUAL(1, (int)formatted_lines.size());

    lines_max_w    = 4;
    str         = "12 345678";
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK_EQUAL("12",               formatted_lines[0]);
    CHECK_EQUAL("345678",           formatted_lines[1]);
    CHECK_EQUAL(2, (int)formatted_lines.size());

    str = "";
    text_format::split(str, lines_max_w, formatted_lines);
    CHECK(formatted_lines.empty());
}

TEST_FIXTURE(BasicFixture, line_calculation)
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

    // Test disallowing outside map
    line_calc::calc_new_line(P(1, 0), P(-9, 0), true, 999, false, line);
    CHECK(line.size() == 2);
    CHECK(line[0] == P(1, 0));
    CHECK(line[1] == P(0, 0));

    // Test travel limit parameter
    line_calc::calc_new_line(origin, P(20, 0), true, 2, true, line);
    CHECK(line.size() == 3);
    CHECK(line[0] == origin);
    CHECK(line[1] == P(1, 0));
    CHECK(line[2] == P(2, 0));

    // Test precalculated FOV line offsets
    const std::vector<P>* delta_line =
        line_calc::fov_delta_line(P(3, 3), fov_std_radi_db);

    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(1, 1));
    CHECK(delta_line->at(2) == P(2, 2));
    CHECK(delta_line->at(3) == P(3, 3));

    delta_line = line_calc::fov_delta_line(P(-3, 3), fov_std_radi_db);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(-1, 1));
    CHECK(delta_line->at(2) == P(-2, 2));
    CHECK(delta_line->at(3) == P(-3, 3));

    delta_line = line_calc::fov_delta_line(P(3, -3), fov_std_radi_db);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(1, -1));
    CHECK(delta_line->at(2) == P(2, -2));
    CHECK(delta_line->at(3) == P(3, -3));

    delta_line = line_calc::fov_delta_line(P(-3, -3), fov_std_radi_db);
    CHECK(delta_line->size() == 4);
    CHECK(delta_line->at(0) == P(0, 0));
    CHECK(delta_line->at(1) == P(-1, -1));
    CHECK(delta_line->at(2) == P(-2, -2));
    CHECK(delta_line->at(3) == P(-3, -3));

    // Check constraints for retrieving FOV offset lines
    // Delta > parameter max distance
    delta_line = line_calc::fov_delta_line(P(3, 0), 2);
    CHECK(!delta_line);
    // Delta > limit of precalculated
    delta_line = line_calc::fov_delta_line(P(50, 0), 999);
    CHECK(!delta_line);
}

TEST_FIXTURE(BasicFixture, fov)
{
    bool blocked[map_w][map_h] = {};

    const int x = map_w_half;
    const int y = map_h_half;

    map::player->pos = P(x, y);

    LosResult fov[map_w][map_h];

    fov::run(map::player->pos, blocked, fov);

    const int r = fov_std_radi_int;

    // Not blocked
    CHECK(!fov[x    ][y    ].is_blocked_hard);
    CHECK(!fov[x + 1][y    ].is_blocked_hard);
    CHECK(!fov[x - 1][y    ].is_blocked_hard);
    CHECK(!fov[x    ][y + 1].is_blocked_hard);
    CHECK(!fov[x    ][y - 1].is_blocked_hard);
    CHECK(!fov[x + 2][y + 2].is_blocked_hard);
    CHECK(!fov[x - 2][y + 2].is_blocked_hard);
    CHECK(!fov[x + 2][y - 2].is_blocked_hard);
    CHECK(!fov[x - 2][y - 2].is_blocked_hard);
    CHECK(!fov[x + r][y    ].is_blocked_hard);
    CHECK(!fov[x - r][y    ].is_blocked_hard);
    CHECK(!fov[x    ][y + r].is_blocked_hard);
    CHECK(!fov[x    ][y - r].is_blocked_hard);

    // Blocked (not within FOV range)
    CHECK(fov[x + r + 1][y        ].is_blocked_hard);
    CHECK(fov[x - r - 1][y        ].is_blocked_hard);
    CHECK(fov[x        ][y + r + 1].is_blocked_hard);
    CHECK(fov[x        ][y - r - 1].is_blocked_hard);

    // Corners
    CHECK(fov[x + r][y - r].is_blocked_hard);
    CHECK(fov[x - r][y - r].is_blocked_hard);
    CHECK(fov[x + r][y + r].is_blocked_hard);
    CHECK(fov[x - r][y + r].is_blocked_hard);

    CHECK(fov[x + r - 1][y - r + 1].is_blocked_hard);
    CHECK(fov[x - r + 1][y - r + 1].is_blocked_hard);
    CHECK(fov[x + r - 1][y + r - 1].is_blocked_hard);
    CHECK(fov[x - r + 1][y + r - 1].is_blocked_hard);
}

TEST_FIXTURE(BasicFixture, light_map)
{
    // Put walls on the edge of the map, and floor in all other cells, and make
    // all cells dark
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const P p(x, y);

            Cell& cell = map::cells[p.x][p.y];

            cell.is_dark    = true;
            cell.is_lit     = false;

            if (map::is_pos_inside_map(p, false))
            {
                map::put(new Floor(p));
            }
            else // Is on edge of map
            {
                map::put(new Wall(p));
            }
        }
    }

    map::player->pos.set(40, 12);

    const P burn_pos(40, 10);

    Rigid* const burn_rigid = map::cells[burn_pos.x][burn_pos.y].rigid;

    while (burn_rigid->burn_state() != BurnState::burning)
    {
        burn_rigid->hit(DmgType::fire, DmgMethod::elemental);
    }

    game_time::update_light_map();

    map::player->update_fov();

    // Check that the cells around the burning floor is lit
    CHECK(map::cells[burn_pos.x    ][burn_pos.y    ].is_lit);
    CHECK(map::cells[burn_pos.x    ][burn_pos.y + 1].is_lit);
    CHECK(map::cells[burn_pos.x    ][burn_pos.y - 1].is_lit);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y    ].is_lit);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y + 1].is_lit);
    CHECK(map::cells[burn_pos.x - 1][burn_pos.y - 1].is_lit);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y    ].is_lit);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y + 1].is_lit);
    CHECK(map::cells[burn_pos.x + 1][burn_pos.y - 1].is_lit);

    // The cells around the burning floor should still be "dark"
    // (not affected by light)
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

TEST_FIXTURE(BasicFixture, throw_items)
{
    // -----------------------------------------------------------------
    // Throwing a throwing knife at a wall should make it land
    // in front of the wall - i.e. the cell it travelled through
    // before encountering the wall.
    //
    // . <- (5, 7)
    // # <- If aiming at wall here... (5, 8)
    // . <- ...throwing knife should finally land here (5, 9)
    // @ <- Player position (5, 10).
    // -----------------------------------------------------------------

    map::put(new Floor(P(5, 7)));
    map::put(new Floor(P(5, 9)));
    map::put(new Floor(P(5, 10)));
    map::player->pos = P(5, 10);
    P tgt(5, 8);
    Item* item = item_factory::mk(ItemId::thr_knife);
    throwing::throw_item(*(map::player), tgt, *item);
    CHECK(map::cells[5][9].item);
}

TEST_FIXTURE(BasicFixture, explosions)
{
    const int x0 = 5;
    const int y0 = 7;

    map::put(new Floor(P(x0, y0)));

    // Check wall destruction
    for (int i = 0; i < 2; ++i)
    {
        explosion::run(P(x0, y0), ExplType::expl);

        // Cells around the center, at a distance of 1, should be destroyed
        int r = 1;
        CHECK(map::cells[x0 + r][y0    ].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0 - r][y0    ].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0    ][y0 + r].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0    ][y0 - r].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0 + r][y0 + r].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0 + r][y0 - r].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0 - r][y0 + r].rigid->id() != FeatureId::wall);
        CHECK(map::cells[x0 - r][y0 - r].rigid->id() != FeatureId::wall);

        // Cells around the center, at a distance of 2, should NOT be destroyed
        r = 2;
        CHECK(map::cells[x0 + r][y0    ].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0 - r][y0    ].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0    ][y0 + r].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0    ][y0 - r].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0 + r][y0 + r].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0 + r][y0 - r].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0 - r][y0 + r].rigid->id() == FeatureId::wall);
        CHECK(map::cells[x0 - r][y0 - r].rigid->id() == FeatureId::wall);
    }

    // Check damage to actors
    Actor* a1 = actor_factory::mk(ActorId::rat, P(x0 + 1, y0));
    explosion::run(P(x0, y0), ExplType::expl);
    CHECK_EQUAL((int)ActorState::destroyed, int(a1->state()));

    // Check that corpses can be destroyed, and do not block living actors
    const int nr_corpses = 3;
    Actor* corpses[nr_corpses];

    for (int i = 0; i < nr_corpses; ++i)
    {
        corpses[i] = actor_factory::mk(ActorId::rat, P(x0 + 1, y0));
        corpses[i]->die(false, false, false);
    }

    a1 = actor_factory::mk(ActorId::rat, P(x0 + 1, y0));
    explosion::run(P(x0, y0), ExplType::expl);

    for (int i = 0; i < nr_corpses; ++i)
    {
        CHECK_EQUAL((int)ActorState::destroyed, int(corpses[i]->state()));
    }

    CHECK_EQUAL((int)ActorState::destroyed, int(a1->state()));

    // Check explosion applying Burning to living and dead actors
    for (int i = 0; i < nr_corpses; ++i)
    {
        corpses[i] = actor_factory::mk(ActorId::rat, P(x0 + 1, y0));
        corpses[i]->die(false, false, false);
    }

    a1        = actor_factory::mk(ActorId::rat, P(x0 - 1, y0));
    Actor* a2 = actor_factory::mk(ActorId::rat, P(x0 + 1, y0));

    explosion::run(P(x0, y0),
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   0,
                   ExplExclCenter::no,
                   {new PropBurning(PropTurns::std)});

    CHECK(a1->prop_handler().prop(PropId::burning));
    CHECK(a2->prop_handler().prop(PropId::burning));

    CHECK(a1->has_prop(PropId::burning));
    CHECK(a2->has_prop(PropId::burning));

    for (int i = 0; i < nr_corpses; ++i)
    {
        PropHandler& props = corpses[i]->prop_handler();

        CHECK(props.prop(PropId::burning));
        CHECK(props.has_prop(PropId::burning));
    }

    // Check that the explosion can handle the map edge (e.g. that it does not
    // destroy the edge wall, or go outside the map - possibly causing a crash)

    // North-west edge
    int x = 1;
    int y = 1;
    map::put(new Floor(P(x, y)));
    explosion::run(P(x, y), ExplType::expl);
    CHECK(map::cells[x + 1][y    ].rigid->id() != FeatureId::wall);
    CHECK(map::cells[x    ][y + 1].rigid->id() != FeatureId::wall);
    CHECK(map::cells[x - 1][y    ].rigid->id() == FeatureId::wall);
    CHECK(map::cells[x    ][y - 1].rigid->id() == FeatureId::wall);

    // South-east edge
    x = map_w - 2;
    y = map_h - 2;
    map::put(new Floor(P(x, y)));
    explosion::run(P(x, y), ExplType::expl);
    CHECK(map::cells[x - 1][y    ].rigid->id() != FeatureId::wall);
    CHECK(map::cells[x    ][y - 1].rigid->id() != FeatureId::wall);
    CHECK(map::cells[x + 1][y    ].rigid->id() == FeatureId::wall);
    CHECK(map::cells[x    ][y + 1].rigid->id() == FeatureId::wall);
}

TEST_FIXTURE(BasicFixture, monster_stuck_in_spider_web)
{
    // -----------------------------------------------------------------
    // Test that-
    // * a monster can get stuck in a spider web,
    // * the monster can get loose, and
    // * the web can get destroyed
    // -----------------------------------------------------------------

    const P pos_l(1, 4);
    const P pos_r(2, 4);

    // Spawn left floor cell
    map::put(new Floor(pos_l));

    // Conditions for finished test
    bool tested_stuck               = false;
    bool tested_loose_web_interact  = false;
    bool tested_loose_web_destroyed = false;

    while (!tested_stuck ||
           !tested_loose_web_interact ||
           !tested_loose_web_destroyed)
    {
        // Spawn right floor cell
        map::put(new Floor(pos_r));

        // Spawn a monster that can get stuck in the web
        Actor* const  actor = actor_factory::mk(ActorId::zombie, pos_l);
        Mon* const    mon   = static_cast<Mon*>(actor);

        // Create a spider web in the right cell
        const auto    mimic_id    = map::cells[pos_r.x][pos_r.y].rigid->id();
        const auto&   mimic_data  = feature_data::data(mimic_id);
        auto* const   mimic       = static_cast<Rigid*>(mimic_data.mk_obj(pos_r));

        map::put(new Trap(pos_r, mimic, TrapId::web));

        // Move the monster into the trap, and back again
        mon->aware_of_player_counter_ = 20000; // > 0 req. for triggering trap
        mon->pos = pos_l;
        mon->move(Dir::right);

        CHECK(mon->pos == pos_r);

        mon->move(Dir::left);
        mon->move(Dir::left);

        // Check conditions
        if (mon->pos == pos_r)
        {
            // Monster is stuck
            tested_stuck = true;
        }
        else if (mon->pos == pos_l)
        {
            // Monster managed to break free

            const auto feature_id = map::cells[pos_r.x][pos_r.y].rigid->id();

            if (feature_id == FeatureId::floor)
            {
                tested_loose_web_destroyed = true;
            }
            else // Not floor
            {
                tested_loose_web_interact = true;
            }
        }

        // Remove the monster
        actor_factory::delete_all_mon();
    }

    // Check that all cases have been triggered (not really necessary, it just
    // verifies that the loop above is correctly written).
    CHECK(tested_stuck);
    CHECK(tested_loose_web_interact);
    CHECK(tested_loose_web_destroyed);
}

TEST_FIXTURE(BasicFixture, using_inventory)
{
    const P p(10, 10);
    map::put(new Floor(p));
    map::player->pos = p;

    Inventory& inv = map::player->inv();

    InvSlot& body_slot = inv.slots_[(size_t)SlotId::body];

    delete body_slot.item;

    body_slot.item = nullptr;

    PropHandler& props = map::player->prop_handler();

    // Check that no props are enabled
    for (size_t i = 0; i < (size_t)PropId::END; ++i)
    {
        CHECK(!props.has_prop((PropId)i));
        CHECK(!props.prop((PropId)i));
    }

    // Wear asbesthos suit
    Item* item = item_factory::mk(ItemId::armor_asb_suit);

    inv.put_in_slot(SlotId::body,
                    item,
                    Verbosity::verbose);

    // Check that the props are applied
    int nr_props = 0;

    for (size_t i = 0u; i < (size_t)PropId::END; ++i)
    {
        if (props.has_prop((PropId)i))
        {
            ++nr_props;
        }
    }

    CHECK_EQUAL(3, nr_props);

    CHECK(props.prop(PropId::r_fire));
    CHECK(props.prop(PropId::r_elec));
    CHECK(props.prop(PropId::r_acid));

    CHECK(props.has_prop(PropId::r_fire));
    CHECK(props.has_prop(PropId::r_elec));
    CHECK(props.has_prop(PropId::r_acid));

    // Take off asbeshos suit
    inv.try_unequip_slot(SlotId::body);

    CHECK(inv.backpack_idx(ItemId::armor_asb_suit) != -1);

    // Check that the properties are cleared
    for (int i = 0; i < (int)PropId::END; ++i)
    {
        CHECK(!props.has_prop((PropId)i));
        CHECK(!props.prop((PropId)i));
    }

    // Wear the asbeshos suit again
    inv.equip_backpack_item(inv.backpack_idx(ItemId::armor_asb_suit),
                            SlotId::body);

    game_time::tick();

    // Check that the props are applied
    nr_props = 0;

    for (int i = 0; i < (int)PropId::END; ++i)
    {
        if (props.has_prop((PropId)i))
        {
            ++nr_props;
        }
    }

    CHECK_EQUAL(3, nr_props);

    CHECK(props.prop(PropId::r_fire));
    CHECK(props.prop(PropId::r_elec));
    CHECK(props.prop(PropId::r_acid));

    CHECK(props.has_prop(PropId::r_fire));
    CHECK(props.has_prop(PropId::r_elec));
    CHECK(props.has_prop(PropId::r_acid));

    // Drop the asbeshos suit on the ground
    item_drop::try_drop_item_from_inv(*map::player,
                                      InvType::slots,
                                      (int)SlotId::body,
                                      1);

    // Check that no item exists in body slot
    CHECK(!body_slot.item);

    // Check that the item is on the ground
    Cell& cell = map::cells[p.x][p.y];
    CHECK(cell.item);

    // Check that the properties are cleared
    for (int i = 0; i < (int)PropId::END; ++i)
    {
        CHECK(!props.has_prop((PropId)i));
        CHECK(!props.prop((PropId)i));
    }

    // Wear the same dropped asbesthos suit again
    inv.put_in_slot(SlotId::body,
                    cell.item,
                    Verbosity::verbose);

    cell.item = nullptr;

    // Check that the props are applied
    nr_props = 0;

    for (int i = 0; i < (int)PropId::END; ++i)
    {
        if (props.has_prop((PropId)i))
        {
            ++nr_props;
        }
    }

    CHECK_EQUAL(3, nr_props);

    CHECK(props.prop(PropId::r_fire));
    CHECK(props.prop(PropId::r_elec));
    CHECK(props.prop(PropId::r_acid));

    CHECK(props.has_prop(PropId::r_fire));
    CHECK(props.has_prop(PropId::r_elec));
    CHECK(props.has_prop(PropId::r_acid));

    // Destroy the asbesthos suit
    for (int i = 0; i < 10; ++i)
    {
        map::player->restore_hp(99999, true /* Restoring above max */);

        explosion::run(map::player->pos, ExplType::expl);

        // Clear wound property
        props.end_prop(PropId::wound);
    }

    // Check that the asbesthos suit is destroyed
    CHECK(!body_slot.item);

    // Check that all properties from asbesthos suit are cleared
    for (int i = 0; i < (int)PropId::END; ++i)
    {
        CHECK(!props.has_prop((PropId)i));
        CHECK(!props.prop((PropId)i));
    }
}

TEST_FIXTURE(BasicFixture, saving_game)
{
    // Item data
    item_data::data[(size_t)ItemId::scroll_telep].is_tried = true;
    item_data::data[(size_t)ItemId::scroll_opening].is_identified = true;

    // Bonus
    player_bon::pick_bg(Bg::rogue);
    player_bon::traits[(size_t)Trait::healer] = true;

    // Player inventory
    Inventory& inv = map::player->inv();

    // First, remove all present items to get a clean state
    std::vector<Item*>& gen = inv.backpack_;

    for (Item* item : gen)
    {
        delete item;
    }

    gen.clear();

    for (size_t i = 0; i < (size_t)SlotId::END; ++i)
    {
        auto& slot = inv.slots_[i];

        if (slot.item)
        {
            delete slot.item;
            slot.item = nullptr;
        }
    }

    // Put new items
    Item* item = item_factory::mk(ItemId::mi_go_gun);

    inv.put_in_slot(SlotId::wpn,
                    item,
                    Verbosity::verbose);

    // Wear asbestos suit to test properties from wearing items
    item = item_factory::mk(ItemId::armor_asb_suit);
    inv.put_in_slot(SlotId::body,
                    item,
                    Verbosity::verbose);

    item = item_factory::mk(ItemId::pistol_mag);
    static_cast<AmmoMag*>(item)->ammo_ = 1;
    inv.put_in_backpack(item);

    item = item_factory::mk(ItemId::pistol_mag);
    static_cast<AmmoMag*>(item)->ammo_ = 2;
    inv.put_in_backpack(item);

    item = item_factory::mk(ItemId::pistol_mag);
    static_cast<AmmoMag*>(item)->ammo_ = 3;
    inv.put_in_backpack(item);

    item = item_factory::mk(ItemId::pistol_mag);
    static_cast<AmmoMag*>(item)->ammo_ = 3;
    inv.put_in_backpack(item);

    item = item_factory::mk(ItemId::device_blaster);
    static_cast<StrangeDevice*>(item)->condition_ = Condition::shoddy;
    inv.put_in_backpack(item);

    item = item_factory::mk(ItemId::lantern);

    DeviceLantern* lantern = static_cast<DeviceLantern*>(item);

    lantern->nr_turns_left_         = 789;
    lantern->nr_flicker_turns_left_ = 456;
    lantern->working_state_         = LanternWorkingState::flicker;
    lantern->is_activated_          = true;

    inv.put_in_backpack(item);

    // Player
    ActorDataT& def = map::player->data();

    def.name_a = def.name_the = "TEST PLAYER";

    map::player->change_max_hp(5, Verbosity::silent);

    // map
    map::dlvl = 7;

    // Actor data
    actor_data::data[(size_t)ActorId::END - 1].nr_kills = 123;

    // Learned spells
    player_spells::learn_spell(SpellId::bless, Verbosity::silent);
    player_spells::learn_spell(SpellId::aza_wrath, Verbosity::silent);

    // Applied properties
    PropHandler& props = map::player->prop_handler();
    props.try_add(new PropRSleep(PropTurns::specific, 3));
    props.try_add(new PropDiseased(PropTurns::indefinite));
    props.try_add(new PropBlessed(PropTurns::std));

    // Check a a few of the props applied
    Prop* prop = props.prop(PropId::diseased);
    CHECK(prop);

    prop = props.prop(PropId::blessed);
    CHECK(prop);

    // Check a prop that was NOT applied
    prop = props.prop(PropId::confused);
    CHECK(!prop);

    // map sequence
    map_travel::map_list[5] = {MapType::rat_cave, IsMainDungeon::yes};
    map_travel::map_list[7] = {MapType::leng,              IsMainDungeon::no};

    saving::save_game();
    CHECK(saving::is_save_available());
}

TEST_FIXTURE(BasicFixture, loading_game)
{
    CHECK(saving::is_save_available());

    const int player_max_hp_before_load = map::player->hp_max(true);

    saving::load_game();

    // Item data
    CHECK_EQUAL(true,  item_data::data[int(ItemId::scroll_telep)].is_tried);
    CHECK_EQUAL(false, item_data::data[int(ItemId::scroll_telep)].is_identified);
    CHECK_EQUAL(true,  item_data::data[int(ItemId::scroll_opening)].is_identified);
    CHECK_EQUAL(false, item_data::data[int(ItemId::scroll_opening)].is_tried);
    CHECK_EQUAL(false, item_data::data[int(ItemId::scroll_det_mon)].is_tried);
    CHECK_EQUAL(false, item_data::data[int(ItemId::scroll_det_mon)].is_identified);

    // Bonus
    CHECK_EQUAL((int)Bg::rogue, int(player_bon::bg()));
    CHECK(player_bon::traits[(size_t)Trait::healer]);
    CHECK(!player_bon::traits[size_t(Trait::fast_shooter)]);

    // Player inventory
    Inventory& inv  = map::player->inv();
    auto& genInv    = inv.backpack_;

    CHECK_EQUAL(6, int(genInv.size()));
    CHECK_EQUAL(int(ItemId::mi_go_gun), int(inv.item_in_slot(SlotId::wpn)->data().id));
    CHECK_EQUAL(int(ItemId::armor_asb_suit), int(inv.item_in_slot(SlotId::body)->data().id));

    int nr_mag_with_1 = 0;
    int nr_mag_with_2 = 0;
    int nr_mag_with_3 = 0;
    bool is_sentry_device_found = false;
    bool is_lantern_found       = false;

    for (Item* item : genInv)
    {
        ItemId id = item->id();

        if (id == ItemId::pistol_mag)
        {
            switch (static_cast<AmmoMag*>(item)->ammo_)
            {
            case 1:
                ++nr_mag_with_1;
                break;

            case 2:
                ++nr_mag_with_2;
                break;

            case 3:
                ++nr_mag_with_3;
                break;

            default:
                break;
            }
        }
        else if (id == ItemId::device_blaster)
        {
            is_sentry_device_found = true;
            CHECK_EQUAL((int)Condition::shoddy,
                        int(static_cast<StrangeDevice*>(item)->condition_));
        }
        else if (id == ItemId::lantern)
        {
            is_lantern_found = true;
            DeviceLantern* lantern = static_cast<DeviceLantern*>(item);
            CHECK_EQUAL(789, lantern->nr_turns_left_);
            CHECK_EQUAL(456, lantern->nr_flicker_turns_left_);
            CHECK_EQUAL((int)LanternWorkingState::flicker, int(lantern->working_state_));
            CHECK(lantern->is_activated_);
        }
    }

    CHECK_EQUAL(1, nr_mag_with_1);
    CHECK_EQUAL(1, nr_mag_with_2);
    CHECK_EQUAL(2, nr_mag_with_3);
    CHECK(is_sentry_device_found);
    CHECK(is_lantern_found);

    // Player
    ActorDataT& def = map::player->data();

    CHECK_EQUAL("TEST PLAYER", def.name_a);
    CHECK_EQUAL("TEST PLAYER", def.name_the);

    // Check max HP (affected by disease)
    CHECK_EQUAL((player_max_hp_before_load + 5) / 2, map::player->hp_max(true));

    // map
    CHECK_EQUAL(7, map::dlvl);

    // Actor data
    CHECK_EQUAL(123, actor_data::data[(int)ActorId::END - 1].nr_kills);

    // Learned spells
    CHECK(player_spells::is_spell_learned(SpellId::bless));
    CHECK(player_spells::is_spell_learned(SpellId::aza_wrath));
    CHECK_EQUAL(false, player_spells::is_spell_learned(SpellId::mayhem));

    // Properties
    PropHandler& props = map::player->prop_handler();
    Prop* prop = props.prop(PropId::diseased);
    CHECK(prop);
    CHECK(props.has_prop(PropId::diseased));
    CHECK_EQUAL(-1, prop->nr_turns_left());

    // Check currrent HP (should not be affected)
    CHECK_EQUAL(map::player->data().hp, map::player->hp());

    prop = props.prop(PropId::r_sleep);
    CHECK(prop);
    CHECK(props.has_prop(PropId::r_sleep));
    CHECK_EQUAL(3, prop->nr_turns_left());

    prop = props.prop(PropId::blessed);
    CHECK(prop);
    CHECK(props.has_prop(PropId::blessed));
    CHECK(prop->nr_turns_left() > 0);

    // Properties from worn item
    prop = props.prop(PropId::r_acid);
    CHECK(prop);
    CHECK(prop->nr_turns_left() == -1);
    prop = props.prop(PropId::r_fire);
    CHECK(prop);
    CHECK(prop->nr_turns_left() == -1);

    // map sequence
    auto mapData = map_travel::map_list[3];
    CHECK(mapData.type              == MapType::std);
    CHECK(mapData.is_main_dungeon   == IsMainDungeon::yes);

    mapData = map_travel::map_list[5];
    CHECK(mapData.type              == MapType::rat_cave);
    CHECK(mapData.is_main_dungeon   == IsMainDungeon::yes);

    mapData = map_travel::map_list[7];
    CHECK(mapData.type              == MapType::leng);
    CHECK(mapData.is_main_dungeon   == IsMainDungeon::no);

    // Game time
    CHECK_EQUAL(0, game_time::turn_nr());
}

TEST_FIXTURE(BasicFixture, floodfilling)
{
    bool blocked[map_w][map_h] = {};

    // Set the edge of the map as blocking
    for (int y = 0; y < map_h; ++y)
    {
        blocked[0][y] = blocked[map_w - 1][y] = true;
    }

    for (int x = 0; x < map_w; ++x)
    {
        blocked[x][0] = blocked[x][map_h - 1] = true;
    }

    int flood[map_w][map_h];

    floodfill(P(20, 10),
              blocked,
              flood,
              INT_MAX,
              P(-1, -1),
              true);

    CHECK_EQUAL(0, flood[20][10]);
    CHECK_EQUAL(1, flood[19][10]);
    CHECK_EQUAL(1, flood[21][10]);
    CHECK_EQUAL(1, flood[20][11]);
    CHECK_EQUAL(1, flood[21][11]);
    CHECK_EQUAL(4, flood[24][12]);
    CHECK_EQUAL(4, flood[24][14]);
    CHECK_EQUAL(5, flood[24][15]);
    CHECK_EQUAL(0, flood[0][0]);
    CHECK_EQUAL(0, flood[map_w - 1][map_h - 1]);
}

TEST_FIXTURE(BasicFixture, pathfinding)
{
    std::vector<P> path;
    bool blocked[map_w][map_h] = {};

    for (int y = 0; y < map_h; ++y)
    {
        blocked[0][y] = blocked[map_w - 1][y] = false;
    }

    for (int x = 0; x < map_w; ++x)
    {
        blocked[x][0] = blocked[x][map_h - 1] = false;
    }

    pathfind(P(20, 10),
                  P(25, 10),
                  blocked,
                  path);

    CHECK(!path.empty());
    CHECK(path.back() != P(20, 10));
    CHECK_EQUAL(25, path.front().x);
    CHECK_EQUAL(10, path.front().y);
    CHECK_EQUAL(5, int(path.size()));

    pathfind(P(20, 10),
                  P(5, 3),
                  blocked,
                  path);

    CHECK(!path.empty());
    CHECK(path.back() != P(20, 10));
    CHECK_EQUAL(5, path.front().x);
    CHECK_EQUAL(3, path.front().y);
    CHECK_EQUAL(15, int(path.size()));

    blocked[10][5] = true;

    pathfind(P(7, 5),
                  P(20, 5),
                  blocked,
                  path);

    CHECK(!path.empty());
    CHECK(path.back() != P(7, 5));
    CHECK_EQUAL(20, path.front().x);
    CHECK_EQUAL(5, path.front().y);
    CHECK_EQUAL(13, int(path.size()));
    CHECK(find(begin(path), end(path), P(10, 5)) == end(path));

    blocked[19][4] = blocked[19][5] =  blocked[19][6] = true;

    pathfind(P(7, 5),
                  P(20, 5),
                  blocked,
                  path);

    CHECK(!path.empty());
    CHECK(path.back() != P(7, 5));
    CHECK_EQUAL(20, path.front().x);
    CHECK_EQUAL(5, path.front().y);
    CHECK_EQUAL(14, int(path.size()));
    CHECK(find(begin(path), end(path), P(19, 4)) == end(path));
    CHECK(find(begin(path), end(path), P(19, 5)) == end(path));
    CHECK(find(begin(path), end(path), P(19, 6)) == end(path));

    pathfind(P(40, 10),
                  P(43, 15),
                  blocked,
                  path,
                  false);

    CHECK(!path.empty());
    CHECK(path.back() != P(40, 10));
    CHECK_EQUAL(43, path.front().x);
    CHECK_EQUAL(15, path.front().y);
    CHECK_EQUAL(8, int(path.size()));

    blocked[41][10] = blocked[40][11]  = true;

    pathfind(P(40, 10),
                  P(43, 15),
                  blocked,
                  path,
                  false);

    CHECK(!path.empty());
    CHECK(path.back() != P(40, 10));
    CHECK_EQUAL(43, path.front().x);
    CHECK_EQUAL(15, path.front().y);
    CHECK_EQUAL(10, int(path.size()));
}

TEST_FIXTURE(BasicFixture, map_parse_expand_one)
{
    bool in[map_w][map_h] = {};

    in[10][5] = true;

    bool out[map_w][map_h];
    map_parsers::expand(in, out);

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
    map_parsers::expand(in, out);

    CHECK(out[10][5]);
    CHECK(out[11][5]);
    CHECK(!out[12][5]);
    CHECK(out[13][5]);
    CHECK(out[14][5]);

    in[12][5] = true;
    map_parsers::expand(in, out);
    CHECK(out[12][4]);
    CHECK(out[12][5]);
    CHECK(out[12][6]);

    // Check that old values are cleared
    std::fill_n(*in, nr_map_cells, 0);
    in[40][10] = true;
    map_parsers::expand(in, out);
    CHECK(out[39][10]);
    CHECK(out[40][10]);
    CHECK(out[41][10]);
    CHECK(!out[10][5]);
    CHECK(!out[12][5]);
    CHECK(!out[14][5]);
}

TEST_FIXTURE(BasicFixture, find_corridor_entries)
{
    auto bool_map = [](const std::vector<P>& vec, bool out[map_w][map_h])
    {
        std::fill_n(*out, nr_map_cells, false);

        for (const P& p : vec)
        {
            out[p.x][p.y] = true;
        }
    };

    // ------------------------------------------------ Square, normal sized room
    R room_rect(20, 5, 30, 10);

    Room* room = room_factory::mk(RoomType::plain, room_rect);

    for (int y = room_rect.p0.y; y <= room_rect.p1.y; ++y)
    {
        for (int x = room_rect.p0.x; x <= room_rect.p1.x; ++x)
        {
            map::put(new Floor(P(x, y)));
            map::room_map[x][y] = room;
        }
    }

    std::vector<P> entry_list;
    mapgen::valid_corridor_entries(*room, entry_list);

    bool entry_map[map_w][map_h];
    bool_map(entry_list, entry_map);

    CHECK(!entry_map[19][4]);
    CHECK (entry_map[19][5]);
    CHECK (entry_map[20][4]);
    CHECK(!entry_map[20][5]);
    CHECK( entry_map[21][4]);
    CHECK( entry_map[25][4]);
    CHECK(!entry_map[25][8]);
    CHECK( entry_map[29][4]);
    CHECK( entry_map[30][4]);
    CHECK(!entry_map[31][4]);

    // Check that a cell in the middle of the room is not an entry, even if it's not
    // belonging to the room
    map::room_map[25][7] = nullptr;
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    CHECK(!entry_map[25][7]);

    // The cell should also not be an entry if it's a wall and belonging to the room
    map::room_map[25][7] = room;
    map::put(new Wall(P(25, 7)));
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    CHECK(!entry_map[25][7]);

    // The cell should also not be an entry if it's a wall and not belonging to the room
    map::room_map[25][7] = nullptr;
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    CHECK(!entry_map[25][7]);

    // Check that the room can share an antry point with a nearby room
    room_rect = R(10, 5, 18, 10);

    Room* nearby_room = room_factory::mk(RoomType::plain, room_rect);

    for (int y = room_rect.p0.y; y <= room_rect.p1.y; ++y)
    {
        for (int x = room_rect.p0.x; x <= room_rect.p1.x; ++x)
        {
            map::put(new Floor(P(x, y)));
            map::room_map[x][y] = nearby_room;
        }
    }

    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    std::vector<P> entry_list_nearby_room;
    mapgen::valid_corridor_entries(*nearby_room, entry_list_nearby_room);
    bool entry_map_near_room[map_w][map_h];
    bool_map(entry_list_nearby_room, entry_map_near_room);

    for (int y = 5; y <= 10; ++y)
    {
        CHECK(entry_map[19][y]);
        CHECK(entry_map_near_room[19][y]);
    }

    delete nearby_room;

    // ------------------------------------------------ Room with only one cell
    delete room;
    room = room_factory::mk(RoomType::plain, {60, 10, 60, 10});
    map::put(new Floor(P(60, 10)));
    map::room_map[60][10] = room;
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    // 59 60 61
    // #  #  # 9
    // #  .  # 10
    // #  #  # 11
    CHECK(!entry_map[59][9]);
    CHECK( entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK( entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK( entry_map[61][10]);
    CHECK(!entry_map[59][11]);
    CHECK( entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    // Add an adjacent floor above the room
    // 59 60 61
    // #  .  # 9
    // #  .  # 10
    // #  #  # 11
    map::put(new Floor(P(60, 9)));
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    CHECK(!entry_map[59][9]);
    CHECK(!entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK( entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK( entry_map[61][10]);
    CHECK(!entry_map[59][11]);
    CHECK( entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    // Mark the adjacent floor as a room and check again
    Room* adj_room = room_factory::mk(RoomType::plain, {60, 9, 60, 9});
    map::room_map[60][9] = adj_room;
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

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

    // Make the room wider, entries should not be placed next to adjacent floor
    // 58 59 60 61
    // #  #  .  # 9
    // #  .  .  # 10
    // #  #  #  # 11
    room->r_.p0.x = 59;
    map::put(new Floor(P(59, 10)));
    map::room_map[59][10] = room;
    mapgen::valid_corridor_entries(*room, entry_list);
    bool_map(entry_list, entry_map);

    CHECK(!entry_map[58][9]);
    CHECK( entry_map[59][9]);
    CHECK(!entry_map[60][9]);
    CHECK(!entry_map[61][9]);
    CHECK( entry_map[58][10]);
    CHECK(!entry_map[59][10]);
    CHECK(!entry_map[60][10]);
    CHECK( entry_map[61][10]);
    CHECK(!entry_map[58][11]);
    CHECK( entry_map[59][11]);
    CHECK( entry_map[60][11]);
    CHECK(!entry_map[61][11]);

    // Remove the adjacent room, and check that the blocked entries are now placed
    // TODO

    delete room;
}

TEST(find_choke_points)
{
    bool blocked[map_w][map_h];

    // --------------------------------------------------------------------------
    // Very simple test with three adjacent free positions. Verify that the
    // center position is a choke point.
    // --------------------------------------------------------------------------
    std::fill_n(*blocked, nr_map_cells, true);

    blocked[20][10] = false;
    blocked[21][10] = false;
    blocked[22][10] = false;

    ChokePointData d;

    bool is_choke_point = mapgen::is_choke_point(P(21, 10),
                                                 blocked,
                                                 d);

    CHECK(is_choke_point);

    CHECK(d.p == P(21, 10));

    CHECK(d.sides[0].size() == 1);
    CHECK(d.sides[1].size() == 1);

    CHECK(d.sides[0][0] == P(20, 10));
    CHECK(d.sides[1][0] == P(22, 10));

    // --------------------------------------------------------------------------
    // The left position should NOT be a choke point
    // --------------------------------------------------------------------------
    is_choke_point = mapgen::is_choke_point(P(20, 10),
                                            blocked,
                                            d);

    CHECK(!is_choke_point);

    // --------------------------------------------------------------------------
    // Set the position above the center position to free, and verify that the
    // center position is no longer a choke point
    // --------------------------------------------------------------------------
    blocked[21][9] = false;

    is_choke_point = mapgen::is_choke_point(P(21, 10),
                                            blocked,
                                            d);

    CHECK(!is_choke_point);
}

TEST_FIXTURE(BasicFixture, connect_rooms_with_corridor)
{
    R room_area_1(P(1, 1), P(10, 10));
    R room_area_2(P(15, 4), P(23, 14));

    Room* room0 = room_factory::mk(RoomType::plain, room_area_1);
    Room* room1 = room_factory::mk(RoomType::plain, room_area_2);

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

    mapgen::mk_pathfind_corridor(*room0, *room1);

    int flood[map_w][map_h];

    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCmn(ParseActors::no)
        .run(blocked);

    floodfill(5,
                    blocked,
                    flood,
                    INT_MAX, -1,
                    true);

    CHECK(flood[20][10] > 0);

    delete room0;
    delete room1;
}

TEST_FIXTURE(BasicFixture, map_parse_cells_within_dist_of_others)
{
    bool in[map_w][map_h]   = {};
    bool out[map_w][map_h]  = {};

    in[20][10] = true;

    map_parsers::cells_within_dist_of_others(in, out, Range(0, 1));
    CHECK_EQUAL(false, out[18][10]);
    CHECK_EQUAL(true,  out[19][10]);
    CHECK_EQUAL(false, out[20][ 8]);
    CHECK_EQUAL(true,  out[20][ 9]);
    CHECK_EQUAL(true,  out[20][10]);
    CHECK_EQUAL(true,  out[20][11]);
    CHECK_EQUAL(true,  out[21][11]);

    map_parsers::cells_within_dist_of_others(in, out, Range(1, 1));
    CHECK_EQUAL(true,  out[19][10]);
    CHECK_EQUAL(false, out[20][10]);
    CHECK_EQUAL(true,  out[21][11]);

    map_parsers::cells_within_dist_of_others(in, out, Range(1, 5));
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

    map_parsers::cells_within_dist_of_others(in, out, Range(1, 1));
    CHECK_EQUAL(false, out[18][10]);
    CHECK_EQUAL(true,  out[19][10]);
    CHECK_EQUAL(false, out[20][10]);
    CHECK_EQUAL(true,  out[21][10]);
    CHECK_EQUAL(true,  out[22][10]);
    CHECK_EQUAL(false, out[23][10]);
    CHECK_EQUAL(true,  out[24][10]);
    CHECK_EQUAL(false, out[25][10]);
}

// -----------------------------------------------------------------------------
// Some code exercise
// -----------------------------------------------------------------------------
namespace
{

void check_connected()
{
    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCmn(ParseActors::no)
        .run(blocked);

    P stair_p(-1, -1);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const auto id = map::cells[x][y].rigid->id();

            if (id == FeatureId::stairs)
            {
                blocked[x][y] = false;

                stair_p.set(x, y);
            }
            else if (id == FeatureId::door)
            {
                blocked[x][y] = false;
            }
        }
    }

    // Check path from player to stairs
    std::vector<P> path;

    if (stair_p.x != -1)
    {
        pathfind(map::player->pos,
                       stair_p,
                       blocked,
                       path);
    }

    P path_front;

    if (!path.empty())
    {
        path_front = path.front();
    }

    // Stairs found?
    CHECK(stair_p.x != -1);

    // Path to stairs found?
    CHECK(!path.empty());

    // The end of the path should be the stairs position
    CHECK(path_front == stair_p);

    // Check that all of the map is connected
    const bool IS_CONNECTED = map_parsers::is_map_connected(blocked);

    CHECK(IS_CONNECTED);
}

void check_wall_placement(const P& origin)
{
    bool w[2][2]; // Wall
    bool f[2][2]; // Floor

    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            const P map_p(origin + P(x, y));

            const FeatureId id = map::cells[map_p.x][map_p.y].rigid->id();

            w[x][y] = id == FeatureId::wall;
            f[x][y] = id == FeatureId::floor;
        }
    }

    // Check that walls are not placed like this:
    // .#
    // #.
    bool bad_walls_found =
        f[0][0] && w[1][0] &&
        w[0][1] && f[1][1];

    // Check that walls are not placed like this:
    // #.
    // .#
    bad_walls_found =
        bad_walls_found ||
        (w[0][0] && f[1][0] &&
         f[0][1] && w[1][1]);

    CHECK(!bad_walls_found);
}

} // namespace

TEST_FIXTURE(BasicFixture, mapgen_std)
{
    for (int i = 0; i < 100; ++i)
    {
        bool map_ok = false;

        while (!map_ok)
        {
            map_ok = mapgen::mk_std_lvl();
        }

        check_connected();

        map::player->teleport();

        check_connected();

        const int x1 = map_w - 2;
        const int y1 = map_h - 2;

        for (int x = 0; x <= x1; ++x)
        {
            for (int y = 0; y <= y1; ++y)
            {
                check_wall_placement(P(x, y));
            }
        }
    }
}

#ifdef _WIN32
#undef main
#endif
int main()
{
    UnitTest::RunAllTests();
    return 0;
}
