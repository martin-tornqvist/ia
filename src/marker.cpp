#include "marker.hpp"

#include <vector>

#include "input.hpp"
#include "inventory_handling.hpp"
#include "input.hpp"
#include "actor_player.hpp"
#include "attack.hpp"
#include "log.hpp"
#include "look.hpp"
#include "throwing.hpp"
#include "render.hpp"
#include "map.hpp"
#include "item_factory.hpp"
#include "line_calc.hpp"
#include "utils.hpp"
#include "config.hpp"

using namespace std;

namespace marker
{

namespace
{

Pos pos_;

void set_pos_to_closest_enemy_if_visible()
{
    vector<Actor*> seen_foes;
    Map::player->get_seen_foes(seen_foes);
    vector<Pos> seen_foes_cells;

    Utils::get_actor_cells(seen_foes, seen_foes_cells);

    //If player sees enemies, suggest one for targeting
    if (!seen_foes_cells.empty())
    {
        pos_ = Utils::get_closest_pos(Map::player->pos, seen_foes_cells);

        Map::player->tgt_ = Utils::get_actor_at_pos(pos_);
    }
}

void try_move(const Dir dir)
{
    const Pos new_pos(pos_ + Dir_utils::get_offset(dir));
    if (Utils::is_pos_inside_map(new_pos)) {pos_ = new_pos;}
}

bool set_pos_to_target_if_visible()
{
    const Actor* const target = Map::player->tgt_;

    if (target)
    {
        vector<Actor*> seen_foes;
        Map::player->get_seen_foes(seen_foes);

        if (!seen_foes.empty())
        {
            for (auto* const actor : seen_foes)
            {
                if (target == actor)
                {
                    pos_ = actor->pos;
                    return true;
                }
            }
        }
    }
    return false;
}

} //namespace

Pos run(const Marker_draw_tail draw_trail, const Marker_use_player_tgt use_target,
        function<void(const Pos&)> on_marker_at_pos,
        function<Marker_done(const Pos&, const Key_data&)> on_key_press,
        const int EFFECTIVE_RANGE_LMT)
{
    pos_ = Map::player->pos;

    if (use_target == Marker_use_player_tgt::yes)
    {
        //First, attempt to place marker at target.
        if (!set_pos_to_target_if_visible())
        {
            //If no target available, attempt to place marker at closest visible monster.
            //This sets a new target if successful.
            Map::player->tgt_ = nullptr;
            set_pos_to_closest_enemy_if_visible();
        }
    }

    Marker_done is_done = Marker_done::no;

    while (is_done == Marker_done::no)
    {
        //Print info such as name of actor at current position, etc.
        on_marker_at_pos(pos_);

        Render::draw_map_and_interface(false);

        vector<Pos> trail;

        if (draw_trail == Marker_draw_tail::yes)
        {
            const Pos origin(Map::player->pos);
            Line_calc::calc_new_line(origin, pos_, true, INT_MAX, false, trail);
        }

        Render::draw_marker(pos_, trail, EFFECTIVE_RANGE_LMT);

        Render::update_screen();

        const Key_data& d = Input::get_input();

        if (d.sdl_key == SDLK_RIGHT    || d.key == '6' || d.key == 'l')
        {
            if (d.is_shift_held)
            {
                try_move(Dir::up_right);
            }
            else if (d.is_ctrl_held)
            {
                try_move(Dir::down_right);
            }
            else
            {
                try_move(Dir::right);
            }
            continue;
        }
        if (d.sdl_key == SDLK_UP       || d.key == '8' || d.key == 'k')
        {
            try_move(Dir::up);
            continue;
        }
        if (d.sdl_key == SDLK_LEFT     || d.key == '4' || d.key == 'h')
        {
            if (d.is_shift_held)
            {
                try_move(Dir::up_left);
            }
            else if (d.is_ctrl_held)
            {
                try_move(Dir::down_left);
            }
            else {try_move(Dir::left);}
            continue;
        }
        if (d.sdl_key == SDLK_DOWN     || d.key == '2' || d.key == 'j')
        {
            try_move(Dir::down);
            continue;
        }
        if (d.sdl_key == SDLK_PAGEUP   || d.key == '9' || d.key == 'u')
        {
            try_move(Dir::up_right);
            continue;
        }
        if (d.sdl_key == SDLK_HOME     || d.key == '7' || d.key == 'y')
        {
            try_move(Dir::up_left);
            continue;
        }
        if (d.sdl_key == SDLK_END      || d.key == '1' || d.key == 'b')
        {
            try_move(Dir::down_left);
            continue;
        }
        if (d.sdl_key == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n')
        {
            try_move(Dir::down_right);
            continue;
        }

        //Run custom keypress events (firing ranged weapon, casting spell, etc)
        is_done = on_key_press(pos_, d);

        if (is_done == Marker_done::yes)
        {
            Render::draw_map_and_interface();
        }
    }
    return pos_;
}

} //Marker
