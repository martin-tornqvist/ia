#include "create_character.hpp"

#include "io.hpp"
#include "actor_player.hpp"
#include "browser.hpp"
#include "text_format.hpp"
#include "map.hpp"
#include "game.hpp"
#include "msg_log.hpp"
#include "init.hpp"
#include "popup.hpp"
#include "map_travel.hpp"
#include "mapgen.hpp"

namespace
{

const int top_more_y_   = 1;
const int btm_more_y_   = screen_h - 1;
const int opt_x0_       = 0;
const int opt_y0_       = top_more_y_ + 1;
const int opt_x1_       = 22;
const int opt_y1_       = btm_more_y_ - 1;
const int opt_h_        = opt_y1_ - opt_y0_ + 1;

const int descr_x0_     = opt_x1_ + 2;
const int descr_y0_     = opt_y0_;
const int descr_x1_     = screen_w - 1;
const int descr_y1_     = opt_y1_;

const int descr_w_      = descr_x1_ - descr_y1_ + 1;

} // namespace

// -----------------------------------------------------------------------------
// New game state
// -----------------------------------------------------------------------------
void NewGameState::on_pushed()
{
    std::unique_ptr<State> name_state(  new EnterNameState);
    std::unique_ptr<State> trait_state( new PickTraitState);
    std::unique_ptr<State> bg_state(    new PickBgState);

    states::push(std::move(name_state));
    states::push(std::move(trait_state));
    states::push(std::move(bg_state));
}

void NewGameState::on_resume()
{
    // Some backgrounds and traits may have affected maximum hp and spi (either
    // positively or negatively), so here we need to set the current hp and SPI
    // equal to the maximum values.
    map::player->set_hp_and_spi_to_max();

    states::pop();

    std::unique_ptr<State> game_state(new GameState);

    states::push(std::move(game_state));

    if (config::is_intro_lvl_skipped())
    {
        // Build first dungeon level
        map_travel::go_to_nxt();
    }
    else // Using intro level
    {
        // Build forest.
        mapgen::mk_intro_lvl();
    }
}

void NewGameState::on_popped()
{
    game::add_history_event("Started journey");

    io::clear_screen();

    const std::string msg =
        "I stand on a cobbled forest path, ahead lies a "
        "shunned and decrepit old church. I know of the things "
        "that dwell below, and of the Cult of Starry Wisdom "
        "and the monstrous sacrifices dedicated to their "
        "rulers. But now they are weak - only deranged "
        "fanatics grasping at false promises. I will enter "
        "these sprawling catacombs and rob them of treasures "
        "and knowledge! At the depths of the abyss lies my "
        "true destiny, an artifact of non-human origin called "
        "\"The shining Trapezohedron\" - a window to all the "
        "secrets of the universe.";

    popup::show_msg(msg,
                    "The story so far...",
                    SfxId::END,
                    1);
}

// -----------------------------------------------------------------------------
// Gain level state
// -----------------------------------------------------------------------------
void GainLvlState::on_start()
{
    game::incr_clvl();

    const int clvl = game::clvl();

    msg_log::add("Welcome to level " + to_str(clvl) + "!",
                 clr_green,
                 false,
                 MorePromptOnMsg::yes);

    // Let the player pick a trait
    std::unique_ptr<State> trait_state(new PickTraitState);

    states::push(std::move(trait_state));
}

void GainLvlState::update()
{
    // (Player has picked a trait, and the trait state has been popped)

    // Increase hit points and spirit
    map::player->change_max_hp(hp_per_lvl);

    map::player->change_max_spi(spi_per_lvl);

    // OK, we're done gaining a level, back to the game!
    states::pop();
}

// -----------------------------------------------------------------------------
// Pick background state
// -----------------------------------------------------------------------------
PickBgState::PickBgState() :
    State       (),
    browser_    ()
{

}

void PickBgState::on_start()
{
    player_bon::pickable_bgs(bgs_);

    browser_.reset(bgs_.size(), opt_h_);

    // Let the browser start at Rogue, to recommend it as the default choice
    browser_.set_y((int)Bg::rogue);
}

void PickBgState::update()
{
    if (config::is_bot_playing())
    {
        player_bon::pick_bg(rnd::element(bgs_));

        states::pop();

        return;
    }

    const auto input = io::get();

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    case MenuAction::selected_shift:
    {
        const Bg bg = bgs_[browser_.y()];

        player_bon::pick_bg(bg);

        states::pop();
    }
    break;

    default:
        break;
    }
}

