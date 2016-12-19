#include "character_descr.hpp"

#include <algorithm>

#include "player_bon.hpp"
#include "actor_player.hpp"
#include "io.hpp"
#include "text_format.hpp"
#include "item_potion.hpp"
#include "item_scroll.hpp"
#include "item_factory.hpp"
#include "item.hpp"
#include "map.hpp"
#include "game.hpp"

namespace
{

const int max_nr_lines_on_scr = screen_h - 2;

} // namespace

void CharacterDescr::on_start()
{
    lines_.clear();

    const std::string offset = "   ";

    const int max_w_descr = (map_w * 3) / 4;

    const Clr& clr_heading = clr_white_high;

    const Clr& clr_text = clr_white;

    const Clr& clr_text_dark = clr_gray;

    lines_.push_back(
        StrAndClr("Current properties", clr_heading));

    const auto prop_list = map::player->prop_handler().props_list();

    if (prop_list.empty())
    {
        lines_.push_back(
            StrAndClr(offset + "None",
                      clr_text));

        lines_.push_back(StrAndClr("", clr_text));
    }
    else // Has properties
    {
        for (const auto& e : prop_list)
        {
            const auto& title = e.first;

            lines_.push_back(
                StrAndClr(offset + title.str,
                          title.clr));

            std::vector<std::string> descr_formatted;

            text_format::split(e.second,
                               max_w_descr,
                               descr_formatted);

            for (const auto& descr_line : descr_formatted)
            {
                lines_.push_back(
                    StrAndClr(offset + descr_line,
                              clr_text_dark));
            }

            lines_.push_back(StrAndClr("", clr_text));
        }
    }

    lines_.push_back(
        StrAndClr("Mental disorders",
                  clr_heading));

    const std::vector<const InsSympt*> sympts = insanity::active_sympts();

    if (sympts.empty())
    {
        lines_.push_back(
            StrAndClr(offset + "None",
                      clr_text));
    }
    else // Has insanity symptoms
    {
        for (const InsSympt* const sympt : sympts)
        {
            const std::string sympt_descr = sympt->char_descr_msg();

            if (!sympt_descr.empty())
            {
                lines_.push_back(
                    StrAndClr(offset + sympt_descr, clr_text));
            }
        }
    }

    lines_.push_back(StrAndClr("", clr_text));

    lines_.push_back(
        StrAndClr("Potion knowledge",
                  clr_heading));

    std::vector<StrAndClr> potion_list;
    std::vector<StrAndClr> manuscript_list;

    for (int i = 0; i < (int)ItemId::END; ++i)
    {
        const ItemDataT& d = item_data::data[i];

        if (d.is_tried || d.is_identified)
        {
            if (d.type == ItemType::potion)
            {
                Item* item = item_factory::mk(d.id);

                const std::string name = item->name(ItemRefType::plain);

                potion_list.push_back(
                    StrAndClr(offset + name,
                              d.clr));

                delete item;
            }
            else if (d.type == ItemType::scroll)
            {
                Item* item = item_factory::mk(d.id);

                const std::string name = item->name(ItemRefType::plain);

                manuscript_list.push_back(
                    StrAndClr(offset + name, item->interface_clr()));

                delete item;
            }
        }
    }

    auto str_and_clr_sort = [](const StrAndClr & e1, const StrAndClr & e2)
    {
        return e1.str < e2.str;
    };

    if (potion_list.empty())
    {
        lines_.push_back(
            StrAndClr(offset + "No known potions",
                      clr_text));
    }
    else
    {
        sort(potion_list.begin(),
             potion_list.end(),
             str_and_clr_sort);

        for (StrAndClr& e : potion_list)
        {
            lines_.push_back(e);
        }
    }

    lines_.push_back(StrAndClr("", clr_text));


    lines_.push_back(
        StrAndClr("Manuscript knowledge",
                  clr_heading));

    if (manuscript_list.empty())
    {
        lines_.push_back(
            StrAndClr(offset + "No known manuscripts",
                      clr_text));
    }
    else
    {
        sort(manuscript_list.begin(),
             manuscript_list.end(),
             str_and_clr_sort);

        for (StrAndClr& e : manuscript_list)
        {
            lines_.push_back(e);
        }
    }

    lines_.push_back(StrAndClr("", clr_text));

    lines_.push_back(
        StrAndClr("History of " + map::player->name_the(),
                  clr_heading));

    const std::vector<HistoryEvent>& events = game::history();

    int longest_turn_w = 0;

    for (const auto& event : events)
    {
        const int turn_w = std::to_string(event.turn).size();

        longest_turn_w = std::max(turn_w, longest_turn_w);
    }

    for (const auto& event : events)
    {
        std::string ev_str = std::to_string(event.turn);

        const int turn_w = ev_str.size();

        ev_str.append(longest_turn_w - turn_w, ' ');

        ev_str += " : " + event.msg;

        lines_.push_back(
            StrAndClr(offset + ev_str,
                      clr_text));
    }

    lines_.push_back(StrAndClr("", clr_text));

    lines_.push_back(
        StrAndClr("Traits gained",
                  clr_heading));

    for (size_t i = 0; i < (size_t)Trait::END; ++i)
    {
        if (player_bon::traits[i])
        {
            const Trait trait = Trait(i);

            const std::string title = player_bon::trait_title(trait);
            const std::string descr = player_bon::trait_descr(trait);

            lines_.push_back(
                StrAndClr(offset + title,
                          clr_text));

            std::vector<std::string> descr_lines;

            text_format::split(descr, max_w_descr, descr_lines);

            for (std::string& descr_line : descr_lines)
            {
                lines_.push_back(
                    StrAndClr(offset + descr_line,
                              clr_text_dark));
            }

            lines_.push_back(StrAndClr("", clr_text));
        }
    }
}

void CharacterDescr::draw()
{
    io::draw_info_scr_interface("Character description",
                                InfScreenType::scrolling);

    int y_pos = 1;

    const int nr_lines_tot = lines_.size();

    int btm_nr = std::min(top_idx_ + max_nr_lines_on_scr - 1,
                          nr_lines_tot - 1);

    for (int i = top_idx_; i <= btm_nr; ++i)
    {
        const StrAndClr& line = lines_[i];

        io::draw_text(line.str,
                      Panel::screen,
                      P(0, y_pos),
                      line.clr);

        ++y_pos;
    }
}

void CharacterDescr::update()
{
    const int line_jump = 3;
    const int nr_lines_tot = lines_.size();

    const auto input = io::get(false);

    switch (input.key)
    {
    case '2':
    case SDLK_DOWN:
    case 'j':
        top_idx_ += line_jump;

        if (nr_lines_tot <= max_nr_lines_on_scr)
        {
            top_idx_ = 0;
        }
        else
        {
            top_idx_ = std::min(nr_lines_tot - max_nr_lines_on_scr,
                                top_idx_);
        }
        break;

    case '8':
    case SDLK_UP:
    case 'k':
        top_idx_ = std::max(0, top_idx_ - line_jump);
        break;

    case SDLK_SPACE:
    case SDLK_ESCAPE:
        //
        // Exit screen
        //
        states::pop();
        break;

    default:
        break;
    }
}
