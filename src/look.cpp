#include "look.hpp"

#include <string>

#include "actor_mon.hpp"
#include "feature.hpp"
#include "item.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "text_format.hpp"
#include "query.hpp"
#include "marker.hpp"
#include "inventory.hpp"
#include "attack.hpp"
#include "game_time.hpp"
#include "render.hpp"
#include "utils.hpp"
#include "feature_mob.hpp"
#include "feature_rigid.hpp"

using namespace std;

namespace auto_descr_actor
{

namespace
{

string get_normal_group_size_str(const actor_data_t& def)
{
    const Mon_group_size s = def.group_size;

    return
        s == Mon_group_size::alone ? "alone"           :
        s == Mon_group_size::few    ? "in small groups" :
        s == Mon_group_size::group  ? "in groups"       :
        s == Mon_group_size::horde  ? "in hordes"       : "in swarms";
}

string get_speed_str(const actor_data_t& def)
{
    switch (def.speed)
    {
    case Actor_speed::sluggish:   {return "sluggishly";}
    case Actor_speed::slow:       {return "slowly";}
    case Actor_speed::normal:     {return "at normal speed";}
    case Actor_speed::fast:       {return "fast";}
    case Actor_speed::fastest:    {return "very fast";}
    case Actor_speed::END: {} break;
    }
    return "";
}

string get_dwelling_lvl_str(const actor_data_t& def)
{
    return to_str(max(1, def.spawn_min_dLVL - 1));
}

} //namespace

void add_auto_description_lines(const Actor& actor, string& line)
{
    const actor_data_t& def = actor.get_data();

    if (def.is_unique)
    {
        if (def.spawn_min_dLVL < DLVL_LAST)
        {
            line += " " + def.name_the + " is normally found beneath level " +
                    get_dwelling_lvl_str(def) + ". ";
        }
    }
    else
    {
        line += " They tend to dwell " + get_normal_group_size_str(def) + ",";
        line += " and usually stay at depths beneath level " +
                get_dwelling_lvl_str(def) + ".";
        line += " They appear to move " + get_speed_str(def) + ". ";
    }
}

} //Auto_descr_actor

namespace look
{

void print_location_info_msgs(const Pos& pos)
{
    const Cell& cell = map::cells[pos.x][pos.y];

    if (map::cells[pos.x][pos.y].is_seen_by_player)
    {
        msg_log::add("I see here:");

        //Describe rigid.
        string str = cell.rigid->get_name(Article::a);

        text_format::first_to_upper(str);

        msg_log::add(str + ".");

        //Describe mobile features.
        for (auto* mob : game_time::mobs_)
        {
            if (mob->get_pos() == pos)
            {
                str = mob->get_name(Article::a);

                text_format::first_to_upper(str);

                msg_log::add(str  + ".");
            }
        }

        //Describe item.
        Item* item = cell.item;
        if (item)
        {
            str = item->get_name(Item_ref_type::plural, Item_ref_inf::yes,
                                 Item_ref_att_inf::wpn_context);

            text_format::first_to_upper(str);

            msg_log::add(str + ".");
        }

        //Describe dead actors.
        for (Actor* actor : game_time::actors_)
        {
            if (actor->is_corpse() && actor->pos == pos)
            {
                str = actor->get_corpse_name_a();

                text_format::first_to_upper(str);

                msg_log::add(str + ".");
            }
        }

        //Describe living actor.
        Actor* actor = utils::get_actor_at_pos(pos);
        if (actor && actor != map::player)
        {
            if (actor->is_alive())
            {
                if (map::player->can_see_actor(*actor, nullptr))
                {
                    str = actor->get_name_a();

                    text_format::first_to_upper(str);

                    msg_log::add(str + ".");
                }
            }
        }

    }
    else //Cell not seen
    {
        msg_log::add("I have no vision here.");
    }
}

void print_detailed_actor_descr(const Actor& actor)
{
    //Add written description.
    string descr = actor.get_data().descr;

    //Add auto-description.
    if (actor.get_data().is_auto_descr_allowed)
    {
        auto_descr_actor::add_auto_description_lines(actor, descr);
    }

    vector<string> formatted_text;
    text_format::line_to_lines(descr, MAP_W - 1, formatted_text);

    const size_t NR_OF_LINES = formatted_text.size();

    render::cover_area(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

    int y = 1;
    for (string& s : formatted_text)
    {
        render::draw_text(s, Panel::screen, Pos(0, y), clr_white_high);
        y++;
    }

    render::update_screen();

    query::wait_for_key_press();
}

} //look
