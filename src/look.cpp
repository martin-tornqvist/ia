#include "look.h"

#include <string>

#include "actor_mon.h"
#include "feature.h"
#include "item.h"
#include "log.h"
#include "map.h"
#include "actor_player.h"
#include "text_format.h"
#include "query.h"
#include "marker.h"
#include "inventory.h"
#include "attack.h"
#include "game_time.h"
#include "render.h"
#include "utils.h"
#include "feature_mob.h"
#include "feature_rigid.h"

using namespace std;

namespace Auto_descr_actor
{

namespace
{

string get_normal_group_size_str(const Actor_data_t& def)
{
    const Mon_group_size s = def.group_size;

    return
        s == Mon_group_size::alone ? "alone"           :
        s == Mon_group_size::few    ? "in small groups" :
        s == Mon_group_size::group  ? "in groups"       :
        s == Mon_group_size::horde  ? "in hordes"       : "in swarms";
}

string get_speed_str(const Actor_data_t& def)
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

string get_dwelling_lvl_str(const Actor_data_t& def)
{
    return to_str(max(1, def.spawn_min_dLVL - 1));
}

} //namespace

void add_auto_description_lines(const Actor& actor, string& line)
{
    const Actor_data_t& def = actor.get_data();

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

namespace Look
{

void print_location_info_msgs(const Pos& pos)
{
    const Cell& cell = Map::cells[pos.x][pos.y];

    if (Map::cells[pos.x][pos.y].is_seen_by_player)
    {
        Log::add_msg("I see here:");

        //Describe rigid.
        string str = cell.rigid->get_name(Article::a);

        Text_format::first_to_upper(str);

        Log::add_msg(str + ".");

        //Describe mobile features.
        for (auto* mob : Game_time::mobs_)
        {
            if (mob->get_pos() == pos)
            {
                str = mob->get_name(Article::a);

                Text_format::first_to_upper(str);

                Log::add_msg(str  + ".");
            }
        }

        //Describe item.
        Item* item = cell.item;
        if (item)
        {
            str = item->get_name(Item_ref_type::plural, Item_ref_inf::yes,
                                Item_ref_att_inf::wpn_context);

            Text_format::first_to_upper(str);

            Log::add_msg(str + ".");
        }

        //Describe dead actors.
        for (Actor* actor : Game_time::actors_)
        {
            if (actor->is_corpse() && actor->pos == pos)
            {
                str = actor->get_corpse_name_a();

                Text_format::first_to_upper(str);

                Log::add_msg(str + ".");
            }
        }

        //Describe living actor.
        Actor* actor = Utils::get_actor_at_pos(pos);
        if (actor && actor != Map::player)
        {
            if (actor->is_alive())
            {
                if (Map::player->can_see_actor(*actor, nullptr))
                {
                    str = actor->get_name_a();

                    Text_format::first_to_upper(str);

                    Log::add_msg(str + ".");
                }
            }
        }

    }
    else //Cell not seen
    {
        Log::add_msg("I have no vision here.");
    }
}

void print_detailed_actor_descr(const Actor& actor)
{
    //Add written description.
    string descr = actor.get_data().descr;

    //Add auto-description.
    if (actor.get_data().is_auto_descr_allowed)
    {
        Auto_descr_actor::add_auto_description_lines(actor, descr);
    }

    vector<string> formatted_text;
    Text_format::line_to_lines(descr, MAP_W - 1, formatted_text);

    const size_t NR_OF_LINES = formatted_text.size();

    Render::cover_area(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

    int y = 1;
    for (string& s : formatted_text)
    {
        Render::draw_text(s, Panel::screen, Pos(0, y), clr_white_high);
        y++;
    }

    Render::update_screen();

    Query::wait_for_key_press();
}

} //Look
