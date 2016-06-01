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
#include "render.hpp"
#include "sdl_wrapper.hpp"
#include "knockback.hpp"
#include "drop.hpp"

Att_data::Att_data(Actor* const attacker,
                   Actor* const defender,
                   const Item& att_item) :
    attacker        (attacker),
    defender        (defender),
    att_result      (fail),
    dmg             (0),
    is_intrinsic_att(att_item.data().type == Item_type::melee_wpn_intr ||
                     att_item.data().type == Item_type::ranged_wpn_intr),
    is_ethereal_defender_missed(false) {}

Melee_att_data::Melee_att_data(Actor* const attacker,
                               Actor& defender,
                               const Wpn& wpn) :
    Att_data            (attacker, &defender, wpn),
    is_defender_dodging (false),
    is_backstab         (false),
    is_weak_attack      (false)
{
    const P&            def_pos             = defender.pos;
    bool                is_defender_aware   = true;
    const Actor_data_t& defender_data       = defender.data();

    if (defender.is_player())
    {
        if (attacker)
        {
            is_defender_aware = static_cast<Mon*>(attacker)->player_aware_of_me_counter_ > 0 ||
                                player_bon::traits[(size_t)Trait::vigilant];
        }
        else //No attacker actor (e.g. a trap)
        {
            is_defender_aware = true;
        }
    }
    else //Defender is monster
    {
        is_defender_aware = static_cast<Mon*>(&defender)->aware_counter_ > 0;
    }

    if (is_defender_aware)
    {
        const int DEFENDER_DODGE_SKILL = defender.ability(Ability_id::dodge_att, true);

        const int DODGE_MOD_AT_FEATURE = map::cells[def_pos.x][def_pos.y].rigid->dodge_modifier();

        const int DODGE_CHANCE_TOT = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

        if (DODGE_CHANCE_TOT > 0)
        {
            is_defender_dodging = ability_roll::roll(DODGE_CHANCE_TOT, &defender) >= success;
        }
    }

    if (!is_defender_dodging)
    {
        //--------------------------------------- DETERMINE ATTACK RESULT
        int tot_skill = 0;

        if (attacker)
        {
            tot_skill = attacker->ability(Ability_id::melee, true);
        }

        tot_skill += wpn.data().melee.hit_chance_mod;

        bool is_attacker_aware = true;

        //Attacker gets a penalty against unseen targets
        //NOTE: The AI never attacks unseen targets, so in the case of a
        //      monster attacker, we can assume the target is seen. We only
        //      need to check if target is seen when player is attacking.
        bool can_attacker_see_tgt = true;

        if (attacker)
        {
            if (attacker->is_player())
            {
                Mon& mon = static_cast<Mon&>(defender);

                is_attacker_aware = mon.player_aware_of_me_counter_ > 0;

                can_attacker_see_tgt = map::player->can_see_actor(defender);
            }
            else //Attacker is monster
            {
                Mon* const mon = static_cast<Mon*>(attacker);

                is_attacker_aware = mon->aware_counter_ > 0;
            }
        }

        //If attacker is aware of the defender, check for extra attack bonuses
        //such as defender being immobilized or blind.
        if (is_attacker_aware)
        {
            bool is_big_att_bon    = false;
            bool is_small_att_bon  = false;

            if (!is_defender_aware)
            {
                //Give big attack bonus if defender is unaware of the attacker.
                is_big_att_bon = true;
            }

            if (!is_big_att_bon)
            {
                //Give big attack bonus if defender is stuck in trap (web).
                const auto* const f = map::cells[def_pos.x][def_pos.y].rigid;

                if (f->id() == Feature_id::trap)
                {
                    const auto* const t = static_cast<const Trap*>(f);

                    if (t->trap_type() == Trap_id::web)
                    {
                        const auto* const web = static_cast<const Trap_web*>(t->trap_impl());

                        if (web->is_holding())
                        {
                            is_big_att_bon = true;
                        }
                    }
                }
            }

            if (!is_big_att_bon)
            {
                //Check if attacker gets a bonus due to a defender property.

                if (
                    defender.has_prop(Prop_id::paralyzed) ||
                    defender.has_prop(Prop_id::nailed)    ||
                    defender.has_prop(Prop_id::fainted))
                {
                    //Give big attack bonus if defender is completely unable to fight.
                    is_big_att_bon = true;
                }
                else if (
                    defender.has_prop(Prop_id::confused) ||
                    defender.has_prop(Prop_id::slowed)   ||
                    defender.has_prop(Prop_id::burning))
                {
                    //Give small attack bonus if defender has problems fighting.
                    is_small_att_bon = true;
                }
            }

            //Give small attack bonus if defender cannot see.
            if (!is_big_att_bon && !is_small_att_bon)
            {
                if (!defender.prop_handler().allow_see())
                {
                    is_small_att_bon = true;
                }
            }

            //Apply the hit chance bonus (if any)
            tot_skill += is_big_att_bon ? 50 : (is_small_att_bon ? 20 : 0);

            //Lower hit chance if attacker cannot see target (e.g. attacking invisible creature)
            if (!can_attacker_see_tgt)
            {
                tot_skill -= 25;
            }
        }

        //NOTE: Total skill may be negative (attacker may still critically hit)
        att_result = ability_roll::roll(tot_skill, attacker);

        const bool APPLY_UNDEAD_BANE_BON = attacker == map::player &&
                                           player_bon::gets_undead_bane_bon(defender_data);

        //Ethereal target missed?
        if (defender.has_prop(Prop_id::ethereal) && !APPLY_UNDEAD_BANE_BON)
        {
            is_ethereal_defender_missed = rnd::fraction(2, 3);
        }

        //--------------------------------------- DETERMINE DAMAGE
        Dice_param dmg_dice = wpn.dmg(Att_mode::melee, attacker);

        if (APPLY_UNDEAD_BANE_BON)
        {
            dmg_dice.plus += 2;
        }

        if (attacker && attacker->has_prop(Prop_id::weakened))
        {
            //Weak attack (min damage)
            dmg             = dmg_dice.min();
            is_weak_attack  = true;
        }
        else //Attacker not weakened, or not an actor attacking (e.g. an attack from a trap)
        {
            if (att_result == success_critical)
            {
                //Critical hit (max damage)
                dmg = std::max(1, dmg_dice.max());
            }
            else //Not critical hit
            {
                dmg = std::max(1, dmg_dice.roll());
            }

            if (attacker && is_attacker_aware && !is_defender_aware)
            {
                //Backstab (extra damage)

                int dmg_pct = 150;

                //+50% if player and has the "Vicious" trait.
                if (attacker == map::player && player_bon::traits[(size_t)Trait::vicious])
                {
                    dmg_pct += 50;
                }

                //Double damage percent if attacking with a dagger.
                if (wpn.data().id == Item_id::dagger)
                {
                    dmg_pct *= 2;
                }

                dmg         = (dmg * dmg_pct) / 100;
                is_backstab = true;
            }
        }
    }
}

