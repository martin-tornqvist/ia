#include "attack_data.hpp"

#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "item.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "property.hpp"
#include "feature_rigid.hpp"
#include "feature_trap.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static bool is_defender_aware_of_attack(const Actor* const attacker,
                                        const Actor& defender)
{
        bool is_aware = false;

        if (defender.is_player())
        {
                if (attacker)
                {
                        const auto* const mon =
                                static_cast<const Mon*>(attacker);

                        is_aware = mon->player_aware_of_me_counter_ > 0;
                }
                else // No attacker actor (e.g. a trap)
                {
                        is_aware = true;
                }
        }
        else // Defender is monster
        {
                auto* const mon = static_cast<const Mon*>(&defender);

                is_aware = mon->aware_of_player_counter_ > 0;
        }

        return is_aware;
}

static bool player_has_aiming_prop_dmg_bon()
{
        const Prop* const prop =
                map::player->properties()
                .prop(PropId::aiming);

        if (prop)
        {
                auto* aiming = static_cast<const PropAiming*>(prop);

                return aiming->is_max_ranged_dmg();
        }

        return false;
}

// -----------------------------------------------------------------------------
// Attack data
// -----------------------------------------------------------------------------
AttData::AttData(Actor* const attacker,
                 Actor* const defender,
                 const Item& att_item) :
        attacker(attacker),
        defender(defender),
        skill_mod(0),
        wpn_mod(0),
        dodging_mod(0),
        state_mod(0),
        hit_chance_tot(0),
        att_result(ActionResult::fail),
        dmg(0),
        is_intrinsic_att(att_item.data().type == ItemType::melee_wpn_intr ||
                         att_item.data().type == ItemType::ranged_wpn_intr) {}

