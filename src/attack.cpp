#include "attack.hpp"

#include "init.hpp"

#include "item.hpp"
#include "game_time.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "feature_trap.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "line_calc.hpp"
#include "io.hpp"
#include "sdl_base.hpp"
#include "knockback.hpp"
#include "drop.hpp"
#include "text_format.hpp"

AttData::AttData(Actor* const attacker,
                 Actor* const defender,
                 const Item& att_item) :
    attacker        (attacker),
    defender        (defender),
    skill_mod       (0),
    wpn_mod         (0),
    dodging_mod     (0),
    state_mod       (0),
    hit_chance_tot  (0),
    att_result      (ActionResult::fail),
    dmg             (0),
    is_intrinsic_att(att_item.data().type == ItemType::melee_wpn_intr ||
                     att_item.data().type == ItemType::ranged_wpn_intr) {}

bool AttData::is_defender_aware(const Actor* const attacker,
                                const Actor& defender) const
{
    bool is_aware = false;

    if (defender.is_player())
    {
        if (attacker)
        {
            const auto* const mon = static_cast<const Mon*>(attacker);

            is_aware =
                (mon->player_aware_of_me_counter_ > 0) ||
                player_bon::traits[(size_t)Trait::vigilant];
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

MeleeAttData::MeleeAttData(Actor* const attacker,
                           Actor& defender,
                           const Wpn& wpn) :
    AttData             (attacker, &defender, wpn),
    is_backstab         (false),
    is_weak_attack      (false)
{
    const P& def_pos = defender.pos;

    const bool is_defender_aware =
        AttData::is_defender_aware(attacker, defender);

    const ActorDataT& defender_data = defender.data();

    //
    // Determine attack result
    //
    skill_mod =
        attacker ?
        attacker->ability(AbilityId::melee, true) :
        50;

    wpn_mod = wpn.data().melee.hit_chance_mod;

    dodging_mod = 0;

    if (is_defender_aware)
    {
        dodging_mod -= defender.ability(AbilityId::dodging, true);

        // Evasion bonus to melee attacks from melee traits?
        if (defender.is_player())
        {
            if (player_bon::traits[(size_t)Trait::adept_melee_fighter])
            {
                dodging_mod -= 10;
            }

            if (player_bon::traits[(size_t)Trait::expert_melee_fighter])
            {
                dodging_mod -= 10;
            }

            if (player_bon::traits[(size_t)Trait::master_melee_fighter])
            {
                dodging_mod -= 10;
            }
        }
    }

    // Attacker gets a penalty against unseen targets

    //
    // NOTE: The AI never attacks unseen targets, so in the case of a
    //       monster attacker, we can assume the target is seen. We only
    //       need to check if target is seen when player is attacking.
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
            // Give big attack bonus if defender is completely unable to
            // fight.
            is_big_att_bon = true;
        }
        else if (defender.has_prop(PropId::confused) ||
                 defender.has_prop(PropId::slowed) ||
                 defender.has_prop(PropId::burning))
        {
            // Give small attack bonus if defender has problems fighting
            is_small_att_bon = true;
        }
    }

    // Give small attack bonus if defender cannot see.
    if (!is_big_att_bon &&
        !is_small_att_bon)
    {
        if (!defender.prop_handler().allow_see())
        {
            is_small_att_bon = true;
        }
    }

    state_mod =
        is_big_att_bon      ? 50 :
        is_small_att_bon    ? 20 : 0;

    // Lower hit chance if attacker cannot see target (e.g. attacking
    // invisible creature)
    if (!can_attacker_see_tgt)
    {
        state_mod -= 25;
    }

    // Lower hit chance if defender is ethereal (except if Bane of the Undead
    // bonus applies)
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

    //
    // NOTE: Total skill may be negative or above 100 (the attacker may still
    //       critically hit or miss)
    //
    att_result = ability_roll::roll(hit_chance_tot);

    //
    // Determine damage
    //
    Dice dmg_dice = wpn.dmg(AttMode::melee, attacker);

    if (apply_undead_bane_bon)
    {
        dmg_dice.plus += 2;
    }

    if (attacker && attacker->has_prop(PropId::weakened))
    {
        // Weak attack (min damage)
        dmg = dmg_dice.min();
        is_weak_attack = true;
    }
    else // Attacker not weakened, or not an actor attacking (e.g. a trap)
    {
        if (att_result == ActionResult::success_critical)
        {
            // Critical hit (max damage)
            dmg = std::max(1, dmg_dice.max());
        }
        else // Not critical hit
        {
            dmg = std::max(1, dmg_dice.roll());
        }

        if (attacker && !is_defender_aware)
        {
            // Backstab, +50% damage
            int dmg_pct = 150;

            // +150% if player is Vicious
            if ((attacker == map::player) &&
                player_bon::traits[(size_t)Trait::vicious])
            {
                dmg_pct += 150;
            }

            // +300% damage if attacking with a dagger
            if (wpn.data().id == ItemId::dagger)
            {
                dmg_pct += 300;
            }

            dmg = (dmg * dmg_pct) / 100;

            is_backstab = true;
        }
    }
}

RangedAttData::RangedAttData(Actor* const attacker,
                             const P& attacker_origin,
                             const P& aim_pos,
                             const P& current_pos,
                             const Wpn& wpn,
                             ActorSize aim_lvl) :
    AttData             (attacker, nullptr, wpn),
    aim_pos             (aim_pos),
    intended_aim_lvl    (ActorSize::undefined),
    defender_size       (ActorSize::undefined),
    verb_player_attacks (wpn.data().ranged.att_msgs.player),
    verb_other_attacks  (wpn.data().ranged.att_msgs.other),
    dist_mod            (0)
{
    Actor* const actor_aimed_at = map::actor_at_pos(aim_pos);

    // If aim level parameter not given, determine intended aim level now
    if (aim_lvl == ActorSize::undefined)
    {
        if (actor_aimed_at)
        {
            intended_aim_lvl = actor_aimed_at->data().actor_size;
        }
        else // No actor aimed at
        {
            const bool is_cell_blocked =
                map_parsers::BlocksProjectiles().
                cell(current_pos);

            intended_aim_lvl =
                is_cell_blocked ?
                ActorSize::humanoid :
                ActorSize::floor;
        }
    }
    else // Aim level already provided by parameter
    {
        intended_aim_lvl = aim_lvl;
    }

    defender = map::actor_at_pos(current_pos);

    if (defender && (defender != attacker))
    {
        TRACE_VERBOSE << "Defender found" << std::endl;

        const ActorDataT& defender_data = defender->data();

        const P& def_pos(defender->pos);

        skill_mod =
            attacker ?
            attacker->ability(AbilityId::ranged, true) :
            50;

        wpn_mod = wpn.data().ranged.hit_chance_mod;

        const bool is_defender_aware =
            AttData::is_defender_aware(attacker, *defender);

        dodging_mod =
            is_defender_aware ?
            -defender->ability(AbilityId::dodging, true) :
            0;

        const int dist_to_tgt = king_dist(attacker_origin, def_pos);

        dist_mod = 15 - (dist_to_tgt * 5);

        defender_size = defender_data.actor_size;

        state_mod = 0;

        // Lower hit chance if attacker cannot see target (e.g. attacking
        // invisible creature)
        if (attacker)
        {
            bool can_attacker_see_tgt = true;

            if (attacker->is_player())
            {
                can_attacker_see_tgt = map::player->can_see_actor(*defender);
            }
            else // Attacker is monster
            {
                Mon* const mon = static_cast<Mon*>(attacker);

                bool hard_blocked_los[map_w][map_h];

                const R fov_rect = fov::get_fov_rect(attacker->pos);

                map_parsers::BlocksLos()
                    .run(hard_blocked_los,
                         MapParseMode::overwrite,
                         fov_rect);

                can_attacker_see_tgt =
                    mon->can_see_actor(*defender, hard_blocked_los);
            }

            if (!can_attacker_see_tgt)
            {
                state_mod -= 25;
            }
        }

        // Player gets attack bonus for attacking unaware monster
        if ((attacker == map::player) &&
            (static_cast<Mon*>(defender)->aware_of_player_counter_ <= 0))
        {
            state_mod += 25;
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

            bool player_aim_x3 = false;

            if (attacker == map::player)
            {
                const Prop* const prop =
                    attacker->prop_handler().prop(PropId::aiming);

                if (prop)
                {
                    auto* aiming = static_cast<const PropAiming*>(prop);

                    player_aim_x3 = aiming->is_max_ranged_dmg();
                }
            }

            Dice dmg_dice = wpn.dmg(AttMode::ranged, attacker);

            if ((attacker == map::player) &&
                player_bon::gets_undead_bane_bon(defender_data))
            {
                dmg_dice.plus += 2;
            }

            dmg =
                player_aim_x3 ?
                dmg_dice.max() :
                dmg_dice.roll();

            // Outside effective range limit?
            if (!wpn.is_in_effective_range_lmt(attacker_origin, defender->pos))
            {
                TRACE_VERBOSE << "Outside effetive range limit" << std::endl;
                dmg = std::max(1, dmg / 2);
            }
        }
    }
}

ThrowAttData::ThrowAttData(Actor* const attacker,
                           const P& aim_pos,
                           const P& current_pos,
                           const Item& item,
                           ActorSize aim_lvl) :
    AttData             (attacker, nullptr, item),
    intended_aim_lvl    (ActorSize::undefined),
    defender_size       (ActorSize::undefined),
    dist_mod            (0)
{
    Actor* const actor_aimed_at = map::actor_at_pos(aim_pos);

    // If aim level parameter not given, determine intended aim level now
    if (aim_lvl == ActorSize::undefined)
    {
        if (actor_aimed_at)
        {
            intended_aim_lvl = actor_aimed_at->data().actor_size;
        }
        else // Not aiming at actor
        {
            const bool is_cell_blocked =
                map_parsers::BlocksProjectiles()
                .cell(current_pos);

            intended_aim_lvl =
                is_cell_blocked ?
                ActorSize::humanoid :
                ActorSize::floor;
        }
    }
    else // Aim level already provided by parameter
    {
        intended_aim_lvl = aim_lvl;
    }

    defender = map::actor_at_pos(current_pos);

    if (defender && (defender != attacker))
    {
        TRACE_VERBOSE << "Defender found" << std::endl;

        const ActorDataT& defender_data = defender->data();

        skill_mod =
            attacker ?
            attacker->ability(AbilityId::ranged, true) :
            50;

        wpn_mod = item.data().ranged.throw_hit_chance_mod;

        const bool is_defender_aware =
            AttData::is_defender_aware(attacker, *defender);

        dodging_mod =
            is_defender_aware ?
            -defender->ability(AbilityId::dodging, true) :
            0;

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
            can_attacker_see_tgt = map::player->can_see_actor(*defender);
        }

        // Lower hit chance if attacker cannot see target (e.g. attacking
        // invisible creature)
        if (!can_attacker_see_tgt)
        {
            state_mod -= 25;
        }

        // Player gets attack bonus for attacking unaware monster
        if ((attacker == map::player) &&
            (static_cast<Mon*>(defender)->aware_of_player_counter_ <= 0))
        {
            state_mod += 25;
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

            bool player_aim_x3 = false;

            if (attacker == map::player)
            {
                const Prop* const prop =
                    attacker->prop_handler().prop(PropId::aiming);

                if (prop)
                {
                    auto* aiming = static_cast<const PropAiming*>(prop);

                    player_aim_x3 = aiming->is_max_ranged_dmg();
                }
            }

            Dice dmg_dice = item.dmg(AttMode::thrown, attacker);

            if (apply_undead_bane_bon)
            {
                dmg_dice.plus += 2;
            }

            dmg = player_aim_x3 ? dmg_dice.max() : dmg_dice.roll();

            // Outside effective range limit?
            if (!item.is_in_effective_range_lmt(attacker->pos, defender->pos))
            {
                TRACE_VERBOSE << "Outside effetive range limit" << std::endl;
                dmg = std::max(1, dmg / 2);
            }
        }
    }
}

