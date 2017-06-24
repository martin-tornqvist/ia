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

namespace
{

const int top_more_y_ = 1;
const int btm_more_y_ = screen_h - 1;
const int opt_x0_ = 0;
const int opt_y0_ = top_more_y_ + 1;
const int opt_x1_ = 23;
const int opt_y1_ = btm_more_y_ - 1;
const int opt_h_ = opt_y1_ - opt_y0_ + 1;

const int descr_x0_ = opt_x1_ + 2;
const int descr_y0_ = opt_y0_;
const int descr_x1_ = screen_w - 1;
const int descr_y1_ = opt_y1_;

const int descr_w_ = descr_x1_ - descr_y1_ + 1;

} // namespace

// -----------------------------------------------------------------------------
// New game state
// -----------------------------------------------------------------------------
NewGameState::NewGameState()
{

}

NewGameState::~NewGameState()
{

}

StateId NewGameState::id()
{
    return StateId::new_game;
}

void NewGameState::on_pushed()
{
    std::unique_ptr<State> game_state(
        new GameState(GameEntryMode::new_game));

    std::unique_ptr<State> name_state(new EnterNameState);

    std::unique_ptr<State> trait_state(new PickTraitState);

    std::unique_ptr<State> bg_state(new PickBgState);

    states::push(std::move(game_state));
    states::push(std::move(name_state));
    states::push(std::move(trait_state));
    states::push(std::move(bg_state));
}

void NewGameState::on_resume()
{
    states::pop();
}

// -----------------------------------------------------------------------------
// Gain level state
// -----------------------------------------------------------------------------
GainLvlState::GainLvlState()
{

}

GainLvlState::~GainLvlState()
{

}

StateId GainLvlState::id()
{
    return StateId::new_level;
}

