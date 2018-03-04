#include "property_handler.hpp"

#include "property.hpp"
#include "property_factory.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "saving.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "text_format.hpp"

// -----------------------------------------------------------------------------
// Property handler
// -----------------------------------------------------------------------------
PropHandler::PropHandler(Actor* owner) :
        owner_(owner)
{
        // Reset the active props info
        std::fill(std::begin(prop_count_cache_),
                  std::end(prop_count_cache_),
                  0);
}

void PropHandler::apply_natural_props_from_actor_data()
{
        const ActorData& d = owner_->data();

        // Add natural properties
        for (size_t i = 0; i < (size_t)PropId::END; ++i)
        {
                prop_count_cache_[i] = 0;

                if (d.natural_props[i])
                {
                        Prop* const prop = property_factory::make(PropId(i));

                        prop->set_indefinite();

                        apply(prop,
                              PropSrc::intr,
                              true,
                              Verbosity::silent);
                }
        }
}

PropHandler::~PropHandler()
{
#ifndef NDEBUG
        // Sanity check the property cache
        for (auto& prop : props_)
        {
                decr_prop_count(prop->id_);
        }

        // All property counts should be exactly zero now
        for (size_t i = 0; i < (size_t)PropId::END; ++i)
        {
                if (prop_count_cache_[i] != 0)
                {
                        TRACE << "Active property info at id " << i
                              << " not zero" << std::endl;
                        ASSERT(false);
                }
        }
#endif // NDEBUG
}

void PropHandler::save() const
{
        // Save intrinsic properties to file

        ASSERT(owner_);

        int nr_intr_props_ = 0;

        for (auto& prop : props_)
        {
                if (prop->src_ == PropSrc::intr)
                {
                        ++nr_intr_props_;
                }
        }

        saving::put_int(nr_intr_props_);

        for (auto& prop : props_)
        {
                if (prop->src_ == PropSrc::intr)
                {
                        saving::put_int((int)prop->id_);
                        saving::put_int(prop->nr_turns_left_);

                        prop->save();
                }
        }
}

void PropHandler::load()
{
        // Load intrinsic properties from file

        ASSERT(owner_);

        const int nr_props = saving::get_int();

        for (int i = 0; i < nr_props; ++i)
        {
                const auto prop_id = (PropId)saving::get_int();

                const int nr_turns = saving::get_int();

                Prop* const prop = property_factory::make(prop_id);

                if (nr_turns == -1)
                {
                        prop->set_indefinite();
                }
                else
                {
                        prop->set_duration(nr_turns);
                }

                prop->owner_ = owner_;

                prop->src_ = PropSrc::intr;

                props_.push_back(std::unique_ptr<Prop>(prop));

                incr_prop_count(prop_id);

                prop->load();
        }
}

void PropHandler::apply(Prop* const prop,
                        PropSrc src,
                        const bool force_effect,
                        const Verbosity verbosity)
{
        prop->owner_ = owner_;

        prop->src_ = src;

        std::unique_ptr<Prop> prop_owned(prop);

        // Check if property is resisted
        if (!force_effect)
        {
                if (is_resisting_prop(prop->id_))
                {
                        if (verbosity == Verbosity::verbose &&
                            owner_->is_alive())
                        {
                                print_resist_msg(*prop);
                        }

                        return;
                }
        }

        // The property can be applied

        if (prop->src_ == PropSrc::intr)
        {
                const bool did_apply_more =
                        try_apply_more_on_existing_intr_prop(*prop);

                if (did_apply_more)
                {
                        return;
                }
        }

        // The property should be applied individually

        props_.push_back(std::move(prop_owned));

        incr_prop_count(prop->id_);

        if (verbosity == Verbosity::verbose &&
            owner_->is_alive())
        {
                if (prop->should_update_vision_on_toggled())
                {
                        map::update_vision();
                }

                print_start_msg(*prop);
        }

        prop->on_applied();

        return;
}

void PropHandler::print_resist_msg(const Prop& prop)
{
        if (owner_->is_player())
        {
                const auto msg = prop.data_.msg_res_player;

                if (!msg.empty())
                {
                        msg_log::add(msg,
                                     colors::text(),
                                     true);
                }
        }
        else // Is a monster
        {
                if (map::player->can_see_actor(*owner_))
                {
                        const auto msg = prop.data_.msg_res_mon;

                        if (!msg.empty())
                        {
                                const std::string monster_name =
                                        text_format::first_to_upper(
                                                owner_->name_the());

                                msg_log::add(monster_name + " " + msg);
                        }
                }
        }
}