namespace attack
{

namespace
{

void print_melee_msg_and_mk_snd(const MeleeAttData& att_data, const Wpn& wpn)
{
    ASSERT(att_data.defender);

    std::string other_name = "";
    std::string snd_msg = "";

    auto snd_alerts_mon = AlertsMon::no;

    const bool is_player_silent = player_bon::traits[(size_t)Trait::silent];

    // Should the attack alert monsters?
    if (wpn.data().melee.is_noisy &&
        !((att_data.attacker == map::player) && is_player_silent))
    {
        snd_alerts_mon = AlertsMon::yes;
    }

    // Only print a message if player is not involved
    if ((att_data.defender != map::player) &&
        (att_data.attacker != map::player))
    {
        // TODO: This message is not always appropriate (e.g. spear traps)
        snd_msg = "I hear fighting.";
    }

    if (att_data.att_result <= ActionResult::fail)
    {
        SfxId sfx = wpn.data().melee.miss_sfx;

        //
        // Bad aiming
        //
        if (att_data.attacker == map::player)
        {
            msg_log::add("I miss.");
        }
        // Attacker is monster
        else if (att_data.attacker)
        {
            if (att_data.defender->is_player())
            {
                if (map::player->can_see_actor(*att_data.attacker))
                {
                    other_name =
                        text_format::first_to_upper(
                            att_data.attacker->name_the());
                }
                else // Player cannot see attacker
                {
                    other_name = "It";
                }

                msg_log::add(other_name + " misses me.", clr_text, true);
            }
        };

        Snd snd(snd_msg,
                sfx,
                IgnoreMsgIfOriginSeen::yes,
                att_data.defender->pos,
                att_data.attacker,
                SndVol::low,
                snd_alerts_mon);

        snd_emit::run(snd);
    }
    else // Attack hits target
    {
        // Determine the relative "size" of the hit
        const Dice dmg_dice =
            wpn.dmg(AttMode::melee, att_data.attacker);

        const int max_dmg = dmg_dice.max();

        MeleeHitSize hit_size = MeleeHitSize::small;

        if (max_dmg >= 4)
        {
            if (att_data.dmg > (max_dmg * 5) / 6)
            {
                hit_size = MeleeHitSize::hard;
            }
            else if (att_data.dmg >  max_dmg / 2)
            {
                hit_size = MeleeHitSize::medium;
            }
        }

        // Punctuation depends on attack strength
        std::string dmg_punct = ".";

        switch (hit_size)
        {
        case MeleeHitSize::small:
            break;

        case MeleeHitSize::medium:
            dmg_punct = "!";
            break;

        case MeleeHitSize::hard:
            dmg_punct = "!!!";
            break;
        }

        if (att_data.attacker == map::player)
        {
            const std::string wpn_verb = wpn.data().melee.att_msgs.player;

            if (map::player->can_see_actor(*att_data.defender))
            {
                other_name = att_data.defender->name_the();
            }
            else // Player cannot see defender
            {
                other_name = "it";
            }

            if (att_data.is_intrinsic_att)
            {
                const std::string att_mod_str =
                    att_data.is_weak_attack ?
                    " feebly" : "";

                msg_log::add("I " +
                             wpn_verb +
                             " " +
                             other_name +
                             att_mod_str +
                             dmg_punct,
                             clr_msg_good);
            }
            else // Not intrinsic attack
            {
                const std::string att_mod_str =
                    att_data.is_weak_attack ? "feebly " :
                    att_data.is_backstab ? "covertly "  : "";

                const Clr clr =
                    att_data.is_backstab ?
                    clr_blue_lgt :
                    clr_msg_good;

                const std::string wpn_name_a =
                    wpn.name(ItemRefType::a,
                             ItemRefInf::none);

                msg_log::add("I " +
                             wpn_verb +
                             " " +
                             other_name +
                             " " +
                             att_mod_str +
                             "with " +
                             wpn_name_a +
                             dmg_punct,
                             clr);
            }
        }
        else if (att_data.attacker) // Attacker is monster
        {
            if (att_data.defender->is_player())
            {
                const std::string wpn_verb =
                    wpn.data().melee.att_msgs.other;

                if (map::player->can_see_actor(*att_data.attacker))
                {
                    other_name =
                        text_format::first_to_upper(
                            att_data.attacker->name_the());
                }
                else // Player cannot see attacker
                {
                    other_name = "It";
                }

                msg_log::add(other_name +
                             " " +
                             wpn_verb +
                             dmg_punct,
                             clr_msg_bad,
                             true);
            }
        }
        else // No attacker (e.g. trap attack)
        {
            if (att_data.defender == map::player)
            {
                msg_log::add("I am hit" + dmg_punct,
                             clr_msg_bad,
                             true);
            }
            else // Defender is monster
            {
                if (map::player->can_see_actor(*att_data.defender))
                {
                    other_name =
                        text_format::first_to_upper(
                            att_data.defender->name_the());

                    Clr msg_clr = clr_msg_good;

                    if (map::player->is_leader_of(att_data.defender))
                    {
                        // Monster is allied to player, use a neutral color
                        // instead (we do not use red color here, since that
                        // is reserved for player taking damage).
                        msg_clr = clr_white;
                    }

                    msg_log::add(other_name + " is hit" + dmg_punct,
                                 msg_clr,
                                 true);
                }
            }
        }

        SfxId hit_sfx = SfxId::END;

        switch (hit_size)
        {
        case MeleeHitSize::small:
            hit_sfx = wpn.data().melee.hit_small_sfx;
            break;

        case MeleeHitSize::medium:
            hit_sfx = wpn.data().melee.hit_medium_sfx;
            break;

        case MeleeHitSize::hard:
            hit_sfx = wpn.data().melee.hit_hard_sfx;
            break;
        }

        Snd snd(snd_msg,
                hit_sfx,
                IgnoreMsgIfOriginSeen::yes,
                att_data.defender->pos,
                att_data.attacker,
                SndVol::low,
                snd_alerts_mon);

        snd_emit::run(snd);
    }
}

void print_ranged_init_msgs(const RangedAttData& data)
{
    if (!data.attacker)
    {
        // No attacker actor (e.g. a trap firing a dart)
        return;
    }

    if (data.attacker == map::player)
    {
        // Player is attacking
        msg_log::add("I " + data.verb_player_attacks + ".");
    }
    else // Attacker is monster
    {
        // Only print message if aiming at player
        if (data.aim_pos == map::player->pos)
        {
            const P& p = data.attacker->pos;

            if (map::cells[p.x][p.y].is_seen_by_player)
            {
                const std::string attacker_name =
                    text_format::first_to_upper(
                        data.attacker->name_the());

                const std::string attack_verb = data.verb_other_attacks;

                msg_log::add(attacker_name + " " + attack_verb + ".",
                             clr_white,
                             true);
            }
        }
    }
}

void print_proj_at_actor_msgs(const RangedAttData& data,
                              const bool is_hit,
                              const Wpn& wpn)
{
    ASSERT(data.defender);

    // Print messages if player can see the cell (note that the player does not
    // have to see the actual actor being hit - it will simply say "it" is hit
    // otherwise)

    const P& defender_pos = data.defender->pos;

    const bool is_cell_seen =
        map::cells[defender_pos.x][defender_pos.y].is_seen_by_player;

    if (is_hit && is_cell_seen)
    {
        // Punctuation depends on attack strength
        const Dice dmg_dice = wpn.dmg(AttMode::ranged, data.attacker);

        const int max_dmg = dmg_dice.max();

        std::string dmg_punct = ".";

        if (max_dmg >= 4)
        {
            dmg_punct =
                data.dmg > max_dmg * 5 / 6 ? "!!!" :
                data.dmg > max_dmg / 2 ? "!"   : dmg_punct;
        }

        if (data.defender->is_player())
        {
            msg_log::add("I am hit" + dmg_punct, clr_msg_bad, true);
        }
        else // Defender is monster
        {
            std::string other_name = "It";

            if (map::player->can_see_actor(*data.defender))
            {
                other_name =
                    text_format::first_to_upper(
                        data.defender->name_the());
            }

            msg_log::add(other_name + " is hit" + dmg_punct, clr_msg_good);
        }
    }
}

void fire_projectiles(Actor* const attacker,
                      const P& origin,
                      const P& aim_pos,
                      Wpn& wpn)
{
    std::vector<Projectile*> projectiles;

    const bool is_machine_gun = wpn.data().ranged.is_machine_gun;

    const int nr_projectiles = is_machine_gun ? nr_mg_projectiles : 1;

    for (int i = 0; i < nr_projectiles; ++i)
    {
        Projectile* const p = new Projectile;

        auto att_data =
            new RangedAttData(attacker,
                              origin,   // Attacker pos
                              aim_pos,  // Aim pos
                              origin,   // Current pos
                              wpn);

        p->set_att_data(att_data);

        projectiles.push_back(p);
    }

    const ActorSize aim_lvl =
        projectiles[0]->att_data->intended_aim_lvl;

    const int delay =
        config::delay_projectile_draw() /
        (is_machine_gun ? 2 : 1);

    print_ranged_init_msgs(*projectiles[0]->att_data);

    const bool stop_at_tgt = aim_lvl == ActorSize::floor;

    const int cheb_trvl_lim = 30;

    // Get projectile path
    std::vector<P> path;

    line_calc::calc_new_line(origin,
                             aim_pos,
                             stop_at_tgt,
                             cheb_trvl_lim,
                             false,
                             path);

    const Clr projectile_clr = wpn.data().ranged.projectile_clr;
    char projectile_glyph = wpn.data().ranged.projectile_glyph;

    if (projectile_glyph == '/')
    {
        const int i = path.size() > 2 ? 2 : 1;

        if (path[i].y == origin.y)
        {
            projectile_glyph = '-';
        }
        else if (path[i].x == origin.x)
        {
            projectile_glyph = '|';
        }
        else if ((path[i].x > origin.x && path[i].y < origin.y) ||
                 (path[i].x < origin.x && path[i].y > origin.y))
        {
            projectile_glyph = '/';
        }
        else if ((path[i].x > origin.x && path[i].y > origin.y) ||
                 (path[i].x < origin.x && path[i].y < origin.y))
        {
            projectile_glyph = '\\';
        }
    }

    TileId projectile_tile = wpn.data().ranged.projectile_tile;

    if (projectile_tile == TileId::projectile_std_front_slash)
    {
        if (projectile_glyph == '-')
        {
            projectile_tile = TileId::projectile_std_dash;
        }
        else if (projectile_glyph == '|')
        {
            projectile_tile = TileId::projectile_std_vertical_bar;
        }
        else if (projectile_glyph == '\\')
        {
            projectile_tile = TileId::projectile_std_back_slash;
        }
    }

    const bool leave_trail = wpn.data().ranged.projectile_leaves_trail;

    const SndVol vol = wpn.data().ranged.snd_vol;

    const std::string snd_msg = wpn.data().ranged.snd_msg;

    // An "update" here means each time we update the positions of all
    // projectiles (from the player perspective, each update is done
    // simultaneously for all projectiles)
    //
    // The total number of updates we need equals the length of the path, plus
    // the distance between the first and the last projectile (since the first
    // projectile will finish before the last projectile)
    const int nr_updates_tot =
        path.size() +
        ((nr_projectiles - 1) * nr_cell_jumps_mg_projectiles);

    for (int update_count = 1;
         update_count < nr_updates_tot;
         ++update_count)
    {
        for (int projectile_idx = 0;
             projectile_idx < nr_projectiles;
             ++projectile_idx)
        {
            Projectile* const proj = projectiles[projectile_idx];

            // The current projectile's index in the path equals the current
            // update count, minus the projectiles distance from the first
            int path_element =
                update_count -
                (projectile_idx * nr_cell_jumps_mg_projectiles);

            // Emit sound
            if (path_element == 1)
            {
                const SfxId sfx = wpn.data().ranged.att_sfx;

                if (!snd_msg.empty())
                {
                    std::string snd_msg_used = snd_msg;

                    if (attacker == map::player)
                    {
                        snd_msg_used = "";
                    }

                    //
                    // NOTE: The initial attack sound(s) must NOT alert monsters
                    //       since this would immediately make them aware before
                    //       any attack data is set. This would result in the
                    //       player never geting a ranged attack bonus against
                    //       unaware monsters (unless the monster is deaf).
                    //       Instead, an extra sound is run after the attack
                    //       (without message or sound effect).
                    //
                    Snd snd(snd_msg_used,
                            sfx,
                            IgnoreMsgIfOriginSeen::yes,
                            origin,
                            attacker,
                            vol,
                            AlertsMon::no); // We do not yet alert monsters

                    snd.run();
                }
            }

            // All the following collision checks etc are only made if the
            // projectiles current path element corresponds to an element in
            // the real path vector
            if ((path_element >= 1) &&
                (path_element < (int)path.size()) &&
                !proj->is_dead)
            {
                proj->pos = path[path_element];

                // Are we out of range?
                const int max_range = wpn.data().ranged.max_range;

                if (king_dist(origin, proj->pos) > max_range)
                {
                    proj->is_dead = true;

                    continue;
                }

                proj->is_seen_by_player =
                    map::cells[proj->pos.x][proj->pos.y].is_seen_by_player;

                // Get attack data again for every cell traveled through
                auto atta_data =
                    new RangedAttData(attacker,
                                      origin,       // Attacker origin
                                      aim_pos,      // Aim position
                                      proj->pos,    // Current position
                                      wpn,
                                      aim_lvl);

                proj->set_att_data(atta_data);

                // Hit actor?
                const bool is_actor_aimed_for = proj->pos == aim_pos;

                const auto& att_data = *proj->att_data;

                const bool can_hit_height =
                    att_data.defender_size >= ActorSize::humanoid ||
                    is_actor_aimed_for;

                if (att_data.defender &&
                    !proj->is_dead &&
                    (att_data.att_result >= ActionResult::success) &&
                    can_hit_height)
                {
                    if (attacker == map::player)
                    {
                        static_cast<Mon*>(att_data.defender)->
                            set_player_aware_of_me();
                    }

                    Snd snd("A creature is hit.",
                            SfxId::hit_small,
                            IgnoreMsgIfOriginSeen::yes,
                            proj->pos,
                            nullptr,
                            SndVol::low,
                            AlertsMon::no);

                    snd.run();

                    // Render actor hit
                    if (proj->is_seen_by_player)
                    {
                        if (config::is_tiles_mode())
                        {
                            proj->set_tile(TileId::blast1, clr_red_lgt);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay / 2);

                            proj->set_tile(TileId::blast2, clr_red_lgt);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay / 2);
                        }
                        else // Text mode
                        {
                            proj->set_glyph('*', clr_red_lgt);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay);
                        }

                        // MESSAGES FOR ACTOR HIT
                        print_proj_at_actor_msgs(att_data, true, wpn);

                        // Need to draw again here to show log message
                        io::draw_projectiles(projectiles, !leave_trail);
                    }

                    proj->is_done_rendering = true;

                    proj->is_dead = true;

                    proj->actor_hit = att_data.defender;

                    proj->obstructed_in_element = path_element;

                    auto died = ActorDied::no;

                    if (att_data.dmg > 0)
                    {
                        died = proj->actor_hit->hit(
                            att_data.dmg,
                            wpn.data().ranged.dmg_type,
                            DmgMethod::END,
                            AllowWound::yes,
                            attacker);
                    }

                    //
                    // NOTE: This is run regardless of if defender died or not,
                    //       it is the hook implementors responsibility to check
                    //       this if it matters.
                    //
                    wpn.on_ranged_hit(*proj->actor_hit);

                    if (died == ActorDied::no)
                    {
                        // Hit properties
                        PropHandler& defender_prop_handler =
                            proj->actor_hit->prop_handler();

                        defender_prop_handler.apply_from_att(wpn, false);

                        // Knock-back?
                        if (wpn.data().ranged.knocks_back &&
                            attacker)
                        {
                            const AttData* const current_data = proj->att_data;

                            if (current_data->att_result >=
                                ActionResult::success)
                            {
                                const bool is_spike_gun =
                                    wpn.data().id == ItemId::spike_gun;

                                knockback::run(
                                    *(current_data->defender),
                                    current_data->attacker->pos,
                                    is_spike_gun);
                            }
                        }
                    }
                } // if actor hit

                // Projectile hit feature?
                std::vector<Mob*> mobs;

                game_time::mobs_at_pos(proj->pos, mobs);

                Feature* feature_blocking_shot = nullptr;

                for (auto* mob : mobs)
                {
                    if (!mob->is_projectile_passable())
                    {
                        feature_blocking_shot = mob;
                    }
                }

                Rigid* rigid = map::cells[proj->pos.x][proj->pos.y].rigid;

                if (!rigid->is_projectile_passable())
                {
                    feature_blocking_shot = rigid;
                }

                if (feature_blocking_shot && !proj->is_dead)
                {
                    proj->obstructed_in_element = path_element - 1;

                    proj->is_dead = true;

                    if (wpn.data().ranged.makes_ricochet_snd)
                    {
                        Snd snd("I hear a ricochet.",
                                SfxId::ricochet,
                                IgnoreMsgIfOriginSeen::yes,
                                proj->pos,
                                nullptr,
                                SndVol::low,
                                AlertsMon::yes);

                        snd_emit::run(snd);
                    }

                    // RENDER FEATURE HIT
                    if (proj->is_seen_by_player)
                    {
                        if (config::is_tiles_mode())
                        {
                            proj->set_tile(TileId::blast1, clr_yellow);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay / 2);

                            proj->set_tile(TileId::blast2, clr_yellow);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay / 2);
                        }
                        else // Text mode
                        {
                            proj->set_glyph('*', clr_yellow);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay);
                        }
                    }
                } // if feature hit

                // PROJECTILE HIT THE GROUND?
                if ((proj->pos == aim_pos) &&
                    (aim_lvl == ActorSize::floor) &&
                    !proj->is_dead)
                {
                    proj->is_dead = true;
                    proj->obstructed_in_element = path_element;

                    if (wpn.data().ranged.makes_ricochet_snd)
                    {
                        Snd snd("I hear a ricochet.",
                                SfxId::ricochet,
                                IgnoreMsgIfOriginSeen::yes,
                                proj->pos,
                                nullptr,
                                SndVol::low,
                                AlertsMon::yes);

                        snd_emit::run(snd);
                    }

                    // RENDER GROUND HITS
                    if (proj->is_seen_by_player)
                    {
                        if (config::is_tiles_mode())
                        {
                            proj->set_tile(TileId::blast1, clr_yellow);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay / 2);

                            proj->set_tile(TileId::blast2, clr_yellow);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay / 2);
                        }
                        else // Text mode
                        {
                            proj->set_glyph('*', clr_yellow);

                            io::draw_projectiles(projectiles, !leave_trail);

                            sdl_base::sleep(delay);
                        }
                    }
                } // if hit the ground

                // RENDER FLYING PROJECTILES
                if (!proj->is_dead &&
                    proj->is_seen_by_player)
                {
                    if (config::is_tiles_mode())
                    {
                        proj->set_tile(projectile_tile, projectile_clr);

                        io::draw_projectiles(projectiles, !leave_trail);
                    }
                    else // Text mode
                    {
                        proj->set_glyph(projectile_glyph, projectile_clr);

                        io::draw_projectiles(projectiles, !leave_trail);
                    }
                }
            }

        } // projectiles loop

        // If any projectile can be seen and not dead, delay rendering
        for (Projectile* projectile : projectiles)
        {
            const P& pos = projectile->pos;

            if (map::cells[pos.x][pos.y].is_seen_by_player &&
                !projectile->is_dead)
            {
                sdl_base::sleep(delay);
                break;
            }
        }

        // Check if all projectiles are dead
        bool is_all_dead = true;

        for (Projectile* projectile : projectiles)
        {
            if (!projectile->is_dead)
            {
                is_all_dead = false;
            }
        }

        if (is_all_dead)
        {
            break;
        }

    } // path loop

    // So far, only projectile 0 can have special obstruction events, this
    // must be changed if something like an assault-incinerator is added
    const Projectile* const first_projectile = projectiles[0];

    if (first_projectile->is_dead)
    {
        const int element = first_projectile->obstructed_in_element;

        const P& pos = path[element];

        wpn.on_projectile_blocked(pos, first_projectile->actor_hit);
    }
    else // Not blocked
    {
        wpn.on_projectile_blocked(aim_pos, first_projectile->actor_hit);
    }

    // Cleanup
    for (Projectile* projectile : projectiles)
    {
        delete projectile;
    }

    //
    // See note above
    //
    if (!snd_msg.empty())
    {
        Snd snd("",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                origin,
                attacker,
                vol,
                AlertsMon::yes); // Now we alert monsters

        snd.run();
    }
}

