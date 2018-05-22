#include "attack.hpp"

#include "attack_data.hpp"
#include "rl_utils.hpp"
#include "init.hpp"
#include "item.hpp"
#include "game_time.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "feature_trap.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"
#include "msg_log.hpp"
#include "line_calc.hpp"
#include "io.hpp"
#include "sdl_base.hpp"
#include "knockback.hpp"
#include "drop.hpp"
#include "text_format.hpp"
#include "property.hpp"
#include "property_data.hpp"
#include "property_handler.hpp"
#include "property_factory.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
namespace
{

struct Projectile
{
        P pos {0, 0};
        int path_idx = 0;
        bool is_dead = false;
        int obstructed_in_path_idx = -1;
        Actor* actor_hit = nullptr;
        Feature* feature_hit = nullptr;
        bool is_seen_by_player = false;
        TileId tile = TileId::empty;
        char character = -1;
        Color color = colors::white();
        std::unique_ptr<RangedAttData> att_data = nullptr;
};

struct ProjectileFireData
{
        std::vector<Projectile> projectiles {};
        std::vector<P> path {};
        P origin {0, 0};
        P aim_pos {0, 0};
        Actor* attacker = nullptr;
        Wpn* wpn = nullptr;
        int animation_delay = 0;
        bool projectile_animation_leaves_trail = false;
};

enum class HitSize
{
        small,
        medium,
        hard
};

} // namespace

// TODO: This is a hack - it should be handled more generally (perhaps something
// like an 'on_ranged_attack' or 'on_fired' hook for items)
static void mi_go_gun_drain_player_hp(const Wpn& wpn)
{
        const std::string wpn_name =
                wpn.name(ItemRefType::plain,
                         ItemRefInf::none);

        msg_log::add("The " +
                     wpn_name +
                     " draws power from my life force!",
                     colors::msg_bad());

        map::player->hit(
                mi_go_gun_hp_drained,
                DmgType::pure,
                DmgMethod::forced,
                AllowWound::no);
}

static int player_ranged_speed_pct_diff(const Wpn& wpn)
{
        int result = 0;

        if (wpn.data().type == ItemType::ranged_wpn)
        {
                if (player_bon::traits[(size_t)Trait::marksman])
                {
                        result += 10;
                }

                if (player_bon::traits[(size_t)Trait::fast_shooter])
                {
                        result += 100;
                }
        }

        return result;
}

static size_t nr_projectiles_for_ranged_weapon(const Wpn& wpn)
{
        size_t nr_projectiles = 1;

        if (wpn.data().ranged.is_machine_gun)
        {
                nr_projectiles = nr_mg_projectiles;
        }

        return nr_projectiles;
}

static void print_player_melee_miss_msg()
{
        msg_log::add("I miss.");
}

static void print_mon_melee_miss_msg(const MeleeAttData& att_data)
{
        if (att_data.defender->is_player())
        {
                std::string other_name;

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

                msg_log::add(other_name + " misses me.",
                             colors::text(),
                             true);
        }
}

// TODO: It would be better to use absolute numbers, rather than relative to
// the weapon's max damage (if a weapon does 100 maximum damage, it would still
// be pretty catastrophic if it did 50 damage - but currently this would only
// count as a "small" hit). Perhaps use values relative to 'min_dng_to_wound'
// (in global.hpp) as thresholds instead.
static HitSize relative_hit_size(const int dmg, const int wpn_max_dmg)
{
        HitSize result = HitSize::small;

        if (wpn_max_dmg >= 4)
        {
                if (dmg > ((wpn_max_dmg * 5) / 6))
                {
                        result = HitSize::hard;
                }
                else if (dmg >  (wpn_max_dmg / 2))
                {
                        result = HitSize::medium;
                }
        }

        return result;
}

static HitSize relative_hit_size_melee(const AttData& att_data, const Wpn& wpn)
{
        const Dice dmg_dice = wpn.melee_dmg(att_data.attacker);

        const int max_dmg = dmg_dice.max();

        return relative_hit_size(att_data.dmg, max_dmg);
}

static HitSize relative_hit_size_ranged(const AttData& att_data, const Wpn& wpn)
{
        const Dice dmg_dice = wpn.ranged_dmg(att_data.attacker);

        const int max_dmg = dmg_dice.max();

        return relative_hit_size(att_data.dmg, max_dmg);
}

