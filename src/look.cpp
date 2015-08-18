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

namespace auto_descr_actor
{

namespace
{

std::string normal_group_size_str(const Actor_data_t& def)
{
    const Mon_group_size s = def.group_size;

    return
        s == Mon_group_size::alone ? "alone"           :
        s == Mon_group_size::few    ? "in small groups" :
        s == Mon_group_size::group  ? "in groups"       :
        s == Mon_group_size::horde  ? "in hordes"       : "in swarms";
}

std::string speed_str(const Actor_data_t& def)
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

std::string dwelling_lvl_str(const Actor_data_t& def)
{
    return to_str(std::max(1, def.spawn_min_dlvl - 1));
}

} //namespace

void add_auto_description_lines(const Actor& actor, std::string& line)
{
    const Actor_data_t& def = actor.data();

    if (def.is_unique)
    {
        if (def.spawn_min_dlvl < DLVL_LAST)
        {
            line += " " + def.name_the + " is normally found beneath level " +
                    dwelling_lvl_str(def) + ". ";
        }
    }
    else
    {
        line += " They tend to dwell " + normal_group_size_str(def) + ",";
        line += " and usually stay at depths beneath level " +
                dwelling_lvl_str(def) + ".";
        line += " They appear to move " + speed_str(def) + ". ";
    }
}

} //Auto_descr_actor

namespace look
{

void print_location_info_msgs(const Pos& pos)
{
    bool did_see_something = false;

    const Cell& cell = map::cells[pos.x][pos.y];

    const std::string i_see_here_str = "I see here:";

    if (map::cells[pos.x][pos.y].is_seen_by_player)
    {
        did_see_something = true;

        msg_log::add(i_see_here_str);

        //Describe rigid.
        std::string str = cell.rigid->name(Article::a);

        text_format::first_to_upper(str);

        msg_log::add(str + ".");

        //Describe mobile features.
        for (auto* mob : game_time::mobs_)
        {
            if (mob->pos() == pos)
            {
                str = mob->name(Article::a);

                text_format::first_to_upper(str);

                msg_log::add(str  + ".");
            }
        }

        //Describe item.
        Item* item = cell.item;

        if (item)
        {
            str = item->name(Item_ref_type::plural, Item_ref_inf::yes,
                             Item_ref_att_inf::wpn_context);

            text_format::first_to_upper(str);

            msg_log::add(str + ".");
        }

        //Describe dead actors.
        for (Actor* actor : game_time::actors_)
        {
            if (actor->is_corpse() && actor->pos == pos)
            {
                str = actor->corpse_name_a();

                text_format::first_to_upper(str);

                msg_log::add(str + ".");
            }
        }

    }

    //Describe living actor.
    Actor* actor = utils::actor_at_pos(pos);

    if (actor && !actor->is_player() && actor->is_alive() && map::player->can_see_actor(*actor))
    {
        if (!did_see_something)
        {
            //The player only sees the actor but not the cell (e.g. through infravision)
            //Print the initial "I see here" message.
            msg_log::add(i_see_here_str);
        }

        did_see_something = true;

        std::string str = actor->name_a();

        text_format::first_to_upper(str);

        msg_log::add(str + ".");
    }

    if (!did_see_something)
    {
        msg_log::add("I have no vision here.");
    }
}

void print_detailed_actor_descr(const Actor& actor)
{
    //Add written description.
    std::string descr = actor.data().descr;

    //Add auto-description.
    if (actor.data().is_auto_descr_allowed)
    {
        auto_descr_actor::add_auto_description_lines(actor, descr);
    }

    std::vector<std::string> formatted_text;
    text_format::line_to_lines(descr, MAP_W - 1, formatted_text);

    const size_t NR_OF_LINES = formatted_text.size();

    render::cover_area(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

    int y = 1;

    for (std::string& s : formatted_text)
    {
        render::draw_text(s, Panel::screen, Pos(0, y), clr_white_high);
        y++;
    }

    render::update_screen();

    query::wait_for_key_press();
}

} //look