void shotgun(Actor& attacker, const Wpn& wpn, const P& aim_pos)
{
    RangedAttData att_data = RangedAttData(
        &attacker,
        attacker.pos,
        aim_pos,
        attacker.pos,
        wpn);

    print_ranged_init_msgs(att_data);

    const ActorSize intended_aim_lvl = att_data.intended_aim_lvl;

    bool feature_blockers[map_w][map_h];

    map_parsers::BlocksProjectiles()
        .run(feature_blockers);

    Actor* actor_array[map_w][map_h];
    map::mk_actor_array(actor_array);

    const P origin = attacker.pos;
    std::vector<P> path;
    line_calc::calc_new_line(origin, aim_pos, false, 9999, false, path);

    int nr_actors_hit = 0;

    int killed_mon_idx = -1;

    // Emit sound
    const bool is_attacker_player = &attacker == map::player;

    const SndVol vol = wpn.data().ranged.snd_vol;

    std::string snd_msg = wpn.data().ranged.snd_msg;

    if (!snd_msg.empty())
    {
        if (is_attacker_player)
        {
            snd_msg = "";
        }

        const SfxId sfx = wpn.data().ranged.att_sfx;

        //
        // NOTE: The initial attack sound(s) must NOT alert monsters since
        //       this would immediately make them aware before any attack data
        //       is set. This would result in the player never geting a ranged
        //       attack bonus against unaware monsters (unless the monster is
        //       deaf). Instead, an extra sound is run after the attack (without
        //       message or sound effect).
        //
        Snd snd(snd_msg,
                sfx,
                IgnoreMsgIfOriginSeen::yes,
                attacker.pos,
                &attacker,
                vol,
                AlertsMon::no); // We do not yet alert monsters

        snd_emit::run(snd);
    }

    for (size_t path_idx = 1; path_idx < path.size(); ++path_idx)
    {
        // If travelled more than two steps after a killed monster, stop
        // projectile
        if ((killed_mon_idx != -1) &&
            (path_idx > (size_t)(killed_mon_idx + 1)))
        {
            break;
        }

        const P current_pos(path[path_idx]);

        // Are we out of range?
        const int max_range = wpn.data().ranged.max_range;

        if (king_dist(origin, current_pos) > max_range)
        {
            break;
        }

        if (actor_array[current_pos.x][current_pos.y])
        {
            // Only attempt hit if aiming at a level that would hit the actor
            const ActorSize size_of_actor =
                actor_array[current_pos.x][current_pos.y]->data().actor_size;

            if ((size_of_actor >= ActorSize::humanoid) ||
                (current_pos == aim_pos))
            {
                // Actor hit?
                att_data = RangedAttData(
                    &attacker,
                    attacker.pos,
                    aim_pos,
                    current_pos,
                    wpn,
                    intended_aim_lvl);

                if (att_data.att_result >= ActionResult::success)
                {
                    if (&attacker == map::player)
                    {
                        static_cast<Mon*>(att_data.defender)->
                            set_player_aware_of_me();
                    }

                    Snd snd("A creature is hit.",
                            SfxId::hit_small,
                            IgnoreMsgIfOriginSeen::yes,
                            current_pos,
                            nullptr,
                            SndVol::low,
                            AlertsMon::no);

                    snd.run();

                    const auto& cell = map::cells[current_pos.x][current_pos.y];

                    const bool is_seen = cell.is_seen_by_player;

                    if (is_seen)
                    {
                        states::draw();

                        io::cover_cell_in_map(current_pos);

                        if (config::is_tiles_mode())
                        {
                            io::draw_tile(TileId::blast2,
                                              Panel::map,
                                              current_pos,
                                              clr_red_lgt);
                        }
                        else // Text mode
                        {
                            io::draw_glyph('*',
                                               Panel::map,
                                               current_pos,
                                               clr_red_lgt);
                        }

                        io::update_screen();

                        sdl_base::sleep(config::delay_shotgun());
                    }

                    // Messages
                    print_proj_at_actor_msgs(att_data, true, wpn);

                    // Damage
                    if (att_data.dmg > 0)
                    {
                        att_data.defender->hit(
                            att_data.dmg,
                            wpn.data().ranged.dmg_type,
                            DmgMethod::END,
                            AllowWound::yes,
                            &attacker);
                    }

                    ++nr_actors_hit;

                    states::draw();

                    // Special shotgun behavior:
                    // If current defender was killed, and player aimed at
                    // humanoid level or at floor level but beyond the current
                    // position, the shot will continue one cell.
                    const bool is_tgt_killed = !att_data.defender->is_alive();

                    if (is_tgt_killed && killed_mon_idx == -1)
                    {
                        killed_mon_idx = path_idx;
                    }

                    if (!is_tgt_killed ||
                        (nr_actors_hit >= 2) ||
                        ((intended_aim_lvl == ActorSize::floor) &&
                         (current_pos == aim_pos)))
                    {
                        break;
                    }
                }
            }
        }

        // Wall hit?
        if (feature_blockers[current_pos.x][current_pos.y])
        {
            // TODO: Check hit material, soft and wood should not cause ricochet

            Snd snd("I hear a ricochet.",
                    SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes,
                    current_pos,
                    nullptr,
                    SndVol::low,
                    AlertsMon::yes);

            snd_emit::run(snd);

            Cell& cell = map::cells[current_pos.x][current_pos.y];

            if (cell.is_seen_by_player)
            {
                states::draw();

                io::cover_cell_in_map(current_pos);

                if (config::is_tiles_mode())
                {
                    io::draw_tile(TileId::blast2,
                                  Panel::map,
                                  current_pos,
                                  clr_yellow);
                }
                else // Text mode
                {
                    io::draw_glyph('*', Panel::map, current_pos, clr_yellow);
                }

                io::update_screen();

                sdl_base::sleep(config::delay_shotgun());

                states::draw();
            }

            cell.rigid->hit(
                att_data.dmg,
                DmgType::physical,
                DmgMethod::shotgun,
                nullptr);

            break;
        }

        // Floor hit?
        if (intended_aim_lvl == ActorSize::floor &&
            (current_pos == aim_pos))
        {
            Snd snd("I hear a ricochet.",
                    SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes,
                    current_pos,
                    nullptr,
                    SndVol::low,
                    AlertsMon::yes);

            snd_emit::run(snd);

            if (map::cells[current_pos.x][current_pos.y].is_seen_by_player)
            {
                states::draw();

                io::cover_cell_in_map(current_pos);

                if (config::is_tiles_mode())
                {
                    io::draw_tile(TileId::blast2,
                                  Panel::map,
                                  current_pos,
                                  clr_yellow);
                }
                else
                {
                    io::draw_glyph('*',
                                   Panel::map,
                                   current_pos,
                                   clr_yellow);
                }

                io::update_screen();

                sdl_base::sleep(config::delay_shotgun());

                states::draw();
            }

            break;

        } // if floor hit

    } // path loop

    //
    // See note above
    //
    if (!snd_msg.empty())
    {
        Snd snd("",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                origin,
                &attacker,
                vol,
                AlertsMon::yes); // Now we alert monsters

        snd.run();
    }
}

} // namespace