static std::string hit_size_punctuation_str(const HitSize hit_size)
{
        switch (hit_size)
        {
        case HitSize::small:
                return ".";

        case HitSize::medium:
                return "!";

        case HitSize::hard:
                return "!!!";
        }

        return "";
}

static void print_player_hit_mon_melee_msg(const MeleeAttData& att_data,
                                           const Wpn& wpn)
{
        const std::string wpn_verb = wpn.data().melee.att_msgs.player;

        std::string other_name;

        if (map::player->can_see_actor(*att_data.defender))
        {
                other_name = att_data.defender->name_the();
        }
        else // Player cannot see defender
        {
                other_name = "it";
        }

        const std::string dmg_punct =
                hit_size_punctuation_str(
                        relative_hit_size_melee(att_data, wpn));

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
                             colors::msg_good());
        }
        else // Not intrinsic attack
        {
                const std::string att_mod_str =
                        att_data.is_weak_attack ? "feebly " :
                        att_data.is_backstab ? "covertly "  : "";

                const Color color =
                        att_data.is_backstab ?
                        colors::light_blue() :
                        colors::msg_good();

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
                             color);
        }
}

static void print_mon_hit_player_melee_msg(const MeleeAttData& att_data,
                                           const Wpn& wpn)
{
        const std::string wpn_verb = wpn.data().melee.att_msgs.other;

        std::string other_name;

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

        const std::string dmg_punct =
                hit_size_punctuation_str(
                        relative_hit_size_melee(att_data, wpn));

        msg_log::add(other_name +
                     " " +
                     wpn_verb +
                     dmg_punct,
                     colors::msg_bad(),
                     true);
}

static void print_no_attacker_hit_player_melee_msg(const MeleeAttData& att_data,
                                                   const Wpn& wpn)
{
        const std::string dmg_punct =
                hit_size_punctuation_str(
                        relative_hit_size_melee(att_data, wpn));

        msg_log::add("I am hit" + dmg_punct,
                     colors::msg_bad(),
                     true);
}

static void print_no_attacker_hit_mon_melee_msg(const MeleeAttData& att_data,
                                                const Wpn& wpn)
{
        const std::string other_name =
                text_format::first_to_upper(
                        att_data.defender->name_the());

        Color msg_color = colors::msg_good();

        if (map::player->is_leader_of(att_data.defender))
        {
                // Monster is allied to player, use a neutral color
                // instead (we do not use red color here, since that
                // is reserved for player taking damage).
                msg_color = colors::white();
        }

        const std::string dmg_punct =
                hit_size_punctuation_str(
                        relative_hit_size_melee(att_data, wpn));

        msg_log::add(other_name + " is hit" + dmg_punct,
                     msg_color,
                     true);
}

static void print_melee_miss_msg(const MeleeAttData& att_data)
{
        if (att_data.attacker == map::player)
        {
                print_player_melee_miss_msg();
        }
        else if (att_data.attacker)
        {
                print_mon_melee_miss_msg(att_data);
        }
}

static void print_melee_hit_msg(const MeleeAttData& att_data, const Wpn& wpn)
{
        if (att_data.attacker == map::player)
        {
                print_player_hit_mon_melee_msg(att_data, wpn);

                return;
        }

        if (att_data.attacker)
        {
                // Attacker is monster

                if (att_data.defender->is_player())
                {
                        print_mon_hit_player_melee_msg(att_data, wpn);
                }

                return;
        }

        // No attacker (e.g. trap attack)

        if (att_data.defender == map::player)
        {
                print_no_attacker_hit_player_melee_msg(att_data, wpn);
        }
        else // Defender is monster
        {
                if (map::player->can_see_actor(*att_data.defender))
                {
                        print_no_attacker_hit_mon_melee_msg(att_data, wpn);
                }
        }
}

static SfxId melee_hit_sfx(const MeleeAttData& att_data,
                           const Wpn& wpn)
{
        const auto hit_size = relative_hit_size_melee(att_data, wpn);

        switch (hit_size)
        {
        case HitSize::small:
                return wpn.data().melee.hit_small_sfx;

        case HitSize::medium:
                return wpn.data().melee.hit_medium_sfx;

        case HitSize::hard:
                return wpn.data().melee.hit_hard_sfx;
        }

        return SfxId::END;
}

