#include "marker.hpp"

#include <vector>

#include "io.hpp"
#include "inventory_handling.hpp"
#include "actor_player.hpp"
#include "attack.hpp"
#include "msg_log.hpp"
#include "look.hpp"
#include "throwing.hpp"
#include "map.hpp"
#include "item_factory.hpp"
#include "line_calc.hpp"
#include "config.hpp"
#include "feature_rigid.hpp"
#include "explosion.hpp"

// -----------------------------------------------------------------------------
// Marker state
// -----------------------------------------------------------------------------
void MarkerState::on_start()
{
    pos_ = map::player->pos;

    if (use_player_tgt())
    {
        // First, attempt to place marker at player target.
        const bool did_go_to_tgt = try_go_to_tgt();

        if (!did_go_to_tgt)
        {
            // If no target available, attempt to place marker at closest
            // visible monster. This sets a new player target if successful.
            map::player->tgt_ = nullptr;

            try_go_to_closest_enemy();
        }
    }

    on_moved();
}

void MarkerState::draw()
{
    std::vector<P> trail;

    const P origin(map::player->pos);

    line_calc::calc_new_line(origin,
                             pos_,
                             true,      // Stop at target
                             INT_MAX,   // Travel limit
                             false,     // Allow outside map
                             trail);

    int blocked_from_idx = -1;

    if (show_blocked())
    {
        for (size_t i = 0; i < trail.size(); ++i)
        {
            const P& p(trail[i]);

            const Cell& c = map::cells[p.x][p.y];

            if (c.is_seen_by_player &&
                !c.rigid->is_projectile_passable())
            {
                blocked_from_idx = i;
                break;
            }
        }
    }

    const int range = effective_range();

    draw_marker(pos_,
                trail,
                range,
                blocked_from_idx);

    on_draw();
}

void MarkerState::update()
{
    const auto input = io::get(true);

    msg_log::clear();

    switch (input.key)
    {

    //
    // Direction input is handle by the base class
    //
    case SDLK_RIGHT:
    case '6':
    case 'l':
    {
        if (input.is_shift_held)
        {
            move(Dir::up_right);
        }
        else if (input.is_ctrl_held)
        {
            move(Dir::down_right);
        }
        else
        {
            move(Dir::right);
        }
    }
    break;

    case SDLK_UP:
    case '8':
    case 'k':
    {
        move(Dir::up);
    }
    break;

    case SDLK_LEFT:
    case '4':
    case 'h':
    {
        if (input.is_shift_held)
        {
            move(Dir::up_left);
        }
        else if (input.is_ctrl_held)
        {
            move(Dir::down_left);
        }
        else
        {
            move(Dir::left);
        }
    }
    break;

    case SDLK_DOWN:
    case '2':
    case 'j':
    {
        move(Dir::down);
    }
    break;

    case SDLK_PAGEUP:
    case '9':
    case 'u':
    {
        move(Dir::up_right);
    }
    break;

    case SDLK_HOME:
    case '7':
    case 'y':
    {
        move(Dir::up_left);
    }
    break;

    case SDLK_END:
    case '1':
    case 'b':
    {
        move(Dir::down_left);
    }
    break;

    case SDLK_PAGEDOWN:
    case '3':
    case 'n':
    {
        move(Dir::down_right);
    }
    break;

    //
    // Input other than direction keys is passed to the inherited marker state
    //
    default:
    {
        handle_input(input);
    }
    break;
    }
}

