#include "actor_mon.hpp"

#include <vector>
#include <cstring>
#include <algorithm>

#include "init.hpp"
#include "item.hpp"
#include "actor_player.hpp"
#include "game_time.hpp"
#include "attack.hpp"
#include "reload.hpp"
#include "inventory.hpp"
#include "feature_trap.hpp"
#include "feature_mob.hpp"
#include "property.hpp"
#include "property_data.hpp"
#include "property_handler.hpp"
#include "io.hpp"
#include "sound.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "map_parsing.hpp"
#include "ai.hpp"
#include "line_calc.hpp"
#include "gods.hpp"
#include "item_factory.hpp"
#include "actor_factory.hpp"
#include "knockback.hpp"
#include "popup.hpp"
#include "fov.hpp"
#include "text_format.hpp"
#include "feature_door.hpp"
#include "drop.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static void unblock_passable_doors(const ActorData& actor_data,
                                   bool blocked[map_w][map_h])
{
        for (int x = 0; x < map_w; ++x) for (int y = 0; y < map_h; ++y)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        const auto* const f = map::cells[x][y].rigid;

                        if (f->id() != FeatureId::door)
                        {
                                continue;
                        }

                        const auto* const door = static_cast<const Door*>(f);

                        if (door->type() == DoorType::metal)
                        {
                                continue;
                        }

                        if (actor_data.can_open_doors ||
                            actor_data.can_bash_doors)
                        {
                                blocked[x][y] = false;
                        }
                }
        }
}

// -----------------------------------------------------------------------------
// Monster
// -----------------------------------------------------------------------------
Mon::Mon() :
        Actor(),
        wary_of_player_counter_(0),
        aware_of_player_counter_(0),
        player_aware_of_me_counter_(0),
        is_msg_mon_in_view_printed_(false),
        is_player_feeling_msg_allowed_(true),
        last_dir_moved_(Dir::center),
        is_roaming_allowed_(MonRoamingAllowed::yes),
        leader_(nullptr),
        target_(nullptr),
        is_target_seen_(false),
        waiting_(false),
        spells_()
{

}

Mon::~Mon()
{
        for (auto& spell : spells_)
        {
                delete spell.spell;
        }
}

void Mon::on_actor_turn()
{
        if (aware_of_player_counter_ > 0)
        {
                --wary_of_player_counter_;

                --aware_of_player_counter_;
        }
}

void Mon::act()
{
        const bool is_player_leader = is_actor_my_leader(map::player);

#ifndef NDEBUG
        // Sanity check - verify that monster is not outside the map
        if (!map::is_pos_inside_map(pos, false))
        {
                TRACE << "Monster outside map" << std::endl;
                ASSERT(false);
        }

        // Sanity check - verify that monster's leader does not have a leader
        // (never allowed)
        if (leader_ &&
            !is_player_leader &&
            static_cast<Mon*>(leader_)->leader_)
        {
                TRACE << "Two (or more) steps of leader is never allowed"
                      << std::endl;
                ASSERT(false);
        }
#endif // NDEBUG

        if ((wary_of_player_counter_ <= 0) &&
            (aware_of_player_counter_ <= 0) &&
            !is_player_leader)
        {
                waiting_ = !waiting_;

                if (waiting_)
                {
                        game_time::tick();

                        return;
                }
        }
        else // Is wary/aware, or player is leader
        {
                waiting_ = false;
        }

        // Pick a target
        std::vector<Actor*> target_bucket;

        if (has_prop(PropId::conflict))
        {
                target_bucket = seen_actors();

                is_target_seen_ = !target_bucket.empty();
        }
        else // Not conflicted
        {
                target_bucket = seen_foes();

                if (target_bucket.empty())
                {
                        // There are no seen foes
                        is_target_seen_ = false;

                        target_bucket = unseen_foes_aware_of();
                }
                else // There are seen foes
                {
                        is_target_seen_ = true;
                }
        }

        // TODO: This just returns the actor with the closest COORDINATES,
        // not the actor with the shortest free path to it - so the monster
        // could select a target which is not actually the closest to reach.
        // This could perhaps lead to especially dumb situations if the monster
        // does not move by pathding, and considers an enemy behind a wall to be
        // closer than another enemy who is in the same room.
        target_ = map::random_closest_actor(pos, target_bucket);

        if ((wary_of_player_counter_ > 0) ||
            (aware_of_player_counter_ > 0))
        {
                is_roaming_allowed_ = MonRoamingAllowed::yes;

                // Does the monster have a living leader?
                if (leader_ && leader_->is_alive())
                {
                        if ((aware_of_player_counter_ > 0) &&
                            !is_player_leader)
                        {
                                make_leader_aware_silent();
                        }
                }
                else // Monster does not have a living leader
                {
                        // Monster is wary or aware, occasionally make a sound
                        if (is_alive() && rnd::one_in(12))
                        {
                                speak_phrase(AlertsMon::no);
                        }
                }
        }

        // ---------------------------------------------------------------------
        // Special monster actions (e.g. zombies rising)
        // ---------------------------------------------------------------------
        // TODO: This will be removed eventually
        if ((leader_ != map::player) &&
            (!target_ || target_->is_player()))
        {
                if (on_act() == DidAction::yes)
                {
                        return;
                }
        }

        // ---------------------------------------------------------------------
        // Property actions (e.g. Zombie rising, Vortex pulling, ...)
        // ---------------------------------------------------------------------
        if (properties().on_act() == DidAction::yes)
        {
                return;
        }

        // ---------------------------------------------------------------------
        // Common actions (moving, attacking, casting spells, etc)
        // ---------------------------------------------------------------------

        // NOTE: Monsters try to detect the player visually on standard turns,
        // otherwise very fast monsters are much better at finding the player

        if (data_->ai[(size_t)AiId::avoids_blocking_friend] &&
            !is_player_leader &&
            (target_ == map::player) &&
            is_target_seen_ &&
            rnd::coin_toss())
        {
                if (ai::action::make_room_for_friend(*this))
                {
                        return;
                }
        }

        // Cast instead of attacking?
        if (rnd::one_in(5))
        {
                const bool did_cast = ai::action::try_cast_random_spell(*this);

                if (did_cast)
                {
                        return;
                }
        }

        if (data_->ai[(size_t)AiId::attacks] &&
            target_ &&
            is_target_seen_)
        {
                const auto did_attack = try_attack(*target_);

                if (did_attack == DidAction::yes)
                {
                        return;
                }
        }

        if (rnd::coin_toss())
        {
                const bool did_cast = ai::action::try_cast_random_spell(*this);

                if (did_cast)
                {
                        return;
                }
        }

        int erratic_move_pct = (int)data_->erratic_move_pct;

        // Never move erratically if frenzied
        if (has_prop(PropId::frenzied))
        {
                erratic_move_pct = 0;
        }

        // Move less erratically if allied to player
        if (is_player_leader)
        {
                erratic_move_pct /= 2;
        }

        // Move more erratically if confused
        if (has_prop(PropId::confused) &&
            (erratic_move_pct > 0))
        {
                erratic_move_pct += 50;
        }

        set_constr_in_range(0, erratic_move_pct, 95);

        // Occasionally move erratically
        if (data_->ai[(size_t)AiId::moves_randomly_when_unaware] &&
            rnd::percent(erratic_move_pct))
        {
                if (ai::action::move_to_random_adj_cell(*this))
                {
                        return;
                }
        }

        const bool is_terrified = has_prop(PropId::terrified);

        if (data_->ai[(size_t)AiId::moves_to_target_when_los] &&
            !is_terrified)
        {
                if (ai::action::move_to_target_simple(*this))
                {
                        return;
                }
        }

        std::vector<P> path;

        if ((data_->ai[(size_t)AiId::paths_to_target_when_aware] ||
             is_player_leader) &&
            !is_terrified)
        {
                path = ai::info::find_path_to_target(*this);
        }

        if (ai::action::handle_closed_blocking_door(*this, path))
        {
                return;
        }

        if (ai::action::step_path(*this, path))
        {
                return;
        }

        if ((data_->ai[(size_t)AiId::moves_to_leader] ||
             is_player_leader) &&
            !is_terrified)
        {
                path = ai::info::find_path_to_leader(*this);

                if (ai::action::step_path(*this, path))
                {
                        return;
                }
        }

        if (data_->ai[(size_t)AiId::moves_to_lair]  &&
            !is_player_leader &&
            (!target_ || target_->is_player()))
        {
                if (ai::action::step_to_lair_if_los(*this, lair_pos_))
                {
                        return;
                }
                else // No LOS to lair
                {
                        // Try to use pathfinder to travel to lair
                        path = ai::info::find_path_to_lair_if_no_los(
                                *this,
                                lair_pos_);

                        if (ai::action::step_path(*this, path))
                        {
                                return;
                        }
                }
        }

        // When unaware, move randomly
        if (data_->ai[(size_t)AiId::moves_randomly_when_unaware] &&
            (!is_player_leader || rnd::one_in(8)))
        {
                if (ai::action::move_to_random_adj_cell(*this))
                {
                        return;
                }
        }

        // No action could be performed, just let someone else act
        game_time::tick();
}