void PropHandler::print_start_msg(const Prop& prop)
{
        if (owner_->is_player())
        {
                const auto msg = prop.data_.msg_start_player;

                if (!msg.empty())
                {
                        const bool is_interrupting =
                                (prop.alignment() != PropAlignment::good);

                        msg_log::add(msg,
                                     colors::text(),
                                     is_interrupting);
                }
        }
        else // Is monster
        {
                if (map::player->can_see_actor(*owner_))
                {
                        const auto msg = prop.data_.msg_start_mon;

                        if (!msg.empty())
                        {
                                const std::string actor_name_the =
                                        text_format::first_to_upper(
                                                owner_->name_the());

                                msg_log::add(actor_name_the + " " + msg);
                        }
                }
        }
}

bool PropHandler::try_apply_more_on_existing_intr_prop(const Prop& new_prop)
{
        for (auto& old_prop : props_)
        {
                if ((old_prop->src_ == PropSrc::intr) &&
                    (new_prop.id_ == old_prop->id_))
                {
                        const int turns_left_old = old_prop->nr_turns_left_;
                        const int turns_left_new = new_prop.nr_turns_left_;

                        old_prop->on_more(new_prop);

                        const bool is_turns_nr_indefinite =
                                (turns_left_old < 0) ||
                                (turns_left_new < 0);

                        old_prop->nr_turns_left_ =
                                is_turns_nr_indefinite ?
                                -1 :
                                std::max(turns_left_old, turns_left_new);

                        if (new_prop.duration_mode_ ==
                            PropDurationMode::indefinite)
                        {
                                old_prop->duration_mode_ =
                                        PropDurationMode::indefinite;
                        }

                        return true;
                }
        }

        return false;
}

void PropHandler::add_prop_from_equipped_item(const Item* const item,
                                              Prop* const prop,
                                              const Verbosity verbosity)
{
        prop->item_applying_ = item;

        apply(prop,
              PropSrc::inv,
              true,
              verbosity);
}

Prop* PropHandler::prop(const PropId id) const
{
        if (has_prop(id))
        {
                for (auto& prop : props_)
                {
                        if (prop->id_ == id)
                        {
                                return prop.get();
                        }
                }
        }

        return nullptr;
}

void PropHandler::remove_props_for_item(const Item* const item)
{
        for (auto it = begin(props_); it != end(props_); /* No increment */)
        {
                auto* const prop = it->get();

                if (prop->item_applying_ == item)
                {
                        ASSERT(prop->src_ == PropSrc::inv);

                        ASSERT(prop->duration_mode_ ==
                               PropDurationMode::indefinite);

                        auto moved_prop = std::move(*it);

                        it = props_.erase(it);

                        decr_prop_count(moved_prop->id_);

                        on_prop_end(moved_prop.get());
                }
                else // Property was not added by this item
                {
                        ++it;
                }
        }
}

void PropHandler::incr_prop_count(const PropId id)
{
        int& v = prop_count_cache_[(size_t)id];

#ifndef NDEBUG
        if (v < 0)
        {
                TRACE << "Tried to increment property with current value "
                      << v << std::endl;

                ASSERT(false);
        }
#endif // NDEBUG

        ++v;
}

void PropHandler::decr_prop_count(const PropId id)
{
        int& v = prop_count_cache_[(size_t)id];

#ifndef NDEBUG
        if (v <= 0)
        {
                TRACE << "Tried to decrement property with current value "
                      << v << std::endl;

                ASSERT(false);
        }
#endif // NDEBUG

        --v;
}

void PropHandler::on_prop_end(Prop* const prop)
{
        if (prop->should_update_vision_on_toggled())
        {
                map::update_vision();
        }

        // Print end message if this is the last active property of this type
        if ((owner_->state() == ActorState::alive) &&
            prop_count_cache_[(size_t)prop->id_] == 0)
        {
                if (owner_->is_player())
                {
                        const auto msg = prop->msg_end_player();

                        if (!msg.empty())
                        {
                                msg_log::add(msg);
                        }
                }
                // Not player
                else if (map::player->can_see_actor(*owner_))
                {
                        const auto msg = prop->data_.msg_end_mon;

                        if (!msg.empty())
                        {
                                const std::string actor_name_the =
                                        text_format::first_to_upper(
                                                owner_->name_the());

                                msg_log::add(
                                        actor_name_the + " " + msg);
                        }
                }
        }

        prop->on_end();
}