MeleeAttData::MeleeAttData(Actor* const attacker,
                           Actor& defender,
                           const Wpn& wpn) :
        AttData(attacker, &defender, wpn),
        is_backstab(false),
        is_weak_attack(false)
{
        const P& def_pos = defender.pos;

        const bool is_defender_aware =
                is_defender_aware_of_attack(attacker, defender);

        const ActorData& defender_data = defender.data();

        // Determine attack result
        skill_mod =
                attacker ?
                attacker->ability(AbilityId::melee, true) :
                50;

        wpn_mod = wpn.data().melee.hit_chance_mod;

        dodging_mod = 0;

        const bool player_is_handling_armor =
                map::player->nr_turns_until_handle_armor_done_ > 0;

        const int dodging_ability = defender.ability(AbilityId::dodging, true);

        // Player gets melee dodging bonus from wielding a Pitchfork
        if (defender.is_player())
        {
                const auto* const item =
                        defender.inv().item_in_slot(SlotId::wpn);

                if (item && (item->id() == ItemId::pitch_fork))
                {
                        dodging_mod -= 15;
                }
        }

        const bool allow_positive_doge =
                is_defender_aware &&
                !(defender.is_player() &&
                  player_is_handling_armor);

        if (allow_positive_doge ||
            (dodging_ability < 0))
        {
                dodging_mod -= dodging_ability;
        }

        // Attacker gets a penalty against unseen targets

        // NOTE: The AI never attacks unseen targets, so in the case of a
        // monster attacker, we can assume the target is seen. We only need to
        // check if target is seen when player is attacking.
        bool can_attacker_see_tgt = true;

        if (attacker == map::player)
        {
                can_attacker_see_tgt = map::player->can_see_actor(defender);
        }

        // Check for extra attack bonuses, such as defender being immobilized.
        bool is_big_att_bon = false;
        bool is_small_att_bon = false;

        if (!is_defender_aware)
        {
                // Give big attack bonus if defender is unaware of the attacker.
                is_big_att_bon = true;
        }

        if (!is_big_att_bon)
        {
                // Give big attack bonus if defender is stuck in trap.
                const auto* const f = map::cells[def_pos.x][def_pos.y].rigid;

                if (f->id() == FeatureId::trap)
                {
                        const auto* const t = static_cast<const Trap*>(f);

                        if (t->is_holding_actor())
                        {
                                is_big_att_bon = true;
                        }
                }
        }

        if (!is_big_att_bon)
        {
                // Check if attacker gets a bonus due to a defender property.

                if (defender.has_prop(PropId::paralyzed) ||
                    defender.has_prop(PropId::nailed) ||
                    defender.has_prop(PropId::fainted))
                {
                        // Give big attack bonus if defender is completely
                        // unable to fight.
                        is_big_att_bon = true;
                }
                else if (defender.has_prop(PropId::confused) ||
                         defender.has_prop(PropId::slowed) ||
                         defender.has_prop(PropId::burning))
                {
                        // Give small attack bonus if defender has problems
                        // fighting
                        is_small_att_bon = true;
                }
        }

        // Give small attack bonus if defender cannot see.
        if (!is_big_att_bon &&
            !is_small_att_bon)
        {
                if (!defender.properties().allow_see())
                {
                        is_small_att_bon = true;
                }
        }

        state_mod =
                is_big_att_bon ? 50 :
                is_small_att_bon ? 20 : 0;

        // Lower hit chance if attacker cannot see target (e.g. attacking
        // invisible creature)
        if (!can_attacker_see_tgt)
        {
                state_mod -= 25;
        }

        // Lower hit chance if defender is ethereal (except if Bane of the
        // Undead bonus applies)
        const bool apply_undead_bane_bon =
                (attacker == map::player) &&
                player_bon::gets_undead_bane_bon(defender_data);

        const bool apply_ethereal_defender_pen =
                defender.has_prop(PropId::ethereal) &&
                !apply_undead_bane_bon;

        if (apply_ethereal_defender_pen)
        {
                state_mod -= 50;
        }

        hit_chance_tot =
                skill_mod +
                wpn_mod +
                dodging_mod +
                state_mod;

        // NOTE: Total skill may be negative or above 100 (the attacker may
        // still critically hit or miss)
        att_result = ability_roll::roll(hit_chance_tot);

        // Roll the damage dice
        {
                Dice dmg_dice = wpn.melee_dmg(attacker);

                if (apply_undead_bane_bon)
                {
                        dmg_dice.plus += 2;
                }

                // Roll the damage dice
                if (att_result == ActionResult::success_critical)
                {
                        // Critical hit (max damage)
                        dmg = std::max(1, dmg_dice.max());
                }
                else // Not critical hit
                {
                        dmg = std::max(1, dmg_dice.roll());
                }
        }

        if (attacker && attacker->has_prop(PropId::weakened))
        {
                // Weak attack (halved damage)
                dmg /= 2;

                dmg = std::max(1, dmg);

                is_weak_attack = true;
        }
        // Attacker not weakened, or not an actor attacking (e.g. a trap)
        else  if (attacker && !is_defender_aware)
        {
                TRACE << "Melee attack is backstab" << std::endl;

                // Backstab, +50% damage
                int dmg_pct = 150;

                // +150% if player is Vicious
                if ((attacker == map::player) &&
                    player_bon::traits[(size_t)Trait::vicious])
                {
                        dmg_pct += 150;
                }

                // +300% damage if attacking with a dagger
                const auto id = wpn.data().id;

                if ((id == ItemId::dagger) ||
                    (id == ItemId::spirit_dagger))
                {
                        dmg_pct += 300;
                }

                dmg = (dmg * dmg_pct) / 100;

                is_backstab = true;
        }
}