std::vector<Actor*> Mon::unseen_foes_aware_of() const
{
        std::vector<Actor*> result;

        if (is_actor_my_leader(map::player))
        {
                // TODO: This prevents player-allied monsters from casting
                // spells on unreachable hostile monsters - but it probably
                // doesn't matter for now

                bool blocked[map_w][map_h];

                map_parsers::BlocksActor(*this, ParseActors::no)
                        .run(blocked);

                unblock_passable_doors(data(), blocked);

                int flood[map_w][map_h];

                floodfill(pos,
                          blocked,
                          flood);

                // Add all player-hostile monsters which the player is aware of
                for (Actor* const actor : game_time::actors)
                {
                        const P& p = actor->pos;

                        if (!actor->is_player() &&
                            !actor->is_actor_my_leader(map::player) &&
                            (flood[p.x][p.y] > 0))
                        {
                                auto* const mon = static_cast<Mon*>(actor);

                                if (mon->player_aware_of_me_counter_ > 0)
                                {
                                        result.push_back(actor);
                                }
                        }
                }
        }
        // Player is not my leader
        else if (aware_of_player_counter_ > 0)
        {
                result.push_back(map::player);

                for (Actor* const actor : game_time::actors)
                {
                        if (!actor->is_player() &&
                            actor->is_actor_my_leader(map::player))
                        {
                                result.push_back(actor);
                        }
                }
        }

        return result;
}

bool Mon::can_see_actor(const Actor& other,
                        const bool hard_blocked_los[map_w][map_h]) const
{
        const bool is_seeable = is_actor_seeable(other, hard_blocked_los);

        if (!is_seeable)
        {
                return false;
        }

        if (is_actor_my_leader(map::player))
        {
                // Monster is allied to player

                // Player-allied monster looking at the player?
                if (other.is_player())
                {
                        return true;
                }
                else // Player-allied monster looking at other monster
                {
                        const auto* const other_mon =
                                static_cast<const Mon*>(&other);

                        return other_mon->player_aware_of_me_counter_ > 0;
                }
        }
        else // Monster is hostile to player
        {
                return aware_of_player_counter_ > 0;
        }

        return true;
}

bool Mon::is_actor_seeable(const Actor& other,
                           const bool hard_blocked_los[map_w][map_h]) const
{
        if ((this == &other) ||
            (!other.is_alive()))
        {
                return true;
        }

        // Outside FOV range?
        if (!fov::is_in_fov_range(pos, other.pos))
        {
                // Other actor is outside FOV range
                return false;
        }

        // Monster is blind?
        if (!properties_->allow_see())
        {
                return false;
        }

        const LosResult los =
                fov::check_cell(pos,
                                other.pos,
                                hard_blocked_los);

        // LOS blocked hard (e.g. a wall or smoke)?
        if (los.is_blocked_hard)
        {
                return false;
        }

        const bool can_see_invis = has_prop(PropId::see_invis);

        // Actor is invisible, and monster cannot see invisible?
        if ((other.has_prop(PropId::invis) ||
             other.has_prop(PropId::cloaked)) &&
            !can_see_invis)
        {
                return false;
        }

        bool has_darkvision = has_prop(PropId::darkvision);

        const bool can_see_other_in_drk =
                can_see_invis ||
                has_darkvision;

        // Blocked by darkness, and not seeing actor with infravision?
        if (los.is_blocked_by_drk &&
            !can_see_other_in_drk)
        {
                return false;
        }

        // OK, all checks passed, actor can bee seen
        return true;
}