void melee(Actor* const attacker,
           const P& attacker_origin,
           Actor& defender,
           Wpn& wpn)
{
    map::update_vision();

    const MeleeAttData att_data(attacker, defender, wpn);

    print_melee_msg_and_mk_snd(att_data, wpn);

    const bool is_hit = att_data.att_result >= ActionResult::success;

    if (is_hit)
    {
        const bool is_ranged_wpn =
            wpn.data().type == ItemType::ranged_wpn;

        const AllowWound allow_wound =
            is_ranged_wpn ?
            AllowWound::no :
            AllowWound::yes;

        const auto dmg_type = wpn.data().melee.dmg_type;

        defender.hit(
            att_data.dmg,
            dmg_type,
            DmgMethod::END,
            allow_wound,
            attacker);

        //
        // TODO: Why is light damage included here?
        //
        if (defender.data().can_bleed &&
            (dmg_type == DmgType::physical ||
             dmg_type == DmgType::pure ||
             dmg_type == DmgType::light))
        {
            map::mk_blood(defender.pos);
        }

        //
        // NOTE: This is run regardless of if defender died or not, it is the
        //       hook implementors responsibility to check this, if it matters
        //
        wpn.on_melee_hit(defender, att_data.dmg);

        if (defender.is_alive())
        {
            defender.prop_handler().apply_from_att(wpn, true);

            if (wpn.data().melee.knocks_back)
            {
                knockback::run(defender, attacker_origin, false);
            }
        }
        else // Defender was killed
        {
            //
            // NOTE: Destroyed actors are purged on standard turns, so it's no
            //       problem calling this function even if defender was
            //       destroyed (we haven't "ticked" game time yet)
            //
            wpn.on_melee_kill(defender);
        }
    }

    auto& player_inv = map::player->inv();

    const bool is_crit_fail =
        att_data.att_result == ActionResult::fail_critical;

    const bool player_cursed = map::player->has_prop(PropId::cursed);

    const bool is_wielding_wpn = player_inv.item_in_slot(SlotId::wpn) == &wpn;

    const int break_on_crit_fail_one_in_n = 32;

    // If player is cursed and the attack critically fails, occasionally break
    // the weapon.
    if ((attacker == map::player) &&
        is_crit_fail &&
        player_cursed &&
        is_wielding_wpn &&
        rnd::one_in(break_on_crit_fail_one_in_n))
    {
        // Remove item without deleting it
        Item* const item = player_inv.remove_item_in_slot(SlotId::wpn, false);

        ASSERT(item);

        if (item)
        {
            const std::string item_name =
                item->name(ItemRefType::plain, ItemRefInf::none);

            msg_log::add("My " + item_name + " breaks!",
                         clr_msg_note,
                         true,
                         MorePromptOnMsg::yes);

            delete item;
        }
    }

    if (attacker)
    {
        if (defender.is_player())
        {
            Mon* const mon = static_cast<Mon*>(att_data.attacker);

            mon->set_player_aware_of_me();
        }
        else // Defender is monster
        {
            static_cast<Mon&>(defender).become_aware_player(false);
        }

        int speed_pct_diff = 0;

        if (attacker == map::player)
        {
            if (player_bon::traits[(size_t)Trait::adept_melee_fighter])
            {
                speed_pct_diff += 5;
            }

            if (player_bon::traits[(size_t)Trait::expert_melee_fighter])
            {
                speed_pct_diff += 5;
            }

            if (player_bon::traits[(size_t)Trait::master_melee_fighter])
            {
                speed_pct_diff += 5;
            }
        }

        // Attacking ends cloaking
        attacker->prop_handler().end_prop(PropId::cloaked);

        game_time::tick(speed_pct_diff);
    }
}

