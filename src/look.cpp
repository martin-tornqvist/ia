#include "look.hpp"

#include <string>
#include <climits>

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

namespace
{

const int max_nr_lines_on_scr_ = screen_h - 2;

} // namespace

// -----------------------------------------------------------------------------
// View actor description
// -----------------------------------------------------------------------------
StateId ViewActorDescr::id()
{
    return StateId::view_actor;
}

void ViewActorDescr::on_start()
{
    // Fixed description.
    std::string descr = actor_.descr();

    // Auto-description.
    if (actor_.data().is_auto_descr_allowed)
    {
        const std::string auto_descr = auto_description_str();

        descr += auto_descr;
    }

    const auto descr_lines =
        text_format::split(descr,
                           map_w - 1);

    const size_t nr_descr_lines = descr_lines.size();

    P p(0, 0);

    io::cover_area(Panel::screen,
                   p,
                   P(map_w, nr_descr_lines));

    // Add the description
    for (const std::string& s : descr_lines)
    {
        put_text(s,
                 p,
                 clr_white_lgt);
        ++p.y;
    }

    p.y += 2;

    // Hit chances
    const std::string indent = "  ";

    const std::vector<std::string> sections =
    {
        "Melee hit chance",
        indent + "Player vs monster",
        indent + "Monster vs player",
        "",
        "Ranged hit chance",
        indent + "Player vs monster",
        indent + "Player vs monster, throwing",
        indent + "Monster vs player",
    };

    P p_section(p);

    for (const std::string& str : sections)
    {
        if (!str.empty())
        {
            const Clr& clr =
                (str[0] == ' ') ?
                clr_gray :
                clr_white_lgt;

            put_text(str,
                     p_section,
                     clr);
        }

        ++p_section.y;
    }

    p.x = 31;

    const Item* player_wpn_item =
        map::player->inv().item_in_slot(SlotId::wpn);

    const Wpn* player_wpn = nullptr;

    if (player_wpn_item)
    {
        player_wpn = static_cast<const Wpn*>(player_wpn_item);
    }
    else
    {
        player_wpn = &map::player->unarmed_wpn();
    }

    ASSERT(player_wpn);

    std::unique_ptr<const MeleeAttData> player_melee;

    std::unique_ptr<const RangedAttData> player_ranged;

    if (player_wpn->data().melee.is_melee_wpn)
    {
        player_melee.reset(
            new MeleeAttData(map::player,
                             actor_,
                             *player_wpn));
    }

    if (player_wpn->data().ranged.is_ranged_wpn)
    {
        player_ranged.reset(
            new RangedAttData(map::player,
                              map::player->pos, // Origin
                              actor_.pos,       // Aim position
                              actor_.pos,       // Current position
                              *player_wpn));
    }

    std::unique_ptr<const ThrowAttData> player_throwing;

    if (map::player->thrown_item)
    {
        player_throwing.reset(
            new ThrowAttData(map::player,
                             actor_.pos,    // Aim position
                             actor_.pos,    // Current position
                             *map::player->thrown_item));

    }

    //
    // TODO: This would not make sense if the monster has multiple intrinsic
    //       melee weapons (currently, no monster has this however). Perhaps
    //       monsters should instead have one (optional) intrinsic melee weapon
    //       and one (optional) intrinsic ranged weapon (instead of a vector of
    //       intrinsic weapons), for simplicity's sake.
    //
    const Item* mon_melee_wpn_item = nullptr;

    const Item* mon_ranged_wpn_item = nullptr;

    for (Item* item : actor_.inv().intrinsics_)
    {
        if (item->data().melee.is_melee_wpn)
        {
            mon_melee_wpn_item = item;
        }

        if (item->data().ranged.is_ranged_wpn)
        {
            mon_ranged_wpn_item = item;
        }
    }

    // If monster does not have an intrinsic melee or ranged weapon, check if it
    // has a wielded weapon instead
    if (!mon_melee_wpn_item)
    {
        const Item* const wielded = actor_.inv().item_in_slot(SlotId::wpn);

        if (wielded &&
            wielded->data().melee.is_melee_wpn)
        {
            mon_melee_wpn_item = wielded;
        }
    }

    if (!mon_ranged_wpn_item)
    {
        const Item* const wielded = actor_.inv().item_in_slot(SlotId::wpn);

        if (wielded &&
            wielded->data().ranged.is_ranged_wpn)
        {
            mon_ranged_wpn_item = wielded;
        }
    }

    const Wpn* mon_melee_wpn = nullptr;

    std::unique_ptr<const MeleeAttData> mon_melee;

    if (mon_melee_wpn_item)
    {
        mon_melee_wpn = static_cast<const Wpn*>(mon_melee_wpn_item);

        mon_melee.reset(new MeleeAttData(&actor_,
                                         *map::player,
                                         *mon_melee_wpn));
    }

    const Wpn* mon_ranged_wpn = nullptr;

    std::unique_ptr<const RangedAttData> mon_ranged;

    if (mon_ranged_wpn_item)
    {
        mon_ranged_wpn = static_cast<const Wpn*>(mon_ranged_wpn_item);

        mon_ranged.reset(
            new RangedAttData(&actor_,
                              actor_.pos,           // Origin
                              map::player->pos,     // Aim position
                              map::player->pos,     // Current position
                              *mon_ranged_wpn));
    }

    const std::vector<std::string> labels =
    {
        "Skill",
        "Weapon",
        "Dodging",
        "Range",
        "State",
        "Total*"
    };

    const int val_undefined = INT_MAX;

    std::vector<int> player_melee_vals;

    if (player_melee)
    {
        player_melee_vals.push_back(player_melee->skill_mod);
        player_melee_vals.push_back(player_melee->wpn_mod);
        player_melee_vals.push_back(player_melee->dodging_mod);
        player_melee_vals.push_back(val_undefined);
        player_melee_vals.push_back(player_melee->state_mod);
        player_melee_vals.push_back(player_melee->hit_chance_tot);
    }

    std::vector<int> mon_melee_vals;

    if (mon_melee)
    {
        mon_melee_vals.push_back(mon_melee->skill_mod);
        mon_melee_vals.push_back(mon_melee->wpn_mod);
        mon_melee_vals.push_back(mon_melee->dodging_mod);
        mon_melee_vals.push_back(val_undefined);
        mon_melee_vals.push_back(mon_melee->state_mod);
        mon_melee_vals.push_back(mon_melee->hit_chance_tot);
    }

    std::vector<int> player_ranged_vals;

    if (player_ranged)
    {
        player_ranged_vals.push_back(player_ranged->skill_mod);
        player_ranged_vals.push_back(player_ranged->wpn_mod);
        player_ranged_vals.push_back(player_ranged->dodging_mod);
        player_ranged_vals.push_back(player_ranged->dist_mod);
        player_ranged_vals.push_back(player_ranged->state_mod);
        player_ranged_vals.push_back(player_ranged->hit_chance_tot);
    }

    std::vector<int> player_throwing_vals;

    if (player_throwing)
    {
        player_throwing_vals.push_back(player_throwing->skill_mod);
        player_throwing_vals.push_back(player_throwing->wpn_mod);
        player_throwing_vals.push_back(player_throwing->dodging_mod);
        player_throwing_vals.push_back(player_throwing->dist_mod);
        player_throwing_vals.push_back(player_throwing->state_mod);
        player_throwing_vals.push_back(player_throwing->hit_chance_tot);
    }

    std::vector<int> mon_ranged_vals;

    if (mon_ranged)
    {
        mon_ranged_vals.push_back(mon_ranged->skill_mod);
        mon_ranged_vals.push_back(mon_ranged->wpn_mod);
        mon_ranged_vals.push_back(mon_ranged->dodging_mod);
        mon_ranged_vals.push_back(mon_ranged->dist_mod);
        mon_ranged_vals.push_back(mon_ranged->state_mod);
        mon_ranged_vals.push_back(mon_ranged->hit_chance_tot);
    }

    auto print_val = [&](const int val,
                        const P& p,
                        const bool is_sum)
    {
        Clr clr;

        std::string str;

        if (is_sum)
        {
            str = std::to_string(val) + "%";

            clr = clr_white_lgt;
        }
        else // Not sum
        {
            if (val == val_undefined)
            {
                clr = clr_gray_drk;

                str = "N/A";
            }
            else // Value is set
            {
                clr = clr_white;

                str = std::to_string(val);

                if (val >= 0)
                {
                    str.insert(begin(str), '+');
                }

                str += "%";

                clr = clr_white;

                clr =
                (val < 0)   ? clr_msg_bad :
                (val == 0)  ? clr_white : clr_msg_good;
            }
        }

        put_text(str,
                 p,
                 clr);
    };

    const int hit_chances_y0 = p.y;

    for (size_t i = 0; i < labels.size(); ++i)
    {
        const bool is_sum = i == (labels.size() - 1);

        const Clr label_clr = clr_white_lgt;

        const std::string& label = labels[i];

        p.y = hit_chances_y0;

        put_text(label,
                 p,
                 label_clr);

        ++p.y;

        //
        // Player melee
        //
        if (player_melee)
        {
            print_val(player_melee_vals[i],
                      p,
                      is_sum);
        }

        ++p.y;

        //
        // Monster melee
        //
        if (mon_melee)
        {
            print_val(mon_melee_vals[i],
                      p,
                      is_sum);
        }

        p.y += 3;

        //
        // Player ranged
        //
        if (player_ranged)
        {
            print_val(player_ranged_vals[i],
                      p,
                      is_sum);
        }

        ++p.y;

        //
        // Player throwing
        //
        if (player_throwing)
        {
            print_val(player_throwing_vals[i],
                      p,
                      is_sum);
        }

        ++p.y;

        //
        // Monster ranged
        //
        if (mon_ranged)
        {
            print_val(mon_ranged_vals[i],
                      p,
                      is_sum);
        }

        p.x += label.size() + 2;
    }

    p.x = 0;

    p.y += 2;

    put_text(" * Attacks can always critically fail/succeed (2% chance each)",
             p,
             clr_gray);

    p.y += 3;

    // Add properties
    const std::string offset = "   ";

    put_text("Current properties",
             p,
             clr_white_lgt);

    const auto prop_list = actor_.prop_handler().props_list();

    ++p.y;

    p.x += 3;

    if (prop_list.empty())
    {
        put_text("None",
                 p,
                 clr_white);

        ++p.y;
    }
    else // Has properties
    {
        const int max_w_descr = (map_w * 3) / 4;

        for (const auto& e : prop_list)
        {
            const auto& title = e.first;

            put_text(title.str,
                     p,
                     title.clr);

            ++p.y;

            const auto descr_formatted =
                text_format::split(e.second,
                                   max_w_descr);

            for (const auto& descr_line : descr_formatted)
            {
                put_text(descr_line,
                         p,
                         clr_gray);

                ++p.y;
            }

            ++p.y;
        }
    }

    // Add a single empty line at the end (looks better)
    lines_.resize(lines_.size() + 1);
}