std::vector<Actor*> Mon::seen_actors() const
{
        std::vector<Actor*> out;

        bool blocked_los[map_w][map_h];

        R los_rect(std::max(0, pos.x - fov_std_radi_int),
                   std::max(0, pos.y - fov_std_radi_int),
                   std::min(map_w - 1, pos.x + fov_std_radi_int),
                   std::min(map_h - 1, pos.y + fov_std_radi_int));

        map_parsers::BlocksLos()
                .run(blocked_los,
                     MapParseMode::overwrite,
                     los_rect);

        for (Actor* actor : game_time::actors)
        {
                if ((actor != this) &&
                    actor->is_alive())
                {
                        const Mon* const mon = static_cast<const Mon*>(this);

                        if (mon->can_see_actor(*actor, blocked_los))
                        {
                                out.push_back(actor);
                        }
                }
        }

        return out;
}

std::vector<Actor*> Mon::seen_foes() const
{
        std::vector<Actor*> out;

        bool blocked_los[map_w][map_h];

        R los_rect(std::max(0, pos.x - fov_std_radi_int),
                   std::max(0, pos.y - fov_std_radi_int),
                   std::min(map_w - 1, pos.x + fov_std_radi_int),
                   std::min(map_h - 1, pos.y + fov_std_radi_int));

        map_parsers::BlocksLos()
                .run(blocked_los,
                     MapParseMode::overwrite,
                     los_rect);

        for (Actor* actor : game_time::actors)
        {
                if ((actor != this) &&
                    actor->is_alive())
                {
                        const bool is_hostile_to_player =
                                !is_actor_my_leader(map::player);

                        const bool is_other_hostile_to_player =
                                actor->is_player() ?
                                false :
                                !actor->is_actor_my_leader(map::player);

                        const bool is_enemy =
                                is_hostile_to_player !=
                                is_other_hostile_to_player;

                        const Mon* const mon = static_cast<const Mon*>(this);

                        if (is_enemy &&
                            mon->can_see_actor(*actor, blocked_los))
                        {
                                out.push_back(actor);
                        }
                }
        }

        return out;
}

std::vector<Actor*> Mon::seeable_foes() const
{
        std::vector<Actor*> out;

        bool blocked_los[map_w][map_h];

        const R fov_rect = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
                .run(blocked_los,
                     MapParseMode::overwrite,
                     fov_rect);

        for (Actor* actor : game_time::actors)
        {
                if ((actor != this) &&
                    actor->is_alive())
                {
                        const bool is_hostile_to_player =
                                !is_actor_my_leader(map::player);

                        const bool is_other_hostile_to_player =
                                actor->is_player() ?
                                false :
                                !actor->is_actor_my_leader(map::player);

                        const bool is_enemy =
                                is_hostile_to_player !=
                                is_other_hostile_to_player;

                        if (is_enemy &&
                            is_actor_seeable(*actor, blocked_los))
                        {
                                out.push_back(actor);
                        }
                }
        }

        return out;
}

bool Mon::is_sneaking() const
{
        // NOTE: We require a stealth ability greater than zero, both for the
        // basic skill value, AND when including properties properties -
        // This prevents monsters who should never be able to sneak from
        // suddenly gaining this ability due to some bonus

        const int stealth_base = ability(AbilityId::stealth, false);

        const int stealth_current = ability(AbilityId::stealth, true);

        return
                (player_aware_of_me_counter_ <= 0) &&
                (stealth_base > 0) &&
                (stealth_current > 0) &&
                !is_actor_my_leader(map::player);
}

void Mon::make_leader_aware_silent() const
{
        ASSERT(leader_);

        if (!leader_)
        {
                return;
        }

        Mon* const leader_mon = static_cast<Mon*>(leader_);

        leader_mon->aware_of_player_counter_ =
                std::max(leader_mon->data().nr_turns_aware,
                         leader_mon->aware_of_player_counter_);
}

void Mon::on_std_turn()
{
        // Countdown all spell cooldowns
        for (auto& spell : spells_)
        {
                int& cooldown = spell.cooldown;

                if (cooldown > 0)
                {
                        --cooldown;
                }
        }

        // Monsters try to detect the player visually on standard turns,
        // otherwise very fast monsters are much better at finding the player
        if (is_alive() &&
            data_->ai[(size_t)AiId::looks] &&
            (leader_ != map::player) &&
            (!target_ || target_->is_player()))
        {
                const bool did_become_aware_now = ai::info::look(*this);

                // If the monster became aware, give it some reaction time
                if (did_become_aware_now)
                {
                        auto prop = new PropWaiting();

                        prop->set_duration(1);

                        apply_prop(prop);
                }
        }

        on_std_turn_hook();
}

void Mon::on_hit(int& dmg,
                 const DmgType dmg_type,
                 const DmgMethod method,
                 const AllowWound allow_wound)
{
        (void)dmg;
        (void)dmg_type;
        (void)method;
        (void)allow_wound;

        aware_of_player_counter_ =
                std::max(data_->nr_turns_aware,
                         aware_of_player_counter_);
}

