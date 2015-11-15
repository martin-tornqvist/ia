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

std::string mon_speed_str(const Actor_data_t& def)
{
    switch (def.speed)
    {
    case Actor_speed::sluggish:
        return "sluggishly";

    case Actor_speed::slow:
        return "slowly";

    case Actor_speed::normal:
        return "";

    case Actor_speed::fast:
        return "swiftly";

    case Actor_speed::fastest:
        return "very swiftly";

    case Actor_speed::END:
        break;
    }

    return "";
}

std::string mon_dwell_lvl_str(const Actor_data_t& def)
{
    if (def.spawn_max_dlvl >= DLVL_LAST)
    {
        return "";
    }

    return to_str(std::max(1, def.spawn_min_dlvl - 1));
}

void mon_shock_str(const Actor_data_t& def,
                   std::string& shock_str_out,
                   std::string& punct_str_out)
{
    shock_str_out = "";
    punct_str_out = "";

    switch (def.mon_shock_lvl)
    {
    case Mon_shock_lvl::unsettling:
        shock_str_out = "unsettling";
        punct_str_out = ".";
        break;

    case Mon_shock_lvl::frightening:
        shock_str_out = "frightening";
        punct_str_out = ".";
        break;

    case Mon_shock_lvl::terrifying:
        shock_str_out = "terrifying";
        punct_str_out = "!";
        break;

    case Mon_shock_lvl::mind_shattering:
        shock_str_out = "mind shattering";
        punct_str_out = "!";
        break;

    case Mon_shock_lvl::none:
    case Mon_shock_lvl::END:
        break;
    }
}

} //namespace

void add_auto_description_lines(const Actor& actor, std::string& line)
{
    const Actor_data_t& def = actor.data();

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

    const int NR_TURNS_AWARE = def.nr_turns_aware;

    const std::string name_a = actor.name_a();

    if (NR_TURNS_AWARE > 0)
    {
        if (NR_TURNS_AWARE < 50)
        {
            const std::string nr_turns_aware_str = to_str(NR_TURNS_AWARE);

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
            str = item->name(Item_ref_type::plural, Item_ref_inf::yes,
                             Item_ref_att_inf::wpn_context);

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
    std::string descr = actor.descr();

    //Add auto-description.
    if (actor.data().is_auto_descr_allowed)
    {
        auto_descr_actor::add_auto_description_lines(actor, descr);
    }

    std::vector<std::string> formatted_text;
    text_format::split(descr, MAP_W - 1, formatted_text);

    const size_t NR_OF_LINES = formatted_text.size();

    render::cover_area(Panel::screen, P(0, 1), P(MAP_W, NR_OF_LINES));

    int y = 1;

    for (std::string& s : formatted_text)
    {
        render::draw_text(s, Panel::screen, P(0, y), clr_white_high);
        y++;
    }

    render::update_screen();

    query::wait_for_key_press();
}

} //look
