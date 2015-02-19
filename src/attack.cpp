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
#include "utils.hpp"
#include "log.hpp"
#include "line_calc.hpp"
#include "render.hpp"
#include "sdl_wrapper.hpp"
#include "knockback.hpp"

using namespace std;

Att_data::Att_data(Actor& attacker_, const Item& att_item_) :
    attacker                  (&attacker_),
    defender                  (nullptr),
    attack_result              (fail_small),
    nr_dmg_rolls                (0),
    nr_dmg_sides                (0),
    dmg_plus                   (0),
    dmg_roll                   (0),
    dmg                       (0),
    is_intrinsic_att            (att_item_.get_data().type == Item_type::melee_wpn_intr ||
                               att_item_.get_data().type == Item_type::ranged_wpn_intr),
    is_ethereal_defender_missed  (false) {}

Melee_att_data::Melee_att_data(Actor& attacker_, const Wpn& wpn_, Actor& defender_) :
    Att_data           (attacker_, wpn_),
    is_defender_dodging (false),
    is_backstab        (false),
    is_weak_attack      (false)
{
    defender                          = &defender_;
    const Pos&        def_pos          = defender->pos;
    bool              is_defender_aware = true;
    const Actor_data_t& defender_data    = defender->get_data();

    if (attacker->is_player())
    {
        is_defender_aware = static_cast<Mon*>(defender)->aware_counter_ > 0;
    }
    else //Attacker is monster
    {
        is_defender_aware = Map::player->can_see_actor(*attacker, nullptr) ||
                          Player_bon::traits[int(Trait::vigilant)];
    }

    if (is_defender_aware)
    {
        const int DEFENDER_DODGE_SKILL =
            defender_data.ability_vals.get_val(Ability_id::dodge_att, true, *defender);

        const int DODGE_MOD_AT_FEATURE =
            Map::cells[def_pos.x][def_pos.y].rigid->get_dodge_modifier();

        const int DODGE_CHANCE_TOT = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

        if (DODGE_CHANCE_TOT > 0)
        {
            is_defender_dodging = Ability_roll::roll(DODGE_CHANCE_TOT) >= success_small;
        }
    }

    if (!is_defender_dodging)
    {
        //--------------------------------------- DETERMINE ATTACK RESULT
        const int ATTACKER_SKILL      = attacker->get_data().ability_vals.get_val(
                                            Ability_id::melee, true, *attacker);

        const int WPN_HIT_CHANCE_MOD  = wpn_.get_data().melee.hit_chance_mod;

        int hit_chance_tot              = ATTACKER_SKILL + WPN_HIT_CHANCE_MOD;

        bool is_attacker_aware = true;
        if (attacker->is_player())
        {
            is_attacker_aware = Map::player->can_see_actor(*defender, nullptr);
        }
        else
        {
            Mon* const mon = static_cast<Mon*>(attacker);
            is_attacker_aware = mon->aware_counter_ > 0;
        }

        Prop_handler& def_prop_hlr = defender->get_prop_handler();
        bool def_props[size_t(Prop_id::END)];
        def_prop_hlr.get_prop_ids(def_props);

        //If attacker is aware of the defender, check
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
                const auto* const f = Map::cells[def_pos.x][def_pos.y].rigid;
                if (f->get_id() == Feature_id::trap)
                {
                    const auto* const t = static_cast<const Trap*>(f);
                    if (t->get_trap_type() == Trap_id::web)
                    {
                        const auto* const web =
                            static_cast<const Trap_web*>(t->get_specific_trap());
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
                    def_props[int(Prop_id::paralyzed)] ||
                    def_props[int(Prop_id::nailed)]    ||
                    def_props[int(Prop_id::fainted)])
                {
                    //Give big attack bonus if defender is completely unable to fight.
                    is_big_att_bon = true;
                }
                else if (
                    def_props[int(Prop_id::confused)] ||
                    def_props[int(Prop_id::slowed)]   ||
                    def_props[int(Prop_id::burning)])
                {
                    //Give small attack bonus if defender has problems fighting.
                    is_small_att_bon = true;
                }
            }

            //Give small attack bonus if defender cannot see.
            if (!is_big_att_bon && !is_small_att_bon)
            {
                if (!def_prop_hlr.allow_see()) {is_small_att_bon = true;}
            }

            //Apply the hit chance bonus (if any)
            hit_chance_tot += is_big_att_bon ? 50 : (is_small_att_bon ? 20 : 0);
        }

        attack_result = Ability_roll::roll(hit_chance_tot);

        //Ethereal target missed?
        if (
            def_props[int(Prop_id::ethereal)] &&
            !Player_bon::gets_undead_bane_bon(*attacker, defender_data))
        {
            is_ethereal_defender_missed = Rnd::fraction(2, 3);
        }

        //--------------------------------------- DETERMINE DAMAGE
        nr_dmg_rolls  = wpn_.get_data().melee.dmg.first;
        nr_dmg_sides  = wpn_.get_data().melee.dmg.second;
        dmg_plus     = wpn_.melee_dmg_plus_;

        if (Player_bon::gets_undead_bane_bon(*attacker, defender_data))
        {
            dmg_plus += 2;
        }

        bool att_props[size_t(Prop_id::END)];
        attacker->get_prop_handler().get_prop_ids(att_props);

        if (att_props[int(Prop_id::weakened)])
        {
            //Weak attack (min damage)
            dmg_roll       = nr_dmg_rolls;
            dmg           = dmg_roll + dmg_plus;
            is_weak_attack  = true;
        }
        else //Attacker not weakened
        {
            if (attack_result == success_critical)
            {
                //Critical hit (max damage)
                dmg_roll = nr_dmg_rolls * nr_dmg_sides;
                dmg     = max(0, dmg_roll + dmg_plus);
            }
            else
            {
                //Normal hit
                dmg_roll = Rnd::dice(nr_dmg_rolls, nr_dmg_sides);
                dmg     = max(0, dmg_roll + dmg_plus);
            }

            if (is_attacker_aware && !is_defender_aware)
            {
                //Backstab (extra damage)

                int dmg_pct = 150;

                //Double damage percent if attacking with a dagger.
                if (wpn_.get_data().id == Item_id::dagger) {dmg_pct *= 2;}

                //+50% if player and has the "Vicious" trait.
                if (attacker == Map::player)
                {
                    if (Player_bon::traits[int(Trait::vicious)])
                    {
                        dmg_pct += 50;
                    }
                }

                dmg         = ((dmg_roll + dmg_plus) * dmg_pct) / 100;
                is_backstab  = true;
            }
        }
    }
}