void Mon::move(Dir dir)
{
#ifndef NDEBUG
        if (dir == Dir::END)
        {
                TRACE << "Illegal direction parameter" << std::endl;
                ASSERT(false);
        }

        if (!map::is_pos_inside_map(pos, false))
        {
                TRACE << "Monster outside map" << std::endl;
                ASSERT(false);
        }
#endif // NDEBUG

        properties().affect_move_dir(pos, dir);

        // Trap affects leaving?
        if (dir != Dir::center)
        {
                auto* f = map::cells[pos.x][pos.y].rigid;

                if (f->id() == FeatureId::trap)
                {
                        dir = static_cast<Trap*>(f)
                                ->actor_try_leave(*this, dir);
                }

                if (dir == Dir::center)
                {
                        TRACE_VERBOSE << "Monster move prevented by trap"
                                      << std::endl;

                        game_time::tick();
                        return;
                }
        }

        // Movement direction is stored for AI purposes
        last_dir_moved_ = dir;

        const P target_p(pos + dir_utils::offset(dir));

        if ((dir != Dir::center) &&
            map::is_pos_inside_map(target_p, false))
        {
                pos = target_p;

                // Bump features in target cell (i.e. to trigger traps)
                std::vector<Mob*> mobs;
                game_time::mobs_at_pos(pos, mobs);

                for (auto* m : mobs)
                {
                        m->bump(*this);
                }

                map::cells[pos.x][pos.y].rigid->bump(*this);
        }

        game_time::tick();
}

Color Mon::color() const
{
        if (state_ != ActorState::alive)
        {
                return data_->color;
        }

        Color tmp_color;

        if (properties_->affect_actor_color(tmp_color))
        {
                return tmp_color;
        }

        return data_->color;
}

SpellSkill Mon::spell_skill(const SpellId id) const
{
        (void)id;

        for (const auto& spell : spells_)
        {
                if (spell.spell->id() == id)
                {
                        return spell.skill;
                }
        }

        ASSERT(false);

        return SpellSkill::basic;
}

void Mon::hear_sound(const Snd& snd)
{
        if (has_prop(PropId::deaf))
        {
                return;
        }

        snd.on_heard(*this);

        // The monster may have become deaf through the sound callback (e.g.
        // from the Horn of Deafening artifact)
        if (has_prop(PropId::deaf))
        {
                return;
        }

        if (is_alive() &&
            snd.is_alerting_mon())
        {
                const bool was_aware_before = (aware_of_player_counter_ > 0);

                become_aware_player(false);

                // Give the monster some reaction time
                if (!was_aware_before &&
                    !is_actor_my_leader(map::player))
                {
                        auto prop = new PropWaiting();

                        prop->set_duration(1);

                        apply_prop(prop);
                }
        }
}

void Mon::speak_phrase(const AlertsMon alerts_others)
{
        const bool is_seen_by_player = map::player->can_see_actor(*this);

        std::string msg =
                is_seen_by_player ?
                aware_msg_mon_seen() :
                aware_msg_mon_hidden();

        msg = text_format::first_to_upper(msg);

        const SfxId sfx =
                is_seen_by_player ?
                aware_sfx_mon_seen() :
                aware_sfx_mon_hidden();

        Snd snd(msg,
                sfx,
                IgnoreMsgIfOriginSeen::no,
                pos,
                this,
                SndVol::low,
                alerts_others);

        snd_emit::run(snd);
}

void Mon::become_aware_player(const bool is_from_seeing,
                              const int factor)
{
        if (!is_alive() || is_actor_my_leader(map::player))
        {
                return;
        }

        const int nr_turns = data_->nr_turns_aware * factor;

        const int aware_counter_before = aware_of_player_counter_;

        aware_of_player_counter_=
                std::max(nr_turns,
                         aware_counter_before);

        wary_of_player_counter_ = aware_of_player_counter_;

        if (aware_counter_before <= 0)
        {
                if (is_from_seeing &&
                    map::player->can_see_actor(*this))
                {
                        print_player_see_mon_become_aware_msg();
                }

                if (rnd::coin_toss())
                {
                        speak_phrase(AlertsMon::yes);
                }
        }
}

void Mon::become_wary_player()
{
        if (!is_alive() || is_actor_my_leader(map::player))
        {
                return;
        }

        // NOTE: Reusing aware duration to determine number of wary turns
        const int nr_turns = data_->nr_turns_aware;

        const int wary_counter_before = wary_of_player_counter_;

        wary_of_player_counter_=
                std::max(nr_turns, wary_counter_before);

        if (wary_counter_before <= 0)
        {
                if (map::player->can_see_actor(*this))
                {
                        print_player_see_mon_become_wary_msg();
                }

                if (rnd::one_in(4))
                {
                        speak_phrase(AlertsMon::no);
                }
        }
}

void Mon::print_player_see_mon_become_aware_msg() const
{
        std::string msg = text_format::first_to_upper(name_the()) + " sees me!";

        const std::string dir_str =
                dir_utils::compass_dir_name(map::player->pos, pos);

        msg += "(" + dir_str + ")";

        msg_log::add(msg);
}

void Mon::print_player_see_mon_become_wary_msg() const
{
        std::string msg = text_format::first_to_upper(data_->wary_msg);

        if (msg.empty())
        {
                return;
        }

        const std::string dir_str =
                dir_utils::compass_dir_name(map::player->pos, pos);

        msg += "(" + dir_str + ")";

        msg_log::add(msg);
}

void Mon::set_player_aware_of_me(int duration_factor)
{
        int nr_turns = 2 * duration_factor;

        if (player_bon::bg() == Bg::rogue)
        {
                nr_turns *= 8;
        }

        player_aware_of_me_counter_ =
                std::max(player_aware_of_me_counter_, nr_turns);
}

DidAction Mon::try_attack(Actor& defender)
{
        if (state_ != ActorState::alive ||
            ((aware_of_player_counter_ <= 0) &&
             (leader_ != map::player)))
        {
                return DidAction::no;
        }

        map::update_vision();

        const AiAvailAttacksData my_avail_attacks = avail_attacks(defender);

        const AiAttData att = choose_attack(my_avail_attacks);

        if (!att.wpn)
        {
                return DidAction::no;
        }

        if (att.is_melee)
        {
                if (att.wpn->data().melee.is_melee_wpn)
                {
                        attack::melee(this, pos, defender, *att.wpn);

                        return DidAction::yes;
                }

                return DidAction::no;
        }

        if (att.wpn->data().ranged.is_ranged_wpn)
        {
                if (my_avail_attacks.should_reload)
                {
                        reload::try_reload(*this, att.wpn);

                        return DidAction::yes;
                }

                const bool is_ignoring_blocking_friend =
                        rnd::fraction(1, 5);

                if (!is_ignoring_blocking_friend &&
                    is_friend_blocking_ranged_attack(defender.pos))
                {
                        return DidAction::no;
                }

                auto prop = new PropDisabledRanged();

                prop->set_duration(data_->ranged_cooldown_turns);

                properties_->apply(prop);

                const auto did_attack =
                        attack::ranged(
                                this,
                                pos,
                                defender.pos,
                                *att.wpn);

                return did_attack;
        }

        return DidAction::no;
}