void MarkerState::draw_marker(const P& p,
                              const std::vector<P>& trail,
                              const int effective_range,
                              const int blocked_from_idx)
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            auto& d = marker_render_data_[x][y];

            d.tile  = TileId::empty;
            d.glyph = 0;
        }
    }

    Clr clr = clr_green_lgt;

    // Tail
    for (size_t i = 1; i < trail.size(); ++i)
    {
        const P& tail_p = trail[i];

        const bool is_blocked =
            (blocked_from_idx != -1) &&
            ((int)i >= blocked_from_idx);

        // NOTE: Final color is stored for drawing the head
        if (is_blocked)
        {
            clr = clr_red_lgt;
        }
        else if (effective_range != -1)
        {
            const int cheb_dist = king_dist(trail[0], tail_p);

            if (cheb_dist > effective_range)
            {
                clr = clr_orange;
            }
        }

        // Draw tail until (but not including) the head
        if (i < (trail.size() - 1))
        {
            io::cover_cell_in_map(tail_p);

            auto& d = marker_render_data_[tail_p.x][tail_p.y];

            d.tile = TileId::aim_marker_trail;

            d.glyph = '*';

            d.clr = clr;

            d.clr_bg = clr_black;

            // If blocked, always draw a character (more distinct)
            if (config::is_tiles_mode() && !is_blocked)
            {
                io::draw_tile(d.tile,
                              Panel::map,
                              tail_p,
                              d.clr,
                              d.clr_bg);
            }
            else // Text mode, or blocked
            {
                io::draw_glyph(d.glyph,
                               Panel::map,
                               tail_p,
                               d.clr,
                               true,
                               d.clr_bg);
            }
        }
    }

    auto& d = marker_render_data_[p.x][p.y];

    d.tile = TileId::aim_marker_head;

    d.glyph = 'X';

    d.clr = clr;

    d.clr_bg = clr_black;

    // Head
    if (config::is_tiles_mode())
    {
        io::draw_tile(d.tile,
                      Panel::map,
                      p,
                      d.clr,
                      d.clr_bg);
    }
    else // Text mode
    {
        io::draw_glyph(d.glyph,
                       Panel::map,
                       p,
                       d.clr,
                       true,
                       d.clr_bg);
    }
}

void MarkerState::move(const Dir dir)
{
    const P new_pos(pos_ + dir_utils::offset(dir));

    if (map::is_pos_inside_map(new_pos))
    {
        pos_ = new_pos;

        on_moved();
    }
}

bool MarkerState::try_go_to_tgt()
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

void MarkerState::try_go_to_closest_enemy()
{
    std::vector<Actor*> seen_foes;
    map::player->seen_foes(seen_foes);

    std::vector<P> seen_foes_cells;
    map::actor_cells(seen_foes, seen_foes_cells);

    // If player sees enemies, suggest one for targeting
    if (!seen_foes_cells.empty())
    {
        pos_ = closest_pos(map::player->pos, seen_foes_cells);

        map::player->tgt_ = map::actor_at_pos(pos_);
    }
}

// -----------------------------------------------------------------------------
// View state
// -----------------------------------------------------------------------------
void Viewing::on_moved()
{
    msg_log::clear();
    look::print_location_info_msgs(pos_);

    const auto* const actor = map::actor_at_pos(pos_);

    if (actor                   &&
        actor != map::player    &&
        map::player->can_see_actor(*actor))
    {
        msg_log::add("[v] for description");
    }

    msg_log::add(cancel_info_str_no_space);
}

void Viewing::handle_input(const InputData& input)
{
    if (input.key == 'v')
    {
        const auto* const actor = map::actor_at_pos(pos_);

        if (actor                   &&
            actor != map::player    &&
            map::player->can_see_actor(*actor))
        {
            msg_log::clear();

            look::print_detailed_actor_descr(*actor);
        }
    }
    else if (input.key == SDLK_SPACE ||
             input.key == SDLK_ESCAPE)
    {
        msg_log::clear();

        states::pop();
    }
}

// -----------------------------------------------------------------------------
// Aim marker state
// -----------------------------------------------------------------------------
void Aiming::on_moved()
{
    look::print_location_info_msgs(pos_);

    auto* const actor = map::actor_at_pos(pos_);

    if (actor &&
        !actor->is_player() &&
        map::player->can_see_actor(*actor))
    {
        const bool gets_undead_bane_bon =
            player_bon::gets_undead_bane_bon(actor->data());

        if (!actor->has_prop(PropId::ethereal) ||
            gets_undead_bane_bon)
        {
            RangedAttData data(map::player,
                               map::player->pos,    // Origin
                               actor->pos,          // Aim position
                               actor->pos,          // Current position
                               wpn_);

            msg_log::add(to_str(data.hit_chance_tot) + "% hit chance.");
        }
    }

    msg_log::add("[f] to fire" + cancel_info_str);
}

void Aiming::handle_input(const InputData& input)
{
    switch (input.key)
    {
    case SDLK_ESCAPE:
    case SDLK_SPACE:
    {
        states::pop();
    }
    break;

    case 'f':
    case SDLK_RETURN:
    {
        if (pos_ != map::player->pos)
        {
            msg_log::clear();

            Actor* const actor = map::actor_at_pos(pos_);

            if (actor && map::player->can_see_actor(*actor))
            {
                map::player->tgt_ = actor;
            }

            const P     pos = pos_;
            Wpn* const  wpn = &wpn_;

            states::pop();

            // NOTE: This object is now destroyed!

            attack::ranged(map::player,
                           map::player->pos,
                           pos,
                           *wpn);
        }
    }
    break;

    default:
        break;
    }
}

