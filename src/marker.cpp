#include "marker.hpp"

#include <vector>
#include <cstring>

#include "io.hpp"
#include "inventory_handling.hpp"
#include "actor_player.hpp"
#include "attack.hpp"
#include "attack_data.hpp"
#include "msg_log.hpp"
#include "look.hpp"
#include "throwing.hpp"
#include "map.hpp"
#include "item_factory.hpp"
#include "line_calc.hpp"
#include "config.hpp"
#include "feature_rigid.hpp"
#include "explosion.hpp"
#include "map_parsing.hpp"
#include "draw_map.hpp"

// -----------------------------------------------------------------------------
// Marker state
// -----------------------------------------------------------------------------
StateId MarkerState::id()
{
        return StateId::marker;
}

void MarkerState::on_start()
{
        pos_ = map::player->pos;

        if (use_player_tgt())
        {
                // First, attempt to place marker at player target.
                const bool did_go_to_tgt = try_go_to_tgt();

                if (!did_go_to_tgt)
                {
                        // If no target available, attempt to place marker at
                        // closest visible monster. This sets a new player
                        // target if successful.
                        map::player->tgt_ = nullptr;

                        try_go_to_closest_enemy();
                }
        }

        on_start_hook();

        on_moved();
}

void MarkerState::draw()
{
        auto line =
                line_calc::calc_new_line(
                        origin_,
                        pos_,
                        true, // Stop at target
                        INT_MAX, // Travel limit
                        false); // Allow outside map

        // Remove origin position
        if (!line.empty())
        {
                line.erase(line.begin());
        }

        const int orange_from_dist = orange_from_king_dist();

        const int red_from_dist = red_from_king_dist();

        int red_from_idx = -1;

        if (show_blocked())
        {
                for (size_t i = 0; i < line.size(); ++i)
                {
                        const P& p(line[i]);

                        const Cell& c = map::cells[p.x][p.y];

                        if (c.is_seen_by_player &&
                            !c.rigid->is_projectile_passable())
                        {
                                red_from_idx = i;
                                break;
                        }
                }
        }

        draw_marker(line,
                    orange_from_dist,
                    red_from_dist,
                    red_from_idx);

        on_draw();
}

void MarkerState::update()
{
        InputData input;

        if (!config::is_bot_playing())
        {
                input = io::get(true);
        }

        msg_log::clear();

        switch (input.key)
        {

                // Direction input is handle by the base class
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

        // Input other than direction keys is delegated
        default:
        {
                handle_input(input);
        }
        break;
        }
}

void MarkerState::draw_marker(
        const std::vector<P>& line,
        const int orange_from_king_dist,
        const int red_from_king_dist,
        const int red_from_idx)
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        auto& d = marker_render_data_[x][y];

                        d.tile  = TileId::empty;
                        d.character = 0;
                }
        }

        Color color = colors::light_green();

        // Draw the line

        // NOTE: We include the head index in this loop, so that we can set up
        // which color it should be drawn with, but we do the actual drawing of
        // the head after the loop
        for (size_t line_idx = 0; line_idx < line.size(); ++line_idx)
        {
                const P& line_pos = line[line_idx];

                const int dist = king_dist(origin_, line_pos);

                // Draw red due to index, or due to distance?
                const bool red_by_idx =
                        (red_from_idx != -1) &&
                        ((int)line_idx >= red_from_idx);

                const bool red_by_dist =
                        (red_from_king_dist != -1) &&
                        (dist >= red_from_king_dist);

                const bool is_red = red_by_idx || red_by_dist;

                // NOTE: Final color is stored for drawing the head
                if (is_red)
                {
                        color = colors::light_red();
                }
                // Not red - orange by distance?
                else if ((orange_from_king_dist != -1) &&
                         (dist >= orange_from_king_dist))
                {
                        color = colors::orange();
                }

                // Do not draw the head yet
                const int tail_size_int = (int)line.size() - 1;

                if ((int)line_idx < tail_size_int)
                {
                        auto& d = marker_render_data_[line_pos.x][line_pos.y];

                        d.tile = TileId::aim_marker_line;

                        d.character = '*';

                        d.color = color;

                        d.color_bg = colors::black();

                        // If red, always draw a character (more distinct)
                        if (config::is_tiles_mode() && !is_red)
                        {
                                io::draw_tile(
                                        d.tile,
                                        Panel::map,
                                        line_pos,
                                        d.color,
                                        d.color_bg);
                        }
                        else // Text mode, or blocked
                        {
                                io::draw_character(
                                        d.character,
                                        Panel::map,
                                        line_pos,
                                        d.color,
                                        d.color_bg);
                        }
                }
        } // line loop

        // Draw the head
        const P& head_pos =
                line.empty() ?
                origin_ :
                line.back();

        auto& d = marker_render_data_[head_pos.x][head_pos.y];

        d.tile = TileId::aim_marker_head;

        d.character = 'X';

        d.color = color;

        d.color_bg = colors::black();

        io::draw_symbol(
                d.tile,
                d.character,
                Panel::map,
                head_pos,
                d.color,
                d.color_bg);
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

        if (!tgt)
        {
                return false;
        }

        const auto seen_foes = map::player->seen_foes();

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

        return false;
}