bool PropHandler::end_prop(const PropId id)
{
        for (auto it = begin(props_); it != end(props_); ++it)
        {
                Prop* const prop = it->get();

                if ((prop->id_ == id) &&
                    (prop->src_ == PropSrc::intr))
                {
                        auto moved_prop = std::move(*it);

                        props_.erase(it);

                        decr_prop_count(moved_prop->id_);

                        on_prop_end(moved_prop.get());

                        return true;
                }
        }

        return false;
}

bool PropHandler::end_prop_silent(const PropId id)
{
        for (auto it = begin(props_); it != end(props_); ++it)
        {
                Prop* const prop = it->get();

                if ((prop->id_ == id) &&
                    (prop->src_ == PropSrc::intr))
                {
                        auto moved_prop = std::move(*it);

                        props_.erase(it);

                        decr_prop_count(moved_prop->id_);

                        return true;
                }
        }

        return false;
}

void PropHandler::on_placed()
{
        for (auto& prop: props_)
        {
                prop->on_placed();

                if (!owner_->is_alive())
                {
                        break;
                }
        }
}

void PropHandler::on_turn_begin()
{
        for (size_t i = 0; i < props_.size(); /* No increment */)
        {
                Prop* prop = props_[i].get();

                // Count down number of turns
                if (prop->nr_turns_left_ > 0)
                {
                        ASSERT(prop->src_ == PropSrc::intr);

                        --prop->nr_turns_left_;
                }

                // Aggravates monster?
                if (!owner_->is_player() &&
                    prop->is_making_mon_aware())
                {
                        auto* mon = static_cast<Mon*>(owner_);

                        mon->become_aware_player(false);
                }

                const auto prop_ended = prop->on_tick();

                // NOTE: The property may have removed itself at this point, if
                // so it signals this by returning 'PropEnded::yes'

                if (prop_ended == PropEnded::no)
                {
                        // Property has not been removed
                        ++i;
                }
        }
}

void PropHandler::on_turn_end()
{
        for (auto it = begin(props_); it != end(props_); /* No increment */)
        {
                Prop* prop = it->get();

                if (prop->is_finished())
                {
                        auto prop_moved = std::move(*it);

                        it = props_.erase(it);

                        decr_prop_count(prop_moved->id_);

                        on_prop_end(prop_moved.get());
                }
                else  // Property has not been removed
                {
                        ++it;
                }
        }
}

void PropHandler::on_std_turn()
{
        for (auto& prop: props_)
        {
                prop->on_std_turn();
        }
}

DidAction PropHandler::on_act()
{
        for (size_t i = 0; i < props_.size(); /* No increment */)
        {
                Prop* prop = props_[i].get();

                const auto result = prop->on_act();

                // NOTE: The property may have removed itself at this point, if
                // so it signals this by setting 'is_prop_ended' to true

                if (result.prop_ended == PropEnded::no)
                {
                        // Property has not been removed
                        ++i;
                }

                if (result.did_action == DidAction::yes)
                {
                        return DidAction::yes;
                }
        }

        return DidAction::no;
}

bool PropHandler::is_temporary_negative_prop(const Prop& prop)
{
        const auto id = prop.id_;

        const bool is_natural_prop = owner_->data().natural_props[(size_t)id];

        return
                !is_natural_prop &&
                (prop.duration_mode_ != PropDurationMode::indefinite) &&
                (prop.alignment() == PropAlignment::bad);
}

std::vector<PropTextListEntry> PropHandler::text_list_temporary_negative_props()
{
        ASSERT(owner_ != map::player);

        auto prop_list = owner_->properties().text_list();

        // Remove all non-negative properties (we should not show temporary
        // spell resistance for example), and all natural properties (properties
        // which all monsters of this type starts with)
        for (auto it = begin(prop_list); it != end(prop_list);)
        {
                auto* const prop = it->prop;

                if (is_temporary_negative_prop(*prop))
                {
                        ++it;
                }
                else // Not a temporary negative property
                {
                        it = prop_list.erase(it);
                }
        }

        return prop_list;
}