static AlertsMon is_melee_snd_alerting_mon(const Actor* const attacker,
                                           const Wpn& wpn)
{
        auto alerts = AlertsMon::no;

        const bool is_player_silent = player_bon::traits[(size_t)Trait::silent];

        const bool is_wpn_noisy = wpn.data().melee.is_noisy;

        if (is_wpn_noisy &&
            ((attacker != map::player) || is_player_silent))
        {
                alerts = AlertsMon::yes;
        }

        return alerts;
}

static void print_melee_msg(const MeleeAttData& att_data, const Wpn& wpn)
{
        ASSERT(att_data.defender);

        if (att_data.att_result <= ActionResult::fail)
        {
                print_melee_miss_msg(att_data);
        }
        else // Attack hits target
        {
                print_melee_hit_msg(att_data, wpn);
        }
}

static std::string melee_snd_msg(const MeleeAttData& att_data)
{
        std::string snd_msg = "";

        // Only print a message if player is not involved
        if ((att_data.defender != map::player) &&
            (att_data.attacker != map::player))
        {
                // TODO: This message is not appropriate for traps
                snd_msg = "I hear fighting.";
        }

        return snd_msg;
}

static void emit_melee_snd(const MeleeAttData& att_data, const Wpn& wpn)
{
        const auto snd_alerts_mon =
                is_melee_snd_alerting_mon(att_data.attacker, wpn);

        const std::string snd_msg = melee_snd_msg(att_data);

        SfxId sfx = SfxId::END;

        if (att_data.att_result <= ActionResult::fail)
        {
                sfx = wpn.data().melee.miss_sfx;
        }
        else
        {
                sfx = melee_hit_sfx(att_data, wpn);
        }

        Snd snd(snd_msg,
                sfx,
                IgnoreMsgIfOriginSeen::yes,
                att_data.defender->pos,
                att_data.attacker,
                SndVol::low,
                snd_alerts_mon);

        snd.run();
}

static void print_player_fire_ranged_msg(const Wpn& wpn)
{
        const std::string attack_verb = wpn.data().ranged.att_msgs.player;

        msg_log::add("I " + attack_verb + ".");
}

static void print_mon_fire_ranged_msg(const RangedAttData& att_data,
                                      const Wpn& wpn)
{
        if (att_data.aim_pos != map::player->pos)
        {
                return;
        }

        if (map::player->can_see_actor(*att_data.attacker))
        {
                const std::string attacker_name =
                        text_format::first_to_upper(
                                att_data.attacker->name_the());

                const std::string attack_verb =
                        wpn.data().ranged.att_msgs.other;

                msg_log::add(attacker_name + " " + attack_verb + ".",
                             colors::white(),
                             true);
        }
}

static void print_ranged_fire_msg(const RangedAttData& att_data, const Wpn& wpn)
{
        if (!att_data.attacker)
        {
                // No attacker actor (e.g. a trap firing a dart)
                return;
        }

        if (att_data.attacker == map::player)
        {
                print_player_fire_ranged_msg(wpn);
        }
        else
        {
                print_mon_fire_ranged_msg(att_data, wpn);
        }
}

static void print_projectile_hit_player_msg(const AttData& att_data,
                                            const Wpn& wpn)
{
        const std::string dmg_punct =
                hit_size_punctuation_str(
                        relative_hit_size_ranged(att_data, wpn));

        msg_log::add("I am hit" + dmg_punct,
                     colors::msg_bad(),
                     true);
}

static void print_projectile_hit_mon_msg(const AttData& att_data,
                                         const Wpn& wpn)
{
        std::string other_name = "It";

        if (map::player->can_see_actor(*att_data.defender))
        {
                other_name =
                        text_format::first_to_upper(
                                att_data.defender->name_the());
        }

        const std::string dmg_punct =
                hit_size_punctuation_str(
                        relative_hit_size_ranged(att_data, wpn));

        msg_log::add(other_name + " is hit" +
                     dmg_punct,
                     colors::msg_good());
}

static void print_projectile_hit_actor_msg(const RangedAttData& att_data,
                                           const Wpn& wpn)
{
        ASSERT(att_data.defender);

        if (att_data.defender->is_player())
        {
                print_projectile_hit_player_msg(att_data, wpn);
        }
        else // Defender is monster
        {
                const P& pos = att_data.defender->pos;

                if (!map::cells[pos.x][pos.y].is_seen_by_player)
                {
                        return;
                }

                print_projectile_hit_mon_msg(att_data, wpn);
        }
}