void PickBgState::draw()
{
    io::draw_text_center("What is your background?",
                         Panel::screen,
                         P(map_w_half, 0),
                         clr_title,
                         clr_black,
                         true);

    int y = opt_y0_;

    const Bg bg_marked = bgs_[browser_.y()];

    //
    // Backgrounds
    //
    std::string key_str = "a) ";

    for (const Bg bg : bgs_)
    {
        const std::string   bg_name     = player_bon::bg_title(bg);
        const bool          is_marked   = bg == bg_marked;

        const Clr& drw_clr =
            is_marked ?
            clr_menu_highlight : clr_menu_drk;

        io::draw_text(key_str + bg_name,
                      Panel::screen,
                      P(opt_x0_, y),
                      drw_clr);

        ++y;
        ++key_str[0];
    }

    //
    // Description
    //
    y = descr_y0_;

    const std::vector<std::string> descr =
        player_bon::bg_descr(bg_marked);

    ASSERT(!descr.empty());

    for (const std::string& descr_entry : descr)
    {
        if (descr_entry.empty())
        {
            ++y;
            continue;
        }

        std::vector<std::string> formatted_lines;

        text_format::split(descr_entry,
                           descr_w_,
                           formatted_lines);

        for (const std::string& line : formatted_lines)
        {
            io::draw_text(line,
                          Panel::screen,
                          P(descr_x0_, y),
                          clr_text);
            ++y;
        }
    }
}

// -----------------------------------------------------------------------------
// Pick trait state
// -----------------------------------------------------------------------------
PickTraitState::PickTraitState() :
    State       (),
    browser_    ()
{

}

void PickTraitState::on_start()
{
    const Bg player_bg = player_bon::bg();

    player_bon::trait_list_for_bg(player_bg, traits_);

    browser_.reset(traits_.size(), opt_h_);
}

void PickTraitState::update()
{
    if (config::is_bot_playing())
    {
        states::pop();

        return;
    }

    const auto input = io::get();

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    case MenuAction::selected_shift:
    {
        const Trait     trait       = traits_[browser_.y()];
        const bool      is_picked   = player_bon::traits[(size_t)trait];

        if (!is_picked)
        {
            const bool is_prereqs_ok = player_bon::is_prereqs_ok(trait);

            if (is_prereqs_ok)
            {
                player_bon::pick_trait(trait);

                states::pop();
            }
        }
    }
    break;

    default:
        break;
    }
}