bool ranged(Actor* const attacker,
            const P& origin,
            const P& aim_pos,
            Wpn& wpn)
{
    map::update_vision();

    bool did_attack = false;

    const bool has_inf_ammo = wpn.data().ranged.has_infinite_ammo;

    if (wpn.data().ranged.is_shotgun)
    {
        ASSERT(attacker);

        if ((wpn.nr_ammo_loaded_ != 0) ||
            has_inf_ammo)
        {
            shotgun(*attacker,
                    wpn,
                    aim_pos);

            did_attack = true;

            if (!has_inf_ammo)
            {
                wpn.nr_ammo_loaded_ -= 1;
            }
        }
    }
    else // Not a shotgun
    {
        int nr_of_projectiles = 1;

        if (wpn.data().ranged.is_machine_gun)
        {
            nr_of_projectiles = nr_mg_projectiles;
        }

        if ((wpn.nr_ammo_loaded_ >= nr_of_projectiles) ||
            has_inf_ammo)
        {
            //
            // TODO: A hack for the Mi-go gun - refactor
            //
            if ((attacker == map::player) &&
                (wpn.data().id == ItemId::mi_go_gun))
            {
                const std::string wpn_name =
                    wpn.name(ItemRefType::plain,
                             ItemRefInf::none);

                msg_log::add("The " +
                             wpn_name +
                             " draws power from my life force!",
                             clr_msg_bad);

                attacker->hit(mi_go_gun_hp_drained,
                              DmgType::pure,
                              DmgMethod::forced,
                              AllowWound::no);
            }

            fire_projectiles(attacker,
                             origin,
                             aim_pos,
                             wpn);

            did_attack = true;

            if (!has_inf_ammo)
            {
                wpn.nr_ammo_loaded_ -= nr_of_projectiles;
            }
        }
    }

    // Player could have for example fired an explosive weapon into a wall and
    // killed themselves - if so, abort early
    if (!map::player->is_alive())
    {
        return true;
    }

    states::draw();

    // Only pass time if an actor is attacking (not if it's e.g. a trap)
    if (did_attack && attacker)
    {
        int speed_pct_diff = 0;

        if (attacker == map::player &&
            wpn.data().type == ItemType::ranged_wpn)
        {
            if (player_bon::traits[(size_t)Trait::adept_marksman])
            {
                speed_pct_diff += 25;
            }

            if (player_bon::traits[(size_t)Trait::expert_marksman])
            {
                speed_pct_diff += 25;
            }

            if (player_bon::traits[(size_t)Trait::master_marksman])
            {
                speed_pct_diff += 25;
            }

            if (player_bon::traits[(size_t)Trait::fast_shooter])
            {
                speed_pct_diff += 100;
            }
        }

        // Attacking ends cloaking
        attacker->prop_handler().end_prop(PropId::cloaked);

        game_time::tick(speed_pct_diff);
    }

    return did_attack;
}

} // attack