void GainLvlState::on_start()
{
    game::incr_clvl();

    const int clvl = game::clvl();

    msg_log::add("Welcome to level " + std::to_string(clvl) + "!",
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
    State(),
    browser_()
{

}

PickBgState::~PickBgState()
{

}

StateId PickBgState::id()
{
    return StateId::pick_background;
}

void PickBgState::on_start()
{
    bgs_ = player_bon::pickable_bgs();

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

    const auto input = io::get(false);

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

    case MenuAction::esc:
    {
      states::pop_until(StateId::menu);
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
        const std::string bg_name = player_bon::bg_title(bg);
        const bool is_marked = bg == bg_marked;

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

    const auto descr = player_bon::bg_descr(bg_marked);

    ASSERT(!descr.empty());

    for (const auto& descr_entry : descr)
    {
        if (descr_entry.str.empty())
        {
            ++y;

            continue;
        }

        const auto formatted_lines =
            text_format::split(descr_entry.str, descr_w_);

        for (const std::string& line : formatted_lines)
        {
            io::draw_text(line,
                          Panel::screen,
                          P(descr_x0_, y),
                          descr_entry.clr);
            ++y;
        }
    }
}

// -----------------------------------------------------------------------------
// Pick trait state
// -----------------------------------------------------------------------------
PickTraitState::PickTraitState() :
    State(),
    browser_traits_avail_(),
    browser_traits_unavail_(),
    traits_avail_(),
    traits_unavail_(),
    screen_mode_(TraitScreenMode::pick_new)
{

}

PickTraitState::~PickTraitState()
{

}

StateId PickTraitState::id()
{
    return StateId::pick_trait;
}

void PickTraitState::on_start()
{
    const Bg player_bg = player_bon::bg();

    player_bon::unpicked_traits_for_bg(player_bg,
                                       traits_avail_,
                                       traits_unavail_);

    browser_traits_avail_.reset(traits_avail_.size(), opt_h_);

    browser_traits_unavail_.reset(traits_unavail_.size(), opt_h_);
}

void PickTraitState::update()
{
    if (config::is_bot_playing())
    {
        states::pop();

        return;
    }

    const auto input = io::get(false);

    //
    // Switch trait screen mode?
    //
    if (input.key == SDLK_TAB)
    {
        screen_mode_ =
            screen_mode_ == TraitScreenMode::pick_new ?
            TraitScreenMode::view_unavail :
            TraitScreenMode::pick_new;

        return;
    }

    MenuBrowser& browser =
        screen_mode_ == TraitScreenMode::pick_new ?
        browser_traits_avail_ :
        browser_traits_unavail_;

    const MenuAction action =
        browser.read(input,
                     MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::esc:
    {
      if (states::contains_state(StateId::pick_name))
          states::pop_until(StateId::menu);
    }
    break;

    case MenuAction::selected:
    case MenuAction::selected_shift:
    {
        if (screen_mode_ == TraitScreenMode::pick_new)
        {
            const Trait trait = traits_avail_[browser.y()];

            player_bon::pick_trait(trait);

            states::pop();
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

    std::string title;

    if (screen_mode_ == TraitScreenMode::pick_new)
    {
        title =
            (clvl == 1) ?
            "Which extra trait do you start with?" :
            "Which trait do you gain?";

        title += " [TAB] to view unavailable traits";
    }
    else // Viewing unavailable traits
    {
        title = "Currently unavailable traits";

        title += " [TAB] to view available traits";
    }

    io::draw_text_center(title,
                         Panel::screen,
                         P(map_w_half, 0),
                         clr_title,
                         clr_black,
                         true);

    MenuBrowser* browser;

    std::vector<Trait>* traits;

    if (screen_mode_ == TraitScreenMode::pick_new)
    {
        browser = &browser_traits_avail_;

        traits = &traits_avail_;
    }
    else // Viewing unavailable traits
    {
        browser = &browser_traits_unavail_;

        traits = &traits_unavail_;
    }

    const int browser_y = browser->y();

    const Trait trait_marked = traits->at(browser_y);

    const Bg player_bg = player_bon::bg();

    //
    // Traits
    //
    int y = opt_y0_;

    const Range idx_range_shown = browser->range_shown();

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const Trait trait = traits->at(i);

        std::string trait_name = player_bon::trait_title(trait);

        const bool is_idx_marked = browser_y == i;

        Clr clr = clr_magenta_lgt;

        if (screen_mode_ == TraitScreenMode::pick_new)
        {
            if (is_idx_marked)
            {
                clr = clr_menu_highlight;
            }
            else // Not marked
            {
                clr = clr_menu_drk;
            }
        }
        else // Viewing unavailable traits
        {
            if (is_idx_marked)
            {
                clr = clr_red_lgt;
            }
            else // Not marked
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
    if (!browser->is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      Panel::screen,
                      P(opt_x0_, top_more_y_),
                      clr_white_lgt);
    }

    if (!browser->is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      Panel::screen,
                      P(opt_x0_, btm_more_y_),
                      clr_white_lgt);
    }

    //
    // Description
    //
    y = descr_y0_;

    std::string descr = player_bon::trait_descr(trait_marked);

    const auto formatted_descr = text_format::split(descr, descr_w_);

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

    const int y0_prereqs = 10;

    y = y0_prereqs;

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

        const Clr& clr_prereq_ok = clr_green;

        const Clr& clr_prereq_not_ok = clr_red;

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
    State(),
    current_str_()
{

}

EnterNameState::~EnterNameState()
{

}

StateId EnterNameState::id()
{
    return StateId::pick_name;
}

void EnterNameState::on_start()
{
    const std::string default_name = config::default_player_name();

    current_str_ = default_name;
}

void EnterNameState::update()
{
    if (config::is_bot_playing())
    {
        ActorDataT& d = map::player->data();

        d.name_a = d.name_the = "Bot";

        states::pop();

        return;
    }

    const auto input = io::get(false);

    if (input.key == SDLK_ESCAPE)
    {
      states::pop_until(StateId::menu);
      return;
    }

    if (input.key == SDLK_RETURN)
    {
        if (current_str_.empty())
        {
            current_str_ = "Player";
        }
        else // Player has entered a string
        {
            config::set_default_player_name(current_str_);
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
            current_str_.push_back(' ');

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