static std::unique_ptr<Snd> ranged_fire_snd(const AttData& att_data,
                                            const Wpn& wpn,
                                            const P& origin)
{
        std::unique_ptr<Snd> snd;

        const std::string snd_msg = wpn.data().ranged.snd_msg;

        if (!snd_msg.empty())
        {
                const SfxId sfx = wpn.data().ranged.att_sfx;

                const SndVol vol = wpn.data().ranged.snd_vol;

                std::string snd_msg_used = snd_msg;

                if (att_data.attacker == map::player)
                {
                        snd_msg_used = "";
                }

                snd = std::make_unique<Snd>(
                        snd_msg_used,
                        sfx,
                        IgnoreMsgIfOriginSeen::yes,
                        origin,
                        att_data.attacker,
                        vol,
                        AlertsMon::yes);
        }

        return snd;
}

static void emit_projectile_hit_actor_snd(const P& pos)
{
        Snd snd("A creature is hit.",
                SfxId::hit_small,
                IgnoreMsgIfOriginSeen::yes,
                pos,
                nullptr,
                SndVol::low,
                AlertsMon::no);

        snd.run();
}

static void emit_projectile_hit_feature_snd(const P& pos, const Wpn& wpn)
{
        if (wpn.data().ranged.makes_ricochet_snd)
        {
                // TODO: Check hit material, soft and wood should not cause
                // a ricochet sound
                Snd snd("I hear a ricochet.",
                        SfxId::ricochet,
                        IgnoreMsgIfOriginSeen::yes,
                        pos,
                        nullptr,
                        SndVol::low,
                        AlertsMon::yes);

                snd_emit::run(snd);
        }
}

static Actor* get_actor_hit_by_projectile(const Projectile& projectile)
{
        const auto& att_data = *projectile.att_data;

        if (!att_data.defender)
        {
                return nullptr;
        }

        const bool is_actor_aimed_for = (projectile.pos == att_data.aim_pos);

        const bool can_hit_height =
                (att_data.defender_size >= ActorSize::humanoid) ||
                is_actor_aimed_for;

        if (!projectile.is_dead &&
            (att_data.att_result >= ActionResult::success) &&
            can_hit_height)
        {
                return att_data.defender;
        }

        return nullptr;
}

static Feature* get_feature_blocking_projectile(const P& pos)
{
        auto* rigid = map::cells[pos.x][pos.y].rigid;

        if (!rigid->is_projectile_passable())
        {
                return rigid;
        }

        return nullptr;
}

static Feature* get_ground_blocking_projectile(const Projectile& projectile)
{
        // Hit the ground?
        const auto& att_data = *projectile.att_data;

        const auto& pos = projectile.pos;

        const bool has_hit_ground =
                (pos == att_data.aim_pos) &&
                (att_data.aim_lvl == ActorSize::floor);

        if (has_hit_ground)
        {
                return map::cells[pos.x][pos.y].rigid;
        }

        return nullptr;
}

static void try_apply_attack_property_on_actor(
        const ItemAttProp& att_prop,
        Actor& actor,
        const DmgType& dmg_type)
{
        if (att_prop.prop &&
            rnd::percent(att_prop.pct_chance_to_apply))
        {
                auto& properties = actor.properties();

                const bool is_resisting_dmg =
                        properties.is_resisting_dmg(
                                dmg_type,
                                Verbosity::silent);

                if (!is_resisting_dmg)
                {
                        auto* const prop_cpy =
                                property_factory::make(
                                        att_prop.prop->id());

                        const auto duration_mode =
                                att_prop.prop->duration_mode();

                        if (duration_mode == PropDurationMode::specific)
                        {
                                prop_cpy->set_duration(
                                        att_prop.prop->nr_turns_left());
                        }
                        else if (duration_mode == PropDurationMode::indefinite)
                        {
                                prop_cpy->set_indefinite();
                        }

                        properties.apply(prop_cpy);
                }
        }
}