RangedAttData::RangedAttData(Actor* const attacker,
                             const P& attacker_origin,
                             const P& aim_pos,
                             const P& current_pos,
                             const Wpn& wpn) :
        AttData(attacker, nullptr, wpn),
        aim_pos(aim_pos),
        aim_lvl((ActorSize)0),
        defender_size((ActorSize)0),
        dist_mod(0)
{
        // Determine aim level
        // TODO: Quick hack, Incinerators always aim at the floor
        if (wpn.id() == ItemId::incinerator)
        {
                aim_lvl = ActorSize::floor;
        }
        else // Not incinerator
        {
                Actor* const actor_aimed_at = map::actor_at_pos(aim_pos);

                if (actor_aimed_at)
                {
                        aim_lvl = actor_aimed_at->data().actor_size;
                }
                else // No actor aimed at
                {
                        const bool is_cell_blocked =
                                map_parsers::BlocksProjectiles().
                                cell(aim_pos);

                        aim_lvl =
                                is_cell_blocked ?
                                ActorSize::humanoid :
                                ActorSize::floor;
                }
        }

        defender = map::actor_at_pos(current_pos);

        if (defender && (defender != attacker))
        {
                TRACE_VERBOSE << "Defender found" << std::endl;

                const ActorData& defender_data = defender->data();

                const P& def_pos(defender->pos);

                skill_mod =
                        attacker ?
                        attacker->ability(AbilityId::ranged, true) :
                        50;

                wpn_mod = wpn.data().ranged.hit_chance_mod;

                const bool is_defender_aware =
                        is_defender_aware_of_attack(attacker, *defender);

                dodging_mod = 0;

                const bool player_is_handling_armor =
                        map::player->nr_turns_until_handle_armor_done_ > 0;

                const bool allow_positive_doge =
                        is_defender_aware &&
                        !(defender->is_player() &&
                          player_is_handling_armor);

                const int dodging_ability =
                        defender->ability(AbilityId::dodging, true);

                if (allow_positive_doge ||
                    (dodging_ability < 0))
                {
                        dodging_mod =
                                -defender->ability(AbilityId::dodging, true);
                }

                const int dist_to_tgt = king_dist(attacker_origin, def_pos);

                dist_mod = 15 - (dist_to_tgt * 5);

                defender_size = defender_data.actor_size;

                state_mod = 0;

                // Lower hit chance if attacker cannot see target (e.g.
                // attacking invisible creature)
                if (attacker)
                {
                        bool can_attacker_see_tgt = true;

                        if (attacker->is_player())
                        {
                                can_attacker_see_tgt =
                                        map::player->can_see_actor(*defender);
                        }
                        else // Attacker is monster
                        {
                                Mon* const mon = static_cast<Mon*>(attacker);

                                bool hard_blocked_los[map_w][map_h];

                                const R fov_rect =
                                        fov::get_fov_rect(attacker->pos);

                                map_parsers::BlocksLos()
                                        .run(hard_blocked_los,
                                             MapParseMode::overwrite,
                                             fov_rect);

                                can_attacker_see_tgt =
                                        mon->can_see_actor(*defender,
                                                           hard_blocked_los);
                        }

                        if (!can_attacker_see_tgt)
                        {
                                state_mod -= 25;
                        }
                }

                // Player gets attack bonus for attacking unaware monster
                if (attacker == map::player)
                {
                        auto* const mon = static_cast<Mon*>(defender);

                        if (mon->aware_of_player_counter_ <= 0)
                        {
                                state_mod += 25;
                        }
                }

                const bool apply_undead_bane_bon =
                        (attacker == map::player) &&
                        player_bon::gets_undead_bane_bon(defender_data);

                const bool apply_ethereal_defender_pen =
                        defender->has_prop(PropId::ethereal) &&
                        !apply_undead_bane_bon;

                if (apply_ethereal_defender_pen)
                {
                        state_mod -= 50;
                }

                hit_chance_tot =
                        skill_mod +
                        wpn_mod +
                        dodging_mod +
                        dist_mod +
                        state_mod;

                set_constr_in_range(5, hit_chance_tot, 99);

                att_result = ability_roll::roll(hit_chance_tot);

                if (att_result >= ActionResult::success)
                {
                        TRACE_VERBOSE << "Attack roll succeeded" << std::endl;

                        bool player_has_aim_bon = false;

                        if (attacker == map::player)
                        {
                                player_has_aim_bon =
                                        player_has_aiming_prop_dmg_bon();
                        }

                        Dice dmg_dice = wpn.ranged_dmg(attacker);

                        if ((attacker == map::player) &&
                            player_bon::gets_undead_bane_bon(defender_data))
                        {
                                dmg_dice.plus += 2;
                        }

                        dmg =
                                player_has_aim_bon ?
                                dmg_dice.max() :
                                dmg_dice.roll();

                        // Outside effective range limit?
                        if (!wpn.is_in_effective_range_lmt(attacker_origin,
                                                           defender->pos))
                        {
                                TRACE_VERBOSE << "Outside effetive range limit"
                                              << std::endl;

                                dmg = std::max(1, dmg / 2);
                        }
                }
        }
}