Ranged_att_data::Ranged_att_data(Actor&         attacker_,
                             const Wpn&     wpn_,
                             const Pos&     aim_pos_,
                             const Pos&     cur_pos_,
                             Actor_size      intended_aim_lvl_) :
    Att_data           (attacker_, wpn_),
    hit_chance_tot      (0),
    intended_aim_lvl    (Actor_size::none),
    defender_size      (Actor_size::none),
    verb_player_attacks (wpn_.get_data().ranged.att_msgs.player),
    verb_other_attacks  (wpn_.get_data().ranged.att_msgs.other)
{
    Actor* const actor_aimed_at = Utils::get_actor_at_pos(aim_pos_);

    //If aim level parameter not given, determine it now
    if (intended_aim_lvl_ == Actor_size::none)
    {
        if (actor_aimed_at)
        {
            intended_aim_lvl = actor_aimed_at->get_data().actor_size;
        }
        else
        {
            bool blocked[MAP_W][MAP_H];
            Map_parse::run(Cell_check::Blocks_projectiles(), blocked);
            intended_aim_lvl = blocked[cur_pos_.x][cur_pos_.y] ?
                             Actor_size::humanoid : Actor_size::floor;
        }
    }
    else //Aim level was already set
    {
        intended_aim_lvl = intended_aim_lvl_;
    }

    defender = Utils::get_actor_at_pos(cur_pos_);

    if (defender)
    {
        TRACE << "Defender found" << endl;

        const Actor_data_t& defender_data = defender->get_data();

        const int ATTACKER_SKILL    = attacker->get_data().ability_vals.get_val(
                                          Ability_id::ranged, true, *attacker);
        const int WPN_MOD           = wpn_.get_data().ranged.hit_chance_mod;
        const Pos& att_pos(attacker->pos);
        const Pos& def_pos(defender->pos);
        const int DIST_TO_TGT       = Utils::king_dist(
                                          att_pos.x, att_pos.y, def_pos.x, def_pos.y);
        const int DIST_MOD          = 15 - (DIST_TO_TGT * 5);
        const Actor_speed def_speed   = defender_data.speed;
        const int SPEED_MOD =
            def_speed == Actor_speed::sluggish ?  20 :
            def_speed == Actor_speed::slow     ?  10 :
            def_speed == Actor_speed::normal   ?   0 :
            def_speed == Actor_speed::fast     ? -10 : -30;
        defender_size                = defender_data.actor_size;
        const int SIZE_MOD          = defender_size == Actor_size::floor ? -10 : 0;

        int unaware_def_mod = 0;
        const bool IS_ROGUE = Player_bon::get_bg() == Bg::rogue;
        if (attacker == Map::player && defender != Map::player && IS_ROGUE)
        {
            if (static_cast<Mon*>(defender)->aware_counter_ <= 0)
            {
                unaware_def_mod = 25;
            }
        }

        hit_chance_tot = max(5,
                           ATTACKER_SKILL +
                           WPN_MOD    +
                           DIST_MOD   +
                           SPEED_MOD  +
                           SIZE_MOD   +
                           unaware_def_mod);

        constr_in_range(5, hit_chance_tot, 99);

        attack_result = Ability_roll::roll(hit_chance_tot);

        if (attack_result >= success_small)
        {
            TRACE << "Attack roll succeeded" << endl;

            bool props[size_t(Prop_id::END)];
            defender->get_prop_handler().get_prop_ids(props);

            if (
                props[int(Prop_id::ethereal)] &&
                !Player_bon::gets_undead_bane_bon(*attacker, defender_data))
            {
                is_ethereal_defender_missed = Rnd::fraction(2, 3);
            }

            bool player_aim_x3 = false;
            if (attacker->is_player())
            {
                const Prop* const prop =
                    attacker->get_prop_handler().get_prop(Prop_id::aiming, Prop_src::applied);
                if (prop)
                {
                    player_aim_x3 = static_cast<const Prop_aiming*>(prop)->is_max_ranged_dmg();
                }
            }

            nr_dmg_rolls  = wpn_.get_data().ranged.dmg.rolls;
            nr_dmg_sides  = wpn_.get_data().ranged.dmg.sides;
            dmg_plus     = wpn_.get_data().ranged.dmg.plus;

            if (Player_bon::gets_undead_bane_bon(*attacker, defender_data))
            {
                dmg_plus += 2;
            }

            dmg_roll = player_aim_x3 ? (nr_dmg_rolls * nr_dmg_sides) :
                      Rnd::dice(nr_dmg_rolls, nr_dmg_sides);

            //Outside effective range limit?
            if (!wpn_.is_in_effective_range_lmt(attacker->pos, defender->pos))
            {
                TRACE << "Outside effetive range limit" << endl;
                dmg_roll = max(1, dmg_roll / 2);
                dmg_plus /= 2;
            }

            dmg = dmg_roll + dmg_plus;
        }
    }
}