bool PropHandler::has_temporary_negative_prop_mon()
{
        ASSERT(owner_ != map::player);

        for (const auto& prop: props_)
        {
                if (is_temporary_negative_prop(*prop))
                {
                        return true;
                }
        }

        return false;
}

std::vector<ColoredString> PropHandler::text_line() const
{
        std::vector<ColoredString> line;

        for (auto& prop : props_)
        {
                std::string str = prop->name_short();

                if (str.empty())
                {
                        continue;
                }

                const int turns_left  = prop->nr_turns_left_;

                if (prop->duration_mode_ == PropDurationMode::indefinite)
                {
                        if (prop->src() == PropSrc::intr)
                        {
                                str = text_format::all_to_upper(str);
                        }
                }
                else // Not indefinite
                {
                        // Player can see number of turns left on own properties
                        // with Self-aware?
                        if (owner_->is_player() &&
                            player_bon::traits[(size_t)Trait::self_aware] &&
                            prop->allow_display_turns())
                        {
                                // NOTE: Since turns left are decremented before
                                // the actors turn, and checked after the turn -
                                // "turns_left" practically represents how many
                                // more times the actor will act with the
                                // property enabled, EXCLUDING the current
                                // (ongoing) turn.
                                //
                                // I.e. one "turns_left" means that the property
                                // will be enabled the whole next turn, while
                                // Zero "turns_left", means that it will only be
                                // active the current turn. However, from a
                                // players perspective, this is unintuitive;
                                // "one turn left" means the current turn, plus
                                // the next - but is likely interpreted as just
                                // the current turn. Therefore we add +1 to the
                                // displayed value, so that a displayed value of
                                // one means that the property will end after
                                // performing the next action.
                                const int turns_displayed = turns_left + 1;

                                str += ":" + std::to_string(turns_displayed);
                        }
                }

                const PropAlignment alignment = prop->alignment();

                Color color;

                if (alignment == PropAlignment::good)
                {
                        color = colors::msg_good();
                }
                else if (alignment == PropAlignment::bad)
                {
                        color = colors::msg_bad();
                }
                else
                {
                        color = colors::white();
                }

                line.push_back(ColoredString(str, color));
        }

        return line;
}

// TODO: Lots of copy paste from 'text_line' above, refactor
std::vector<PropTextListEntry> PropHandler::text_list() const
{
        std::vector<PropTextListEntry> list;

        for (const auto& prop : props_)
        {
                const std::string name_long = prop->name();

                std::string name_short = prop->name_short();

                if (name_long.empty() && name_short.empty())
                {
                        continue;
                }

                if (!name_short.empty())
                {
                        const int turns_left  = prop->nr_turns_left_;

                        const bool is_indefinite =
                                prop->duration_mode_ ==
                                PropDurationMode::indefinite;

                        const bool is_intr = prop->src() == PropSrc::intr;

                        if (is_indefinite && is_intr)
                        {
                                // Indefinite intrinsic properties are printed
                                // in upper case
                                name_short =
                                        text_format::all_to_upper(name_short);
                        }
                        else // Not indefinite
                        {
                                // Player can see number of turns left on own
                                // properties with Self-aware?
                                if (owner_->is_player() &&
                                    player_bon::has_trait(Trait::self_aware) &&
                                    prop->allow_display_turns())
                                {
                                        // See NOTE in 'props_line' above.
                                        const int turns_displayed =
                                                turns_left + 1;

                                        name_short +=
                                                ":" +
                                                std::to_string(turns_displayed);
                                }
                        }
                }

                std::string title;

                if (!name_long.empty())
                {
                        title = name_long;
                }

                if (!name_short.empty() &&
                    name_short != name_long)
                {
                        title += " (" + name_short + ")";
                }

                const PropAlignment alignment = prop->alignment();

                Color color;

                if (alignment == PropAlignment::good)
                {
                        color = colors::msg_good();
                }
                else if (alignment == PropAlignment::bad)
                {
                        color = colors::msg_bad();
                }
                else
                {
                        color = colors::white();
                }

                const std::string descr = prop->descr();

                const size_t new_size = list.size() + 1;

                list.resize(new_size);

                auto& entry = list[new_size - 1];

                entry.title.str = title;

                entry.title.color = color;

                entry.descr = descr;

                entry.prop = prop.get();
        }

        return list;
}