bool Mon::is_friend_blocking_ranged_attack(const P& target_pos) const
{
        const auto line =
                line_calc::calc_new_line(
                        pos,
                        target_pos,
                        true,
                        9999,
                        false);

        for (const P& line_pos : line)
        {
                if ((line_pos != pos) &&
                    (line_pos != target_pos))
                {
                        Actor* const actor_here = map::actor_at_pos(line_pos);

                        // TODO: This does not consider who is allied/hostile!
                        if (actor_here)
                        {
                                return true;
                        }
                }
        }

        return false;
}

AiAvailAttacksData Mon::avail_attacks(Actor& defender) const
{
        AiAvailAttacksData result;

        if (!properties_->allow_attack(Verbosity::silent))
        {
                return result;
        }

        result.is_melee = is_pos_adj(pos, defender.pos, false);

        if (result.is_melee)
        {
                if (!properties_->allow_attack_melee(Verbosity::silent))
                {
                        return result;
                }

                result.weapons = avail_intr_melee();

                auto* wielded_wpn = avail_wielded_melee();

                if (wielded_wpn)
                {
                        result.weapons.push_back(wielded_wpn);
                }
        }
        else // Ranged attack
        {
                if (!properties_->allow_attack_ranged(Verbosity::silent))
                {
                        return result;
                }

                result.weapons = avail_intr_ranged();

                auto* const wielded_wpn = avail_wielded_ranged();

                if (wielded_wpn)
                {
                        result.weapons.push_back(wielded_wpn);

                        result.should_reload = should_reload(*wielded_wpn);
                }
        }

        return result;
}

Wpn* Mon::avail_wielded_melee() const
{
        Item* const item = inv_->item_in_slot(SlotId::wpn);

        if (item)
        {
                auto* const wpn = static_cast<Wpn*>(item);

                if (wpn->data().melee.is_melee_wpn)
                {
                        return wpn;
                }
        }

        return nullptr;
}

Wpn* Mon::avail_wielded_ranged() const
{
        Item* const item = inv_->item_in_slot(SlotId::wpn);

        if (item)
        {
                auto* const wpn = static_cast<Wpn*>(item);

                if (wpn->data().ranged.is_ranged_wpn)
                {
                        return wpn;
                }
        }

        return nullptr;
}

std::vector<Wpn*> Mon::avail_intr_melee() const
{
        std::vector<Wpn*> result;

        for (Item* const item : inv_->intrinsics_)
        {
                auto* wpn = static_cast<Wpn*>(item);

                if (wpn->data().melee.is_melee_wpn)
                {
                        result.push_back(wpn);
                }
        }

        return result;
}

std::vector<Wpn*> Mon::avail_intr_ranged() const
{
        std::vector<Wpn*> result;

        for (Item* const item : inv_->intrinsics_)
        {
                auto* const wpn = static_cast<Wpn*>(item);

                if (wpn->data().ranged.is_ranged_wpn)
                {
                        result.push_back(wpn);
                }
        }

        return result;
}

bool Mon::should_reload(const Wpn& wpn) const
{
        // TODO: This could be made more sophisticated, e.g. if the monster does
        // not see any enemies it should reload even if the weapon is not
        // completely empty
        return
                (wpn.ammo_loaded_ == 0) &&
                !wpn.data().ranged.has_infinite_ammo &&
                inv_->has_ammo_for_firearm_in_inventory();
}

AiAttData Mon::choose_attack(const AiAvailAttacksData& avail_attacks) const
{
        AiAttData result;

        result.is_melee = avail_attacks.is_melee;

        if (avail_attacks.weapons.empty())
        {
                return result;
        }

        result.wpn = rnd::element(avail_attacks.weapons);

        return result;
}

bool Mon::is_leader_of(const Actor* const actor) const
{
        if (actor && !actor->is_player())
        {
                const auto* const mon = static_cast<const Mon*>(actor);

                return (mon->leader_ == this);
        }

        return false;
}

bool Mon::is_actor_my_leader(const Actor* const actor) const
{
        return leader_ == actor;
}

int Mon::nr_mon_in_group()
{
        const Actor* const group_leader = leader_ ? leader_ : this;

        int ret = 1; // Starting at one to include leader

        for (const Actor* const actor : game_time::actors)
        {
                if (actor->is_actor_my_leader(group_leader))
                {
                        ++ret;
                }
        }

        return ret;
}

void Mon::add_spell(SpellSkill skill, Spell* const spell)
{
        const auto search = std::find_if(
                begin(spells_),
                end(spells_),
                [spell](MonSpell& spell_entry)
                {
                        return spell_entry.spell->id() == spell->id();
                });

        if (search != end(spells_))
        {
                delete spell;

                return;
        }

        MonSpell spell_entry;

        spell_entry.spell = spell;

        spell_entry.skill = skill;

        spells_.push_back(spell_entry);
}

void Mon::add_spell(SpellSkill skill, SpellId id)
{
        auto* const spell = spell_handling::make_spell_from_id(id);

        add_spell(skill, spell);
}

void  Mon::add_random_spell(SpellSkill skill)
{
        auto* spell = spell_handling::random_spell_for_mon();

        add_spell(skill, spell);
}

