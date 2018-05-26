#include "look.hpp"

#include <string>
#include <climits>

#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "attack_data.hpp"
#include "feature.hpp"
#include "feature_mob.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"
#include "inventory.hpp"
#include "io.hpp"
#include "item.hpp"
#include "map.hpp"
#include "marker.hpp"
#include "msg_log.hpp"
#include "property.hpp"
#include "property_data.hpp"
#include "property_handler.hpp"
#include "query.hpp"
#include "text_format.hpp"

// -----------------------------------------------------------------------------
// private
// -----------------------------------------------------------------------------
namespace
{

static std::string get_mon_memory_turns_descr(const Actor& actor)
{
        std::string str = "";

        const int nr_turns_aware = actor.data().nr_turns_aware;

        if (nr_turns_aware <= 0)
        {
                return "";
        }

        const std::string name_a = actor.name_a();

        if (nr_turns_aware < 50)
        {
                const std::string nr_turns_aware_str =
                        std::to_string(nr_turns_aware);

                str =
                        name_a +
                        " will remember hostile creatures for at least " +
                        nr_turns_aware_str +
                        " turns.";
        }
        else // Very high number of turns awareness
        {
                str =
                        name_a +
                        " remembers hostile creatures for a very long time.";
        }

        return str;
}

static std::string get_mon_dlvl_descr(const Actor& actor)
{
        std::string str = "";

        const auto& d = actor.data();

        const int dlvl = d.spawn_min_dlvl;

        if ((dlvl <= 1) || (dlvl >= dlvl_last))
        {
                return "";
        }

        const std::string dlvl_str = std::to_string(dlvl);

        if (d.is_unique)
        {
                str =
                        d.name_the +
                        " usually dwells beneath level " +
                        dlvl_str +
                        ".";
        }
        else // Not unique
        {
                str =
                        "They usually dwell beneath level " +
                        dlvl_str +
                        ".";
        }

        return str;
}

static std::string mon_speed_type_to_str(const Actor& actor)
{
        const auto& d = actor.data();

        if (d.speed_pct > (int)ActorSpeed::fast)
        {
                return "very swiftly";
        }

        if (d.speed_pct > ((int)ActorSpeed::normal + 20))
        {
                return "fast";
        }

        if (d.speed_pct > (int)ActorSpeed::normal)
        {
                return "somewhat fast";
        }

        if (d.speed_pct == (int)ActorSpeed::normal)
        {
                return "";
        }

        if (d.speed_pct >= (int)ActorSpeed::slow)
        {
                return "slowly";
        }

        return "sluggishly";
}

static std::string get_mon_speed_descr(const Actor& actor)
{
        std::string str = "";

        const auto& d = actor.data();

        const std::string speed_type_str = mon_speed_type_to_str(actor);

        if (speed_type_str.empty())
        {
                return str;
        }

        if (d.is_unique)
        {
                str =
                        " " +
                        d.name_the +
                        " appears to move " +
                        speed_type_str +
                        ".";
        }
        else // Not unique
        {
                str =
                        " They appear to move " +
                        speed_type_str +
                        ".";
        }

        return str;
}

static void mon_shock_lvl_to_str(const Actor& actor,
                                 std::string& shock_str_out,
                                 std::string& punct_str_out)
{
        shock_str_out = "";
        punct_str_out = "";

        switch (actor.data().mon_shock_lvl)
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

static std::string get_mon_shock_descr(const Actor& actor)
{
        std::string str = "";

        std::string shock_str = "";

        std::string shock_punct_str = "";

        mon_shock_lvl_to_str(actor, shock_str, shock_punct_str);

        if (shock_str.empty())
        {
                return str;
        }

        if (actor.data().is_unique)
        {
                str +=
                        actor.name_the() +
                        " is " +
                        shock_str +
                        " to behold" +
                        shock_punct_str;
        }
        else // Not unique
        {
                str +=
                        "They are " +
                        shock_str +
                        " to behold" +
                        shock_punct_str;
        }

        return str;
}

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
        if (actor_.data().allow_generated_descr)
        {
                const std::string auto_descr = auto_description_str();

                descr += auto_descr;
        }

        const auto descr_lines =
                text_format::split(
                        descr,
                        panels::get_x1(Panel::screen));

        const size_t nr_descr_lines = descr_lines.size();

        P p(0, 0);

        io::cover_area(
                Panel::screen,
                p,
                P(panels::get_w(Panel::screen), nr_descr_lines));

        // Add the description
        for (const std::string& s : descr_lines)
        {
                put_text(s, p, colors::light_white());
                ++p.y;
        }

        p.y += 2;

        // Hit chances
        const std::string indent = "   ";

        const std::vector<std::string> sections =
                {
                        "Chance to hit monster",
                        indent + "Melee",
                        indent + "Ranged weapon",
                        indent + "Throwing",
                };

        P p_section(p);

        for (const std::string& str : sections)
        {
                if (!str.empty())
                {
                        const Color& color =
                                (str[0] == ' ') ?
                                colors::gray() :
                                colors::light_white();

                        put_text(str, p_section, color);
                }

                ++p_section.y;
        }

        p.x = 24;

        auto& player_inv = map::player->inv();

        const Item* player_wpn_item = player_inv.item_in_slot(SlotId::wpn);

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

        auto* player_thrown_item = player_inv.item_in_slot(SlotId::thrown);

        if (player_thrown_item)
        {
                player_throwing.reset(
                        new ThrowAttData(map::player,
                                         actor_.pos, // Aim position
                                         actor_.pos, // Current position
                                         *player_thrown_item));

        }

        const std::vector<std::string> labels =
                {
                        "Skill",
                        "Weapon",
                        "Dodging",
                        "Range",
                        "Monster state",
                        "Total*"
                };

        const int val_undefined = INT_MAX;

        std::vector<int> player_melee_values;

        if (player_melee)
        {
                player_melee_values.push_back(player_melee->skill_mod);
                player_melee_values.push_back(player_melee->wpn_mod);
                player_melee_values.push_back(player_melee->dodging_mod);
                player_melee_values.push_back(val_undefined);
                player_melee_values.push_back(player_melee->state_mod);
                player_melee_values.push_back(player_melee->hit_chance_tot);
        }

        std::vector<int> player_ranged_values;

        if (player_ranged)
        {
                player_ranged_values.push_back(player_ranged->skill_mod);
                player_ranged_values.push_back(player_ranged->wpn_mod);
                player_ranged_values.push_back(player_ranged->dodging_mod);
                player_ranged_values.push_back(player_ranged->dist_mod);
                player_ranged_values.push_back(player_ranged->state_mod);
                player_ranged_values.push_back(player_ranged->hit_chance_tot);
        }

        std::vector<int> player_throw_values;

        if (player_throwing)
        {
                player_throw_values.push_back(player_throwing->skill_mod);
                player_throw_values.push_back(player_throwing->wpn_mod);
                player_throw_values.push_back(player_throwing->dodging_mod);
                player_throw_values.push_back(player_throwing->dist_mod);
                player_throw_values.push_back(player_throwing->state_mod);
                player_throw_values.push_back(player_throwing->hit_chance_tot);
        }

        auto print_val = [&](const int val,
                             const P& p,
                             const bool is_sum)
                {
                        Color color;

                        std::string str;

                        if (is_sum)
                        {
                                str = std::to_string(val) + "%";

                                color = colors::light_white();
                        }
                        else // Not sum
                        {
                                if (val == val_undefined)
                                {
                                        color = colors::dark_gray();

                                        str = "N/A";
                                }
                                else // Value is set
                                {
                                        color = colors::white();

                                        str = std::to_string(val);

                                        if (val >= 0)
                                        {
                                                str.insert(begin(str), '+');
                                        }

                                        str += "%";

                                        color = colors::white();

                                        color =
                                        (val < 0) ? colors::msg_bad() :
                                        (val == 0) ? colors::white() :
                                        colors::msg_good();
                                }
                        }

                        put_text(str, p, color);
                };

        const int hit_chances_y0 = p.y;

        for (size_t i = 0; i < labels.size(); ++i)
        {
                const bool is_sum = i == (labels.size() - 1);

                const Color label_color = colors::light_white();

                const std::string& label = labels[i];

                p.y = hit_chances_y0;

                put_text(label, p, label_color);

                ++p.y;

                // Player melee
                if (player_melee)
                {
                        print_val(player_melee_values[i],
                                  p,
                                  is_sum);
                }

                ++p.y;

                // Player ranged
                if (player_ranged)
                {
                        print_val(player_ranged_values[i],
                                  p,
                                  is_sum);
                }

                ++p.y;

                // Player throwing
                if (player_throwing)
                {
                        print_val(player_throw_values[i],
                                  p,
                                  is_sum);
                }

                ++p.y;

                p.x += label.size() + 2;
        }

        p.x = 0;

        p.y += 1;

        put_text(
                " * Attacks can always critically fail or succeed "
                "(2% chance each)",
                p,
                colors::gray());

        p.y += 3;

        put_text("Current properties", p, colors::light_white());

        p.y += 1;

        auto prop_list =
                actor_.properties()
                .property_names_temporary_negative();

        // Remove all non-negative properties (we should not show temporary
        // spell resistance for example), and all natural properties (properties
        // which all monsters of this type starts with)
        for (auto it = begin(prop_list); it != end(prop_list);)
        {
                auto* const prop = it->prop;

                const auto id = prop->id();

                const bool is_natural_prop =
                        actor_.data().natural_props[(size_t)id];

                if (is_natural_prop ||
                    (prop->duration_mode() == PropDurationMode::indefinite) ||
                    (prop->alignment() != PropAlignment::bad))
                {
                        it = prop_list.erase(it);
                }
                else // Not a natural property
                {
                        ++it;
                }
        }

        const std::string offset = "   ";

        if (prop_list.empty())
        {
                put_text( offset + "None", p, colors::white());

                p.y += 1;
        }
        else // Has properties
        {
                const int max_w_descr = (panels::get_x1(Panel::screen) * 3) / 4;

                for (const auto& e : prop_list)
                {
                        const auto& title = e.title;

                        put_text(offset + title.str, p, e.title.color);

                        p.y += 1;

                        const auto descr_formatted =
                                text_format::split(e.descr,
                                                   max_w_descr);

                        for (const auto& descr_line : descr_formatted)
                        {
                                put_text(offset + descr_line,
                                         p,
                                         colors::gray());

                                p.y += 1;
                        }

                        p.y += 1;
                }
        }

        // Add a single empty line at the end (looks better)
        lines_.resize(lines_.size() + 1);
}

void ViewActorDescr::draw()
{
        draw_interface();

        const int nr_lines_tot = lines_.size();

        int btm_nr = std::min(
                top_idx_ + max_nr_lines_on_screen() - 1,
                nr_lines_tot - 1);

        int screen_y = 1;

        for (int y = top_idx_; y <= btm_nr; ++y)
        {
                const auto& line = lines_[y];

                for (const auto& text : line)
                {
                        io::draw_text(
                                text.str,
                                Panel::screen,
                                P(text.x_pos, screen_y),
                                text.color);
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

                if (nr_lines_tot <= max_nr_lines_on_screen())
                {
                        top_idx_ = 0;
                }
                else
                {
                        top_idx_ = std::min(
                                nr_lines_tot - max_nr_lines_on_screen(),
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

void ViewActorDescr::put_text(const std::string str,
                              const P& p,
                              const Color& color)
{
        if (p.y >= (int)lines_.size())
        {
                lines_.resize(p.y + 1);
        }

        auto& line = lines_[p.y];

        ActorDescrText text;

        text.str = str;

        text.color = color;

        text.x_pos = p.x;

        line.push_back(text);
}

std::string ViewActorDescr::auto_description_str() const
{
        std::string str = "";

        auto append = [](std::string& base, const std::string& addition) {

                if (!base.empty() && !addition.empty())
                {
                        base += " ";
                }

                base += addition;
        };

        append(str, get_mon_dlvl_descr(actor_));

        append(str, get_mon_speed_descr(actor_));

        append(str, get_mon_memory_turns_descr(actor_));

        if (actor_.data().is_undead)
        {
                append(str, "This creature is undead.");
        }

        append(str, get_mon_shock_descr(actor_));

        return str;
}

// -----------------------------------------------------------------------------
// Look
// -----------------------------------------------------------------------------
namespace look
{

void print_location_info_msgs(const P& pos)
{
        const Cell& cell = map::cells.at(pos);

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

                // Describe darkness
                if (map::dark.at(pos) && !map::light.at(pos))
                {
                        msg_log::add("It is very dark here.");
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
                                str = text_format::first_to_upper(
                                        actor->corpse_name_a());

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
                                // The player only sees the actor but not the
                                // cell (e.g. through infravision) - print the
                                // initial "I see here" message
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