static void hit_actor_with_projectile(const Projectile& projectile, Wpn& wpn)
{
        ASSERT(projectile.actor_hit);

        if (!projectile.actor_hit)
        {
                return;
        }

        const AttData& att_data = *projectile.att_data;

        if (att_data.attacker == map::player)
        {
                static_cast<Mon*>(att_data.defender)-> set_player_aware_of_me();
        }

        auto died = ActorDied::no;

        if (att_data.dmg > 0)
        {
                died = projectile.actor_hit->hit(
                        att_data.dmg,
                        wpn.data().ranged.dmg_type,
                        DmgMethod::END,
                        AllowWound::yes);
        }

        // NOTE: This is run regardless of if defender died or not
        wpn.on_ranged_hit(*projectile.actor_hit);

        if (died == ActorDied::no)
        {
                ItemAttProp att_prop =
                        wpn.prop_applied_on_ranged(
                                projectile.att_data->attacker);

                try_apply_attack_property_on_actor(
                        att_prop,
                        *projectile.actor_hit,
                        wpn.data().ranged.dmg_type);

                // Knock-back?
                if (wpn.data().ranged.knocks_back &&
                    att_data.attacker)
                {
                        const bool is_spike_gun =
                                wpn.data().id == ItemId::spike_gun;

                        knockback::run(
                                *(att_data.defender),
                                att_data.attacker->pos,
                                is_spike_gun);
                }
        }
}

static void hit_feature_with_projectile(const P& prev_pos,
                                        const P& current_pos,
                                        Wpn& wpn)
{
        // TODO: This was in the 'shotgun' function (but only the shotgun was
        // calling the feature 'hit' method in attack.cpp - the normal
        // projectile firing did not do this). Reimplement somehow.
        // cell.rigid->hit(
        //         att_data.dmg,
        //         DmgType::physical,
        //         DmgMethod::shotgun,
        //         nullptr);

        wpn.on_projectile_blocked(prev_pos, current_pos);
}

static void init_projectiles_gfx(ProjectileFireData& fire_data)
{
        ASSERT(fire_data.wpn);
        ASSERT(!fire_data.path.empty());

        if (!fire_data.wpn || fire_data.path.empty())
        {
                return;
        }

        fire_data.projectile_animation_leaves_trail =
                fire_data.wpn->data().ranged.projectile_leaves_trail;

        char projectile_character =
                fire_data.wpn->data().ranged.projectile_character;

        if (projectile_character == '/')
        {
                const auto& path = fire_data.path;
                const P& origin = fire_data.origin;

                const int i = (path.size() > 2) ? 2 : 1;

                const P& ref_pos = path[i];

                if (ref_pos.y == origin.y)
                {
                        projectile_character = '-';
                }
                else if (ref_pos.x == origin.x)
                {
                        projectile_character = '|';
                }
                else if ((ref_pos.x > origin.x && ref_pos.y < origin.y) ||
                         (ref_pos.x < origin.x && ref_pos.y > origin.y))
                {
                        projectile_character = '/';
                }
                else if ((ref_pos.x > origin.x && ref_pos.y > origin.y) ||
                         (ref_pos.x < origin.x && ref_pos.y < origin.y))
                {
                        projectile_character = '\\';
                }
        }

        TileId projectile_tile =
                fire_data.wpn->data().ranged.projectile_tile;

        if (projectile_tile == TileId::projectile_std_front_slash)
        {
                if (projectile_character == '-')
                {
                        projectile_tile = TileId::projectile_std_dash;
                }
                else if (projectile_character == '|')
                {
                        projectile_tile = TileId::projectile_std_vertical_bar;
                }
                else if (projectile_character == '\\')
                {
                        projectile_tile = TileId::projectile_std_back_slash;
                }
        }

        const Color projectile_color =
                fire_data.wpn->data().ranged.projectile_color;

        for (auto& projectile: fire_data.projectiles)
        {
                projectile.tile = projectile_tile;

                projectile.character = projectile_character;

                projectile.color = projectile_color;
        }
}

static void init_projectiles_animation_delay(ProjectileFireData& fire_data)
{
        const int denom = fire_data.projectiles.size();

        fire_data.animation_delay = (config::delay_projectile_draw() / denom);
}

static ProjectileFireData init_projectiles_fire_data(
        Actor* const attacker,
        Wpn& wpn,
        const P& origin,
        const P& aim_pos)
{
        ProjectileFireData fire_data;

        fire_data.attacker = attacker;
        fire_data.wpn = &wpn;
        fire_data.origin = origin;
        fire_data.aim_pos = aim_pos;

        const size_t nr_projectiles = nr_projectiles_for_ranged_weapon(wpn);

        fire_data.projectiles.resize(nr_projectiles);

        for (size_t i = 0; i < nr_projectiles; ++i)
        {
                auto& proj = fire_data.projectiles[i];

                // Projectile path indexes are initially set up so that all
                // projectiles start with a negative index (they are queued up
                // to enter the path)
                proj.path_idx = -((int)i * nr_cell_jumps_mg_projectiles) - 1;

                auto att_data =
                        new RangedAttData(
                                fire_data.attacker,
                                fire_data.origin,
                                fire_data.aim_pos,
                                fire_data.origin, // Current position
                                *fire_data.wpn);

                proj.att_data.reset(att_data);
        }

        const ActorSize aim_lvl = fire_data.projectiles[0].att_data->aim_lvl;

        const bool stop_at_target = (aim_lvl == ActorSize::floor);

        fire_data.path = line_calc::calc_new_line(
                fire_data.origin,
                fire_data.aim_pos,
                stop_at_target,
                999,
                false);

        init_projectiles_gfx(fire_data);

        init_projectiles_animation_delay(fire_data);

        return fire_data;
}