// -----------------------------------------------------------------------------
// Specific monsters
// -----------------------------------------------------------------------------
std::string Cultist::cultist_phrase()
{
        std::vector<std::string> phrase_bucket;

        const God* const god = gods::current_god();

        if (god && rnd::coin_toss())
        {
                const std::string name = god->name();
                const std::string descr = god->descr();

                phrase_bucket =
                {
                        name + " save us!",
                        descr + " will save us!",
                        name + " watches over us!",
                        descr + " watches over us!",
                        name + ", guide us!",
                        descr + " guides us!",
                        "For " + name + "!",
                        "For " + descr + "!",
                        "Blood for " + name + "!",
                        "Blood for " + descr + "!",
                        "Perish for " + name + "!",
                        "Perish for " + descr + "!",
                        "In the name of " + name + "!"
                };
        }
        else
        {
                phrase_bucket =
                {
                        "Apigami!",
                        "Bhuudesco invisuu!",
                        "Bhuuesco marana!",
                        "Crudux cruo!",
                        "Cruento paashaeximus!",
                        "Cruento pestis shatruex!",
                        "Cruo crunatus durbe!",
                        "Cruo lokemundux!",
                        "Cruo stragara-na!",
                        "Gero shay cruo!",
                        "In marana domus-bhaava crunatus!",
                        "Caecux infirmux!",
                        "Malax sayti!",
                        "Marana pallex!",
                        "Marana malax!",
                        "Pallex ti!",
                        "Peroshay bibox malax!",
                        "Pestis Cruento!",
                        "Pestis cruento vilomaxus pretiacruento!",
                        "Pretaanluxis cruonit!",
                        "Pretiacruento!",
                        "Stragar-Naya!",
                        "Vorox esco marana!",
                        "Vilomaxus!",
                        "Prostragaranar malachtose!",
                        "Apigami!"
                };
        }

        return phrase_bucket[rnd::range(0, phrase_bucket.size() - 1)];
}

// void Cultist::make_start_items()
// {
//         if (rnd::one_in(12))
//         {
//                 add_random_spell(SpellSkill::basic);

//                 if (rnd::coin_toss())
//                 {
//                         add_random_spell(SpellSkill::basic);
//                 }
//         }
// }

// void CultistPriest::make_start_items()
// {
//         add_spell(SpellSkill::basic, SpellId::heal);
//         add_spell(SpellSkill::basic, SpellId::darkbolt);
//         add_spell(SpellSkill::basic, SpellId::enfeeble);
//         add_spell(SpellSkill::basic, SpellId::knockback);
//         add_spell(SpellSkill::basic, SpellId::spell_shield);

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::basic, SpellId::teleport);
//         }

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::basic, SpellId::summon);
//         }

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::basic, SpellId::pest);
//         }
// }

// void CultistWizard::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::heal);
//         add_spell(SpellSkill::expert, SpellId::darkbolt);
//         add_spell(SpellSkill::expert, SpellId::knockback);
//         add_spell(SpellSkill::expert, SpellId::spell_shield);

//         add_spell(SpellSkill::basic, SpellId::enfeeble);

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::expert, SpellId::teleport);
//         }

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::expert, SpellId::summon);
//         }

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::expert, SpellId::pest);
//         }
// }

// void CultistArchWizard::make_start_items()
// {
//         add_spell(SpellSkill::master, SpellId::heal);
//         add_spell(SpellSkill::master, SpellId::darkbolt);
//         add_spell(SpellSkill::master, SpellId::knockback);
//         add_spell(SpellSkill::master, SpellId::burn);
//         add_spell(SpellSkill::master, SpellId::spell_shield);

//         add_spell(SpellSkill::expert, SpellId::enfeeble);

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::master, SpellId::teleport);
//         }

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::master, SpellId::summon);
//         }

//         if (rnd::coin_toss())
//         {
//                 add_spell(SpellSkill::master, SpellId::pest);
//         }
// }

void Zuul::init_hook()
{
        if (actor_data::data[(size_t)ActorId::zuul].nr_left_allowed_to_spawn > 0)
        {
                // NOTE: Do not call 'die()' here (this would have side effects). Just
                // set the state to destroyed.
                state_ = ActorState::destroyed;

                Actor* actor = actor_factory::make(ActorId::cultist_priest, pos);

                auto* prop = new PropPossByZuul();

                prop->set_indefinite();

                actor->properties().apply(prop,
                                          PropSrc::intr,
                                          true,
                                          Verbosity::silent);

                actor->restore_hp(999, false, Verbosity::silent);
        }
}

// TODO: Make this an intrinsic attack instead
DidAction Ghost::on_act()
{
        if (is_alive() &&
            (aware_of_player_counter_ > 0) &&
            is_pos_adj(pos, map::player->pos, false) &&
            rnd::one_in(4))
        {
                set_player_aware_of_me();

                const bool player_sees_me =
                        map::player->can_see_actor(*this);

                const std::string name =
                        player_sees_me ?
                        text_format::first_to_upper(name_the()) :
                        "It";

                msg_log::add(name + " reaches for me...");

                map::player->apply_prop(new PropWeakened());

                game_time::tick();

                return DidAction::yes;
        }

        return DidAction::no;
}

// void Wraith::make_start_items()
// {
//         add_random_spell(SpellSkill::basic);
//         add_random_spell(SpellSkill::basic);
// }

// void MiGo::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::mi_go_hypno);
// }

// void SentryDrone::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::heal);
//         add_spell(SpellSkill::expert, SpellId::darkbolt);
//         add_spell(SpellSkill::expert, SpellId::burn);
// }

// void BrownJenkin::make_start_items()
// {
//         add_spell(SpellSkill::basic, SpellId::teleport);
// }

void Ghoul::init_hook()
{
        // If player is Ghoul, then Ghouls are allied to player
        if (player_bon::bg() == Bg::ghoul)
        {
                leader_ = map::player;
        }
}

// void ElderVoidTraveler::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::heal);
//         add_spell(SpellSkill::expert, SpellId::burn);
//         add_spell(SpellSkill::expert, SpellId::deafen);
//         add_spell(SpellSkill::expert, SpellId::spell_shield);
// }

// void Mummy::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::disease);
// }

// void MummyUnique::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::darkbolt);
//         add_spell(SpellSkill::expert, SpellId::disease);
//         add_spell(SpellSkill::expert, SpellId::spell_shield);
//         add_spell(SpellSkill::expert, SpellId::burn);