Throw_att_data::Throw_att_data(Actor&       attacker_,
                           const Item&  item_,
                           const Pos&   aim_pos_,
                           const Pos&   cur_pos_,
                           Actor_size    intended_aim_lvl_) :
    Att_data         (attacker_, item_),
    hit_chance_tot    (0),
    intended_aim_lvl  (Actor_size::none),
    defender_size    (Actor_size::none)
{
    Actor* const actor_aimed_at = Utils::get_actor_at_pos(aim_pos_);

    //If aim level parameter not given, determine it now
    if (intended_aim_lvl_ == Actor_size::none)
    {
        if (actor_aimed_at)
        {
            intended_aim_lvl = actor_aimed_at->get_data().actor_size;
        }
        else
        {
            bool blocked[MAP_W][MAP_H];
            Map_parse::run(Cell_check::Blocks_projectiles(), blocked);
            intended_aim_lvl = blocked[cur_pos_.x][cur_pos_.y] ?
                             Actor_size::humanoid : Actor_size::floor;
        }
    }
    else //Aim level was already set
    {
        intended_aim_lvl = intended_aim_lvl_;
    }

    defender = Utils::get_actor_at_pos(cur_pos_);

    if (defender)
    {
        TRACE << "Defender found" << endl;

        const Actor_data_t& defender_data = defender->get_data();

        const int ATTACKER_SKILL    = attacker->get_data().ability_vals.get_val(
                                          Ability_id::ranged, true, *attacker);
        const int WPN_MOD           = item_.get_data().ranged.throw_hit_chance_mod;
        const Pos& att_pos(attacker->pos);
        const Pos& def_pos(defender->pos);
        const int DIST_TO_TGT       = Utils::king_dist(
                                          att_pos.x, att_pos.y, def_pos.x, def_pos.y);
        const int DIST_MOD          = 15 - (DIST_TO_TGT * 5);
        const Actor_speed def_speed   = defender_data.speed;
        const int SPEED_MOD =
            def_speed == Actor_speed::sluggish ?  20 :
            def_speed == Actor_speed::slow     ?  10 :
            def_speed == Actor_speed::normal   ?   0 :
            def_speed == Actor_speed::fast     ? -15 : -35;
        defender_size                = defender_data.actor_size;
        const int SIZE_MOD          = defender_size == Actor_size::floor ? -15 : 0;

        int         unaware_def_mod = 0;
        const bool  IS_ROGUE      = Player_bon::get_bg() == Bg::rogue;

        if (attacker == Map::player && defender != Map::player && IS_ROGUE)
        {
            if (static_cast<Mon*>(defender)->aware_counter_ <= 0)
            {
                unaware_def_mod = 25;
            }
        }

        hit_chance_tot = max(5,
                           ATTACKER_SKILL +
                           WPN_MOD    +
                           DIST_MOD   +
                           SPEED_MOD  +
                           SIZE_MOD   +
                           unaware_def_mod);

        attack_result = Ability_roll::roll(hit_chance_tot);

        if (attack_result >= success_small)
        {
            TRACE << "Attack roll succeeded" << endl;

            bool props[size_t(Prop_id::END)];
            defender->get_prop_handler().get_prop_ids(props);

            if (
                props[int(Prop_id::ethereal)] &&
                !Player_bon::gets_undead_bane_bon(*attacker, defender_data))
            {
                is_ethereal_defender_missed = Rnd::fraction(2, 3);
            }

            bool player_aim_x3 = false;
            if (attacker == Map::player)
            {
                const Prop* const prop =
                    attacker->get_prop_handler().get_prop(Prop_id::aiming, Prop_src::applied);
                if (prop)
                {
                    player_aim_x3 = static_cast<const Prop_aiming*>(prop)->is_max_ranged_dmg();
                }
            }

            nr_dmg_rolls  = item_.get_data().ranged.throw_dmg.rolls;
            nr_dmg_sides  = item_.get_data().ranged.throw_dmg.sides;
            dmg_plus     = item_.get_data().ranged.throw_dmg.plus;

            if (Player_bon::gets_undead_bane_bon(*attacker, defender_data))
            {
                dmg_plus += 2;
            }

            dmg_roll = player_aim_x3 ? (nr_dmg_rolls * nr_dmg_sides) :
                      Rnd::dice(nr_dmg_rolls, nr_dmg_sides);

            //Outside effective range limit?
            if (!item_.is_in_effective_range_lmt(attacker->pos, defender->pos))
            {
                TRACE << "Outside effetive range limit" << endl;
                dmg_roll = max(1, dmg_roll / 2);
                dmg_plus /= 2;
            }

            dmg = dmg_roll + dmg_plus;
        }
    }
}