static bool is_all_projectiles_dead(const std::vector<Projectile>& projectiles)
{
        for (const auto& projectile : projectiles)
        {
                if (!projectile.is_dead)
                {
                        return false;
                }
        }

        return true;
}

static void run_projectile_hits(ProjectileFireData& fire_data)
{
        for (auto& projectile : fire_data.projectiles)
        {
                if (projectile.is_dead)
                {
                        continue;
                }

                if (projectile.actor_hit)
                {
                        hit_actor_with_projectile(projectile, *fire_data.wpn);

                        projectile.is_dead = true;

                        continue;
                }

                if (projectile.feature_hit)
                {
                        const int prev_idx = projectile.path_idx - 1;

                        ASSERT(prev_idx >= 0);

                        if (prev_idx < 0)
                        {
                                continue;
                        }

                        const P& prev_pos = fire_data.path[prev_idx];

                        const P& current_pos = projectile.pos;

                        hit_feature_with_projectile(
                                prev_pos,
                                current_pos,
                                *fire_data.wpn);

                        projectile.is_dead = true;

                        continue;
                }
        }
}

static void advance_projectiles_on_path(ProjectileFireData& fire_data)
{
        for (auto& projectile : fire_data.projectiles)
        {
                if (projectile.is_dead)
                {
                        continue;
                }

                ++projectile.path_idx;

                if (projectile.path_idx >= 0)
                {
                        projectile.pos = fire_data.path[projectile.path_idx];
                }
        }
}

static void update_projectile_states(ProjectileFireData& fire_data)
{
        advance_projectiles_on_path(fire_data);

        for (auto& projectile : fire_data.projectiles)
        {
                if (projectile.is_dead ||
                    (projectile.path_idx < 1))
                {
                        continue;
                }

                const P& projectile_pos =
                        fire_data.path[projectile.path_idx];

                projectile.att_data =
                        std::make_unique<RangedAttData>(
                                fire_data.attacker,
                                fire_data.origin,
                                fire_data.aim_pos,
                                projectile_pos,
                                *fire_data.wpn);

                projectile.is_seen_by_player =
                        map::cells[projectile_pos.x][projectile_pos.y]
                        .is_seen_by_player;

                // Projectile out of range?
                const int max_range = fire_data.wpn->data().ranged.max_range;

                if (king_dist(fire_data.origin, projectile_pos) > max_range)
                {
                        projectile.is_dead = true;
                        projectile.obstructed_in_path_idx = projectile.path_idx;

                        continue;
                }

                projectile.actor_hit = get_actor_hit_by_projectile(projectile);

                if (projectile.actor_hit)
                {
                        projectile.obstructed_in_path_idx = projectile.path_idx;

                        continue;
                }

                projectile.feature_hit =
                        get_feature_blocking_projectile(projectile.pos);

                if (projectile.feature_hit)
                {
                        projectile.obstructed_in_path_idx = projectile.path_idx;

                        continue;
                }

                projectile.feature_hit =
                        get_ground_blocking_projectile(projectile);

                if (projectile.feature_hit)
                {
                        projectile.obstructed_in_path_idx = projectile.path_idx;

                        continue;
                }
        }
}

