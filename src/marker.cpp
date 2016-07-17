#include "marker.hpp"

#include <vector>

#include "input.hpp"
#include "inventory_handling.hpp"
#include "input.hpp"
#include "actor_player.hpp"
#include "attack.hpp"
#include "msg_log.hpp"
#include "look.hpp"
#include "throwing.hpp"
#include "render.hpp"
#include "map.hpp"
#include "item_factory.hpp"
#include "line_calc.hpp"
#include "config.hpp"
#include "feature_rigid.hpp"

namespace marker
{

namespace
{

P pos_;

void set_pos_to_closest_enemy_if_visible()
{
    std::vector<Actor*> seen_foes;
    map::player->seen_foes(seen_foes);
    std::vector<P> seen_foes_cells;

    map::actor_cells(seen_foes, seen_foes_cells);

    //If player sees enemies, suggest one for targeting
    if (!seen_foes_cells.empty())
    {
        pos_ = closest_pos(map::player->pos, seen_foes_cells);

        map::player->tgt_ = map::actor_at_pos(pos_);
    }
}

void try_move(const Dir dir)
{
    const P new_pos(pos_ + dir_utils::offset(dir));

    if (map::is_pos_inside_map(new_pos))
    {
        pos_ = new_pos;
    }
}

bool set_pos_to_tgt_if_visible()
{
    const Actor* const tgt = map::player->tgt_;

    if (tgt)
    {
        std::vector<Actor*> seen_foes;
        map::player->seen_foes(seen_foes);

        if (!seen_foes.empty())
        {
            for (auto* const actor : seen_foes)
            {
                if (tgt == actor)
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

P run(const MarkerUsePlayerTgt use_tgt,
      std::function<void(const P&,
                         CellOverlay overlay[map_w][map_h])> on_marker_at_pos,
      std::function<MarkerDone(const P&,
                                const KeyData&)> on_key_press,
      MarkerShowBlocked show_blocked,
      const int effective_range_lmt)
{
    pos_ = map::player->pos;

    if (use_tgt == MarkerUsePlayerTgt::yes)
    {
        //First, attempt to place marker at player target.
        if (!set_pos_to_tgt_if_visible())
        {
            //If no target available, attempt to place marker at closest
            //visible monster. This sets a new player target if successful.
            map::player->tgt_ = nullptr;
            set_pos_to_closest_enemy_if_visible();
        }
    }

    MarkerDone is_done = MarkerDone::no;

    CellOverlay overlay[map_w][map_h];

    while (is_done == MarkerDone::no)
    {
        //Print info such as name of actor at current position, etc.
        on_marker_at_pos(pos_, overlay);

        render::draw_map_state(UpdateScreen::no, overlay);

        std::vector<P> trail;

        const P origin(map::player->pos);

        line_calc::calc_new_line(origin,
                                 pos_,
                                 true,
                                 INT_MAX,
                                 false,
                                 trail);

        int blocked_from_idx = -1;

        if (show_blocked == MarkerShowBlocked::yes)
        {
            for (size_t i = 0; i < trail.size(); ++i)
            {
                const P& p(trail[i]);

                const Cell& c = map::cells[p.x][p.y];

                if (c.is_seen_by_player && !c.rigid->is_projectile_passable())
                {
                    blocked_from_idx = i;
                    break;
                }
            }
        }

        render::draw_marker(pos_,
                            trail,
                            effective_range_lmt,
                            blocked_from_idx,
                            overlay);

        render::update_screen();

        const KeyData& d = input::input();

        if (d.sdl_key == SDLK_RIGHT || d.key == '6' || d.key == 'l')
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

        if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            try_move(Dir::up);
            continue;
        }

        if (d.sdl_key == SDLK_LEFT || d.key == '4' || d.key == 'h')
        {
            if (d.is_shift_held)
            {
                try_move(Dir::up_left);
            }
            else if (d.is_ctrl_held)
            {
                try_move(Dir::down_left);
            }
            else
            {
                try_move(Dir::left);
            }

            continue;
        }

        if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            try_move(Dir::down);
            continue;
        }

        if (d.sdl_key == SDLK_PAGEUP || d.key == '9' || d.key == 'u')
        {
            try_move(Dir::up_right);
            continue;
        }

        if (d.sdl_key == SDLK_HOME || d.key == '7' || d.key == 'y')
        {
            try_move(Dir::up_left);
            continue;
        }

        if (d.sdl_key == SDLK_END || d.key == '1' || d.key == 'b')
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

        if (is_done == MarkerDone::yes)
        {
            render::draw_map_state();
        }
    }

    return pos_;
}

} //marker