//         add_spell(SpellSkill::basic, SpellId::enfeeble);
// }

// TODO: This should be controlled by the map
DidAction Khephren::on_act()
{
        // Summon locusts
        if (!is_alive() ||
            (aware_of_player_counter_ <= 0) ||
            has_summoned_locusts)
        {
                return DidAction::no;
        }

        const R fov_rect = fov::get_fov_rect(pos);

        bool blocked[map_w][map_h];

        map_parsers::BlocksLos()
                .run(blocked,
                     MapParseMode::overwrite,
                     fov_rect);

        if (!can_see_actor(*(map::player), blocked))
        {
                return DidAction::no;
        }

        msg_log::add("Khephren calls a plague of Locusts!");

        map::player->incr_shock(ShockLvl::terrifying,
                                ShockSrc::misc);

        Actor* const leader_of_spawned_mon = leader_ ? leader_ : this;

        const size_t nr_of_spawns = 15;

        const auto summoned =
                actor_factory::spawn(pos, {nr_of_spawns, ActorId::locust})
                .set_leader(leader_of_spawned_mon)
                .make_aware_of_player()
                .for_each([](Mon* const mon)
                {
                        auto prop = new PropSummoned();

                        prop->set_indefinite();

                        mon->apply_prop(prop);
                });

        has_summoned_locusts = true;

        game_time::tick();

        return DidAction::yes;
}

// TODO: Make this into a spell instead
DidAction Ape::on_act()
{
        if (frenzy_cooldown_ > 0)
        {
                --frenzy_cooldown_;
        }

        if ((frenzy_cooldown_ <= 0) &&
            target_ &&
            (hp() <= (hp_max(true) / 2)))
        {
                frenzy_cooldown_ = 30;

                auto prop = new PropFrenzied();

                prop->set_duration(rnd::range(4, 6));

                properties_->apply(prop);
        }

        return DidAction::no;
}

// void DeathFiend::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::enfeeble);
//         add_spell(SpellSkill::expert, SpellId::spell_shield);
// }

// TODO: Specify "buddy" monsters to spawn as allies in monsters.xml
DidAction KeziahMason::on_act()
{
        // Summon Brown Jenkin ASAP
        if (!is_alive() ||
            has_summoned_jenkin)
        {
                return DidAction::no;
        }

        auto summoned =
                actor_factory::spawn(pos, {ActorId::brown_jenkin})
                .set_leader(this)
                .for_each([](Mon* mon)
                {
                        mon->is_player_feeling_msg_allowed_ = false;
                });

        has_summoned_jenkin = true;

        return DidAction::no;
}

// void KeziahMason::make_start_items()
// {
//         add_spell(SpellSkill::expert, SpellId::teleport);
//         add_spell(SpellSkill::expert, SpellId::heal);
//         add_spell(SpellSkill::expert, SpellId::summon);
//         add_spell(SpellSkill::expert, SpellId::pest);
//         add_spell(SpellSkill::expert, SpellId::darkbolt);
//         add_spell(SpellSkill::expert, SpellId::deafen);
//         add_spell(SpellSkill::expert, SpellId::spell_shield);

//         add_spell(SpellSkill::basic, SpellId::enfeeble);
// }

// TODO: This should be controlled by the map
void LengElder::on_std_turn_hook()
{
        if (is_alive())
        {
                aware_of_player_counter_ += 100;

                if (has_given_item_to_player_)
                {
                        bool blocked_los[map_w][map_h];

                        const R fov_rect = fov::get_fov_rect(pos);

                        map_parsers::BlocksLos()
                                .run(blocked_los,
                                     MapParseMode::overwrite,
                                     fov_rect);

                        if (can_see_actor(*map::player, blocked_los))
                        {
                                if (nr_turns_to_hostile_ <= 0)
                                {
                                        msg_log::add("I am ripped to pieces!!!", colors::msg_bad());
                                        map::player->hit(999, DmgType::pure);
                                }
                                else
                                {
                                        --nr_turns_to_hostile_;
                                }
                        }
                }
                else // Has not given item to player
                {
                        const bool is_player_see_me = map::player->can_see_actor(*this);

                        const bool is_player_adj = is_pos_adj(pos, map::player->pos, false);

                        if (is_player_see_me && is_player_adj)
                        {
                                msg_log::add(
                                        "I perceive a cloaked figure standing before me...",
                                        colors::white(),
                                        false,
                                        MorePromptOnMsg::yes);

                                msg_log::add(
                                        "It is the Elder Hierophant of the Leng monastery, ");

                                msg_log::add(
                                        "the High Priest Not to Be Described.",
                                        colors::white(),
                                        false,
                                        MorePromptOnMsg::yes);

                                // auto& inv = map::player->inv();
                                // TODO: Which item to give?
                                // inv.put_in_backpack(item_factory::make(ItemId::hideous_mask));

                                has_given_item_to_player_ = true;
                                nr_turns_to_hostile_ = rnd::range(9, 11);
                        }
                }
        }
}

// TODO: This should be a property (should probably be merged with
// 'corrupts_env_color')
Color StrangeColor::color() const
{
        Color color = colors::light_magenta();

        color.set_r(rnd::range(40, 255));
        color.set_g(rnd::range(40, 255));
        color.set_b(rnd::range(40, 255));

        return color;
}

