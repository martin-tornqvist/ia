#include "character_descr.hpp"

#include <algorithm>

#include "player_bon.hpp"
#include "actor_player.hpp"
#include "render.hpp"
#include "text_format.hpp"
#include "input.hpp"
#include "item_potion.hpp"
#include "item_scroll.hpp"
#include "item_factory.hpp"
#include "item.hpp"
#include "map.hpp"
#include "dungeon_master.hpp"

namespace character_descr
{

namespace
{

std::vector<Str_and_clr> lines_;

void mk_lines()
{
    lines_.clear();

    const std::string   offset          = "   ";
    const Clr&          clr_heading     = clr_white_high;
    const Clr&          clr_text        = clr_white;
    const Clr&          clr_text_dark   = clr_gray;

    lines_.push_back({"History of " + map::player->name_the(), clr_heading});

    const std::vector<History_event>& events = dungeon_master::history();

    for (const auto& event : events)
    {
        std::string ev_str = to_str(event.TURN);

        const int TURN_STR_MAX_W = 10;

        text_format::pad_before_to(ev_str, TURN_STR_MAX_W);

        ev_str += ": " + event.msg;

        lines_.push_back({offset + ev_str, clr_text});
    }

    lines_.push_back({"", clr_text});

    const Ability_vals& abilities = map::player->data().ability_vals;

    lines_.push_back({"Combat skills", clr_heading});

    const int BASE_MELEE =
        std::min(100, abilities.val(Ability_id::melee, true, *(map::player)));

    const int BASE_RANGED =
        std::min(100, abilities.val(Ability_id::ranged, true, *(map::player)));

    const int BASE_DODGE_ATTACKS =
        std::min(100, abilities.val(Ability_id::dodge_att, true, *(map::player)));

    lines_.push_back({offset + "Melee    " + to_str(BASE_MELEE)         + "%", clr_text});
    lines_.push_back({offset + "Ranged   " + to_str(BASE_RANGED)        + "%", clr_text});
    lines_.push_back({offset + "Dodging  " + to_str(BASE_DODGE_ATTACKS) + "%", clr_text});

    lines_.push_back({"", clr_text});

    lines_.push_back({"Mental disorders", clr_heading});

    const std::vector<const Ins_sympt*> sympts = insanity::active_sympts();

    if (sympts.empty())
    {
        lines_.push_back({offset + "None", clr_text});
    }
    else // Has insanity symptoms
    {
        for (const Ins_sympt* const sympt : sympts)
        {
            const std::string sympt_descr = sympt->char_descr_msg();

            if (!sympt_descr.empty())
            {
                lines_.push_back({offset + sympt_descr, clr_text});
            }
        }
    }

    lines_.push_back({"", clr_text});

    lines_.push_back({"Potion knowledge", clr_heading});
    std::vector<Str_and_clr> potion_list;
    std::vector<Str_and_clr> manuscript_list;

    for (int i = 0; i < int(Item_id::END); ++i)
    {
        const Item_data_t& d = item_data::data[i];

        if (d.is_tried || d.is_identified)
        {
            if (d.type == Item_type::potion)
            {
                Item* item = item_factory::mk(d.id);

                const std::string name = item->name(Item_ref_type::plain);

                potion_list.push_back({offset + name, d.clr});

                delete item;
            }
            else if (d.type == Item_type::scroll)
            {
                Item* item = item_factory::mk(d.id);

                const std::string name = item->name(Item_ref_type::plain);

                manuscript_list.push_back(Str_and_clr(offset + name, item->interface_clr()));

                delete item;
            }
        }
    }

    auto str_and_clr_sort = [](const Str_and_clr & e1, const Str_and_clr & e2)
    {
        return e1.str < e2.str;
    };

    if (potion_list.empty())
    {
        lines_.push_back({offset + "No known potions", clr_text});
    }
    else
    {
        sort(potion_list.begin(), potion_list.end(), str_and_clr_sort);

        for (Str_and_clr& e : potion_list) {lines_.push_back(e);}
    }

    lines_.push_back({"", clr_text});


    lines_.push_back({"Manuscript knowledge", clr_heading});

    if (manuscript_list.size() == 0)
    {
        lines_.push_back({offset + "No known manuscripts", clr_text});
    }
    else
    {
        sort(manuscript_list.begin(), manuscript_list.end(), str_and_clr_sort);

        for (Str_and_clr& e : manuscript_list) {lines_.push_back(e);}
    }

    lines_.push_back({"", clr_text});

    lines_.push_back({"Traits gained", clr_heading});

    const int MAX_W_DESCR = (MAP_W * 2) / 3;

    for (int i = 0; i < int(Trait::END); ++i)
    {
        if (player_bon::traits[i])
        {
            const Trait trait = Trait(i);

            const std::string title = player_bon::trait_title(trait);
            const std::string descr = player_bon::trait_descr(trait);

            lines_.push_back({offset + title, clr_text});

            std::vector<std::string> descr_lines;

            text_format::split(descr, MAX_W_DESCR, descr_lines);

            for (std::string& descr_line : descr_lines)
            {
                lines_.push_back({offset + descr_line, clr_text_dark});
            }

            lines_.push_back({"", clr_text});
        }
    }
}

void shock_res_src_title(const Shock_src shock_src, std::string& str_ref)
{
    str_ref = "";

    switch (shock_src)
    {
    case Shock_src::time:
        str_ref = "Time";
        break;

    case Shock_src::cast_intr_spell:
        str_ref = "Casting learned spells";
        break;

    case Shock_src::see_mon:
        str_ref = "Seeing monsters";
        break;

    case Shock_src::use_strange_item:
        str_ref = "Using strange items";
        break;

    case Shock_src::misc:
        str_ref = "Other";
        break;

    case Shock_src::END:
        break;
    }
}

} //namespace

void run()
{
    mk_lines();

    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = lines_.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

    int top_nr = 0;
    int btm_nr = std::min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

    while (true)
    {
        render::clear_screen();

        render::draw_info_scr_interface("Character description",
                                        Inf_screen_type::scrolling);

        int y_pos = 1;

        for (int i = top_nr; i <= btm_nr; ++i)
        {
            const Str_and_clr& line = lines_[i];

            render::draw_text(line.str,
                              Panel::screen,
                              P(0, y_pos++),
                              line.clr);
        }

        render::update_screen();

        const Key_data& d = input::input();

        if (d.key == '2' || d.sdl_key == SDLK_DOWN || d.key == 'j')
        {
            top_nr += LINE_JUMP;

            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = std::min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }
        else if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - LINE_JUMP);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }

        btm_nr = std::min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
    }

    render::draw_map_and_interface();
}

} //character_descr