void PickTraitState::draw()
{
    const int clvl = game::clvl();

    std::string title =
        (clvl == 1) ?
        "Which additional trait do you start with?" :
        "You have reached a new level! Which trait do you gain?";

    io::draw_text_center(title,
                         Panel::screen,
                         P(map_w_half, 0),
                         clr_title,
                         clr_black,
                         true);

    const int   browser_y       = browser_.y();
    const Trait trait_marked    = traits_[browser_y];
    const Bg    player_bg       = player_bon::bg();

    //
    // Traits
    //
    int y = opt_y0_;

    const Range idx_range_shown = browser_.range_shown();

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const Trait trait           = traits_[i];
        std::string trait_name      = player_bon::trait_title(trait);
        const bool  is_idx_marked   = browser_y == i;
        const bool  is_picked       = player_bon::traits[(size_t)trait];
        const bool  is_prereqs_ok   = player_bon::is_prereqs_ok(trait);

        Clr clr = clr_magenta_lgt;

        if (is_prereqs_ok)
        {
            if (is_picked)
            {
                if (is_idx_marked)
                {
                    clr = clr_green_lgt;
                }
                else //Not marked
                {
                    clr = clr_green;
                }
            }
            else //Not picked
            {
                if (is_idx_marked)
                {
                    clr = clr_menu_highlight;
                }
                else //Not marked
                {
                    clr = clr_menu_drk;
                }
            }
        }
        else // Prerequisites not met
        {
            if (is_idx_marked)
            {
                clr = clr_red_lgt;
            }
            else //Not marked
            {
                clr = clr_red;
            }
        }

        io::draw_text(key_str + trait_name,
                          Panel::screen,
                          P(opt_x0_, y),
                          clr);

        ++y;
        ++key_str[0];
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                          Panel::screen,
                          P(opt_x0_, top_more_y_),
                          clr_white_high);
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                          Panel::screen,
                          P(opt_x0_, btm_more_y_),
                          clr_white_high);
    }

    //
    // Description
    //
    y = descr_y0_;

    std::string descr = player_bon::trait_descr(trait_marked);

    std::vector<std::string> formatted_descr;

    text_format::split(descr,
                       descr_w_,
                       formatted_descr);

    for (const std::string& str : formatted_descr)
    {
        io::draw_text(str,
                      Panel::screen,
                      P(descr_x0_, y),
                      clr_white);
        ++y;
    }

    //
    // Prerequisites
    //
    std::vector<Trait> trait_marked_prereqs;

    Bg trait_marked_bg_prereq = Bg::END;

    player_bon::trait_prereqs(trait_marked,
                              player_bg,
                              trait_marked_prereqs,
                              trait_marked_bg_prereq);

    const int Y0_PREREQS = 10;

    y = Y0_PREREQS;

    if (!trait_marked_prereqs.empty() ||
        trait_marked_bg_prereq != Bg::END)
    {
        const std::string label = "Prerequisite(s):";

        io::draw_text(label,
                          Panel::screen,
                          P(descr_x0_, y),
                          clr_white);

        std::vector<StrAndClr> prereq_titles;

        std::string prereq_str = "";

        const Clr& clr_prereq_ok        = clr_green;
        const Clr& clr_prereq_not_ok    = clr_red;

        if (trait_marked_bg_prereq != Bg::END)
        {
            const Clr& clr =
                (player_bon::bg() == trait_marked_bg_prereq) ?
                clr_prereq_ok :
                clr_prereq_not_ok;

            const std::string bg_title =
                player_bon::bg_title(trait_marked_bg_prereq);

            prereq_titles.push_back(StrAndClr(bg_title, clr));
        }

        for (Trait prereq_trait : trait_marked_prereqs)
        {
            const bool is_picked =
                player_bon::traits[(size_t)prereq_trait];

            const Clr& clr =
                is_picked ?
                clr_prereq_ok :
                clr_prereq_not_ok;

            const std::string trait_title =
                player_bon::trait_title(prereq_trait);

            prereq_titles.push_back(StrAndClr(trait_title, clr));
        }

        const int prereq_list_x = descr_x0_ + label.size() + 1;

        for (const StrAndClr& title : prereq_titles)
        {
            io::draw_text(title.str,
                          Panel::screen,
                          P(prereq_list_x, y),
                          title.clr);

            ++y;
        }
    }
}

// -----------------------------------------------------------------------------
// Enter name state
// -----------------------------------------------------------------------------
EnterNameState::EnterNameState() :
    State           (),
    current_str_    ()
{

}

void EnterNameState::update()
{
    if (config::is_bot_playing())
    {
        ActorDataT& d = map::player->data();

        d.name_a = d.name_the = "AZATHOTH";

        states::pop();

        return;
    }

    const auto input = io::get();

    if (input.key == SDLK_RETURN)
    {
        if (current_str_.empty())
        {
            current_str_ = "Player";
        }

        ActorDataT& d = map::player->data();

        d.name_a = d.name_the = current_str_;

        states::pop();

        return;
    }

    if (current_str_.size() < player_name_max_len)
    {
        if (input.key == SDLK_SPACE)
        {
            current_str_.push_back('_');

            return;
        }
        else if ((input.key >= 'a' && input.key <= 'z') ||
                 (input.key >= 'A' && input.key <= 'Z') ||
                 (input.key >= '0' && input.key <= '9'))
        {
            current_str_.push_back(input.key);

            return;
        }
    }

    if (!current_str_.empty())
    {
        if (input.key == SDLK_BACKSPACE)
        {
            current_str_.erase(current_str_.end() - 1);
        }
    }
}

void EnterNameState::draw()
{
    io::draw_text_center("What is your name?",
                         Panel::screen,
                         P(map_w_half, 0),
                         clr_title);

    const int y_name = 3;

    const std::string name_str =
        (current_str_.size() < player_name_max_len) ?
        current_str_ + "_" :
        current_str_;

    const size_t name_x0 = map_w_half - (player_name_max_len / 2);
    const size_t name_x1 = name_x0 + player_name_max_len - 1;

    io::draw_text(name_str,
                  Panel::screen,
                  P(name_x0, y_name),
                  clr_brown_gray);

    R box_rect(P(name_x0 - 1, y_name - 1),
               P(name_x1 + 1, y_name + 1));

    io::draw_box(box_rect);
}