ThrowAttData::ThrowAttData(Actor* const attacker,
                           const P& aim_pos,
                           const P& current_pos,
                           const Item& item) :
        AttData(attacker, nullptr, item),
        aim_lvl((ActorSize)0),
        defender_size((ActorSize)0),
        dist_mod(0)
{
        Actor* const actor_aimed_at = map::actor_at_pos(aim_pos);

        // Determine aim level
        if (actor_aimed_at)
        {
                aim_lvl = actor_aimed_at->data().actor_size;
        }
        else // Not aiming at actor
        {
                const bool is_cell_blocked =
                        map_parsers::BlocksProjectiles()
                        .cell(current_pos);

                aim_lvl =
                        is_cell_blocked ?
                        ActorSize::humanoid :
                        ActorSize::floor;
        }

        defender = map::actor_at_pos(current_pos);

        if (defender && (defender != attacker))
        {
                TRACE_VERBOSE << "Defender found" << std::endl;

                const ActorData& defender_data = defender->data();

                skill_mod =
                        attacker ?
                        attacker->ability(AbilityId::ranged, true) :
                        50;

                wpn_mod = item.data().ranged.throw_hit_chance_mod;

                const bool is_defender_aware =
                        is_defender_aware_of_attack(attacker, *defender);

                dodging_mod = 0;

                const bool player_is_handling_armor =
                        map::player->nr_turns_until_handle_armor_done_ > 0;

                const int dodging_ability =
                        defender->ability(AbilityId::dodging, true);

                const bool allow_positive_doge =
                        is_defender_aware &&
                        !(defender->is_player() &&
                          player_is_handling_armor);

                if (allow_positive_doge ||
                    (dodging_ability < 0))
                {
                        dodging_mod =
                                -defender->ability(AbilityId::dodging, true);
                }

                const P& att_pos(attacker->pos);
                const P& def_pos(defender->pos);

                const int dist_to_tgt =
                        king_dist(att_pos.x,
                                  att_pos.y,
                                  def_pos.x,
                                  def_pos.y);

                dist_mod = 15 - (dist_to_tgt * 5);

                defender_size = defender_data.actor_size;

                state_mod = 0;

                bool can_attacker_see_tgt = true;

                if (attacker == map::player)
                {
                        can_attacker_see_tgt =
                                map::player->can_see_actor(*defender);
                }

                // Lower hit chance if attacker cannot see target (e.g.
                // attacking invisible creature)
                if (!can_attacker_see_tgt)
                {
                        state_mod -= 25;
                }

                // Player gets attack bonus for attacking unaware monster
                if (attacker == map::player)
                {
                        const auto* const mon = static_cast<Mon*>(defender);

                        if (mon->aware_of_player_counter_ <= 0)
                        {
                                state_mod += 25;
                        }
                }

                const bool apply_undead_bane_bon =
                        (attacker == map::player) &&
                        player_bon::gets_undead_bane_bon(defender_data);

                const bool apply_ethereal_defender_pen =
                        defender->has_prop(PropId::ethereal) &&
                        !apply_undead_bane_bon;

                if (apply_ethereal_defender_pen)
                {
                        state_mod -= 50;
                }

                hit_chance_tot =
                        skill_mod +
                        wpn_mod +
                        dodging_mod +
                        dist_mod +
                        state_mod;

                set_constr_in_range(5, hit_chance_tot, 99);

                att_result = ability_roll::roll(hit_chance_tot);

                if (att_result >= ActionResult::success)
                {
                        TRACE_VERBOSE << "Attack roll succeeded" << std::endl;

                        bool player_has_aim_bon = false;

                        if (attacker == map::player)
                        {
                                player_has_aim_bon =
                                        player_has_aiming_prop_dmg_bon();
                        }

                        Dice dmg_dice = item.thrown_dmg(attacker);

                        if (apply_undead_bane_bon)
                        {
                                dmg_dice.plus += 2;
                        }

                        dmg =
                                player_has_aim_bon ?
                                dmg_dice.max() :
                                dmg_dice.roll();

                        // Outside effective range limit?
                        if (!item.is_in_effective_range_lmt(attacker->pos,
                                                            defender->pos))
                        {
                                TRACE_VERBOSE << "Outside effetive range limit"
                                              << std::endl;

                                dmg = std::max(1, dmg / 2);
                        }
                }
        }
}