static void run_projectiles_messages_and_sounds(
        const ProjectileFireData& fire_data)
{
        for (auto& projectile : fire_data.projectiles)
        {
                if (projectile.is_dead)
                {
                        continue;
                }

                // Projectile entering the path this update?
                if (projectile.path_idx == 0)
                {
                        // NOTE: The initial attack sound(s) must NOT alert
                        // monsters, since this would immediately make them
                        // aware before any attack data is set. This would
                        // result in the player not getting ranged attack
                        // bonuses against unaware monsters.
                        // An extra sound is run when the attack ends (without a
                        // message or audio), which may alert monsters.
                        auto snd = ranged_fire_snd(
                                *projectile.att_data,
                                *fire_data.wpn,
                                fire_data.origin);

                        if (snd)
                        {
                                snd->set_alerts_mon(AlertsMon::no);

                                snd->run();
                        }

                        continue;
                }

                if (projectile.actor_hit)
                {
                        emit_projectile_hit_actor_snd(projectile.pos);

                        print_projectile_hit_actor_msg(
                                *projectile.att_data,
                                *fire_data.wpn);

                        continue;
                }

                if (projectile.feature_hit)
                {
                        emit_projectile_hit_feature_snd(projectile.pos,
                                                        *fire_data.wpn);
                }
        }
}

static void draw_projectile(const Projectile& projectile)
{
        if (projectile.tile == TileId::empty)
        {
                return;
        }

        io::draw_symbol(
                projectile.tile,
                projectile.character,
                Panel::map,
                projectile.pos,
                projectile.color);
}

static void draw_projectile_hit(Projectile& projectile,
                                const int animation_delay)
{
        projectile.color = colors::light_red();

        if (config::is_tiles_mode())
        {
                projectile.tile = TileId::blast1;

                draw_projectile(projectile);

                io::update_screen();

                sdl_base::sleep(animation_delay / 2);

                projectile.tile = TileId::blast2;

                draw_projectile(projectile);

                io::update_screen();

                sdl_base::sleep(animation_delay / 2);
        }
        else // Text mode
        {
                projectile.character = '*';

                draw_projectile(projectile);

                io::update_screen();

                sdl_base::sleep(animation_delay);
        }
}

static bool should_draw_projectile_as_travelling(const Projectile& projectile)
{
        return
                !projectile.is_dead &&
                projectile.is_seen_by_player &&
                (projectile.path_idx >= 1);
}

static bool should_draw_projectile_as_hit(const Projectile& projectile)
{
        return
                !projectile.is_dead &&
                projectile.is_seen_by_player &&
                (projectile.obstructed_in_path_idx >= 0);
}

static void draw_projectiles(ProjectileFireData& fire_data)
{
        if (!fire_data.projectile_animation_leaves_trail)
        {
                states::draw();
        }

        // Draw travelling projectiles
        for (auto& projectile : fire_data.projectiles)
        {
                if (!should_draw_projectile_as_travelling(projectile))
                {
                        continue;
                }

                draw_projectile(projectile);
        }

        io::update_screen();

        // Draw projectiles hitting something
        for (auto& projectile : fire_data.projectiles)
        {
                if (!should_draw_projectile_as_hit(projectile))
                {
                        continue;
                }

                draw_projectile_hit(projectile, fire_data.animation_delay);
        }
}

static bool is_any_projectile_seen(const std::vector<Projectile>& projectiles)
{
        for (const auto& projectile: projectiles)
        {
                if (projectile.is_seen_by_player)
                {
                        return true;
                }
        }

        return false;
}

static void fire_projectiles(
        Actor* const attacker,
        Wpn& wpn,
        const P& origin,
        const P& aim_pos)
{
        auto fire_data =
                init_projectiles_fire_data(
                        attacker,
                        wpn,
                        origin,
                        aim_pos);

        print_ranged_fire_msg(*fire_data.projectiles[0].att_data,
                              *fire_data.wpn);

        while(true)
        {
                if (is_all_projectiles_dead(fire_data.projectiles))
                {
                        // Run a sound without message or audio, which can alert
                        // monsters (the initial fire sound is not allowed to
                        // alert monsters, since this would prevent ranged
                        // attack bonuses against unaware monsters)
                        const auto& att_data =
                                *fire_data.projectiles[0].att_data;

                        auto snd = ranged_fire_snd(
                                att_data,
                                *fire_data.wpn,
                                fire_data.origin);

                        if (snd)
                        {
                                snd->clear_msg();

                                snd->clear_sfx();

                                snd->run();
                        }

                        return;
                }

                update_projectile_states(fire_data);

                run_projectiles_messages_and_sounds(fire_data);

                draw_projectiles(fire_data);

                run_projectile_hits(fire_data);

                if (is_any_projectile_seen(fire_data.projectiles))
                {
                        sdl_base::sleep(fire_data.animation_delay);
                }
        }
}

