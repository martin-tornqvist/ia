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
#include "io.hpp"
#include "feature_mob.hpp"
#include "feature_rigid.hpp"

namespace auto_descr_actor
{

namespace
{

std::string mon_speed_str(const ActorDataT& def)
{
    switch (def.speed)
    {
    case ActorSpeed::sluggish:
        return "sluggishly";

    case ActorSpeed::slow:
        return "slowly";

    case ActorSpeed::normal:
        return "";

    case ActorSpeed::fast:
        return "swiftly";

    case ActorSpeed::fastest:
        return "very swiftly";

    case ActorSpeed::END:
        break;
    }

    return "";
}

std::string mon_dwell_lvl_str(const ActorDataT& def)
{
    const int min_dlvl = def.spawn_min_dlvl;

    if (min_dlvl <= 1 || min_dlvl >= dlvl_last)
    {
        return "";
    }

    return to_str(min_dlvl);
}

void mon_shock_str(const ActorDataT& def,
                   std::string& shock_str_out,
                   std::string& punct_str_out)
{
    shock_str_out = "";
    punct_str_out = "";

    switch (def.mon_shock_lvl)
    {
    case MonShockLvl::unsettling:
        shock_str_out = "unsettling";
        punct_str_out = ".";
        break;

    case MonShockLvl::frightening:
        shock_str_out = "frightening";
        punct_str_out = ".";
        break;

    case MonShockLvl::terrifying:
        shock_str_out = "terrifying";
        punct_str_out = "!";
        break;

    case MonShockLvl::mind_shattering:
        shock_str_out = "mind shattering";
        punct_str_out = "!";
        break;

    case MonShockLvl::none:
    case MonShockLvl::END:
        break;
    }
}

} //namespace

void add_auto_description_lines(const Actor& actor, std::string& line)
{
    const ActorDataT& def = actor.data();

    if (def.is_unique)
    {
        const std::string dwell_str = mon_dwell_lvl_str(def);

        if (!dwell_str.empty())
        {
            line += " " + def.name_the + " usually dwells beneath level " + dwell_str + ".";
        }

        const std::string speed_str = mon_speed_str(def);

        if (!speed_str.empty())
        {
            line += " " + def.name_the + " appears to move " + speed_str + ".";
        }
    }
    else //Not unique
    {
        const std::string dwell_str = mon_dwell_lvl_str(def);

        if (!dwell_str.empty())
        {
            line += " They usually dwell beneath level " + dwell_str + ".";
        }

        const std::string speed_str = mon_speed_str(def);

        if (!speed_str.empty())
        {
            line += " They appear to move " + speed_str + ".";
        }
    }

    const int nr_turns_aware = def.nr_turns_aware;

    const std::string name_a = actor.name_a();

    if (nr_turns_aware > 0)
    {
        if (nr_turns_aware < 50)
        {
            const std::string nr_turns_aware_str = to_str(nr_turns_aware);

            line += " " + name_a + " will remember hostile creatures for at least " +
                    nr_turns_aware_str + " turns.";
        }
        else //Very high number of turns awareness
        {
            line += " " + name_a +
                    " remembers hostile creatures for a very long time.";
        }
    }

    if (def.is_undead)
    {
        line += " This creature is undead.";
    }

    std::string shock_str       = "";
    std::string shock_punct_str = "";

    mon_shock_str(def, shock_str, shock_punct_str);

    if (!shock_str.empty())
    {
        if (def.is_unique)
        {
            line += " " + def.name_the + " is " + shock_str + " to behold" + shock_punct_str;
        }
        else //Not unique
        {
            line += " They are " + shock_str + " to behold" + shock_punct_str;
        }
    }
}

} //auto_descr_actor

namespace look
{

void print_location_info_msgs(const P& pos)
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
        for (auto* mob : game_time::mobs)
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
            str = item->name(ItemRefType::plural, ItemRefInf::yes,
                             ItemRefAttInf::wpn_context);

            text_format::first_to_upper(str);

            msg_log::add(str + ".");
        }

        //Describe dead actors.
        for (Actor* actor : game_time::actors)
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
    Actor* actor = map::actor_at_pos(pos);

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
    io::clear_screen();

    io::draw_info_scr_interface("Monster info",
                                    InfScreenType::single_screen);

    //Add written description.
    std::string descr = actor.descr();

    //Add auto-description.
    if (actor.data().is_auto_descr_allowed)
    {
        auto_descr_actor::add_auto_description_lines(actor, descr);
    }

    std::vector<std::string> lines;
    text_format::split(descr, map_w - 1, lines);

    const size_t nr_lines = lines.size();

    P p(0, 1);

    io::cover_area(Panel::screen, p, P(map_w, nr_lines));

    //Draw the description
    for (std::string& s : lines)
    {
        io::draw_text(s, Panel::screen, p, clr_white_high);
        ++p.y;
    }

    //Draw properties line
    std::vector<StrAndClr> props_line;
    actor.prop_handler().props_interface_line(props_line);

    ++p.y;

    io::draw_text("Properties:", Panel::screen, p, clr_white_high);

    ++p.y;

    if (props_line.empty())
    {
        io::draw_text("None", Panel::screen, p, clr_white);
    }
    else //Has properties
    {
        for (const StrAndClr& current_prop_label : props_line)
        {
            io::draw_text(current_prop_label.str, Panel::screen, p, current_prop_label.clr);

            p.x += current_prop_label.str.size() + 1;
        }
    }

    io::update_screen();

    query::wait_for_key_press();
}

} //look