void MarkerState::try_go_to_closest_enemy()
{
        const auto seen_foes = map::player->seen_foes();

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

        if (actor &&
            actor != map::player &&
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
                auto* const actor = map::actor_at_pos(pos_);

                if (actor &&
                    actor != map::player &&
                    map::player->can_see_actor(*actor))
                {
                        msg_log::clear();

                        std::unique_ptr<ViewActorDescr>
                                view_actor_descr(new ViewActorDescr(*actor));

                        states::push(std::move(view_actor_descr));
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

        const bool is_in_range =
                king_dist(origin_, pos_) < red_from_king_dist();

        if (is_in_range)
        {
                auto* const actor = map::actor_at_pos(pos_);

                if (actor &&
                    !actor->is_player() &&
                    map::player->can_see_actor(*actor))
                {
                        RangedAttData data(
                                map::player,
                                origin_,
                                actor->pos,  // Aim position
                                actor->pos,  // Current position
                                wpn_);

                        msg_log::add(
                                std::to_string(data.hit_chance_tot) +
                                "% hit chance.");
                }
        }

        msg_log::add("[f] to fire" + cancel_info_str);
}

void Aiming::handle_input(const InputData& input)
{
        int key = input.key;

        // If the bot is playing, fire at a random position
        if (config::is_bot_playing())
        {
                key = 'f';

                pos_.set(
                        rnd::range(0, map_w - 1),
                        rnd::range(0, map_h - 1));
        }

        switch (key)
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

                        const P pos = pos_;

                        Wpn* const wpn = &wpn_;

                        states::pop();

                        // NOTE: This object is now destroyed!

                        attack::ranged(
                                map::player,
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

int Aiming::orange_from_king_dist() const
{
        const int effective_range = wpn_.data().ranged.effective_range;

        return (effective_range < 0) ? -1 : (effective_range + 1);
}

int Aiming::red_from_king_dist() const
{
        const int max_range = wpn_.data().ranged.max_range;

        return (max_range < 0) ? -1 : (max_range + 1);
}

// -----------------------------------------------------------------------------
// Throw attack marker state
// -----------------------------------------------------------------------------
void Throwing::on_moved()
{
        look::print_location_info_msgs(pos_);

        const bool is_in_range =
                king_dist(origin_, pos_) < red_from_king_dist();

        if (is_in_range)
        {
                auto* const actor = map::actor_at_pos(pos_);

                if (actor &&
                    !actor->is_player() &&
                    map::player->can_see_actor(*actor))
                {
                        ThrowAttData data(
                                map::player,
                                actor->pos,       // Aim position
                                actor->pos,       // Current position
                                *inv_item_);

                        msg_log::add(
                                std::to_string(data.hit_chance_tot) +
                                "% hit chance.");
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

                        const P pos = pos_;

                        Item* item_to_throw =
                                item_factory::copy_item(*inv_item_);

                        item_to_throw->nr_items_ = 1;

                        item_to_throw->clear_actor_carrying();

                        map::player->inv().decr_item(inv_item_);

                        states::pop();

                        // NOTE: This object is now destroyed

                        // Perform the actual throwing
                        throwing::throw_item(
                                *map::player,
                                pos,
                                *item_to_throw);
                }
        }
        break;

        default:
                break;
        }
}

int Throwing::orange_from_king_dist() const
{
        const int effective_range = inv_item_->data().ranged.effective_range;

        return
                (effective_range < 0) ?
                -1 :
                (effective_range + 1);
}

int Throwing::red_from_king_dist() const
{
        const int max_range = inv_item_->data().ranged.max_range;

        return
                (max_range < 0) ?
                -1 :
                (max_range + 1);
}

// -----------------------------------------------------------------------------
// Throw explosive marker state
// -----------------------------------------------------------------------------
void ThrowingExplosive::on_draw()
{
        const ItemId id = explosive_.id();

        if (id != ItemId::dynamite &&
            id != ItemId::molotov &&
            id != ItemId::smoke_grenade)
        {
                return;
        }

        const R expl_area =
                explosion::explosion_area(pos_, expl_std_radi);

        const Color color_bg = colors::red().fraction(2.0);

        // Draw explosion radius area overlay
        for (int y = expl_area.p0.y; y <= expl_area.p1.y; ++y)
        {
                for (int x = expl_area.p0.x; x <= expl_area.p1.x; ++x)
                {
                        const auto& render_d = draw_map::get_drawn_cell(x, y);

                        const auto& marker_render_d = marker_render_data_[x][y];

                        // Draw overlay if the cell contains either a map
                        // symbol, or a marker symbol
                        if (render_d.character != 0 ||
                            marker_render_d.character != 0)
                        {
                                const bool has_marker =
                                        marker_render_d.character != 0;

                                const auto& d =
                                        has_marker ?
                                        marker_render_d :
                                        render_d;

                                const P p(x, y);

                                io::draw_symbol(
                                        d.tile,
                                        d.character,
                                        Panel::map,
                                        p,
                                        d.color,
                                        color_bg);
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

int ThrowingExplosive::red_from_king_dist() const
{
        const int max_range = explosive_.data().ranged.max_range;

        return (max_range < 0) ? -1 : (max_range + 1);
}

// -----------------------------------------------------------------------------
// Teleport control marker state
// -----------------------------------------------------------------------------
CtrlTele::CtrlTele(const P& origin, const bool blocked[map_w][map_h]) :
        MarkerState(origin)
{
        memcpy(blocked_, blocked, nr_map_cells);
}

int CtrlTele::chance_of_success_pct(const P& tgt) const
{
        const int dist = king_dist(map::player->pos, tgt);

        const int chance = constr_in_range(25, 100 - dist, 95);

        return chance;
}

void CtrlTele::on_start_hook()
{
        msg_log::add("I have the power to control teleportation.",
                     colors::white(),
                     false,
                     MorePromptOnMsg::yes);
}

void CtrlTele::on_moved()
{
        look::print_location_info_msgs(pos_);

        if (pos_ != map::player->pos)
        {
                const int chance_pct = chance_of_success_pct(pos_);

                msg_log::add(
                        std::to_string(chance_pct) +
                        "% chance of success.");

                msg_log::add("[enter] to try teleporting here");
        }
}

void CtrlTele::handle_input(const InputData& input)
{
        switch (input.key)
        {
        case SDLK_RETURN:
        {
                if (pos_ != map::player->pos)
                {
                        const int chance = chance_of_success_pct(pos_);

                        const bool roll_ok = rnd::percent(chance);

                        const bool is_tele_success =
                                roll_ok &&
                                !blocked_[pos_.x][pos_.y];

                        const P tgt_p(pos_);

                        states::pop();

                        // NOTE: This object is now destroyed!

                        if (is_tele_success)
                        {
                                // Teleport to this exact destination
                                map::player->teleport(tgt_p, blocked_);
                        }
                        else // Failed to teleport (blocked or roll failed)
                        {
                                msg_log::add("I failed to go there...",
                                             colors::white(),
                                             false,
                                             MorePromptOnMsg::yes);

                                // Run a randomized teleport with teleport
                                // control disabled
                                map::player->teleport(ShouldCtrlTele::never);
                        }
                }
        }
        break;

        default:
                break;
        }
}