void ViewActorDescr::draw()
{
    io::draw_info_scr_interface("Monster info",
                                InfScreenType::scrolling);

    const int nr_lines_tot = lines_.size();

    int btm_nr = std::min(top_idx_ + max_nr_lines_on_scr_ - 1,
                          nr_lines_tot - 1);

    int screen_y = 1;

    for (int y = top_idx_; y <= btm_nr; ++y)
    {
        const auto& line = lines_[y];

        for (const auto& text : line)
        {
            io::draw_text(text.str,
                          Panel::screen,
                          P(text.p.x, screen_y),
                          text.clr);
        }

        ++screen_y;
    }
}

void ViewActorDescr::update()
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

        if (nr_lines_tot <= max_nr_lines_on_scr_)
        {
            top_idx_ = 0;
        }
        else
        {
            top_idx_ = std::min(nr_lines_tot - max_nr_lines_on_scr_, top_idx_);
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

void ViewActorDescr::put_text(const std::string str,
                              const P& p,
                              const Clr& clr)
{
    if (p.y >= (int)lines_.size())
    {
        lines_.resize(p.y + 1);
    }

    auto& line = lines_[p.y];

    ActorDescrText text;

    text.str = str;

    text.clr = clr;

    text.p = p;

    line.push_back(text);
}

std::string ViewActorDescr::mon_speed_str() const
{
    const auto& def = actor_.data();

    if (def.speed_pct > (int)ActorSpeed::fast)
    {
        return "very swiftly";
    }

    if (def.speed_pct > ((int)ActorSpeed::normal + 20))
    {
        return "fast";
    }

    if (def.speed_pct > (int)ActorSpeed::normal)
    {
        return "somewhat fast";
    }

    if (def.speed_pct == (int)ActorSpeed::normal)
    {
        return "";
    }

    if (def.speed_pct >= (int)ActorSpeed::slow)
    {
        return "slowly";
    }

    return "sluggishly";
}

std::string ViewActorDescr::mon_dwell_lvl_str() const
{
    const auto& def = actor_.data();

    const int min_dlvl = def.spawn_min_dlvl;

    if ((min_dlvl <= 1) ||
        (min_dlvl >= dlvl_last))
    {
        return "";
    }

    return std::to_string(min_dlvl);
}

void ViewActorDescr::mon_shock_str(std::string& shock_str_out,
                                   std::string& punct_str_out) const
{
    shock_str_out = "";
    punct_str_out = "";

    const auto& def = actor_.data();

    switch (def.mon_shock_lvl)
    {
    case ShockLvl::unsettling:
        shock_str_out = "unsettling";
        punct_str_out = ".";
        break;

    case ShockLvl::frightening:
        shock_str_out = "frightening";
        punct_str_out = ".";
        break;

    case ShockLvl::terrifying:
        shock_str_out = "terrifying";
        punct_str_out = "!";
        break;

    case ShockLvl::mind_shattering:
        shock_str_out = "mind shattering";
        punct_str_out = "!";
        break;

    case ShockLvl::none:
    case ShockLvl::END:
        break;
    }
}

std::string ViewActorDescr::auto_description_str() const
{
    std::string ret = "";

    const ActorDataT& def = actor_.data();

    if (def.is_unique)
    {
        const std::string dwell_str = mon_dwell_lvl_str();

        if (!dwell_str.empty())
        {
            ret +=
                " " +
                def.name_the +
                " usually dwells beneath level " +
                dwell_str +
                ".";
        }

        const std::string speed_str = mon_speed_str();

        if (!speed_str.empty())
        {
            ret +=
                " " +
                def.name_the +
                " appears to move " +
                speed_str +
                ".";
        }
    }
    else // Not unique
    {
        const std::string dwell_str = mon_dwell_lvl_str();

        if (!dwell_str.empty())
        {
            ret += " They usually dwell beneath level " + dwell_str + ".";
        }

        const std::string speed_str = mon_speed_str();

        if (!speed_str.empty())
        {
            ret += " They appear to move " + speed_str + ".";
        }
    }

    const int nr_turns_aware = def.nr_turns_aware;

    const std::string name_a =
        text_format::first_to_upper(
            actor_.name_a());

    if (nr_turns_aware > 0)
    {
        if (nr_turns_aware < 50)
        {
            const std::string nr_turns_aware_str =
                std::to_string(nr_turns_aware);

            ret +=
                " " +
                name_a +
                " will remember hostile creatures for at least " +
                nr_turns_aware_str +
                " turns.";
        }
        else // Very high number of turns awareness
        {
            ret +=
                " " +
                name_a +
                " remembers hostile creatures for a very long time.";
        }
    }

    if (def.is_undead)
    {
        ret += " This creature is undead.";
    }

    std::string shock_str       = "";
    std::string shock_punct_str = "";

    mon_shock_str(shock_str, shock_punct_str);

    if (!shock_str.empty())
    {
        if (def.is_unique)
        {
            ret +=
                " " +
                def.name_the +
                " is " +
                shock_str +
                " to behold" +
                shock_punct_str;
        }
        else // Not unique
        {
            ret +=
                " They are " +
                shock_str +
                " to behold" +
                shock_punct_str;
        }
    }

    return ret;
}

// -----------------------------------------------------------------------------
// Look
// -----------------------------------------------------------------------------
namespace look
{

void print_location_info_msgs(const P& pos)
{
    const Cell& cell = map::cells[pos.x][pos.y];

    const bool is_cell_seen = cell.is_seen_by_player;

    const std::string i_see_here_str = "I see here:";

    if (is_cell_seen)
    {
        msg_log::add(i_see_here_str);

        // Describe rigid
        std::string str = cell.rigid->name(Article::a);

        str = text_format::first_to_upper(str);

        msg_log::add(str + ".");

        // Describe mobile features
        for (auto* mob : game_time::mobs)
        {
            if (mob->pos() == pos)
            {
                str = mob->name(Article::a);

                str = text_format::first_to_upper(str);

                msg_log::add(str  + ".");
            }
        }

        // Describe item
        Item* item = cell.item;

        if (item)
        {
            str = item->name(ItemRefType::plural, ItemRefInf::yes,
                             ItemRefAttInf::wpn_main_att_mode);

            str = text_format::first_to_upper(str);

            msg_log::add(str + ".");
        }

        // Describe dead actors
        for (Actor* actor : game_time::actors)
        {
            if (actor->is_corpse() && actor->pos == pos)
            {
                str = text_format::first_to_upper(actor->corpse_name_a());

                msg_log::add(str + ".");
            }
        }

    }

    // Describe living actor
    Actor* actor = map::actor_at_pos(pos);

    if (actor &&
        !actor->is_player() &&
        actor->is_alive())
    {
        if (map::player->can_see_actor(*actor))
        {
            if (!is_cell_seen)
            {
                // The player only sees the actor but not the cell (e.g. through
                // infravision) - print the initial "I see here" message
                msg_log::add(i_see_here_str);
            }

            const std::string str =
                text_format::first_to_upper(
                    actor->name_a());

            msg_log::add(str + ".");
        }
        else // Cannot see actor
        {
            const Mon* const mon = static_cast<Mon*>(actor);

            if (mon->player_aware_of_me_counter_ > 0)
            {
                msg_log::add("There is a creature here.");
            }
        }
    }

    if (!is_cell_seen)
    {
        msg_log::add("I have no vision here.");
    }
}

} // look