Ranged_att_data::Ranged_att_data(Actor* const attacker,
                                 const P& attacker_orign,
                                 const P& aim_pos,
                                 const P& cur_pos,
                                 const Wpn& wpn,
                                 Actor_size aim_lvl) :
    Att_data            (attacker, nullptr, wpn),
    aim_pos             (aim_pos),
    hit_chance_tot      (0),
    intended_aim_lvl    (Actor_size::none),
    defender_size       (Actor_size::none),
    verb_player_attacks (wpn.data().ranged.att_msgs.player),
    verb_other_attacks  (wpn.data().ranged.att_msgs.other)
{
    Actor* const actor_aimed_at = map::actor_at_pos(aim_pos);

    //If aim level parameter not given, determine intended aim level now
    if (aim_lvl == Actor_size::none)
    {
        if (actor_aimed_at)
        {
            intended_aim_lvl = actor_aimed_at->data().actor_size;
        }
        else //No actor aimed at
        {
            const bool IS_CELL_BLOCKED =
                map_parse::cell(cell_check::Blocks_projectiles(), cur_pos);

            intended_aim_lvl = IS_CELL_BLOCKED ?
                               Actor_size::humanoid : Actor_size::floor;
        }
    }
    else //Aim level already provided by parameter
    {
        intended_aim_lvl = aim_lvl;
    }

    defender = map::actor_at_pos(cur_pos);

    if (defender && defender != attacker)
    {
        TRACE_VERBOSE << "Defender found" << std::endl;

        const Actor_data_t& defender_data = defender->data();

        const P& def_pos(defender->pos);

        const int ATT_SKILL     = attacker ? attacker->ability(Ability_id::ranged, true) : 50;

        const int WPN_MOD       = wpn.data().ranged.hit_chance_mod;

        const int DIST_TO_TGT   = king_dist(attacker_orign, def_pos);

        const int DIST_MOD      = 15 - (DIST_TO_TGT * 5);

        const Actor_speed def_speed = defender_data.speed;

        const int SPEED_MOD = def_speed == Actor_speed::sluggish ?  20 :
                              def_speed == Actor_speed::slow     ?  10 :
                              def_speed == Actor_speed::normal   ?   0 :
                              def_speed == Actor_speed::fast     ? -10 : -30;

        defender_size       = defender_data.actor_size;

        const int SIZE_MOD  = defender_size == Actor_size::floor ? -10 : 0;

        int unaware_def_mod = 0;

        if (
            attacker->is_player() &&
            static_cast<Mon*>(defender)->aware_counter_ <= 0)
        {
            unaware_def_mod = 25;
        }

        hit_chance_tot = ATT_SKILL  +
                         WPN_MOD    +
                         DIST_MOD   +
                         SPEED_MOD  +
                         SIZE_MOD   +
                         unaware_def_mod;

        set_constr_in_range(5, hit_chance_tot, 99);

        att_result = ability_roll::roll(hit_chance_tot, attacker);

        if (att_result >= success)
        {
            TRACE_VERBOSE << "Attack roll succeeded" << std::endl;

            const bool APPLY_UNDEAD_BANE_BON = attacker == map::player &&
                                               player_bon::gets_undead_bane_bon(defender_data);

            if (defender->has_prop(Prop_id::ethereal) && !APPLY_UNDEAD_BANE_BON)
            {
                is_ethereal_defender_missed = rnd::fraction(2, 3);
            }

            bool player_aim_x3 = false;

            if (attacker == map::player)
            {
                const Prop* const prop = attacker->prop_handler().prop(Prop_id::aiming);

                if (prop)
                {
                    player_aim_x3 = static_cast<const Prop_aiming*>(prop)->is_max_ranged_dmg();
                }
            }

            Dice_param dmg_dice = wpn.dmg(Att_mode::ranged, attacker);

            if (attacker == map::player && player_bon::gets_undead_bane_bon(defender_data))
            {
                dmg_dice.plus += 2;
            }

            dmg = player_aim_x3 ? dmg_dice.max() : dmg_dice.roll();

            //Outside effective range limit?
            if (!wpn.is_in_effective_range_lmt(attacker_orign, defender->pos))
            {
                TRACE_VERBOSE << "Outside effetive range limit" << std::endl;
                dmg = std::max(1, dmg / 2);
            }
        }
    }
}

