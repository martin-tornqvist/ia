#include "character_descr.h"

#include <algorithm>

#include "player_bon.h"
#include "actor_player.h"
#include "render.h"
#include "text_format.h"
#include "input.h"
#include "item_potion.h"
#include "item_scroll.h"
#include "item_factory.h"
#include "item.h"
#include "map.h"

using namespace std;

namespace Character_descr
{

namespace
{

vector<Str_and_clr> lines_;

void mk_lines()
{
    lines_.clear();

    const string offset = "   ";
    const Clr& clr_heading   = clr_white_high;
    const Clr& clr_text      = clr_white;
    const Clr& clr_text_dark  = clr_gray;

    lines_.push_back({Map::player->get_name_the(), clr_menu_highlight});
    lines_.push_back({" ", clr_text});

    const Ability_vals& abilities = Map::player->get_data().ability_vals;

    lines_.push_back({"Combat skills", clr_heading});

    const int BASE_MELEE          = min(100, abilities.get_val(Ability_id::melee,
                                        true, *(Map::player)));

    const int BASE_RANGED         = min(100, abilities.get_val(Ability_id::ranged,
                                        true, *(Map::player)));

    const int BASE_DODGE_ATTACKS  = min(100, abilities.get_val(Ability_id::dodge_att,
                                        true, *(Map::player)));

    lines_.push_back({offset + "Melee    : " + to_str(BASE_MELEE)         + "%", clr_text});
    lines_.push_back({offset + "Ranged   : " + to_str(BASE_RANGED)        + "%", clr_text});
    lines_.push_back({offset + "Dodging  : " + to_str(BASE_DODGE_ATTACKS) + "%", clr_text});

    lines_.push_back({" ", clr_text});

    lines_.push_back({"Mental conditions", clr_heading});
    const int NR_LINES_BEFORE_MENTAL = lines_.size();

    const auto& phobias = Map::player->phobias;

    if (phobias[int(Phobia::dog)])
        lines_.push_back({offset + "Phobia of dogs",            clr_text});

    if (phobias[int(Phobia::rat)])
        lines_.push_back({offset + "Phobia of rats",            clr_text});

    if (phobias[int(Phobia::spider)])
        lines_.push_back({offset + "Phobia of spiders",         clr_text});

    if (phobias[int(Phobia::undead)])
        lines_.push_back({offset + "Phobia of the dead",        clr_text});

    if (phobias[int(Phobia::cramped_place)])
        lines_.push_back({offset + "Phobia of cramped spaces",  clr_text});

    if (phobias[int(Phobia::open_place)])
        lines_.push_back({offset + "Phobia of open places",     clr_text});

    if (phobias[int(Phobia::deep_places)])
        lines_.push_back({offset + "Phobia of deep places",     clr_text});

    if (phobias[int(Phobia::dark)])
        lines_.push_back({offset + "Phobia of darkness",        clr_text});

    if (Map::player->obsessions[int(Obsession::masochism)])
        lines_.push_back({offset + "Masochistic obsession",     clr_text});

    if (Map::player->obsessions[int(Obsession::sadism)])
        lines_.push_back({offset + "Sadistic obsession",        clr_text});

    const int NR_LINES_AFTER_MENTAL = lines_.size();

    if (NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL)
    {
        lines_.push_back({offset + "No special symptoms", clr_text});
    }

    lines_.push_back({" ", clr_text});

    lines_.push_back({"Potion knowledge", clr_heading});
    vector<Str_and_clr> potion_list;
    vector<Str_and_clr> manuscript_list;

    for (int i = 0; i < int(Item_id::END); ++i)
    {
        const Item_data_t* const d = Item_data::data[i];

        if (d->type == Item_type::potion && (d->is_tried || d->is_identified))
        {
            Item* item = Item_factory::mk(d->id);
            potion_list.push_back({offset + item->get_name(Item_ref_type::plain), d->clr});
            delete item;
        }
        else
        {
            if (d->type == Item_type::scroll && (d->is_tried || d->is_identified))
            {
                Item* item = Item_factory::mk(d->id);
                manuscript_list.push_back(Str_and_clr(offset + item->get_name(Item_ref_type::plain),
                                                   item->get_interface_clr()));
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
    lines_.push_back({" ", clr_text});


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
    lines_.push_back({" ", clr_text});

    lines_.push_back({"Traits gained", clr_heading});

    const int MAX_W_DESCR = (MAP_W * 2) / 3;

    for (int i = 0; i < int(Trait::END); ++i)
    {
        if (Player_bon::traits[i])
        {
            const Trait trait = Trait(i);
            string title = "", descr = "";
            Player_bon::get_trait_title(trait, title);
            Player_bon::get_trait_descr(trait, descr);
            lines_.push_back({offset + title, clr_text});
            vector<string> descr_lines;
            Text_format::line_to_lines(descr, MAX_W_DESCR, descr_lines);
            for (string& descr_line : descr_lines)
            {
                lines_.push_back({offset + descr_line, clr_text_dark});
            }
            lines_.push_back({" ", clr_text});
        }
    }
}

void get_shock_res_src_title(const Shock_src shock_src, string& str_ref)
{
    str_ref = "";
    switch (shock_src)
    {
    case Shock_src::time:            str_ref = "Time";                    break;
    case Shock_src::cast_intr_spell:   str_ref = "Casting learned spells";  break;
    case Shock_src::see_mon:          str_ref = "Seeing monsters";         break;
    case Shock_src::use_strange_item:  str_ref = "Using strange items";     break;
    case Shock_src::misc:            str_ref = "Other";                   break;
    case Shock_src::END: {} break;
    }
}

void draw_interface()
{
    const string decoration_line(MAP_W, '-');

    const int X_LABEL = 3;

    Render::draw_text(decoration_line, Panel::screen, Pos(0, 0), clr_gray);

    Render::draw_text(" Displaying character description ", Panel::screen,
                     Pos(X_LABEL, 0), clr_white);

    Render::draw_text(decoration_line, Panel::screen, Pos(0, SCREEN_H - 1),
                     clr_gray);

    Render::draw_text(info_scr_cmd_info, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clr_white);
}

} //namespace

void run()
{
    mk_lines();

    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = lines_.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

    int top_nr = 0;
    int btm_nr = min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

    while (true)
    {
        Render::clear_screen();
        draw_interface();
        int y_pos = 1;
        for (int i = top_nr; i <= btm_nr; ++i)
        {
            const Str_and_clr& line = lines_[i];
            Render::draw_text(line.str , Panel::screen, Pos(0, y_pos++), line.clr);
        }
        Render::update_screen();

        const Key_data& d = Input::get_input();

        if (d.key == '2' || d.sdl_key == SDLK_DOWN || d.key == 'j')
        {
            top_nr += LINE_JUMP;
            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }
        else if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = max(0, top_nr - LINE_JUMP);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
        btm_nr = min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
    }
    Render::draw_map_and_interface();
}

} //Character_descr