bool PropHandler::is_resisting_prop(const PropId id) const
{
        for (auto& prop : props_)
        {
                if (prop->is_resisting_other_prop(id))
                {
                        return true;
                }
        }

        return false;
}

bool PropHandler::is_resisting_dmg(const DmgType dmg_type,
                                   const Verbosity verbosity) const
{
        DmgResistData res_data;

        for (auto& prop : props_)
        {
                res_data = prop->is_resisting_dmg(dmg_type);

                if (res_data.is_resisted)
                {
                        break;
                }
        }

        if (res_data.is_resisted &&
            (verbosity == Verbosity::verbose))
        {
                if (owner_->is_player())
                {
                        msg_log::add(res_data.msg_resist_player);
                }
                else // Is monster
                {
                        const auto* const mon = static_cast<const Mon*>(owner_);

                        if (mon->player_aware_of_me_counter_ > 0)
                        {
                                const bool can_player_see_mon =
                                        map::player->can_see_actor(*owner_);

                                const std::string mon_name =
                                        can_player_see_mon ?
                                        text_format::first_to_upper(
                                                owner_->name_the()) :
                                        "It";

                                msg_log::add(mon_name +
                                             " " +
                                             res_data.msg_resist_mon);
                        }
                }
        }

        return res_data.is_resisted;
}

bool PropHandler::allow_see() const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_see())
                {
                        return false;
                }
        }

        return true;
}

int PropHandler::affect_max_hp(const int hp_max) const
{
        int new_hp_max = hp_max;

        for (auto& prop : props_)
        {
                new_hp_max = prop->affect_max_hp(new_hp_max);
        }

        return new_hp_max;
}

int PropHandler::affect_max_spi(const int spi_max) const
{
        int new_spi_max = spi_max;

        for (auto& prop : props_)
        {
                new_spi_max = prop->affect_max_spi(new_spi_max);
        }

        return new_spi_max;
}

int PropHandler::affect_shock(const int shock) const
{
        int new_shock = shock;

        for (auto& prop : props_)
        {
                new_shock = prop->affect_shock(new_shock);
        }

        return new_shock;
}

void PropHandler::affect_move_dir(const P& actor_pos, Dir& dir) const
{
        for (auto& prop : props_)
        {
                prop->affect_move_dir(actor_pos, dir);
        }
}

bool PropHandler::allow_attack(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_attack_melee(verbosity) &&
                    !prop->allow_attack_ranged(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_attack_melee(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_attack_melee(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_attack_ranged(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_attack_ranged(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_move() const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_move())
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_act() const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_act())
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_read_absolute(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_read_absolute(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_read_chance(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_read_chance(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_cast_intr_spell_absolute(
        const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_cast_intr_spell_absolute(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_cast_intr_spell_chance(
        const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_cast_intr_spell_chance(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_speak(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_speak(verbosity))
                {
                        return false;
                }
        }

        return true;
}

bool PropHandler::allow_eat(const Verbosity verbosity) const
{
        for (auto& prop : props_)
        {
                if (!prop->allow_eat(verbosity))
                {
                        return false;
                }
        }

        return true;
}

void PropHandler::on_hit()
{
        for (auto& prop : props_)
        {
                prop->on_hit();
        }
}

void PropHandler::on_death()
{
        TRACE_FUNC_BEGIN_VERBOSE;

        for (auto& prop : props_)
        {
                prop->on_death();
        }

        TRACE_FUNC_END_VERBOSE;
}

void PropHandler::on_destroyed()
{
        TRACE_FUNC_BEGIN_VERBOSE;

        for (auto& prop : props_)
        {
                prop->on_destroyed();
        }

        TRACE_FUNC_END_VERBOSE;
}

int PropHandler::ability_mod(const AbilityId ability) const
{
        int modifier = 0;

        for (auto& prop : props_)
        {
                modifier += prop->ability_mod(ability);
        }

        return modifier;
}

bool PropHandler::affect_actor_color(Color& color) const
{
        bool did_affect_color = false;

        for (auto& prop : props_)
        {
                if (prop->affect_actor_color(color))
                {
                        did_affect_color = true;

                        // It's probably more likely that a color change due to
                        // a bad property is critical information (e.g.
                        // burning), so then we stop searching and use this
                        // color. If it's a good or neutral property that
                        // affected the color, then we keep searching.
                        if (prop->alignment() == PropAlignment::bad)
                        {
                                break;
                        }
                }
        }

        return did_affect_color;
}