Throw_att_data::Throw_att_data(Actor* const attacker,
                               const P& aim_pos,
                               const P& cur_pos,
                               const Item& item,
                               Actor_size aim_lvl) :
    Att_data            (attacker, nullptr, item),
    hit_chance_tot      (0),
    intended_aim_lvl    (Actor_size::none),
    defender_size       (Actor_size::none)
{
    Actor* const actor_aimed_at = map::actor_at_pos(aim_pos);

    //If aim level parameter not given, determine intended aim level now
    if (aim_lvl == Actor_size::none)
    {
        if (actor_aimed_at)
        {
            intended_aim_lvl = actor_aimed_at->data().actor_size;
        }
        else //Not aiming at actor
        {
            const bool IS_CELL_BLOCKED =
                map_parse::cell(cell_check::Blocks_projectiles(), cur_pos);

            intended_aim_lvl = IS_CELL_BLOCKED ?
                               Actor_size::humanoid : Actor_size::floor;
        }
    }
    else //Aim level already provided by parameter
    {
        intended_aim_lvl = aim_lvl;
    }

    defender = map::actor_at_pos(cur_pos);

    if (defender && defender != attacker)
    {
        TRACE_VERBOSE << "Defender found" << std::endl;

        const Actor_data_t& defender_data   = defender->data();

        const int           ATT_SKILL       = attacker->ability(Ability_id::ranged, true);

        const int           WPN_MOD         = item.data().ranged.throw_hit_chance_mod;

        const P&          att_pos(attacker->pos);
        const P&          def_pos(defender->pos);

        const int DIST_TO_TGT = king_dist(att_pos.x, att_pos.y, def_pos.x, def_pos.y);

        const int           DIST_MOD        = 15 - (DIST_TO_TGT * 5);
        const Actor_speed   def_speed       = defender_data.speed;

        const int SPEED_MOD = def_speed == Actor_speed::sluggish ?  20 :
                              def_speed == Actor_speed::slow     ?  10 :
                              def_speed == Actor_speed::normal   ?   0 :
                              def_speed == Actor_speed::fast     ? -15 : -35;

        defender_size       = defender_data.actor_size;

        const int SIZE_MOD  = defender_size == Actor_size::floor ? -15 : 0;

        int unaware_def_mod = 0;

        if (
            attacker->is_player() &&
            static_cast<Mon*>(defender)->aware_counter_ <= 0)
        {
            unaware_def_mod = 25;
        }

        hit_chance_tot = ATT_SKILL  +
                         WPN_MOD    +
                         DIST_MOD   +
                         SPEED_MOD  +
                         SIZE_MOD   +
                         unaware_def_mod;

        set_constr_in_range(5, hit_chance_tot, 99);

        att_result = ability_roll::roll(hit_chance_tot, attacker);

        if (att_result >= success)
        {
            TRACE_VERBOSE << "Attack roll succeeded" << std::endl;

            const bool APPLY_UNDEAD_BANE_BON = attacker == map::player &&
                                               player_bon::gets_undead_bane_bon(defender_data);

            if (defender->has_prop(Prop_id::ethereal) && !APPLY_UNDEAD_BANE_BON)
            {
                is_ethereal_defender_missed = rnd::fraction(2, 3);
            }

            bool player_aim_x3 = false;

            if (attacker == map::player)
            {
                const Prop* const prop = attacker->prop_handler().prop(Prop_id::aiming);

                if (prop)
                {
                    player_aim_x3 = static_cast<const Prop_aiming*>(prop)->is_max_ranged_dmg();
                }
            }

            Dice_param dmg_dice = item.dmg(Att_mode::thrown, attacker);

            if (APPLY_UNDEAD_BANE_BON)
            {
                dmg_dice.plus += 2;
            }

            dmg = player_aim_x3 ? dmg_dice.max() : dmg_dice.roll();

            //Outside effective range limit?
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

void print_melee_msg_and_mk_snd(const Melee_att_data& att_data, const Wpn& wpn)
{
    ASSERT(att_data.defender);

    std::string other_name      = "";
    std::string snd_msg         = "";
    auto        snd_alerts_mon  = Alerts_mon::no;

    if ((int(wpn.data().weight) > int(Item_weight::light)) && !att_data.is_intrinsic_att)
    {
        snd_alerts_mon = Alerts_mon::yes;
    }

    //Only print a message if player is not involved
    if (att_data.defender != map::player && att_data.attacker != map::player)
    {
        //TODO: This message is not always appropriate (e.g. spear traps)
        snd_msg = "I hear fighting.";
    }

    if (att_data.is_defender_dodging)
    {
        //---------------------------------------------------------- DEFENDER DODGES
        if (att_data.defender->is_player())
        {
            std::string msg = "I dodge";

            if (att_data.attacker)
            {
                if (map::player->can_see_actor(*att_data.attacker))
                {
                    other_name = att_data.attacker->name_the();
                }
                else //Player cannot see attacker
                {
                    other_name = "it";
                }

                msg += " an attack from " + other_name;
            }

            msg_log::add(msg + ".", clr_msg_good);
        }
        else //Defender is monster
        {
            const bool PLAYER_SEE_DEFENDER = map::player->can_see_actor(*att_data.defender);

            if (att_data.attacker == map::player)
            {
                if (PLAYER_SEE_DEFENDER)
                {
                    other_name = att_data.defender->name_the();
                }
                else //Player cannot see defender
                {
                    other_name = "It";
                }

                msg_log::add(other_name + " dodges my attack.");
            }
            else //Attacker is not player
            {
                //Only print message if there is no attacker actor (no messages for monsters
                //fighting each other)
                if (!att_data.attacker && PLAYER_SEE_DEFENDER)
                {
                    other_name = att_data.defender->name_the();

                    msg_log::add(other_name + " dodges.");
                }
            }
        }

        Snd snd(snd_msg, wpn.data().melee.miss_sfx,
                Ignore_msg_if_origin_seen::yes,
                att_data.defender->pos,
                att_data.attacker,
                Snd_vol::low,
                snd_alerts_mon);

        snd_emit::run(snd);
    }
    else if (att_data.att_result <= fail)
    {
        Sfx_id sfx = wpn.data().melee.miss_sfx;

        //---------------------------------------------------------- BAD AIMING
        if (att_data.attacker == map::player)
        {
            if (att_data.att_result == fail_critical)
            {
                msg_log::add("I botch the attack completely!");

                sfx = Sfx_id::END;
            }
            else //Not critical fail
            {
                msg_log::add("I miss.");
            }
        }
        else if (att_data.attacker) //Attacker is monster
        {
            if (att_data.defender->is_player())
            {
                if (map::player->can_see_actor(*att_data.attacker))
                {
                    other_name = att_data.attacker->name_the();
                }
                else //Player cannot see attacker
                {
                    other_name = "It";
                }

                msg_log::add(other_name + " misses me.", clr_white, true);
            }
        };

        Snd snd(snd_msg, sfx, Ignore_msg_if_origin_seen::yes, att_data.defender->pos,
                att_data.attacker, Snd_vol::low, snd_alerts_mon);

        snd_emit::run(snd);
    }
    else //Aim is ok
    {
        if (att_data.is_ethereal_defender_missed)
        {
            //---------------------------------------------------------- MISSED ETHEREAL TARGET
            if (att_data.attacker == map::player)
            {
                if (map::player->can_see_actor(*att_data.defender))
                {
                    other_name = att_data.defender->name_the();
                }
                else //Cannot see defender
                {
                    other_name = "it";
                }

                msg_log::add("My attack passes right through " + other_name + "!");
            }
            else if (att_data.attacker) //Attacker is monster
            {
                if (att_data.defender->is_player())
                {
                    if (map::player->can_see_actor(*att_data.attacker))
                    {
                        other_name = att_data.attacker->name_the();
                    }
                    else //Player cannot see attacker
                    {
                        other_name = "it";
                    }

                    msg_log::add("The attack of " + other_name + " passes right through me!",
                                 clr_msg_good);
                }
            }

            Snd snd(snd_msg,
                    wpn.data().melee.miss_sfx,
                    Ignore_msg_if_origin_seen::yes,
                    att_data.defender->pos,
                    att_data.attacker,
                    Snd_vol::low,
                    snd_alerts_mon);

            snd_emit::run(snd);
        }
        else //Target was hit (not ethereal)
        {
            //---------------------------------------------------------- ATTACK HITS TARGET
            //Determine the relative "size" of the hit
            const Dice_param dmg_dice = wpn.dmg(Att_mode::melee, att_data.attacker);

            const int MAX_DMG = dmg_dice.max();

            Melee_hit_size hit_size = Melee_hit_size::small;

            if (MAX_DMG >= 4)
            {
                if (att_data.dmg > (MAX_DMG * 5) / 6)
                {
                    hit_size = Melee_hit_size::hard;
                }
                else if (att_data.dmg >  MAX_DMG / 2)
                {
                    hit_size = Melee_hit_size::medium;
                }
            }

            //Punctuation depends on attack strength
            std::string dmg_punct = ".";

            switch (hit_size)
            {
            case Melee_hit_size::small:
                break;

            case Melee_hit_size::medium:
                dmg_punct = "!";
                break;

            case Melee_hit_size::hard:
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
                else //Player cannot see defender
                {
                    other_name = "it";
                }

                if (att_data.is_intrinsic_att)
                {
                    const std::string ATT_MOD_STR = att_data.is_weak_attack ? " feebly" : "";

                    msg_log::add("I " + wpn_verb + " " + other_name + ATT_MOD_STR + dmg_punct,
                                 clr_msg_good);
                }
                else //Not intrinsic attack
                {
                    const std::string ATT_MOD_STR = att_data.is_weak_attack ? "feebly "    :
                                                    att_data.is_backstab    ? "covertly "  : "";

                    const Clr clr = att_data.is_backstab ? clr_blue_lgt : clr_msg_good;

                    const std::string wpn_name_a = wpn.name(Item_ref_type::a, Item_ref_inf::none);

                    msg_log::add("I " + wpn_verb + " " + other_name + " " + ATT_MOD_STR +
                                 "with " + wpn_name_a + dmg_punct, clr);
                }
            }
            else if (att_data.attacker) //Attacker is monster
            {
                if (att_data.defender->is_player())
                {
                    const std::string wpn_verb = wpn.data().melee.att_msgs.other;

                    if (map::player->can_see_actor(*att_data.attacker))
                    {
                        other_name = att_data.attacker->name_the();
                    }
                    else //Player cannot see attacker
                    {
                        other_name = "It";
                    }

                    msg_log::add(other_name + " " + wpn_verb + dmg_punct, clr_msg_bad, true);
                }
            }
            else //No attacker (e.g. trap attack)
            {
                if (att_data.defender == map::player)
                {
                    msg_log::add("I am hit" + dmg_punct, clr_msg_bad, true);
                }
                else //Defender is monster
                {
                    if (map::player->can_see_actor(*att_data.defender))
                    {
                        other_name = att_data.defender->name_the();

                        Clr msg_clr = clr_msg_good;

                        if (map::player->is_leader_of(att_data.defender))
                        {
                            //Monster is allied to player, use a neutral color
                            //instead (we do not use red color here, since that
                            //is reserved for player taking damage).
                            msg_clr = clr_white;
                        }

                        msg_log::add(other_name + " is hit" + dmg_punct, msg_clr, true);
                    }
                }
            }

            Sfx_id hit_sfx = Sfx_id::END;

            switch (hit_size)
            {
            case Melee_hit_size::small:
                hit_sfx = wpn.data().melee.hit_small_sfx;
                break;

            case Melee_hit_size::medium:
                hit_sfx = wpn.data().melee.hit_medium_sfx;
                break;

            case Melee_hit_size::hard:
                hit_sfx = wpn.data().melee.hit_hard_sfx;
                break;
            }

            Snd snd(snd_msg, hit_sfx,
                    Ignore_msg_if_origin_seen::yes,
                    att_data.defender->pos,
                    att_data.attacker,
                    Snd_vol::low,
                    snd_alerts_mon);

            snd_emit::run(snd);
        }
    }
}

void print_ranged_init_msgs(const Ranged_att_data& data)
{
    if (!data.attacker)
    {
        //No attacker actor (e.g. a trap firing a dart)
        return;
    }

    if (data.attacker == map::player)
    {
        //Player is attacking
        msg_log::add("I " + data.verb_player_attacks + ".");
    }
    else //Attacker is monster
    {
        //Only print message if aiming at player
        if (data.aim_pos == map::player->pos)
        {
            const P& p = data.attacker->pos;

            if (map::cells[p.x][p.y].is_seen_by_player)
            {
                const std::string attacker_name = data.attacker->name_the();
                const std::string attack_verb   = data.verb_other_attacks;

                msg_log::add(attacker_name + " " + attack_verb + ".",
                             clr_white,
                             true);
            }
        }
    }
}

void print_proj_at_actor_msgs(const Ranged_att_data& data,
                              const bool IS_HIT,
                              const Wpn& wpn)
{
    ASSERT(data.defender);

    //Print messages if player can see the cell (note that the player does not have to see the
    //actual actor being hit - it will simply say "it" is hit otherwise)

    const P& defender_pos = data.defender->pos;

    if (IS_HIT && map::cells[defender_pos.x][defender_pos.y].is_seen_by_player)
    {
        //Punctuation depends on attack strength
        const Dice_param dmg_dice = wpn.dmg(Att_mode::ranged, data.attacker);

        const int   MAX_DMG = dmg_dice.max();

        std::string dmg_punct = ".";

        if (MAX_DMG >= 4)
        {
            dmg_punct = data.dmg > MAX_DMG * 5 / 6 ? "!!!" :
                        data.dmg > MAX_DMG / 2     ? "!"   : dmg_punct;
        }

        if (data.defender->is_player())
        {
            msg_log::add("I am hit" + dmg_punct, clr_msg_bad, true);
        }
        else //Defender is monster
        {
            std::string other_name = "It";

            if (map::player->can_see_actor(*data.defender))
            {
                other_name = data.defender->name_the();
            }

            msg_log::add(other_name + " is hit" + dmg_punct, clr_msg_good);
        }
    }
}

void projectile_fire(Actor* const attacker,
                     const P& origin,
                     const P& aim_pos,
                     Wpn& wpn)
{
    std::vector<Projectile*> projectiles;

    const bool IS_MACHINE_GUN = wpn.data().ranged.is_machine_gun;

    const int NR_PROJECTILES = IS_MACHINE_GUN ? NR_MG_PROJECTILES : 1;

    for (int i = 0; i < NR_PROJECTILES; ++i)
    {
        Projectile* const p = new Projectile;

        auto att_data = new Ranged_att_data(attacker,
                                            origin,     //Attacker pos
                                            aim_pos,    //Aim pos
                                            origin,     //Current pos
                                            wpn);
        p->set_att_data(att_data);
        projectiles.push_back(p);
    }

    const Actor_size aim_lvl = projectiles[0]->att_data->intended_aim_lvl;

    const int DELAY = config::delay_projectile_draw() / (IS_MACHINE_GUN ? 2 : 1);

    print_ranged_init_msgs(*projectiles[0]->att_data);

    const bool stop_at_tgt = aim_lvl == Actor_size::floor;
    const int cheb_trvl_lim = 30;

    //Get projectile path
    std::vector<P> path;

    line_calc::calc_new_line(origin,
                             aim_pos,
                             stop_at_tgt,
                             cheb_trvl_lim,
                             false,
                             path);

    const Clr projectile_clr = wpn.data().ranged.projectile_clr;
    char projectile_glyph    = wpn.data().ranged.projectile_glyph;

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
        else if (
            (path[i].x > origin.x && path[i].y < origin.y) ||
            (path[i].x < origin.x && path[i].y > origin.y))
        {
            projectile_glyph = '/';
        }
        else if (
            (path[i].x > origin.x && path[i].y > origin.y) ||
            (path[i].x < origin.x && path[i].y < origin.y))
        {
            projectile_glyph = '\\';
        }
    }

    Tile_id projectile_tile = wpn.data().ranged.projectile_tile;

    if (projectile_tile == Tile_id::projectile_std_front_slash)
    {
        if (projectile_glyph == '-')
        {
            projectile_tile = Tile_id::projectile_std_dash;
        }
        else if (projectile_glyph == '|')
        {
            projectile_tile = Tile_id::projectile_std_vertical_bar;
        }
        else if (projectile_glyph == '\\')
        {
            projectile_tile = Tile_id::projectile_std_back_slash;
        }
    }

    const bool LEAVE_TRAIL = wpn.data().ranged.projectile_leaves_trail;

    const int SIZE_OF_PATH_PLUS_ONE =
        path.size() + (NR_PROJECTILES - 1) * NR_CELL_JUMPS_MG_PROJECTILES;

    for (int i = 1; i < SIZE_OF_PATH_PLUS_ONE; ++i)
    {
        for (int p_cnt = 0; p_cnt < NR_PROJECTILES; ++p_cnt)
        {
            //Current projectile's place in the path is the current global place (i)
            //minus a certain number of elements
            int path_element = i - (p_cnt * NR_CELL_JUMPS_MG_PROJECTILES);

            //Emit sound
            if (path_element == 1)
            {
                std::string snd_msg = wpn.data().ranged.snd_msg;

                const Sfx_id sfx = wpn.data().ranged.att_sfx;

                if (!snd_msg.empty())
                {
                    if (attacker == map::player)
                    {
                        snd_msg = "";
                    }

                    const Snd_vol vol = wpn.data().ranged.snd_vol;

                    Snd snd(snd_msg,
                            sfx,
                            Ignore_msg_if_origin_seen::yes,
                            origin,
                            attacker,
                            vol,
                            Alerts_mon::yes);

                    snd_emit::run(snd);
                }
            }

            Projectile* const proj = projectiles[p_cnt];

            //All the following collision checks etc are only made if the
            //projectiles current path element corresponds to an element in
            //the real path vector
            if (
                path_element >= 1               &&
                path_element < int(path.size()) &&
                !proj->is_obstructed)
            {
                proj->pos = path[path_element];

                proj->is_seen_by_player = map::cells[proj->pos.x][proj->pos.y].is_seen_by_player;

                //Get attack data again for every cell traveled through
                auto atta_data = new Ranged_att_data(attacker,
                                                     origin,    //Attacker origin
                                                     aim_pos,   //Aim pos
                                                     proj->pos, //Cur pos
                                                     wpn,
                                                     aim_lvl);

                proj->set_att_data(atta_data);

                const P draw_pos(proj->pos);

                //HIT ACTOR?
                const bool IS_ACTOR_AIMED_FOR = proj->pos == aim_pos;

                const auto& att_data = *proj->att_data;

                if (
                    att_data.defender                       &&
                    !proj->is_obstructed                    &&
                    !att_data.is_ethereal_defender_missed   &&
                    att_data.att_result >= success          &&
                    (att_data.defender_size >= Actor_size::humanoid ||
                     IS_ACTOR_AIMED_FOR))
                {
                    //RENDER ACTOR HIT
                    if (proj->is_seen_by_player)
                    {
                        if (config::is_tiles_mode())
                        {
                            proj->set_tile(Tile_id::blast1, clr_red_lgt);

                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);

                            sdl_wrapper::sleep(DELAY / 2);

                            proj->set_tile(Tile_id::blast2, clr_red_lgt);

                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);

                            sdl_wrapper::sleep(DELAY / 2);
                        }
                        else //Text mode
                        {
                            proj->set_glyph('*', clr_red_lgt);

                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);

                            sdl_wrapper::sleep(DELAY);
                        }

                        //MESSAGES FOR ACTOR HIT
                        print_proj_at_actor_msgs(att_data, true, wpn);
                        //Need to draw again here to show log message
                        render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                    }

                    proj->is_done_rendering     = true;
                    proj->is_obstructed         = true;
                    proj->actor_hit             = att_data.defender;
                    proj->obstructed_in_element = path_element;

                    const Actor_died died =
                        proj->actor_hit->hit(att_data.dmg, wpn.data().ranged.dmg_type);

                    //NOTE: This is run regardless of if defender died or not,
                    //it is the hook implementors responsibility to check this
                    //if it matters.
                    wpn.on_ranged_hit(*proj->actor_hit);

                    if (died == Actor_died::no)
                    {
                        //Hit properties
                        Prop_handler& defender_prop_handler = proj->actor_hit->prop_handler();

                        defender_prop_handler.try_add_from_att(wpn, false);

                        //Knock-back?
                        if (wpn.data().ranged.knocks_back)
                        {
                            const Att_data* const cur_data = proj->att_data;

                            if (cur_data->att_result >= success)
                            {
                                const bool IS_SPIKE_GUN = wpn.data().id == Item_id::spike_gun;

                                knock_back::try_knock_back(*(cur_data->defender),
                                                           cur_data->attacker->pos,
                                                           IS_SPIKE_GUN);
                            }
                        }
                    }
                }

                //Projectile hit feature?
                std::vector<Mob*> mobs;
                game_time::mobs_at_pos(proj->pos, mobs);
                Feature* feature_blocking_shot = nullptr;

                for (auto* mob : mobs)
                {
                    if (!mob->is_projectile_passable()) {feature_blocking_shot = mob;}
                }

                Rigid* rigid = map::cells[proj->pos.x][proj->pos.y].rigid;

                if (!rigid->is_projectile_passable())
                {
                    feature_blocking_shot = rigid;
                }

                if (feature_blocking_shot && !proj->is_obstructed)
                {
                    proj->obstructed_in_element = path_element - 1;
                    proj->is_obstructed = true;

                    if (wpn.data().ranged.makes_ricochet_snd)
                    {
                        Snd snd("I hear a ricochet.",
                                Sfx_id::ricochet,
                                Ignore_msg_if_origin_seen::yes,
                                proj->pos,
                                nullptr,
                                Snd_vol::low,
                                Alerts_mon::yes);

                        snd_emit::run(snd);
                    }

                    //RENDER FEATURE HIT
                    if (proj->is_seen_by_player)
                    {
                        if (config::is_tiles_mode())
                        {
                            proj->set_tile(Tile_id::blast1, clr_yellow);
                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            sdl_wrapper::sleep(DELAY / 2);
                            proj->set_tile(Tile_id::blast2, clr_yellow);
                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            sdl_wrapper::sleep(DELAY / 2);
                        }
                        else //Text mode
                        {
                            proj->set_glyph('*', clr_yellow);
                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            sdl_wrapper::sleep(DELAY);
                        }
                    }
                }

                //PROJECTILE HIT THE GROUND?
                if (
                    proj->pos == aim_pos && aim_lvl == Actor_size::floor &&
                    !proj->is_obstructed)
                {
                    proj->is_obstructed = true;
                    proj->obstructed_in_element = path_element;

                    if (wpn.data().ranged.makes_ricochet_snd)
                    {
                        Snd snd("I hear a ricochet.",
                                Sfx_id::ricochet,
                                Ignore_msg_if_origin_seen::yes,
                                proj->pos,
                                nullptr,
                                Snd_vol::low,
                                Alerts_mon::yes);

                        snd_emit::run(snd);
                    }

                    //RENDER GROUND HITS
                    if (proj->is_seen_by_player)
                    {
                        if (config::is_tiles_mode())
                        {
                            proj->set_tile(Tile_id::blast1, clr_yellow);
                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            sdl_wrapper::sleep(DELAY / 2);
                            proj->set_tile(Tile_id::blast2, clr_yellow);
                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            sdl_wrapper::sleep(DELAY / 2);
                        }
                        else //Text mode
                        {
                            proj->set_glyph('*', clr_yellow);
                            render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            sdl_wrapper::sleep(DELAY);
                        }
                    }
                }

                //RENDER FLYING PROJECTILES
                if (!proj->is_obstructed && proj->is_seen_by_player)
                {
                    if (config::is_tiles_mode())
                    {
                        proj->set_tile(projectile_tile, projectile_clr);
                        render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                    }
                    else //Text mode
                    {
                        proj->set_glyph(projectile_glyph, projectile_clr);
                        render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                    }
                }
            }
        } //End projectile loop

        //If any projectile can be seen and not obstructed, delay
        for (Projectile* projectile : projectiles)
        {
            const P& pos = projectile->pos;

            if (
                map::cells[pos.x][pos.y].is_seen_by_player &&
                !projectile->is_obstructed)
            {
                sdl_wrapper::sleep(DELAY);
                break;
            }
        }

        //Check if all projectiles obstructed
        bool is_all_obstructed = true;

        for (Projectile* projectile : projectiles)
        {
            if (!projectile->is_obstructed)
            {
                is_all_obstructed = false;
            }
        }

        if (is_all_obstructed) {break;}

    } //End path-loop

    //So far, only projectile 0 can have special obstruction events
    //Must be changed if something like an assault-incinerator is added
    const Projectile* const first_projectile = projectiles[0];

    if (first_projectile->is_obstructed)
    {
        const int element = first_projectile->obstructed_in_element;
        const P& pos = path[element];
        wpn.on_projectile_blocked(pos, first_projectile->actor_hit);
    }
    else
    {
        wpn.on_projectile_blocked(aim_pos, first_projectile->actor_hit);
    }

    //Cleanup
    for (Projectile* projectile : projectiles)
    {
        delete projectile;
    }

    render::draw_map_state();
}