namespace attack
{

namespace
{

void print_melee_msg_and_play_sfx(const Melee_att_data& data, const Wpn& wpn)
{
    //No melee messages if player is not involved
    if (data.attacker != Map::player && data.defender != Map::player)
    {
        return;
    }

    string other_name = "";

    if (data.is_defender_dodging)
    {
        //----- DEFENDER DODGES --------
        if (data.attacker == Map::player)
        {
            if (Map::player->can_see_actor(*data.defender, nullptr))
            {
                other_name = data.defender->get_name_the();
            }
            else
            {
                other_name = "It ";
            }
            Log::add_msg(other_name + " dodges my attack.");
        }
        else //Attacker is monster
        {
            if (Map::player->can_see_actor(*data.attacker, nullptr))
            {
                other_name = data.attacker->get_name_the();
            }
            else
            {
                other_name = "It";
            }
            Log::add_msg("I dodge an attack from " + other_name + ".", clr_msg_good);
        }
    }
    else if (data.attack_result <= fail_small)
    {
        //----- BAD AIMING --------
        if (data.attacker == Map::player)
        {
            if (data.attack_result == fail_small)
            {
                Log::add_msg("I barely miss!");
            }
            else if (data.attack_result == fail_normal)
            {
                Log::add_msg("I miss.");
            }
            else if (data.attack_result == fail_big)
            {
                Log::add_msg("I miss completely.");
            }
            Audio::play(wpn.get_data().melee.miss_sfx);
        }
        else //Attacker is monster
        {
            if (Map::player->can_see_actor(*data.attacker, nullptr))
            {
                other_name = data.attacker->get_name_the();
            }
            else
            {
                other_name = "It";
            }
            if (data.attack_result == fail_small)
            {
                Log::add_msg(other_name + " barely misses me!", clr_white, true);
            }
            else if (data.attack_result == fail_normal)
            {
                Log::add_msg(other_name + " misses me.", clr_white, true);
            }
            else if (data.attack_result == fail_big)
            {
                Log::add_msg(other_name + " misses me completely.", clr_white, true);
            }
        }
    }
    else //Aim is ok
    {
        if (data.is_ethereal_defender_missed)
        {
            //----- ATTACK MISSED DUE TO ETHEREAL TARGET --------
            if (data.attacker == Map::player)
            {
                if (Map::player->can_see_actor(*data.defender, nullptr))
                {
                    other_name = data.defender->get_name_the();
                }
                else
                {
                    other_name = "It ";
                }
                Log::add_msg(
                    "My attack passes right through " + other_name + "!");
            }
            else
            {
                if (Map::player->can_see_actor(*data.attacker, nullptr))
                {
                    other_name = data.attacker->get_name_the();
                }
                else
                {
                    other_name = "It";
                }
                Log::add_msg(
                    "The attack of " + other_name + " passes right through me!",
                    clr_msg_good);
            }
        }
        else //Target was hit (not ethereal)
        {
            //----- ATTACK CONNECTS WITH DEFENDER --------
            //Determine the relative "size" of the hit
            const auto& wpn_dmg  = wpn.get_data().melee.dmg;
            const int   MAX_DMG = (wpn_dmg.first * wpn_dmg.second) + wpn.melee_dmg_plus_;

            Melee_hit_size hit_size = Melee_hit_size::small;

            if (MAX_DMG >= 4)
            {
                if (data.dmg > (MAX_DMG * 5) / 6)
                {
                    hit_size = Melee_hit_size::hard;
                }
                else if (data.dmg >  MAX_DMG / 2)
                {
                    hit_size = Melee_hit_size::medium;
                }
            }

            //Punctuation depends on attack strength
            string dmg_punct = ".";

            switch (hit_size)
            {
            case Melee_hit_size::small:                     break;
            case Melee_hit_size::medium:  dmg_punct = "!";   break;
            case Melee_hit_size::hard:    dmg_punct = "!!!"; break;
            }

            if (data.attacker == Map::player)
            {
                const string wpn_verb = wpn.get_data().melee.att_msgs.player;

                if (Map::player->can_see_actor(*data.defender, nullptr))
                {
                    other_name = data.defender->get_name_the();
                }
                else
                {
                    other_name = "it";
                }

                if (data.is_intrinsic_att)
                {
                    const string ATT_MOD_STR = data.is_weak_attack ? " feebly" : "";

                    Log::add_msg("I " + wpn_verb + " " + other_name + ATT_MOD_STR + dmg_punct,
                                clr_msg_good);
                }
                else //Not intrinsic attack
                {
                    const string ATT_MOD_STR = data.is_weak_attack  ? "feebly "    :
                                               data.is_backstab    ? "covertly "  : "";
                    const Clr     clr       = data.is_backstab ? clr_blue_lgt : clr_msg_good;
                    const string  wpn_name_a  = wpn.get_name(Item_ref_type::a,
                                                          Item_ref_inf::none);
                    Log::add_msg("I " + wpn_verb + " " + other_name + " " + ATT_MOD_STR +
                                "with " + wpn_name_a + dmg_punct, clr);
                }
            }
            else //Attacker is monster
            {
                const string wpn_verb = wpn.get_data().melee.att_msgs.other;

                if (Map::player->can_see_actor(*data.attacker, nullptr))
                {
                    other_name = data.attacker->get_name_the();
                }
                else
                {
                    other_name = "It";
                }

                Log::add_msg(other_name + " " + wpn_verb + dmg_punct, clr_msg_bad, true);
            }

            Sfx_id hit_sfx = Sfx_id::END;
            switch (hit_size)
            {
            case Melee_hit_size::small:
                hit_sfx = wpn.get_data().melee.hit_small_sfx;
                break;

            case Melee_hit_size::medium:
                hit_sfx = wpn.get_data().melee.hit_medium_sfx;
                break;

            case Melee_hit_size::hard:
                hit_sfx = wpn.get_data().melee.hit_hard_sfx;
                break;
            }
            Audio::play(hit_sfx);
        }
    }
}

void print_ranged_initiate_msgs(const Ranged_att_data& data)
{
    if (data.attacker == Map::player)
    {
        Log::add_msg("I " + data.verb_player_attacks + ".");
    }
    else
    {
        const Pos& p = data.attacker->pos;
        if (Map::cells[p.x][p.y].is_seen_by_player)
        {
            const string attacker_name = data.attacker->get_name_the();
            const string attack_verb = data.verb_other_attacks;
            Log::add_msg(attacker_name + " " + attack_verb + ".", clr_white, true);
        }
    }
}

void print_proj_at_actor_msgs(const Ranged_att_data& data, const bool IS_HIT, const Wpn& wpn)
{
    assert(data.defender);

    //Only print messages if player can see the cell
    const Pos& defender_pos = data.defender->pos;

    if (IS_HIT && Map::cells[defender_pos.x][defender_pos.y].is_seen_by_player)
    {
        //Punctuation depends on attack strength
        const auto& wpn_dmg  = wpn.get_data().ranged.dmg;
        const int   MAX_DMG = (wpn_dmg.rolls * wpn_dmg.sides) + wpn_dmg.plus;

        string dmg_punct = ".";

        if (MAX_DMG >= 4)
        {
            dmg_punct =
                data.dmg > MAX_DMG * 5 / 6 ? "!!!" :
                data.dmg > MAX_DMG / 2     ? "!"   : dmg_punct;
        }

        if (data.defender->is_player())
        {
            Log::add_msg("I am hit" + dmg_punct, clr_msg_bad, true);
        }
        else //Defender is monster
        {
            string other_name = "It";

            if (Map::player->can_see_actor(*data.defender, nullptr))
            {
                other_name = data.defender->get_name_the();
            }

            Log::add_msg(other_name + " is hit" + dmg_punct, clr_msg_good);
        }
    }
}

void projectile_fire(Actor& attacker, Wpn& wpn, const Pos& aim_pos)
{
    const bool IS_ATTACKER_PLAYER = &attacker == Map::player;

    vector<Projectile*> projectiles;

    const bool IS_MACHINE_GUN = wpn.get_data().ranged.is_machine_gun;

    const int NR_PROJECTILES = IS_MACHINE_GUN ? NR_MG_PROJECTILES : 1;

    for (int i = 0; i < NR_PROJECTILES; ++i)
    {
        Projectile* const p = new Projectile;
        p->set_att_data(new Ranged_att_data(attacker, wpn, aim_pos, attacker.pos));
        projectiles.push_back(p);
    }

    const Actor_size aim_lvl =
        projectiles[0]->attack_data->intended_aim_lvl;

    const int DELAY = Config::get_delay_projectile_draw() / (IS_MACHINE_GUN ? 2 : 1);

    print_ranged_initiate_msgs(*projectiles[0]->attack_data);

    const bool stop_at_target = aim_lvl == Actor_size::floor;
    const int cheb_trvl_lim = 30;

    //Get projectile path
    const Pos origin = attacker.pos;
    vector<Pos> path;
    Line_calc::calc_new_line(origin, aim_pos, stop_at_target, cheb_trvl_lim, false, path);

    const Clr projectile_clr = wpn.get_data().ranged.missile_clr;
    char projectile_glyph    = wpn.get_data().ranged.missile_glyph;

    if (projectile_glyph == '/')
    {
        const int i = path.size() > 2 ? 2 : 1;

        if (path[i].y == origin.y) {projectile_glyph = '-';}
        if (path[i].x == origin.x) {projectile_glyph = '|';}
        if (
            (path[i].x > origin.x && path[i].y < origin.y) ||
            (path[i].x < origin.x && path[i].y > origin.y))
        {
            projectile_glyph = '/';
        }
        if (
            (path[i].x > origin.x && path[i].y > origin.y) ||
            (path[i].x < origin.x && path[i].y < origin.y))
        {
            projectile_glyph = '\\';
        }
    }
    Tile_id projectile_tile = wpn.get_data().ranged.missile_tile;
    if (projectile_tile == Tile_id::projectile_std_front_slash)
    {
        if (projectile_glyph == '-')  {projectile_tile = Tile_id::projectile_std_dash;}
        if (projectile_glyph == '|')  {projectile_tile = Tile_id::projectile_std_vertical_bar;}
        if (projectile_glyph == '\\') {projectile_tile = Tile_id::projectile_std_back_slash;}
    }

    const bool LEAVE_TRAIL = wpn.get_data().ranged.missile_leaves_trail;

    const int SIZE_OF_PATH_PLUS_ONE =
        path.size() + (NR_PROJECTILES - 1) * NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES;

    for (int i = 1; i < SIZE_OF_PATH_PLUS_ONE; ++i)
    {
        for (int p_cnt = 0; p_cnt < NR_PROJECTILES; ++p_cnt)
        {
            //Current projectile's place in the path is the current global place (i)
            //minus a certain number of elements
            int path_element = i - (p_cnt * NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES);

            //Emit sound
            if (path_element == 1)
            {
                string snd_msg   = wpn.get_data().ranged.snd_msg;
                const Sfx_id sfx = wpn.get_data().ranged.att_sfx;
                if (!snd_msg.empty())
                {
                    if (IS_ATTACKER_PLAYER) {snd_msg = "";}

                    const Snd_vol vol = wpn.get_data().ranged.snd_vol;

                    Snd_emit::emit_snd({snd_msg, sfx, Ignore_msg_if_origin_seen::yes,
                                      attacker.pos, &attacker, vol, Alerts_mon::yes
                                     });
                }
            }

            Projectile* const cur_proj = projectiles[p_cnt];

            //All the following collision checks etc are only made if the projectiles
            //current path element corresponds to an element in the real path vector
            if (
                path_element >= 1                &&
                path_element < int(path.size())  &&
                !cur_proj->is_obstructed)
            {
                cur_proj->pos = path[path_element];

                cur_proj->is_visible_to_player =
                    Map::cells[cur_proj->pos.x][cur_proj->pos.y].is_seen_by_player;

                //Get attack data again for every cell traveled through
                cur_proj->set_att_data(
                    new Ranged_att_data(attacker, wpn, aim_pos, cur_proj->pos , aim_lvl));

                const Pos draw_pos(cur_proj->pos);

                //HIT ACTOR?
                if (
                    cur_proj->attack_data->defender &&
                    !cur_proj->is_obstructed        &&
                    !cur_proj->attack_data->is_ethereal_defender_missed)
                {
                    const bool IS_ACTOR_AIMED_FOR = cur_proj->pos == aim_pos;

                    if (
                        cur_proj->attack_data->defender_size >= Actor_size::humanoid ||
                        IS_ACTOR_AIMED_FOR)
                    {

                        if (cur_proj->attack_data->attack_result >= success_small)
                        {
                            //RENDER ACTOR HIT
                            if (cur_proj->is_visible_to_player)
                            {
                                if (Config::is_tiles_mode())
                                {
                                    cur_proj->set_tile(Tile_id::blast1, clr_red_lgt);
                                    Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                                    Sdl_wrapper::sleep(DELAY / 2);
                                    cur_proj->set_tile(Tile_id::blast2, clr_red_lgt);
                                    Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                                    Sdl_wrapper::sleep(DELAY / 2);
                                }
                                else //Not tile mode
                                {
                                    cur_proj->set_glyph('*', clr_red_lgt);
                                    Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                                    Sdl_wrapper::sleep(DELAY);
                                }

                                //MESSAGES FOR ACTOR HIT
                                print_proj_at_actor_msgs(*cur_proj->attack_data, true, wpn);
                                //Need to draw again here to show log message
                                Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            }

                            cur_proj->is_done_rendering      = true;
                            cur_proj->is_obstructed         = true;
                            cur_proj->actor_hit             = cur_proj->attack_data->defender;
                            cur_proj->obstructed_in_element  = path_element;

                            const Actor_died died =
                                cur_proj->actor_hit->hit(cur_proj->attack_data->dmg,
                                                       wpn.get_data().ranged.dmg_type);

                            if (died == Actor_died::no)
                            {
                                //Hit properties
                                Prop_handler& defender_prop_handler =
                                    cur_proj->actor_hit->get_prop_handler();

                                defender_prop_handler.try_apply_prop_from_att(wpn, false);

                                //Knock-back?
                                if (wpn.get_data().ranged.knocks_back)
                                {
                                    const Att_data* const cur_data = cur_proj->attack_data;

                                    if (cur_data->attack_result >= success_small)
                                    {
                                        const bool IS_SPIKE_GUN =
                                            wpn.get_data().id == Item_id::spike_gun;
                                        Knock_back::try_knock_back(*(cur_data->defender),
                                                                cur_data->attacker->pos,
                                                                IS_SPIKE_GUN);
                                    }
                                }
                            }
                        }
                    }
                }

                //Projectile hit feature?
                vector<Mob*> mobs;
                Game_time::get_mobs_at_pos(cur_proj->pos, mobs);
                Feature* feature_blocking_shot = nullptr;
                for (auto* mob : mobs)
                {
                    if (!mob->is_projectile_passable()) {feature_blocking_shot = mob;}
                }
                Rigid* rigid = Map::cells[cur_proj->pos.x][cur_proj->pos.y].rigid;
                if (!rigid->is_projectile_passable())
                {
                    feature_blocking_shot = rigid;
                }

                if (feature_blocking_shot && !cur_proj->is_obstructed)
                {
                    cur_proj->obstructed_in_element = path_element - 1;
                    cur_proj->is_obstructed = true;

                    if (wpn.get_data().ranged.makes_ricochet_snd)
                    {
                        Snd snd("I hear a ricochet.", Sfx_id::ricochet,
                                Ignore_msg_if_origin_seen::yes, cur_proj->pos, nullptr,
                                Snd_vol::low, Alerts_mon::yes);
                        Snd_emit::emit_snd(snd);
                    }

                    //RENDER FEATURE HIT
                    if (cur_proj->is_visible_to_player)
                    {
                        if (Config::is_tiles_mode())
                        {
                            cur_proj->set_tile(Tile_id::blast1, clr_yellow);
                            Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            Sdl_wrapper::sleep(DELAY / 2);
                            cur_proj->set_tile(Tile_id::blast2, clr_yellow);
                            Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            Sdl_wrapper::sleep(DELAY / 2);
                        }
                        else
                        {
                            cur_proj->set_glyph('*', clr_yellow);
                            Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            Sdl_wrapper::sleep(DELAY);
                        }
                    }
                }

                //PROJECTILE HIT THE GROUND?
                if (
                    cur_proj->pos == aim_pos && aim_lvl == Actor_size::floor &&
                    !cur_proj->is_obstructed)
                {
                    cur_proj->is_obstructed = true;
                    cur_proj->obstructed_in_element = path_element;

                    if (wpn.get_data().ranged.makes_ricochet_snd)
                    {
                        Snd snd("I hear a ricochet.", Sfx_id::ricochet,
                                Ignore_msg_if_origin_seen::yes, cur_proj->pos, nullptr,
                                Snd_vol::low, Alerts_mon::yes);
                        Snd_emit::emit_snd(snd);
                    }

                    //RENDER GROUND HITS
                    if (cur_proj->is_visible_to_player)
                    {
                        if (Config::is_tiles_mode())
                        {
                            cur_proj->set_tile(Tile_id::blast1, clr_yellow);
                            Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            Sdl_wrapper::sleep(DELAY / 2);
                            cur_proj->set_tile(Tile_id::blast2, clr_yellow);
                            Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            Sdl_wrapper::sleep(DELAY / 2);
                        }
                        else
                        {
                            cur_proj->set_glyph('*', clr_yellow);
                            Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                            Sdl_wrapper::sleep(DELAY);
                        }
                    }
                }

                //RENDER FLYING PROJECTILES
                if (!cur_proj->is_obstructed && cur_proj->is_visible_to_player)
                {
                    if (Config::is_tiles_mode())
                    {
                        cur_proj->set_tile(projectile_tile, projectile_clr);
                        Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                    }
                    else
                    {
                        cur_proj->set_glyph(projectile_glyph, projectile_clr);
                        Render::draw_projectiles(projectiles, !LEAVE_TRAIL);
                    }
                }
            }
        } //End projectile loop

        //If any projectile can be seen and not obstructed, delay
        for (Projectile* projectile : projectiles)
        {
            const Pos& pos = projectile->pos;
            if (
                Map::cells[pos.x][pos.y].is_seen_by_player &&
                !projectile->is_obstructed)
            {
                Sdl_wrapper::sleep(DELAY);
                break;
            }
        }

        //Check if all projectiles obstructed
        bool is_all_obstructed = true;
        for (Projectile* projectile : projectiles)
        {
            if (!projectile->is_obstructed) {is_all_obstructed = false;}
        }
        if (is_all_obstructed) {break;}

    } //End path-loop

    //So far, only projectile 0 can have special obstruction events***
    //Must be changed if something like an assault-incinerator is added
    const Projectile* const first_projectile = projectiles[0];
    if (!first_projectile->is_obstructed)
    {
        wpn.on_projectile_blocked(aim_pos, first_projectile->actor_hit);
    }
    else
    {
        const int element = first_projectile->obstructed_in_element;
        const Pos& pos = path[element];
        wpn.on_projectile_blocked(pos, first_projectile->actor_hit);
    }
    //Cleanup
    for (Projectile* projectile : projectiles) {delete projectile;}

    Render::draw_map_and_interface();
}

void shotgun(Actor& attacker, const Wpn& wpn, const Pos& aim_pos)
{
    Ranged_att_data data = Ranged_att_data(attacker, wpn, aim_pos, attacker.pos);

    print_ranged_initiate_msgs(data);

    const Actor_size intended_aim_lvl = data.intended_aim_lvl;

    bool feature_blockers[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_projectiles(), feature_blockers);

    Actor* actor_array[MAP_W][MAP_H];
    Utils::mk_actor_array(actor_array);

    const Pos origin = attacker.pos;
    vector<Pos> path;
    Line_calc::calc_new_line(origin, aim_pos, false, 9999, false, path);

    int nr_actors_hit = 0;

    int killed_mon_idx = -1;

    //Emit sound
    const bool IS_ATTACKER_PLAYER = &attacker == Map::player;
    string snd_msg = wpn.get_data().ranged.snd_msg;
    if (!snd_msg.empty())
    {
        if (IS_ATTACKER_PLAYER) {snd_msg = "";}
        const Snd_vol vol  = wpn.get_data().ranged.snd_vol;
        const Sfx_id sfx   = wpn.get_data().ranged.att_sfx;
        Snd snd(snd_msg, sfx, Ignore_msg_if_origin_seen::yes, attacker.pos, &attacker,
                vol, Alerts_mon::yes);
        Snd_emit::emit_snd(snd);
    }

    for (size_t i = 1; i < path.size(); ++i)
    {
        //If travelled more than two steps after a killed monster, stop projectile.
        if (killed_mon_idx != -1 && i > size_t(killed_mon_idx + 1))
        {
            break;
        }

        const Pos cur_pos(path[i]);

        if (actor_array[cur_pos.x][cur_pos.y])
        {
            //Only attempt hit if aiming at a level that would hit the actor
            const Actor_size size_of_actor =
                actor_array[cur_pos.x][cur_pos.y]->get_data().actor_size;

            if (size_of_actor >= Actor_size::humanoid || cur_pos == aim_pos)
            {
                //Actor hit?
                data = Ranged_att_data(attacker, wpn, aim_pos, cur_pos, intended_aim_lvl);

                if (data.attack_result >= success_small && !data.is_ethereal_defender_missed)
                {
                    if (Map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
                    {
                        Render::draw_map_and_interface(false);
                        Render::cover_cell_in_map(cur_pos);
                        if (Config::is_tiles_mode())
                        {
                            Render::draw_tile(Tile_id::blast2, Panel::map, cur_pos,
                                             clr_red_lgt);
                        }
                        else
                        {
                            Render::draw_glyph('*', Panel::map, cur_pos, clr_red_lgt);
                        }
                        Render::update_screen();
                        Sdl_wrapper::sleep(Config::get_delay_shotgun());
                    }

                    //Messages
                    print_proj_at_actor_msgs(data, true, wpn);

                    //Damage
                    data.defender->hit(data.dmg, wpn.get_data().ranged.dmg_type);

                    ++nr_actors_hit;

                    Render::draw_map_and_interface();

                    //Special shotgun behavior:
                    //If current defender was killed, and player aimed at humanoid level
                    //or at floor level but beyond the current position, the shot will
                    //continue one cell.
                    const bool IS_TGT_KILLED = !data.defender->is_alive();

                    if (IS_TGT_KILLED && killed_mon_idx == -1)
                    {
                        killed_mon_idx = i;
                    }

                    if (
                        !IS_TGT_KILLED      ||
                        nr_actors_hit >= 2    ||
                        (intended_aim_lvl == Actor_size::floor && cur_pos == aim_pos))
                    {
                        break;
                    }
                }
            }
        }

        //Wall hit?
        if (feature_blockers[cur_pos.x][cur_pos.y])
        {
            //TODO: Check hit material (soft and wood should not cause ricochet)

            Snd snd("I hear a ricochet.", Sfx_id::ricochet, Ignore_msg_if_origin_seen::yes,
                    cur_pos, nullptr, Snd_vol::low, Alerts_mon::yes);
            Snd_emit::emit_snd(snd);

            Cell& cell = Map::cells[cur_pos.x][cur_pos.y];

            if (cell.is_seen_by_player)
            {
                Render::draw_map_and_interface(false);
                Render::cover_cell_in_map(cur_pos);
                if (Config::is_tiles_mode())
                {
                    Render::draw_tile(Tile_id::blast2, Panel::map, cur_pos, clr_yellow);
                }
                else
                {
                    Render::draw_glyph('*', Panel::map, cur_pos, clr_yellow);
                }
                Render::update_screen();
                Sdl_wrapper::sleep(Config::get_delay_shotgun());
                Render::draw_map_and_interface();
            }

            cell.rigid->hit(Dmg_type::physical, Dmg_method::shotgun, nullptr);

            break;
        }

        //Floor hit?
        if (intended_aim_lvl == Actor_size::floor && cur_pos == aim_pos)
        {
            Snd snd("I hear a ricochet.", Sfx_id::ricochet, Ignore_msg_if_origin_seen::yes,
                    cur_pos, nullptr, Snd_vol::low, Alerts_mon::yes);
            Snd_emit::emit_snd(snd);

            if (Map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
            {
                Render::draw_map_and_interface(false);
                Render::cover_cell_in_map(cur_pos);
                if (Config::is_tiles_mode())
                {
                    Render::draw_tile(Tile_id::blast2, Panel::map, cur_pos, clr_yellow);
                }
                else
                {
                    Render::draw_glyph('*', Panel::map, cur_pos, clr_yellow);
                }
                Render::update_screen();
                Sdl_wrapper::sleep(Config::get_delay_shotgun());
                Render::draw_map_and_interface();
            }
            break;
        }
    }
}

} //namespace

void melee(Actor& attacker, const Wpn& wpn, Actor& defender)
{
    const Melee_att_data data(attacker, wpn, defender);

    print_melee_msg_and_play_sfx(data, wpn);

    if (!data.is_ethereal_defender_missed)
    {
        if (data.attack_result >= success_small && !data.is_defender_dodging)
        {
            const Actor_died died =
                data.defender->hit(data.dmg, wpn.get_data().melee.dmg_type);

            if (died == Actor_died::no)
            {
                data.defender->get_prop_handler().try_apply_prop_from_att(wpn, true);
            }
            if (data.attack_result >= success_normal)
            {
                if (data.defender->get_data().can_bleed)
                {
                    Map::mk_blood(data.defender->pos);
                }
            }
            if (died == Actor_died::no)
            {
                if (wpn.get_data().melee.knocks_back)
                {
                    if (data.attack_result > success_small)
                    {
                        Knock_back::try_knock_back(*(data.defender), data.attacker->pos,
                                                false);
                    }
                }
            }
            const Item_data_t& item_data = wpn.get_data();
            if (int(item_data.weight) > int(Item_weight::light) && !data.is_intrinsic_att)
            {
                Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes,
                        data.defender->pos, nullptr, Snd_vol::low, Alerts_mon::yes);
                Snd_emit::emit_snd(snd);
            }
        }
    }

    if (data.defender == Map::player)
    {
        if (data.attack_result >= fail_small)
        {
            static_cast<Mon*>(data.attacker)->is_stealth_ = false;
        }
    }
    else
    {
        Mon* const mon = static_cast<Mon*>(data.defender);
        mon->aware_counter_ = mon->get_data().nr_turns_aware;
    }
    Game_time::tick();
}

bool ranged(Actor& attacker, Wpn& wpn, const Pos& aim_pos)
{
    bool did_attack = false;

    const bool HAS_INF_AMMO = wpn.get_data().ranged.has_infinite_ammo;

    if (wpn.get_data().ranged.is_shotgun)
    {
        if (wpn.nr_ammo_loaded != 0 || HAS_INF_AMMO)
        {
            shotgun(attacker, wpn, aim_pos);

            did_attack = true;
            if (!HAS_INF_AMMO) {wpn.nr_ammo_loaded -= 1;}
        }
    }
    else //Not a shotgun
    {
        int nr_of_projectiles = 1;

        if (wpn.get_data().ranged.is_machine_gun) {nr_of_projectiles = NR_MG_PROJECTILES;}

        if (wpn.nr_ammo_loaded >= nr_of_projectiles || HAS_INF_AMMO)
        {
            projectile_fire(attacker, wpn, aim_pos);

            if (Map::player->is_alive())
            {
                did_attack = true;

                if (!HAS_INF_AMMO) {wpn.nr_ammo_loaded -= nr_of_projectiles;}
            }
            else //Player is dead
            {
                return true;
            }
        }
    }

    Render::draw_map_and_interface();

    if (did_attack)
    {
        Game_time::tick();
    }

    return did_attack;
}

} //Attack