// TODO: This should be a property
void Zombie::on_death()
{
        const int summon_one_in_n = 5;

        // Are we destroyed now? (By a strong attack, or by getting killed twice)
        if (state_ == ActorState::destroyed)
        {
                // If corpse is destroyed, occasionally spawn Zombie parts. Spawning is
                // only allowed if the corpse is not destroyed "too hard" (e.g. by a
                // near explosion or a sledge hammer). This also serves to reward heavy
                // weapons, since they will more often prevent spawning nasty stuff.
                if ((hp_ > -8) &&
                    !map::cells[pos.x][pos.y].rigid->is_bottomless() &&
                    rnd::one_in(summon_one_in_n))
                {
                        ActorId id_to_spawn = ActorId::END;

                        // With a small chance, spawn a Floating Skull, otherwise spawn
                        // Hands or Intestines
                        const int roll =
                                rnd::one_in(50) ?
                                3 :
                                rnd::range(1, 2);

                        const std::string my_name = name_the();

                        std::string spawn_msg = "";

                        if (roll == 1)
                        {
                                id_to_spawn = ActorId::crawling_hand;

                                spawn_msg =
                                        "The hand of " +
                                        my_name +
                                        " comes off and starts crawling around!";
                        }
                        else if (roll == 2)
                        {
                                id_to_spawn = ActorId::crawling_intestines;

                                spawn_msg =
                                        "The intestines of " +
                                        my_name +
                                        " starts crawling around!";
                        }
                        else
                        {
                                id_to_spawn = ActorId::floating_skull;

                                spawn_msg =
                                        "The head of " +
                                        my_name +
                                        " starts floating around!";
                        }

                        if (map::cells[pos.x][pos.y].is_seen_by_player)
                        {
                                ASSERT(!spawn_msg.empty());

                                msg_log::add(spawn_msg);

                                map::player->incr_shock(ShockLvl::frightening,
                                                        ShockSrc::see_mon);
                        }

                        ASSERT(id_to_spawn != ActorId::END);

                        actor_factory::spawn(pos, {id_to_spawn})
                                .make_aware_of_player();

                } // if spawn zombie parts
        }
}

void Zombie::on_destroyed()
{
        // Occasionally make Zombie Dust
        const int make_dust_one_in_n = 7;

        if (!map::cells[pos.x][pos.y].rigid->is_bottomless() &&
            rnd::one_in(make_dust_one_in_n))
        {
                item_factory::make_item_on_floor(ItemId::zombie_dust, pos);
        }
}

// void Thing::make_start_items()
// {
//         add_spell(SpellSkill::basic, SpellId::teleport);
// }

TheHighPriest::TheHighPriest() :
        Mon                 (),
        has_become_aware_   (false) {}

// void TheHighPriest::make_start_items()
// {
//         add_spell(SpellSkill::master, SpellId::darkbolt);
//         add_spell(SpellSkill::master, SpellId::summon);
//         add_spell(SpellSkill::master, SpellId::burn);
//         add_spell(SpellSkill::master, SpellId::pest);
//         add_spell(SpellSkill::master, SpellId::spell_shield);
//         add_spell(SpellSkill::master, SpellId::disease);

//         add_spell(SpellSkill::expert, SpellId::enfeeble);
// }

void TheHighPriest::on_death()
{
        msg_log::add("The ground rumbles...",
                     colors::white(),
                     false,
                     MorePromptOnMsg::yes);

        const P stair_pos(map_w - 2, 11);

        map::put(new Stairs(stair_pos));

        map::put(new RubbleLow(stair_pos - P(1, 0)));

        const size_t nr_snakes = rnd::range(4, 5);

        actor_factory::spawn(
                pos,
                {nr_snakes, ActorId::pit_viper});
}

// TODO: This should be controlled by the map
DidAction TheHighPriest::on_act()
{
        if (!is_alive())
        {
                return DidAction::no;
        }

        if (!has_become_aware_)
        {
                audio::play(SfxId::boss_voice1);

                has_become_aware_ = true;

                become_aware_player(false);

                for (auto* const actor : game_time::actors)
                {
                        if (actor->is_player() ||
                            (actor == this))
                        {
                                continue;
                        }

                        static_cast<Mon*>(actor)->become_aware_player(false);
                }
        }

        return DidAction::no;
}

AnimatedWpn::AnimatedWpn() :
        Mon                     (),
        nr_turns_until_drop_    (rnd::range(225, 250)) {}

void AnimatedWpn::on_death()
{
        inv_->remove_item_in_slot(SlotId::wpn,
                                  true); // Delete the item
}

std::string AnimatedWpn::name_the() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        const std::string name = item->name(ItemRefType::plain,
                                            ItemRefInf::yes,
                                            ItemRefAttInf::none);

        return "The floating " + name;
}

std::string AnimatedWpn::name_a() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        const std::string name = item->name(ItemRefType::plain,
                                            ItemRefInf::yes,
                                            ItemRefAttInf::none);

        return "A floating " + name;
}

char AnimatedWpn::character() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        return item->character();
}

Color AnimatedWpn::color() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        return item->color();
}

TileId AnimatedWpn::tile() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        return item->tile();
}

std::string AnimatedWpn::descr() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        std::string str = item->name(ItemRefType::a,
                                     ItemRefInf::yes,
                                     ItemRefAttInf::none);

        str = text_format::first_to_upper(str);

        str += ", floating through the air as if wielded by an invisible hand.";

        return str;
}

std::string AnimatedWpn::death_msg() const
{
        Item* item = inv_->item_in_slot(SlotId::wpn);

        ASSERT(item);

        if (!item)
        {
                return "";
        }

        const std::string name =
                item->name(ItemRefType::plain,
                           ItemRefInf::yes,
                           ItemRefAttInf::none);

        return "The " + name + " is destroyed.";
}

void AnimatedWpn::on_std_turn_hook()
{
        if (!is_alive())
        {
                return;
        }

        if (nr_turns_until_drop_ <= 0)
        {
                drop();
        }
        else // Not yet time to die
        {
                --nr_turns_until_drop_;
        }
}

void AnimatedWpn::drop()
{
        if (map::player->can_see_actor(*this))
        {
                Item* item = inv_->item_in_slot(SlotId::wpn);

                ASSERT(item);

                if (item)
                {
                        const std::string name =
                                item->name(ItemRefType::plain,
                                           ItemRefInf::yes,
                                           ItemRefAttInf::none);

                        std::string msg =
                                "The " +
                                name +
                                " suddenly becomes lifeless and drops down.";

                        msg_log::add(msg);
                }
        }

        state_ = ActorState::destroyed;

        Item* item =
                inv_->remove_item_in_slot(
                        SlotId::wpn,
                        false); // Do not delete the item

        ASSERT(item);

        if (item)
        {
                item_drop::drop_item_on_map(pos, *item);
        }
}