int Aiming::effective_range() const
{
    return wpn_.data().ranged.effective_range;
}

// -----------------------------------------------------------------------------
// Throw attack marker state
// -----------------------------------------------------------------------------
void Throwing::on_moved()
{
    look::print_location_info_msgs(pos_);

    auto* const actor = map::actor_at_pos(pos_);

    if (actor               &&
        !actor->is_player() &&
        map::player->can_see_actor(*actor))
    {
        const bool gets_undead_bane_bon =
            player_bon::gets_undead_bane_bon(actor->data());

        if (!actor->has_prop(PropId::ethereal) ||
            gets_undead_bane_bon)
        {
            ThrowAttData data(map::player,
                              actor->pos,       // Aim position
                              actor->pos,       // Current position
                              *item_to_throw_);

            msg_log::add(to_str(data.hit_chance_tot) + "% hit chance.");
        }
    }

    msg_log::add("[t] to throw" + cancel_info_str);
}

void Throwing::handle_input(const InputData& input)
{
    switch (input.key)
    {
    case SDLK_ESCAPE:
    case SDLK_SPACE:
    {
        delete item_to_throw_;

        states::pop();
    }
    break;

    case 't':
    case SDLK_RETURN:
    {
        if (pos_ != map::player->pos)
        {
            msg_log::clear();

            Actor* const actor = map::actor_at_pos(pos_);

            if (actor && map::player->can_see_actor(*actor))
            {
                map::player->tgt_ = actor;
            }

            const P     pos             = pos_;
            Item* const item_to_throw   = item_to_throw_;

            states::pop();

            // NOTE: This object is now destroyed!

            throwing::throw_item(*map::player,
                                 pos,
                                 *item_to_throw);

            map::player->inv().decr_item_in_slot(SlotId::thrown);
        }
    }
    break;

    default:
        break;
    }
}

int Throwing::effective_range() const
{
    return item_to_throw_->data().ranged.effective_range;
}

// -----------------------------------------------------------------------------
// Throw explosive marker state
// -----------------------------------------------------------------------------
void ThrowingExplosive::on_draw()
{
    const ItemId id = explosive_.id();

    if (id == ItemId::dynamite ||
        id == ItemId::molotov  ||
        id == ItemId::smoke_grenade)
    {
        const int radi_d =
            player_bon::traits[(size_t)Trait::dem_expert] ?
            1 : 0;

        const int radi = expl_std_radi + radi_d;

        const R expl_area = explosion::explosion_area(pos_, radi);

        Clr clr_bg = clr_red;

        div_clr(clr_bg, 2.0);

        // Draw explosion radius area overlay
        for (int x = expl_area.p0.x;
             x <= expl_area.p1.x;
             ++x)
        {
            for (int y = expl_area.p0.y;
                 y <= expl_area.p1.y;
                 ++y)
            {
                const auto& render_d        = game::render_array[x][y];
                const auto& marker_render_d = marker_render_data_[x][y];

                if (render_d.glyph != 0 || marker_render_d.glyph != 0)
                {
                    const bool has_marker = marker_render_d.glyph != 0;

                    const auto& d =
                        has_marker ?
                        marker_render_d :
                        render_d;

                    const P p(x, y);

                    if (config::is_tiles_mode())
                    {
                        io::draw_tile(d.tile,
                                      Panel::map,
                                      p,
                                      d.clr,
                                      clr_bg);
                    }
                    else // Text mode
                    {
                        io::draw_glyph(d.glyph,
                                       Panel::map,
                                       p,
                                       d.clr,
                                       true,
                                       clr_bg);
                    }
                }
            }
        }
    }
}

void ThrowingExplosive::on_moved()
{
    look::print_location_info_msgs(pos_);

    msg_log::add("[t] to throw" + cancel_info_str);
}

void ThrowingExplosive::handle_input(const InputData& input)
{
    switch (input.key)
    {
    case SDLK_ESCAPE:
    case SDLK_SPACE:
    {
        states::pop();
    }
    break;

    case 't':
    case SDLK_RETURN:
    {
        msg_log::clear();

        const P pos = pos_;

        states::pop();

        // NOTE: This object is now destroyed!

        throwing::player_throw_lit_explosive(pos);
    }
    break;

    default:
        break;
    }
}