void shotgun(Actor& attacker, const Wpn& wpn, const P& aim_pos)
{
    Ranged_att_data data = Ranged_att_data(&attacker,
                                           attacker.pos,
                                           aim_pos,
                                           attacker.pos,
                                           wpn);

    print_ranged_init_msgs(data);

    const Actor_size intended_aim_lvl = data.intended_aim_lvl;

    bool feature_blockers[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_projectiles(), feature_blockers);

    Actor* actor_array[MAP_W][MAP_H];
    map::mk_actor_array(actor_array);

    const P origin = attacker.pos;
    std::vector<P> path;
    line_calc::calc_new_line(origin, aim_pos, false, 9999, false, path);

    int nr_actors_hit = 0;

    int killed_mon_idx = -1;

    //Emit sound
    const bool IS_ATTACKER_PLAYER = &attacker == map::player;
    std::string snd_msg = wpn.data().ranged.snd_msg;

    if (!snd_msg.empty())
    {
        if (IS_ATTACKER_PLAYER)
        {
            snd_msg = "";
        }

        const Snd_vol vol  = wpn.data().ranged.snd_vol;
        const Sfx_id sfx   = wpn.data().ranged.att_sfx;

        Snd snd(snd_msg, sfx,
                Ignore_msg_if_origin_seen::yes,
                attacker.pos,
                &attacker,
                vol,
                Alerts_mon::yes);

        snd_emit::run(snd);
    }

    for (size_t i = 1; i < path.size(); ++i)
    {
        //If travelled more than two steps after a killed monster, stop projectile.
        if (killed_mon_idx != -1 && i > (size_t)(killed_mon_idx + 1))
        {
            break;
        }

        const P cur_pos(path[i]);

        if (actor_array[cur_pos.x][cur_pos.y])
        {
            //Only attempt hit if aiming at a level that would hit the actor
            const Actor_size size_of_actor =
                actor_array[cur_pos.x][cur_pos.y]->data().actor_size;

            if (size_of_actor >= Actor_size::humanoid || cur_pos == aim_pos)
            {
                //Actor hit?
                data = Ranged_att_data(&attacker,
                                       attacker.pos,
                                       aim_pos,
                                       cur_pos,
                                       wpn,
                                       intended_aim_lvl);

                if (data.att_result >= success && !data.is_ethereal_defender_missed)
                {
                    if (map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
                    {
                        render::draw_map_state(Update_screen::no);
                        render::cover_cell_in_map(cur_pos);

                        if (config::is_tiles_mode())
                        {
                            render::draw_tile(Tile_id::blast2,
                                              Panel::map,
                                              cur_pos,
                                              clr_red_lgt);
                        }
                        else //Text mode
                        {
                            render::draw_glyph('*',
                                               Panel::map,
                                               cur_pos,
                                               clr_red_lgt);
                        }

                        render::update_screen();
                        sdl_wrapper::sleep(config::delay_shotgun());
                    }

                    //Messages
                    print_proj_at_actor_msgs(data, true, wpn);

                    //Damage
                    data.defender->hit(data.dmg, wpn.data().ranged.dmg_type);

                    ++nr_actors_hit;

                    render::draw_map_state();

                    //Special shotgun behavior:
                    //If current defender was killed, and player aimed at
                    //humanoid level or at floor level but beyond the current
                    //position, the shot will continue one cell.
                    const bool IS_TGT_KILLED = !data.defender->is_alive();

                    if (IS_TGT_KILLED && killed_mon_idx == -1)
                    {
                        killed_mon_idx = i;
                    }

                    if (
                        !IS_TGT_KILLED      ||
                        nr_actors_hit >= 2  ||
                        (intended_aim_lvl == Actor_size::floor &&
                         cur_pos == aim_pos))
                    {
                        break;
                    }
                }
            }
        }

        //Wall hit?
        if (feature_blockers[cur_pos.x][cur_pos.y])
        {
            //TODO: Check hit material, soft and wood should not cause ricochet

            Snd snd("I hear a ricochet.",
                    Sfx_id::ricochet,
                    Ignore_msg_if_origin_seen::yes,
                    cur_pos,
                    nullptr,
                    Snd_vol::low,
                    Alerts_mon::yes);

            snd_emit::run(snd);

            Cell& cell = map::cells[cur_pos.x][cur_pos.y];

            if (cell.is_seen_by_player)
            {
                render::draw_map_state(Update_screen::no);
                render::cover_cell_in_map(cur_pos);

                if (config::is_tiles_mode())
                {
                    render::draw_tile(Tile_id::blast2,
                                      Panel::map,
                                      cur_pos,
                                      clr_yellow);
                }
                else //Text mode
                {
                    render::draw_glyph('*', Panel::map, cur_pos, clr_yellow);
                }

                render::update_screen();
                sdl_wrapper::sleep(config::delay_shotgun());
                render::draw_map_state();
            }

            cell.rigid->hit(Dmg_type::physical, Dmg_method::shotgun, nullptr);

            break;
        }

        //Floor hit?
        if (intended_aim_lvl == Actor_size::floor && cur_pos == aim_pos)
        {
            Snd snd("I hear a ricochet.",
                    Sfx_id::ricochet,
                    Ignore_msg_if_origin_seen::yes,
                    cur_pos,
                    nullptr,
                    Snd_vol::low,
                    Alerts_mon::yes);

            snd_emit::run(snd);

            if (map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
            {
                render::draw_map_state(Update_screen::no);
                render::cover_cell_in_map(cur_pos);

                if (config::is_tiles_mode())
                {
                    render::draw_tile(Tile_id::blast2,
                                      Panel::map,
                                      cur_pos,
                                      clr_yellow);
                }
                else
                {
                    render::draw_glyph('*',
                                       Panel::map,
                                       cur_pos,
                                       clr_yellow);
                }

                render::update_screen();
                sdl_wrapper::sleep(config::delay_shotgun());
                render::draw_map_state();
            }

            break;
        }
    }
}

} //namespace

void melee(Actor* const attacker,
           const P& attacker_origin,
           Actor& defender,
           Wpn& wpn)
{
    const Melee_att_data att_data(attacker, defender, wpn);

    print_melee_msg_and_mk_snd(att_data, wpn);

    const bool IS_HIT = att_data.att_result >= success          &&
                        !att_data.is_ethereal_defender_missed   &&
                        !att_data.is_defender_dodging;

    if (IS_HIT)
    {
        const bool IS_RANGED_WPN = wpn.data().type == Item_type::ranged_wpn;

        const Allow_wound allow_wound = IS_RANGED_WPN ?
                                        Allow_wound::no : Allow_wound::yes;

        const auto dmg_type = wpn.data().melee.dmg_type;

        defender.hit(att_data.dmg,
                     dmg_type,
                     Dmg_method::END,
                     allow_wound);

        if (
            defender.data().can_bleed &&
            (dmg_type == Dmg_type::physical ||
             dmg_type == Dmg_type::pure     ||
             dmg_type == Dmg_type::light))
        {
            map::mk_blood(defender.pos);
        }

        //NOTE: This is run regardless of if defender died or not, it is the
        //hook implementors responsibility to check this if it matters.
        wpn.on_melee_hit(defender);

        if (defender.is_alive())
        {
            defender.prop_handler().try_add_from_att(wpn, true);

            if (wpn.data().melee.knocks_back)
            {
                knock_back::try_knock_back(defender, attacker_origin, false);
            }
        }
        else //Defender was killed
        {
            //NOTE: Destroyed actors are purged on standard turns, so it's no
            //problem calling this function even if defender was destroyed
            //(we haven't "ticked" game time yet)
            wpn.on_melee_kill(defender);
        }
    }

    //Player critically fails melee attack?
    if (attacker == map::player && att_data.att_result == fail_critical)
    {
        Player& player = *map::player;

        const int ROLL = rnd::range(1, 9);

        switch (ROLL)
        {
        //Exhausted (weakened)
        case 1:
        {
            Prop* prop = new Prop_weakened(Prop_turns::specific,
                                           rnd::range(6, 12));

            player.prop_handler().try_add(prop,
                                          Prop_src::intr,
                                          false,
                                          Verbosity::silent);

            msg_log::add("I am exhausted.",
                         clr_msg_note,
                         true,
                         More_prompt_on_msg::yes);
        }
        break;

        //Off-balance
        case 2:
        {
            msg_log::add("I am off-balance.",
                         clr_msg_note,
                         true,
                         More_prompt_on_msg::yes);

            Prop* prop = new Prop_paralyzed(Prop_turns::specific,
                                            rnd::range(1, 2));

            player.prop_handler().try_add(prop,
                                          Prop_src::intr,
                                          false,
                                          Verbosity::silent);
        }
        break;

        //Drop weaon
        case 3:
        {
            //Only drop weapon if:
            // * Player is Cursed, and
            // * Random roll (we don't want this to happen too often), and
            // * Player is attacking with wielded weapon (and not e.g. a Kick)
            if (
                player.has_prop(Prop_id::cursed)    &&
                rnd::coin_toss()                    &&
                player.inv().item_in_slot(Slot_id::wpn) == &wpn)
            {
                Item* item = player.inv().remove_from_slot(Slot_id::wpn);

                if (item)
                {
                    std::string item_name = item->name(Item_ref_type::plain,
                                                       Item_ref_inf::none);

                    bool blocked[MAP_W][MAP_H];

                    const P fov_p = player.pos;

                    R fov_rect = fov::get_fov_rect(fov_p);

                    map_parse::run(cell_check::Blocks_move_cmn(false),
                                   blocked,
                                   Map_parse_mode::overwrite,
                                   fov_rect);

                    Los_result fov_result[MAP_W][MAP_H];

                    fov::run(fov_p, blocked, fov_result);

                    std::vector<P> p_bucket;

                    for (int x = fov_rect.p0.x; x <= fov_rect.p1.x; ++x)
                    {
                        for (int y = fov_rect.p0.y; y <= fov_rect.p1.y; ++y)
                        {
                            if (!fov_result[x][y].is_blocked_hard)
                            {
                                p_bucket.push_back(P(x, y));
                            }
                        }
                    }

                    P item_p(map::player->pos);

                    if (!p_bucket.empty())
                    {
                        const int IDX = (size_t)(rnd::range(0, p_bucket.size() - 1));

                        item_p = p_bucket[IDX];
                    }

                    msg_log::add("The " + item_name + " flies from my hands!",
                                 clr_msg_note,
                                 true,
                                 More_prompt_on_msg::yes);

                    item_drop::drop_item_on_map(item_p, *item);
                }
            }
        }
        break;

        //Weapon breaks?
        case 4:
        {
            //Only break weapon if:
            // * Player is Cursed, and
            // * Random roll (we really don't want this to happen often), and
            // * Player is attacking with wielded weapon (and not e.g. a Kick)
            if (
                player.has_prop(Prop_id::cursed)    &&
                rnd::one_in(4)                      &&
                player.inv().item_in_slot(Slot_id::wpn) == &wpn)
            {
                Item* item = player.inv().remove_from_slot(Slot_id::wpn);

                if (item)
                {
                    std::string item_name =
                        item->name(Item_ref_type::plain, Item_ref_inf::none);

                    msg_log::add("My " + item_name + " breaks!",
                                 clr_msg_note,
                                 true,
                                 More_prompt_on_msg::yes);

                    delete item;
                }
            }
        }
        break;

        //Nothing happens
        default:
            break;
        }
    }

    if (att_data.attacker)
    {
        if (defender.is_player())
        {
            Mon* const mon = static_cast<Mon*>(att_data.attacker);

            mon->set_player_aware_of_me();

            mon->is_sneaking_ = false;
        }
        else //Defender is monster
        {
            static_cast<Mon&>(defender).become_aware_player(false);
        }
    }

    game_time::tick();
}

bool ranged(Actor* const attacker,
            const P& origin,
            const P& aim_pos,
            Wpn& wpn)
{
    bool did_attack = false;

    const bool HAS_INF_AMMO = wpn.data().ranged.has_infinite_ammo;

    if (wpn.data().ranged.is_shotgun)
    {
        ASSERT(attacker);

        if (wpn.nr_ammo_loaded_ != 0 || HAS_INF_AMMO)
        {
            shotgun(*attacker, wpn, aim_pos);

            did_attack = true;

            if (!HAS_INF_AMMO)
            {
                wpn.nr_ammo_loaded_ -= 1;
            }
        }
    }
    else //Not a shotgun
    {
        int nr_of_projectiles = 1;

        if (wpn.data().ranged.is_machine_gun)
        {
            nr_of_projectiles = NR_MG_PROJECTILES;
        }

        if (wpn.nr_ammo_loaded_ >= nr_of_projectiles || HAS_INF_AMMO)
        {
            projectile_fire(attacker, origin, aim_pos, wpn);

            if (map::player->is_alive())
            {
                did_attack = true;

                if (!HAS_INF_AMMO)
                {
                    wpn.nr_ammo_loaded_ -= nr_of_projectiles;
                }
            }
            else //Player is dead
            {
                return true;
            }
        }
    }

    render::draw_map_state();

    if (did_attack)
    {
        Pass_time pass_time = Pass_time::yes;

        if (attacker == map::player)
        {
            Prop_handler& props = map::player->prop_handler();

            if (
                wpn.data().type == Item_type::ranged_wpn &&
                player_bon::traits[(size_t)Trait::fast_shooter])
            {
                const bool IS_FAST_SHOOTING = props.has_prop(Prop_id::fast_shooting);

                if (IS_FAST_SHOOTING)
                {
                    pass_time = Pass_time::no;
                }
                else //Not fast shooting
                {
                    props.try_add(new Prop_fast_shooting(Prop_turns::std));
                }
            }
        }

        //Only pass time if an actor is attacking (not if it's a trap or something)
        if (attacker)
        {
            game_time::tick(pass_time);
        }

        return true;
    }

    return false;
}

} //attack