// -----------------------------------------------------------------------------
// attack
// -----------------------------------------------------------------------------
namespace attack
{

void melee(Actor* const attacker,
           const P& attacker_origin,
           Actor& defender,
           Wpn& wpn)
{
        map::update_vision();

        const MeleeAttData att_data(attacker, defender, wpn);

        print_melee_msg(att_data, wpn);

        emit_melee_snd(att_data, wpn);

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
                        allow_wound);

                // TODO: Why is light damage included here?
                if (defender.data().can_bleed &&
                    (dmg_type == DmgType::physical ||
                     dmg_type == DmgType::pure ||
                     dmg_type == DmgType::light))
                {
                        map::make_blood(defender.pos);
                }

                wpn.on_melee_hit(defender, att_data.dmg);

                if (defender.is_alive())
                {
                        ItemAttProp att_prop =
                                wpn.prop_applied_on_melee(
                                        att_data.attacker);

                        try_apply_attack_property_on_actor(
                                att_prop,
                                defender,
                                wpn.data().ranged.dmg_type);

                        if (wpn.data().melee.knocks_back)
                        {
                                knockback::run(defender,
                                               attacker_origin,
                                               false);
                        }
                }
                else // Defender was killed
                {
                        wpn.on_melee_kill(defender);
                }
        }

        auto& player_inv = map::player->inv();

        const bool is_crit_fail =
                att_data.att_result == ActionResult::fail_critical;

        const bool player_cursed = map::player->has_prop(PropId::cursed);

        const bool is_wielding_wpn =
                player_inv.item_in_slot(SlotId::wpn) == &wpn;

        // If player is cursed and the attack critically fails, occasionally
        // break the weapon
        const int break_one_in_n = 32;

        const bool break_weapon =
                (attacker == map::player) &&
                is_wielding_wpn &&
                player_cursed &&
                is_crit_fail &&
                rnd::one_in(break_one_in_n);

        if (break_weapon)
        {
                Item* const item =
                        player_inv.remove_item_in_slot(SlotId::wpn, false);

                ASSERT(item);

                if (item)
                {
                        const std::string item_name =
                                item->name(ItemRefType::plain,
                                           ItemRefInf::none);

                        msg_log::add("My " + item_name + " breaks!",
                                     colors::msg_note(),
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
                        if (player_bon::traits[(size_t)Trait::melee_fighter])
                        {
                                speed_pct_diff += 10;
                        }
                }
                else // Attacker is monster
                {
                        static_cast<Mon*>(attacker)->become_aware_player(false);
                }

                // Attacking ends cloaking
                attacker->properties().end_prop(PropId::cloaked);

                game_time::tick(speed_pct_diff);
        }
} // melee

DidAction ranged(Actor* const attacker,
                 const P& origin,
                 const P& aim_pos,
                 Wpn& wpn)
{
        map::update_vision();

        DidAction did_attack = DidAction::no;

        const bool has_inf_ammo = wpn.data().ranged.has_infinite_ammo;

        const int nr_projectiles = nr_projectiles_for_ranged_weapon(wpn);

        if ((wpn.ammo_loaded_ >= nr_projectiles) || has_inf_ammo)
        {
                // TODO: This is a hack - it should be handled more generally
                // (perhaps something like an 'on_ranged_attack' or 'on_fired'
                // hook for items)
                if ((attacker == map::player) &&
                    (wpn.data().id == ItemId::mi_go_gun))
                {
                        mi_go_gun_drain_player_hp(wpn);
                }

                fire_projectiles(
                        attacker,
                        wpn,
                        origin,
                        aim_pos);

                did_attack = DidAction::yes;

                if (!has_inf_ammo)
                {
                        wpn.ammo_loaded_ -= nr_projectiles;
                }

                // Player could have for example fired an explosive weapon into
                // a wall and killed themselves - if so, abort early
                if (!map::player->is_alive())
                {
                        return DidAction::yes;
                }
        }

        states::draw();

        if ((did_attack == DidAction::yes) && attacker)
        {
                // Attacking ends cloaking
                attacker->properties().end_prop(PropId::cloaked);

                if (!attacker->is_player())
                {
                        static_cast<Mon*>(attacker)->become_aware_player(false);
                }

                int speed_pct_diff = 0;

                if (attacker->is_player())
                {
                        speed_pct_diff += player_ranged_speed_pct_diff(wpn);
                }

                game_time::tick(speed_pct_diff);
        }

        return did_attack;
} // ranged

} // attack
