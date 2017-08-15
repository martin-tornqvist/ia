#include "spells.hpp"

#include <algorithm>
#include <vector>

#include "init.hpp"
#include "io.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "postmortem.hpp"
#include "knockback.hpp"
#include "map.hpp"
#include "actor_factory.hpp"
#include "feature_trap.hpp"
#include "feature_door.hpp"
#include "player_spells.hpp"
#include "item_scroll.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "line_calc.hpp"
#include "sdl_base.hpp"
#include "player_bon.hpp"
#include "game.hpp"
#include "explosion.hpp"
#include "text_format.hpp"
#include "item_factory.hpp"

namespace
{

const int summon_hostile_one_in_n = 10;

} // namespace

namespace spell_handling
{

Spell* random_spell_for_mon()
{
    std::vector<SpellId> bucket;

    for (int i = 0; i < (int)SpellId::END; ++i)
    {
        Spell* const spell = mk_spell_from_id((SpellId)i);

        if (spell->mon_can_learn())
        {
            bucket.push_back(SpellId(i));
        }

        delete spell;
    }

    ASSERT(!bucket.empty());

    const int element = rnd::range(0, bucket.size() - 1);

    return mk_spell_from_id(bucket[element]);
}

Spell* mk_spell_from_id(const SpellId spell_id)
{
    switch (spell_id)
    {
    case SpellId::enfeeble:
        return new SpellEnfeeble;

    case SpellId::disease:
        return new SpellDisease;

    case SpellId::darkbolt:
        return new SpellDarkbolt;

    case SpellId::aza_wrath:
        return new SpellAzaWrath;

    case SpellId::summon:
        return new SpellSummonMon;

    case SpellId::heal:
        return new SpellHeal;

    case SpellId::knockback:
        return new SpellKnockBack;

    case SpellId::teleport:
        return new SpellTeleport;

    case SpellId::mayhem:
        return new SpellMayhem;

    case SpellId::pest:
        return new SpellPest;

    case SpellId::anim_wpns:
        return new SpellAnimWpns;

    case SpellId::subdue_wpns:
        return new SpellSubdueWpns;

    case SpellId::searching:
        return new SpellSearching;

    case SpellId::opening:
        return new SpellOpening;

    case SpellId::frenzy:
        return new SpellFrenzy;

    case SpellId::bless:
        return new SpellBless;

    case SpellId::mi_go_hypno:
        return new SpellMiGoHypno;

    case SpellId::burn:
        return new SpellBurn;

    case SpellId::deafen:
        return new SpellDeafen;

    case SpellId::res:
        return new SpellRes;

    case SpellId::pharaoh_staff:
        return new SpellPharaohStaff;

    case SpellId::light:
        return new SpellLight;

    case SpellId::transmut:
        return new SpellTransmut;

    case SpellId::see_invis:
        return new SpellSeeInvis;

    case SpellId::spell_shield:
        return new SpellSpellShield;

    case SpellId::END:
        break;
    }

    ASSERT(false && "No spell found for ID");
    return nullptr;
}

} // spell_handling

StateId BrowseSpell::id()
{
    return StateId::browse_spells;
}

Range Spell::spi_cost(const SpellSkill skill, Actor* const caster) const
{
    int cost_max = max_spi_cost(skill);

    if (caster == map::player)
    {
        //
        // Standing next to an altar reduces the cost
        //
        const int x0 = std::max(0, caster->pos.x - 1);
        const int y0 = std::max(0, caster->pos.y - 1);
        const int x1 = std::min(map_w - 1, caster->pos.x + 1);
        const int y1 = std::min(map_h - 1, caster->pos.y + 1);

        for (int x = x0; x <= x1; ++x)
        {
            for (int y = y0; y <= y1; ++y)
            {
                if (map::cells[x][y].rigid->id() == FeatureId::altar)
                {
                    cost_max -= 1;
                }
            }
        }

        const bool is_blood_sorc =
            player_bon::traits[(size_t)Trait::blood_sorc];

        if (is_blood_sorc)
        {
            --cost_max;
        }

    } // Is player, and use modified cost

    cost_max = std::max(1, cost_max);

    const int cost_min = std::max(1, (cost_max + 1) / 2);

    return Range(cost_min, cost_max);
}

void Spell::cast(Actor* const caster,
                 const SpellSkill skill,
                 const IsIntrinsic intrinsic) const
{
    TRACE_FUNC_BEGIN;

    ASSERT(caster);

    auto& prop_handler = caster->prop_handler();

    // If this is an intrinsic cast, check properties which NEVER allows casting
    // or speaking

    //
    // NOTE: If this is a non-intrinsic cast (e.g. from a scroll), then we
    //       assume that the caller has made all checks themselves
    //
    if ((intrinsic == IsIntrinsic::yes) &&
        (!prop_handler.allow_cast_intr_spell_absolute(Verbosity::verbose) ||
         !prop_handler.allow_speak(Verbosity::verbose)))
    {
        return;
    }

    // OK, we can try to cast

    if (caster->is_player())
    {
        TRACE << "Player casting spell" << std::endl;

        const ShockSrc shock_src =
            (intrinsic == IsIntrinsic::yes) ?
            ShockSrc::cast_intr_spell :
            ShockSrc::use_strange_item;

        const int value = shock_value();

        map::player->incr_shock((double)value, shock_src);

        Snd snd("",
                SfxId::spell_generic,
                IgnoreMsgIfOriginSeen::yes,
                caster->pos,
                caster,
                SndVol::low,
                AlertsMon::yes);

        snd_emit::run(snd);
    }
    else // Caster is monster
    {
        TRACE << "Monster casting spell" << std::endl;

        Mon* const mon = static_cast<Mon*>(caster);

        const bool is_mon_seen = map::player->can_see_actor(*mon);

        std::string spell_str = mon->data().spell_cast_msg;

        if (!spell_str.empty())
        {
            std::string mon_name = "";

            if (is_mon_seen)
            {
                mon_name = text_format::first_to_upper(mon->name_the());
            }
            else // Cannot see monster
            {
                mon_name =
                    mon->data().is_humanoid ?
                    "Someone" :
                    "Something";
            }

            spell_str = mon_name + " " + spell_str;
        }

        Snd snd(spell_str,
                SfxId::END,
                IgnoreMsgIfOriginSeen::no,
                caster->pos,
                caster,
                SndVol::low,
                AlertsMon::no);

        snd_emit::run(snd);
    }

    bool allow_cast = true;

    if (intrinsic == IsIntrinsic::yes)
    {
        const Range cost = spi_cost(skill, caster);

        caster->hit_spi(cost.roll(), Verbosity::silent);

        // Check properties which MAY allow casting, with a random chance
        allow_cast =
            prop_handler.allow_cast_intr_spell_chance(
                Verbosity::verbose);
    }

    if (allow_cast &&
        caster->is_alive())
    {
        run_effect(caster, skill);
    }

    // Casting spells ends cloaking
    caster->prop_handler().end_prop(PropId::cloaked);

    game_time::tick();

    TRACE_FUNC_END;
}

void Spell::on_resist(Actor& target) const
{
    const bool is_player = target.is_player();

    const bool player_see_target = map::player->can_see_actor(target);

    if (player_see_target)
    {
        msg_log::add(spell_resist_msg);

        if (is_player)
        {
            audio::play(SfxId::spell_shield_break);
        }

        io::draw_blast_at_cells({target.pos}, clr_white);
    }

    target.prop_handler().end_prop(PropId::r_spell);

    if (is_player &&
        player_bon::traits[(size_t)Trait::absorb])
    {
        map::player->restore_spi(
            rnd::dice(1, 6),
            false, // Not allowed above max
            Verbosity::verbose);
    }
}

std::vector<std::string> Spell::descr(const SpellSkill skill) const
{
    auto ret = descr_specific(skill);

    if (can_be_improved_with_skill())
    {
        std::string skill_str = "";

        switch (skill)
        {
        case SpellSkill::basic:
            skill_str = "Basic";
            break;

        case SpellSkill::expert:
            skill_str = "Expert";
            break;

        case SpellSkill::master:
            skill_str = "Master";
            break;
        }

        ret.push_back("Skill level: " + skill_str);
    }

    return ret;
}

int Spell::shock_value() const
{
    const SpellShock type = shock_type();

    int value = 0;

    switch (type)
    {
    case SpellShock::mild:
        value = 4;
        break;

    case SpellShock::disturbing:
        value = 16;
        break;

    case SpellShock::severe:
        value = 24;
        break;
    }

    // Blessed/cursed affects shock
    if (map::player->has_prop(PropId::blessed))
    {
        value -= 2;
    }
    else if (map::player->has_prop(PropId::cursed))
    {
        value += 2;
    }

    return value;
}

// -----------------------------------------------------------------------------
// Darkbolt
// -----------------------------------------------------------------------------
void SpellDarkbolt::run_effect(Actor* const caster,
                               const SpellSkill skill) const
{
    // if (caster == map::player &&
    //     player_bon::traits[(size_t)Trait::warlock] &&
    //     rnd::percent(warlock_multi_cast_chance_pct))
    // {
    //     run_effect(caster, skill);

    //     if (!caster->is_alive())
    //     {
    //         return;
    //     }
    // }

    std::vector<Actor*> target_bucket;

    // When master, any creature that the caster is aware of can be targeted
    if (caster->is_player() &&
        (skill == SpellSkill::master))
    {
        for (Actor* const actor : game_time::actors)
        {
            if (!actor->is_alive() ||
                actor->is_player() ||
                actor->is_actor_my_leader(map::player))
            {
                continue;
            }

            const Mon* const mon = static_cast<const Mon*>(actor);

            if (mon->player_aware_of_me_counter_ > 0)
            {
                target_bucket.push_back(actor);
            }
        }
    }
    else // Not player, or not master skill
    {
        target_bucket = caster->seen_foes();
    }

    bool blocked[map_w][map_h];

    map_parsers::BlocksProjectiles()
        .run(blocked);

    int flood[map_w][map_h];

    const int travel_lmt = 12;

    floodfill(caster->pos,
              blocked,
              flood,
              travel_lmt);

    // Remove all actors to which there is no path
    for (auto it = begin(target_bucket); it != end(target_bucket); )
    {
        auto* const actor = *it;

        const P& p = actor->pos;

        // A path exists to this actor?
        if (flood[p.x][p.y] > 0)
        {
            ++it;
        }
        else // No path
        {
            it = target_bucket.erase(it);
        }
    }

    if (target_bucket.empty())
    {
        if (caster->is_player())
        {
            msg_log::add(
                "A dark sphere materializes, but quickly fizzles out.");
        }

        return;
    }

    Actor* const target = map::random_closest_actor(caster->pos, target_bucket);

    // Spell resistance?
    if (target->has_prop(PropId::r_spell))
    {
        on_resist(*target);

        // Spell reflection?
        if (target->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*target))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            // Run effect with the target as caster instead
            run_effect(target, skill);
        }

        return;
    }

    std::vector<P> path;

    pathfind_with_flood(
        caster->pos,
        target->pos,
        flood,
        path);

    ASSERT(!path.empty());

    if (path.empty())
    {
        return;
    }

    states::draw();

    const int idx_0 = (int)(path.size()) - 1;

    for (int i = idx_0; i > 0; --i)
    {
        const P& p = path[i];

        if (!map::cells[p.x][p.y].is_seen_by_player)
        {
            continue;
        }

        states::draw();

        if (config::is_tiles_mode())
        {
            io::draw_tile(TileId::blast1,
                          Panel::map,
                          p,
                          clr_magenta);
        }
        else // Text mode
        {
            io::draw_glyph('*',
                           Panel::map,
                           p,
                           clr_magenta);
        }

        io::update_screen();

        sdl_base::sleep(config::delay_projectile_draw());
    }

    const P& tgt_p = target->pos;

    const bool player_see_cell = map::cells[tgt_p.x][tgt_p.y].is_seen_by_player;

    const bool player_see_tgt = map::player->can_see_actor(*target);

    if (player_see_tgt ||
        player_see_cell)
    {
        io::draw_blast_at_cells({target->pos}, clr_magenta);

        Clr msg_clr = clr_msg_good;

        std::string str_begin = "I am";

        if (target->is_player())
        {
            msg_clr = clr_msg_bad;
        }
        else // Target is monster
        {
            const std::string name_the =
                player_see_tgt ?
                text_format::first_to_upper(target->name_the()) :
                "It";

            str_begin = name_the + " is";

            if (map::player->is_leader_of(target))
            {
                msg_clr = clr_white;
            }
        }

        msg_log::add(str_begin + " struck by a blast!", msg_clr);
    }

    // Skill  damage     avg
    // -----------------------
    // 0       4  - 9     6.5
    // 1       8  - 15   11.5
    // 2      12  - 21   16.5
    Range dmg_range(4 + (int)skill * 4,
                    9 + (int)skill * 6);

    // If a monster is casting, only do 50% damage to avoid making this spell
    // extremely dangerous for the player
    if (!caster->is_player())
    {
        dmg_range.min /= 2;
        dmg_range.max /= 2;
    }

    target->hit(dmg_range.roll(),
                DmgType::physical,
                DmgMethod::END,
                AllowWound::no);

    if (target->is_alive())
    {
        target->prop_handler().apply(
            new PropParalyzed(PropTurns::specific, 2));
    }

    if ((skill == SpellSkill::master) &&
        target->is_alive())
    {
        target->prop_handler().apply(
            new PropBurning(PropTurns::specific, 3));
    }

    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            target->pos,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);
}

std::vector<std::string> SpellDarkbolt::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Siphons power from some infernal dimension, which is focused into a "
        "bolt hurled towards a target with great force. The conjured bolt has "
        "some will on its own - the caster cannot determine exactly which "
        "creature will be struck.");

    const Range dmg_range(4 + (int)skill * 4,
                          9 + (int)skill * 6);

    descr.push_back(
        "The impact does " +
        std::to_string(dmg_range.min) +
        "-" +
        std::to_string(dmg_range.max) +
        " damage.");

    if (skill == SpellSkill::master)
    {
        descr.push_back("The target is paralyzed, and set aflame.");

        descr.push_back("The caster does not have to see the target, the bolt "
                        "will travel to any creature that the caster knows the "
                        "position of - if the target is reachable. The bolt "
                        "can travel up to 12 steps.");
    }
    else // Not master
    {
        descr.push_back("The target is paralyzed.");
    }

    return descr;
}

bool SpellDarkbolt::allow_mon_cast_now(Mon& mon) const
{
    //
    // NOTE: Monsters with master spell skill level COULD cast this spell
    //       without LOS to the player, but we do not allow the AI to do this,
    //       since it would probably be very hard and/or annoying for the player
    //       to deal with
    //
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Azathoths wrath
// -----------------------------------------------------------------------------
void SpellAzaWrath::run_effect(Actor* const caster,
                               const SpellSkill skill) const
{
    // if (caster == map::player &&
    //     player_bon::traits[(size_t)Trait::warlock] &&
    //     rnd::percent(warlock_multi_cast_chance_pct))
    // {
    //     run_effect(caster, skill);

    //     if (!caster->is_alive())
    //     {
    //         return;
    //     }
    // }

    const auto targets = caster->seen_foes();

    if (targets.empty())
    {
        if (caster->is_player())
        {
            msg_log::add(
                "There is a faint rumbling sound, like distant thunder.");
        }
    }

    io::draw_blast_at_seen_actors(targets, clr_red_lgt);

    for (Actor* const target : targets)
    {
        // Spell resistance?
        if (target->has_prop(PropId::r_spell))
        {
            on_resist(*target);

            // Spell reflection?
            if (target->has_prop(PropId::spell_reflect))
            {
                if (map::player->can_see_actor(*target))
                {
                    msg_log::add(spell_reflect_msg,
                                 clr_white,
                                 false,
                                 MorePromptOnMsg::yes);
                }

                // Run effect with the target as caster
                run_effect(target, skill);
            }

            continue;
        }

        std::string str_begin = "I am";

        Clr msg_clr = clr_msg_good;

        if (target->is_player())
        {
            msg_clr = clr_msg_bad;
        }
        else // Target is monster
        {
            str_begin = text_format::first_to_upper(target->name_the()) + " is";

            if (map::player->is_leader_of(target))
            {
                msg_clr = clr_white;
            }
        }

        if (map::player->can_see_actor(*target))
        {
            msg_log::add(str_begin + " struck by a roaring blast!", msg_clr);
        }

        // Skill  damage     avg
        // -----------------------
        // 0      2  - 5     3.5
        // 1      4  - 8     6.0
        // 2      6  - 11    8.5
        Range dmg_range(2 + (int)skill * 2,
                        5 + (int)skill * 3);

        target->hit(dmg_range.roll(),
                    DmgType::physical,
                    DmgMethod::END,
                    AllowWound::no);

        if (target->is_alive())
        {
            target->prop_handler().apply(
                new PropParalyzed(PropTurns::specific, 1));
        }

        if ((skill == SpellSkill::master) &&
            target->is_alive())
        {
            target->prop_handler().apply(
                new PropBurning(PropTurns::specific, 2));
        }

        Snd snd("",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                target->pos,
                nullptr,
                SndVol::high, AlertsMon::yes);

        snd_emit::run(snd);
    }
}

std::vector<std::string> SpellAzaWrath::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Channels the destructive force of Azathoth unto all visible enemies.");

    Range dmg_range(2 + (int)skill * 2,
                    5 + (int)skill * 3);

    descr.push_back(
        "The spell does " +
        std::to_string(dmg_range.min) +
        "-" +
        std::to_string(dmg_range.max) +
        " damage per creature.");

    if (skill == SpellSkill::master)
    {
        descr.push_back("The target is paralyzed, and set aflame.");
    }
    else // Not master
    {
        descr.push_back("The target is paralyzed.");
    }

    return descr;
}

bool SpellAzaWrath::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Mayhem
// -----------------------------------------------------------------------------
void SpellMayhem::run_effect(Actor* const caster,
                             const SpellSkill skill) const
{
    // if (caster == map::player &&
    //     player_bon::traits[(size_t)Trait::warlock] &&
    //     rnd::percent(warlock_multi_cast_chance_pct))
    // {
    //     run_effect(caster, skill);

    //     if (!caster->is_alive())
    //     {
    //         return;
    //     }
    // }

    const bool is_player = caster->is_player();

    if (map::player->can_see_actor(*caster))
    {
        std::string caster_name =
            is_player ?
            "me" :
            caster->name_the();

        msg_log::add("Destruction rages around " + caster_name + "!");
    }

    const P& caster_pos = caster->pos;

    //
    // Run explosions
    //
    {
        const int expl_area_radi = fov_std_radi_int + 2;

        const int x0 = std::max(1, caster_pos.x - expl_area_radi);
        const int y0 = std::max(1, caster_pos.y - expl_area_radi);
        const int x1 = std::min(map_w - 1, caster_pos.x + expl_area_radi) - 1;
        const int y1 = std::min(map_h - 1, caster_pos.y + expl_area_radi) - 1;

        std::vector<P> p_bucket;

        const int expl_radi_diff = -1;

        for (int y = y0; y <= y1; ++y)
        {
            for (int x = x0; x <= x1; ++x)
            {
                const P p(x, y);

                const int dist = king_dist(caster_pos, p);

                const int min_dist = expl_std_radi + 1 + expl_radi_diff;

                if (dist >= min_dist)
                {
                    p_bucket.push_back(p);
                }
            }
        }

        int nr_expl = 2 + (int)skill;

        for (int i = 0; i < nr_expl; ++i)
        {
            if (p_bucket.empty())
            {
                return;
            }

            const size_t idx = rnd::range(0, p_bucket.size() - 1);

            const P& p = rnd::element(p_bucket);

            explosion::run(p,
                           ExplType::expl,
                           ExplSrc::misc,
                           EmitExplSnd::yes,
                           expl_radi_diff);

            p_bucket.erase(p_bucket.begin() + idx);
        }
    }

    //
    // Destroy the surrounding environment
    //
    {
        const int nr_sweeps = 1 + (int)skill;

        const int destr_radi = fov_std_radi_int + (int)skill * 2;

        const int x0 = std::max(1, caster_pos.x - destr_radi);
        const int y0 = std::max(1, caster_pos.y - destr_radi);
        const int x1 = std::min(map_w - 1, caster_pos.x + destr_radi) - 1;
        const int y1 = std::min(map_h - 1, caster_pos.y + destr_radi) - 1;

        for (int i = 0; i < nr_sweeps; ++i)
        {
            for (int x = x0; x <= x1; ++x)
            {
                for (int y = y0; y <= y1; ++y)
                {
                    bool is_adj_to_walkable_cell = false;

                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        for (int dy = -1; dy <= 1; ++dy)
                        {
                            const Rigid* const f =
                                map::cells[x + dx][y + dy].rigid;

                            if (f->can_move_common())
                            {
                                is_adj_to_walkable_cell = true;
                            }
                        }
                    }

                    if (is_adj_to_walkable_cell &&
                        rnd::one_in(8))
                    {
                        map::cells[x][y].rigid->hit(
                            1, // Damage (doesn't matter)
                            DmgType::physical,
                            DmgMethod::explosion,
                            nullptr);
                    }
                }
            }
        }

        // Let's add some extra chaos - put blood, and set stuff on fire
        for (int y = y0; y <= y1; ++y)
        {
            for (int x = x0; x <= x1; ++x)
            {
                auto* const f = map::cells[x][y].rigid;

                if (f->can_have_blood() &&
                    rnd::one_in(10))
                {
                    f->mk_bloody();

                    if (rnd::one_in(3))
                    {
                        f->try_put_gore();
                    }
                }

                if ((P(x, y) != caster->pos) &&
                    rnd::one_in(6))
                {
                    f->hit(1, // Damage (doesn't matter)
                           DmgType::fire,
                           DmgMethod::elemental,
                           nullptr);
                }
            }
        }
    }

    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            caster_pos,
            nullptr,
            SndVol::high,
            AlertsMon::yes);

    snd_emit::run(snd);
}

std::vector<std::string> SpellMayhem::descr_specific(
    const SpellSkill skill) const
{
    (void)skill;

    std::vector<std::string> descr;

    descr.push_back("Blasts the surrounding area with terrible force.");

    descr.push_back("Higher skill levels increases the magnitude of the "
                    "destruction.");

    return descr;
}

bool SpellMayhem::allow_mon_cast_now(Mon& mon) const
{
    return
        mon.tgt_ &&
        (mon.is_tgt_seen_ || rnd::one_in(20));
}

// -----------------------------------------------------------------------------
// Pestilence
// -----------------------------------------------------------------------------
void SpellPest::run_effect(Actor* const caster,
                           const SpellSkill skill) const
{
    const size_t nr_mon = 6 + (int)skill * 6;

    Actor* leader = nullptr;

    bool did_player_summon_hostile = false;

    if (caster->is_player())
    {
        did_player_summon_hostile = rnd::one_in(summon_hostile_one_in_n);

        leader = did_player_summon_hostile ? nullptr : caster;
    }
    else // Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;

        leader =
            caster_leader ?
            caster_leader :
            caster;
    }

    const auto mon_summoned =
        actor_factory::spawn(
            caster->pos,
            {nr_mon, ActorId::rat},
            MakeMonAware::yes,
            leader);

    if (mon_summoned.empty())
    {
        return;
    }

    bool is_any_seen_by_player = false;

    for (Mon* const mon : mon_summoned)
    {
        mon->prop_handler().apply(
            new PropSummoned(PropTurns::indefinite));

        mon->prop_handler().apply(
            new PropWaiting(PropTurns::specific, 2));

        if (map::player->can_see_actor(*mon))
        {
            is_any_seen_by_player = true;
        }

        // Haste the rats if master
        if (skill == SpellSkill::master)
        {
            mon->prop_handler().apply(
                new PropHasted(PropTurns::indefinite),
                PropSrc::intr,
                true,
                Verbosity::silent);
        }
    }

    if (caster->is_player() ||
        is_any_seen_by_player)
    {
        std::string caster_str = "me";

        if (!caster->is_player())
        {
            if (map::player->can_see_actor(*caster))
            {
                caster_str = caster->name_the();
            }
            else
            {
                caster_str = "it";
            }
        }

        msg_log::add("Rats appear around " + caster_str + "!");

        if (did_player_summon_hostile)
        {
            msg_log::add("They are hostile!",
                         clr_msg_note,
                         true,
                         MorePromptOnMsg::yes);
        }
    }
}

std::vector<std::string> SpellPest::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back("A pack of rats appear around the caster.");

    const size_t nr_mon = 6 + (int)skill * 6;

    descr.push_back("Summons " + std::to_string(nr_mon) + " rats.");

    if (skill == SpellSkill::master)
    {
        descr.push_back("The rats are Hasted (+100% speed).");
    }

    descr.push_back(summon_warning_str);

    return descr;
}

bool SpellPest::allow_mon_cast_now(Mon& mon) const
{
    return
        mon.tgt_ &&
        (mon.is_tgt_seen_ || rnd::one_in(30));
}

// -----------------------------------------------------------------------------
// Animate weapons
// -----------------------------------------------------------------------------
void SpellAnimWpns::run_effect(Actor* const caster,
                               const SpellSkill skill) const
{
    if (!caster->is_player())
    {
        return;
    }

    Actor* actor_array[map_w][map_h];

    map::mk_actor_array(actor_array);

    std::vector<P> positions_to_anim;

    {
        std::vector<P> anim_pos_bucket;

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                Cell& cell = map::cells[x][y];

                Item* const item = cell.item;

                if (!cell.is_seen_by_player ||
                    actor_array[x][y] ||
                    !item ||
                    (item->data().type != ItemType::melee_wpn))
                {
                    continue;
                }

                // OK, we have an item here that we can animate
                anim_pos_bucket.push_back(P(x, y));
            }
        }

        if (anim_pos_bucket.empty())
        {
            msg_log::add(
                "The dust and gravel on the ground starts shooting everywhere.");

            return;
        }

        if (skill == SpellSkill::basic)
        {
            // Animate one random weapon
            positions_to_anim.push_back(rnd::element(anim_pos_bucket));
        }
        else // Skill is higher than basic
        {
            // Animate all possible weapons
            positions_to_anim = anim_pos_bucket;
        }
    }

    for (const P& p : positions_to_anim)
    {
        const auto summoned =
            actor_factory::spawn(
                p,
                {1, ActorId::animated_wpn},
                MakeMonAware::no,
                map::player);

        if (summoned.empty())
        {
            return;
        }

        Mon* const anim_wpn = summoned[0];

        Cell& cell = map::cells[p.x][p.y];

        Item* const item = cell.item;

        cell.item = nullptr;

        Inventory& inv = anim_wpn->inv();

        ASSERT(!inv.item_in_slot(SlotId::wpn));

        inv.put_in_slot(SlotId::wpn,
                        item,
                        Verbosity::silent);

        const std::string item_name =
            item->name(ItemRefType::plain,
                       ItemRefInf::yes,
                       ItemRefAttInf::none);

        msg_log::add("The " + item_name + " rises into thin air!");

        if (skill == SpellSkill::master)
        {
            anim_wpn->prop_handler().apply(
                new PropSeeInvis(PropTurns::indefinite),
                PropSrc::intr,
                true,
                Verbosity::silent);
        }

        anim_wpn->prop_handler().apply(
            new PropWaiting(PropTurns::specific, 2));
    }
}

std::vector<std::string> SpellAnimWpns::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Infuses lifeless weapons with a spirit of their own, causing "
        "them to rise up into the air and protect their master (for a "
        "while). It is only possible to animate basic melee weapons "
        "however - \"modern\" mechanisms such as pistols or machine guns "
        "are far too complex.");

    if (skill == SpellSkill::basic)
    {
        descr.push_back("Animates one visible weapon.");
    }
    else // Skill is higher than basic
    {
        descr.push_back("Animates all visible weapons.");
    }

    if (skill == SpellSkill::master)
    {
        descr.push_back("The weapons can see invisible creatures.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Subdue Animated weapons
// -----------------------------------------------------------------------------
void SpellSubdueWpns::run_effect(Actor* const caster,
                                 const SpellSkill skill) const
{
    (void)skill;

    if (!caster->is_player())
    {
        return;
    }

    // Drop all seen animated weapons

    for (auto* const actor : game_time::actors)
    {
        if ((actor->id() != ActorId::animated_wpn) ||
            !map::player->can_see_actor(*actor))
        {
            continue;
        }

        auto* anim_wpn = static_cast<AnimatedWpn*>(actor);

        anim_wpn->drop();
    }
}

std::vector<std::string> SpellSubdueWpns::descr_specific(
    const SpellSkill skill) const
{
    (void)skill;

    std::vector<std::string> descr;

    descr.push_back(
        "Causes all visible Animated Weapons to become lifeless, and drop down "
        "to the ground again.");

    return descr;
}

// -----------------------------------------------------------------------------
// Pharaoh staff
// -----------------------------------------------------------------------------
void SpellPharaohStaff::run_effect(Actor* const caster,
                                   const SpellSkill skill) const
{
    (void)skill;

    // First try to heal a friendly mummy (as per the spell description)
    for (Actor* const actor : game_time::actors)
    {
        const auto actor_id = actor->data().id;

        const bool is_actor_id_ok = actor_id == ActorId::mummy ||
                                    actor_id == ActorId::croc_head_mummy;

        if (is_actor_id_ok && caster->is_leader_of(actor))
        {
            actor->restore_hp(999);

            return;
        }
    }

    // This point reached means no mummy controlled, summon a new one
    Actor* leader = nullptr;

    bool did_player_summon_hostile = false;

    if (caster->is_player())
    {
        did_player_summon_hostile = rnd::one_in(summon_hostile_one_in_n);

        leader =
            did_player_summon_hostile ?
            nullptr :
            caster;
    }
    else // Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;

        leader =
            caster_leader ?
            caster_leader :
            caster;
    }

    const auto actor_id =
        rnd::coin_toss() ?
        ActorId::mummy :
        ActorId::croc_head_mummy;

    const auto summoned_mon =
        actor_factory::spawn(caster->pos,
                              {actor_id},
                              MakeMonAware::yes,
                              leader);

    Mon* const mon = summoned_mon[0];

    mon->prop_handler().apply(
        new PropWaiting(PropTurns::specific, 2));

    if (map::player->can_see_actor(*mon))
    {
        msg_log::add(text_format::first_to_upper(mon->name_a()) + " appears!");

        if (did_player_summon_hostile)
        {
            msg_log::add("It is hostile!",
                         clr_msg_note,
                         true,
                         MorePromptOnMsg::yes);
        }
    }
    else // Player cannot see monster
    {
        msg_log::add("I sense a new presence.");
    }
}

std::vector<std::string> SpellPharaohStaff::descr_specific(
    const SpellSkill skill) const
{
    (void)skill;

    return
    {
        "Summons a loyal Mummy servant which will fight for the caster.",

        "If an allied Mummy is already present, this spell will instead "
        "heal it.",

        summon_warning_str
    };
}

bool SpellPharaohStaff::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_;
}

// -----------------------------------------------------------------------------
// Searching
// -----------------------------------------------------------------------------
void SpellSearching::run_effect(Actor* const caster,
                                const SpellSkill skill) const
{
    (void)caster;

    const int range =
        (skill == SpellSkill::basic) ? 8 :
        (skill == SpellSkill::expert) ? 16 :
        999;

    const int orig_x = map::player->pos.x;
    const int orig_y = map::player->pos.y;

    const int x0 = std::max(0, orig_x - range);
    const int y0 = std::max(0, orig_y - range);
    const int x1 = std::min(map_w - 1, orig_x + range);
    const int y1 = std::min(map_h - 1, orig_y + range);

    std::vector<P> positions_detected;

    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            auto& cell = map::cells[x][y];

            auto* const f = cell.rigid;

            const auto id = f->id();

            if ((id == FeatureId::trap) ||
                (id == FeatureId::door) ||
                (id == FeatureId::monolith) ||
                (id == FeatureId::stairs))
            {
                f->reveal(Verbosity::silent);

                cell.is_seen_by_player = true;

                cell.is_explored = true;

                positions_detected.push_back(P(x, y));
            }

            if (((int)skill >= (int)SpellSkill::expert) &&
                cell.item)
            {
                cell.is_seen_by_player = true;

                cell.is_explored = true;

                positions_detected.push_back(P(x, y));
            }
        }
    }

    const int det_mon_multiplier = 20;

    for (Actor* actor : game_time::actors)
    {
        const P& p = actor->pos;

        if (!actor->is_player() &&
            actor->is_alive() &&
            (king_dist(caster->pos, p) <= range))
        {
            static_cast<Mon*>(actor)->set_player_aware_of_me(
                det_mon_multiplier);

            positions_detected.push_back(p);
        }
    }

    if (positions_detected.empty())
    {
        msg_log::add("I find nothing.");
    }
    else // Items detected
    {
        // To update the render data
        states::draw();

        map::player->update_fov();

        states::draw();

        io::draw_blast_at_cells(positions_detected, clr_white);
    }
}

std::vector<std::string> SpellSearching::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Reveals the presence of creatures, doors, traps, stairs, and other "
        "locations of interest in the surrounding area.");

    if ((int)skill >= (int)SpellSkill::expert)
    {
        descr.push_back("Also reveals items.");
    }

    if (skill == SpellSkill::master)
    {
        descr.push_back("Detects across the whole map.");
    }
    else // Not master
    {
        const int range =
            (skill == SpellSkill::basic) ?
            8 :
            16;

        descr.push_back("The spell has a range of " +
                        std::to_string(range) +
                        " cells.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Opening
// -----------------------------------------------------------------------------
void SpellOpening::run_effect(Actor* const caster,
                              const SpellSkill skill) const
{
    (void)caster;

    const int range = 1 + (int)skill * 3;

    const int orig_x = map::player->pos.x;
    const int orig_y = map::player->pos.y;

    const int x0 = std::max(0, orig_x - range);
    const int y0 = std::max(0, orig_y - range);
    const int x1 = std::min(map_w - 1, orig_x + range);
    const int y1 = std::min(map_h - 1, orig_y + range);

    bool is_any_opened = false;

    const int chance_to_open = 50 + (int)skill * 25;

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            if (!rnd::percent(chance_to_open))
            {
                continue;
            }

            const auto& cell = map::cells[x][y];

            auto* const f = cell.rigid;

            auto did_open = DidOpen::no;

            bool is_metal_door = false;

            // Is this a metal door?
            if (f->id() == FeatureId::door)
            {
                auto* const door = static_cast<Door*>(f);

                is_metal_door = (door->type() == DoorType::metal);

                // If at least expert skill, then metal doors are also opened
                if (is_metal_door &&
                    !door->is_open() &&
                    ((int)skill >= (int)SpellSkill::expert))
                {
                    for (int x_lever = 0; x_lever < map_w; ++x_lever)
                    {
                        for (int y_lever = 0; y_lever < map_h; ++y_lever)
                        {
                            auto* const f_lever =
                                map::cells[x_lever][y_lever].rigid;

                            if (f_lever->id() != FeatureId::lever)
                            {
                                continue;
                            }

                            auto* const lever = static_cast<Lever*>(f_lever);

                            if (lever->is_linked_to(*f))
                            {
                                lever->toggle();

                                did_open = DidOpen::yes;

                                break;
                            }
                        } // Lever y loop

                        if (did_open == DidOpen::yes)
                        {
                            break;
                        }
                    } // Lever x loop
                }
            }

            if ((did_open != DidOpen::yes) &&
                !is_metal_door)
            {
                did_open = cell.rigid->open(nullptr);
            }

            if (did_open == DidOpen::yes)
            {
                is_any_opened = true;
            }
        }
    }

    if (is_any_opened)
    {
        map::update_vision();
    }
    else // Nothing was opened
    {
        msg_log::add("I hear faint rattling and knocking.");
    }
}

std::vector<std::string> SpellOpening::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    std::string str = "Opens all locks, lids and doors";

    if (skill == SpellSkill::basic)
    {
        str += " (except heavy doors operated externally by a switch)";
    }

    str += ".";

    descr.push_back(str);

    if ((int)skill >= (int)SpellSkill::expert)
    {
        descr.push_back(
            "If cast within range of a door operated by a lever, then the "
            "lever is toggled.");
    }

    const int range = 1 + (int)skill * 3;

    if (range == 1)
    {
        descr.push_back("Only adjacent objects are opened.");
    }
    else // Range > 1
    {
        descr.push_back("Opens objects within a distance of " +
                        std::to_string(range) +
                        " cells.");
    }

    const int chance_to_open = 50 + (int)skill * 25;

    descr.push_back(std::to_string(chance_to_open) + "% chance to open.");

    return descr;
}

// -----------------------------------------------------------------------------
// Ghoul frenzy
// -----------------------------------------------------------------------------
void SpellFrenzy::run_effect(Actor* const caster,
                             const SpellSkill skill) const
{
    (void)skill;

    const int nr_turns = rnd::range(12, 18);

    PropFrenzied* frenzy = new PropFrenzied(PropTurns::specific, nr_turns);

    caster->prop_handler().apply(frenzy);
}

std::vector<std::string> SpellFrenzy::descr_specific(
    const SpellSkill skill) const
{
    (void)skill;

    return
    {
        "Incites a great rage in the caster, which will charge their "
        "enemies with a terrible, uncontrollable fury."
    };
}

// -----------------------------------------------------------------------------
// Bless
// -----------------------------------------------------------------------------
void SpellBless::run_effect(Actor* const caster,
                            const SpellSkill skill) const
{
    const int nr_turns = 20 + (int)skill * 100;

    caster->prop_handler().apply(
        new PropBlessed(PropTurns::specific, nr_turns));
}

std::vector<std::string> SpellBless::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back("Bends reality in favor of the caster.");

    const int nr_turns = 20 + (int)skill * 60;

    descr.push_back(
        "The spell lasts " +
        std::to_string(nr_turns) +
        " turns.");

    return descr;
}

// -----------------------------------------------------------------------------
// Light
// -----------------------------------------------------------------------------
void SpellLight::run_effect(Actor* const caster,
                            const SpellSkill skill) const
{
    const int nr_turns = 20 + (int)skill * 20;

    caster->prop_handler().apply(
        new PropRadiant(PropTurns::specific, nr_turns));

    if (skill == SpellSkill::master)
    {
        std::vector<Prop*> props;

        props.push_back(new PropBlind(PropTurns::std));

        explosion::run(caster->pos,
                       ExplType::apply_prop,
                       ExplSrc::misc,
                       EmitExplSnd::no,
                       0,
                       ExplExclCenter::yes,
                       props,
                       &clr_yellow);
    }
}

std::vector<std::string> SpellLight::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back("Illuminates the area around the caster.");

    const int nr_turns = 20 + (int)skill * 20;

    descr.push_back(
        "The spell lasts " +
        std::to_string(nr_turns) +
        " turns.");

    if (skill == SpellSkill::master)
    {
        descr.push_back(
            "On casting, causes a blinding flash centered on the caster "
            "(but not affecting the caster itself).");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// See Invisible
// -----------------------------------------------------------------------------
void SpellSeeInvis::run_effect(Actor* const caster,
                            const SpellSkill skill) const
{
    const Range duration_range =
        (skill == SpellSkill::basic) ? Range(5, 8) :
        (skill == SpellSkill::expert) ? Range(40, 80) :
        Range(400, 600);

    caster->prop_handler().apply(
        new PropSeeInvis(PropTurns::specific, duration_range.roll()));
}

std::vector<std::string> SpellSeeInvis::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Grants the caster the ability to see that which is "
        "normally invisible.");

    const Range duration_range =
        (skill == SpellSkill::basic) ? Range(5, 8) :
        (skill == SpellSkill::expert) ? Range(40, 80) :
        Range(400, 600);

    descr.push_back(
        "The spell lasts " +
        std::to_string(duration_range.min) +
        "-" +
        std::to_string(duration_range.max) +
        " turns.");

    return descr;
}

bool SpellSeeInvis::allow_mon_cast_now(Mon& mon) const
{
    return
        !mon.has_prop(PropId::see_invis) &&
        (mon.aware_of_player_counter_ > 0) &&
        rnd::one_in(8);
}

// -----------------------------------------------------------------------------
// Spell Shield
// -----------------------------------------------------------------------------
int SpellSpellShield::max_spi_cost(const SpellSkill skill) const
{
    return 5 - (int)skill;
}

void SpellSpellShield::run_effect(Actor* const caster,
                                  const SpellSkill skill) const
{
    (void)skill;

    caster->prop_handler().apply(new PropRSpell(PropTurns::indefinite));
}

std::vector<std::string> SpellSpellShield::descr_specific(
    const SpellSkill skill) const
{
    (void)skill;

    std::vector<std::string> descr;

    descr.push_back(
        "Grants protection against harmful spells. The effect lasts until a "
        "spell is blocked.");

    descr.push_back("Skill level affects the amount of Spirit one needs to "
                    "spend to cast the spell.");

    return descr;
}

bool SpellSpellShield::allow_mon_cast_now(Mon& mon) const
{
    return !mon.has_prop(PropId::r_spell);
}

// -----------------------------------------------------------------------------
// Teleport
// -----------------------------------------------------------------------------
void SpellTeleport::run_effect(Actor* const caster,
                               const SpellSkill skill) const
{
    if ((int)skill >= (int)SpellSkill::expert)
    {
        const int nr_turns = 3;

        caster->prop_handler().apply(
            new PropInvisible(PropTurns::specific, nr_turns));
    }

    auto should_ctrl = ShouldCtrlTele::if_tele_ctrl_prop;

    if ((skill == SpellSkill::master) &&
        (caster == map::player))
    {
        should_ctrl = ShouldCtrlTele::always;
    }

    caster->teleport(should_ctrl);
}

bool SpellTeleport::allow_mon_cast_now(Mon& mon) const
{
    const bool is_low_hp = mon.hp() <= (mon.hp_max(true) / 2);

    return
        (mon.aware_of_player_counter_ > 0) &&
        is_low_hp &&
        rnd::fraction(3, 4);
}

std::vector<std::string> SpellTeleport::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back("Instantly moves the caster to a different position.");

    if ((int)skill >= (int)SpellSkill::expert)
    {
        const int nr_turns = 3;

        descr.push_back(
            "On teleporting, the caster is invisible for " +
            std::to_string(nr_turns) +
            " turns.");
    }

    if (skill == SpellSkill::master)
    {
        descr.push_back("The caster can control the teleport destination.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Resistance
// -----------------------------------------------------------------------------
void SpellRes::run_effect(Actor* const caster,
                          const SpellSkill skill) const
{
    int nr_turns = 10 + (int)skill * 50;

    PropHandler& prop_hlr = caster->prop_handler();

    prop_hlr.apply(new PropRFire(PropTurns::specific, nr_turns));
    prop_hlr.apply(new PropRElec(PropTurns::specific, nr_turns));
}

std::vector<std::string> SpellRes::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    int nr_turns = 10 + (int)skill * 50;

    descr.push_back(
        "The caster is completely shielded from fire and electricity.");

    descr.push_back(
        "The spell lasts " +
        std::to_string(nr_turns) +
        " turns.");

    return descr;
}

bool SpellRes::allow_mon_cast_now(Mon& mon) const
{
    const bool has_rfire = mon.prop_handler().has_prop(PropId::r_fire);
    const bool has_relec = mon.prop_handler().has_prop(PropId::r_elec);

    return
        (!has_rfire || !has_relec) &&
        mon.tgt_;
}

// -----------------------------------------------------------------------------
// Knockback
// -----------------------------------------------------------------------------
void SpellKnockBack::run_effect(Actor* const caster,
                                const SpellSkill skill) const
{
    (void)skill;

    ASSERT(!caster->is_player());

    Clr msg_clr = clr_msg_good;

    std::string target_str = "me";

    Actor* caster_used = caster;

    auto* const mon = static_cast<Mon*>(caster_used);

    Actor* target = mon->tgt_;

    ASSERT(target);

    ASSERT(mon->is_tgt_seen_);

    // Spell resistance?
    if (target->has_prop(PropId::r_spell))
    {
        on_resist(*target);

        // Spell reflection?
        if (target->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*target))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            std::swap(caster_used, target);
        }
        else // No spell reflection, just abort
        {
            return;
        }
    }

    if (target->is_player())
    {
        msg_clr = clr_msg_bad;
    }
    else // Target is monster
    {
        target_str = target->name_the();

        if (map::player->is_leader_of(target)) {msg_clr = clr_white;}
    }

    if (map::player->can_see_actor(*target))
    {
        msg_log::add("A force pushes " + target_str + "!", msg_clr);
    }

    knockback::run(*target, caster->pos, false);
}

bool SpellKnockBack::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Enfeeble
// -----------------------------------------------------------------------------
void SpellEnfeeble::run_effect(Actor* const caster,
                                  const SpellSkill skill) const
{
    auto targets = caster->seen_foes();

    if (targets.empty())
    {
        switch (skill)
        {
        case SpellSkill::basic:
            msg_log::add(
                "The bugs on the ground suddenly moves very feebly.");
            break;

        case SpellSkill::expert:
            msg_log::add(
                "The bugs on the ground suddenly moves very slowly.");
            break;

        case SpellSkill::master:
            msg_log::add(
                "The bugs on the ground suddenly stops moving.");
            break;

        default:
            ASSERT(false);
            break;
        }

        return;
    }

    // There are targets available

    io::draw_blast_at_seen_actors(targets, clr_magenta);

    for (Actor* const target : targets)
    {
        PropHandler& prop_handler = target->prop_handler();

        // Spell resistance?
        if (target->has_prop(PropId::r_spell))
        {
            on_resist(*target);

            // Spell reflection?
            if (target->has_prop(PropId::spell_reflect))
            {
                if (map::player->can_see_actor(*target))
                {
                    msg_log::add(spell_reflect_msg,
                                 clr_text,
                                 false,
                                 MorePromptOnMsg::yes);
                }

                // Run effect with target as caster
                run_effect(target, skill);
            }

            continue;
        }

        prop_handler.apply(new PropWeakened(PropTurns::std));

        if ((int)skill >= (int)SpellSkill::expert)
        {
            prop_handler.apply(new PropSlowed(PropTurns::std));
        }

        if (skill >= SpellSkill::master)
        {
            //
            // NOTE: Being slowed effectively causes longer paralyze duration
            //       (property turns counts down on the actor's turn), so
            //       applying a full standard paralyze here would probably be
            //       overpowered - just do a couple turns
            //
            prop_handler.apply(new PropParalyzed(PropTurns::specific, 2));
        }
    }
}

std::vector<std::string> SpellEnfeeble::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back("Physically enfeebles all visible hostile creatures.");

    std::string str = "Causes weakening (halved melee damage)";

    if (skill == SpellSkill::expert)
    {
        str += " and slowing";
    }
    else if (skill == SpellSkill::master)
    {
        str += ", slowing, and paralyzation";
    }

    str += ".";

    descr.push_back(str);

    return descr;
}

bool SpellEnfeeble::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Disease
// -----------------------------------------------------------------------------
void SpellDisease::run_effect(Actor* const caster,
                              const SpellSkill skill) const
{
    (void)skill;

    ASSERT(!caster->is_player());

    Actor* caster_used = caster;

    auto* const mon = static_cast<Mon*>(caster_used);

    Actor* target = mon->tgt_;

    ASSERT(target);

    ASSERT(mon->is_tgt_seen_);

    // Spell resistance?
    if (target->has_prop(PropId::r_spell))
    {
        on_resist(*target);

        // Spell reflection?
        if (target->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*target))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            std::swap(caster_used, target);
        }
        else // No spell reflection, just abort
        {
            return;
        }
    }

    std::string actor_name = "me";

    if (!target->is_player())
    {
        actor_name = target->name_the();
    }

    if (map::player->can_see_actor(*target))
    {
        msg_log::add("A horrible disease is starting to afflict " +
                     actor_name +
                     "!");
    }

    target->prop_handler().apply(new PropDiseased(PropTurns::std));
}

bool SpellDisease::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Summon monster
// -----------------------------------------------------------------------------
void SpellSummonMon::run_effect(Actor* const caster,
                                const SpellSkill skill) const
{
    // Try to summon a creature inside the player's FOV (inside the standard
    // range), in a free visible cell. If no such cell is available, instead
    // summon near the caster.

    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCommon(ParseActors::yes)
        .run(blocked);

    std::vector<P> free_cells_seen_by_player;

    const int radi = fov_std_radi_int;

    const P player_pos(map::player->pos);

    const int x0 = std::max(0, player_pos.x - radi);
    const int y0 = std::max(0, player_pos.y - radi);
    const int x1 = std::min(map_w, player_pos.x + radi) - 1;
    const int y1 = std::min(map_h, player_pos.y + radi) - 1;

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            if (!blocked[x][y] && map::cells[x][y].is_seen_by_player)
            {
                free_cells_seen_by_player.push_back(P(x, y));
            }
        }
    }

    P summon_pos(-1, -1);

    if (free_cells_seen_by_player.empty())
    {
        // No free cells seen by player, instead summon near the caster.
        auto free_cells_vector = to_vec(blocked, false);

        if (!free_cells_vector.empty())
        {
            sort(free_cells_vector.begin(), free_cells_vector.end(),
                 IsCloserToPos(caster->pos));
            summon_pos = free_cells_vector[0];
        }
    }
    else // There are free cells seen by the player available
    {
        const size_t idx =
            rnd::range(0, free_cells_seen_by_player.size() - 1);

        summon_pos = free_cells_seen_by_player[idx];
    }

    std::vector<ActorId> summon_bucket;

    if (caster->is_player())
    {
        // Player summong, pick from predefined lists
        if ((skill == SpellSkill::basic) ||
            (skill == SpellSkill::expert))
        {
            summon_bucket.push_back(ActorId::rat_thing);
            summon_bucket.push_back(ActorId::floating_skull);
            summon_bucket.push_back(ActorId::raven);
            summon_bucket.push_back(ActorId::pit_viper);
            summon_bucket.push_back(ActorId::wolf);
            summon_bucket.push_back(ActorId::ooze_black);
        }

        if ((skill == SpellSkill::expert) ||
            (skill == SpellSkill::master))
        {
            summon_bucket.push_back(ActorId::green_spider);
            summon_bucket.push_back(ActorId::white_spider);
            summon_bucket.push_back(ActorId::red_spider);
            summon_bucket.push_back(ActorId::fire_hound);
            summon_bucket.push_back(ActorId::giant_bat);
            summon_bucket.push_back(ActorId::vampire_bat);
            summon_bucket.push_back(ActorId::ooze_poison);
            summon_bucket.push_back(ActorId::fire_vortex);
        }

        if (skill == SpellSkill::master)
        {
            summon_bucket.push_back(ActorId::leng_spider);
            summon_bucket.push_back(ActorId::energy_hound);
            summon_bucket.push_back(ActorId::energy_vortex);
        }
    }
    else // Caster is monster
    {
        int max_dlvl_spawned =
            (skill == SpellSkill::basic) ? dlvl_last_early_game :
            (skill == SpellSkill::expert) ? dlvl_last_mid_game :
            dlvl_last;

        const int nr_dlvls_ood_allowed = 3;

        max_dlvl_spawned =
            std::min(max_dlvl_spawned,
                     map::dlvl + nr_dlvls_ood_allowed);

        const int min_dlvl_spawned = max_dlvl_spawned - 10;

        for (size_t i = 0; i < (size_t)ActorId::END; ++i)
        {
            const ActorDataT& data = actor_data::data[i];

            if (!data.can_be_summoned)
            {
                continue;
            }

            if ((data.spawn_min_dlvl <= max_dlvl_spawned) &&
                (data.spawn_min_dlvl >= min_dlvl_spawned))
            {
                summon_bucket.push_back(ActorId(i));
            }
        }

        // If no monsters could be found which matched the level criteria,
        // try again, but without the min level criterium
        if (summon_bucket.empty())
        {
            for (size_t i = 0; i < (size_t)ActorId::END; ++i)
            {
                const ActorDataT& data = actor_data::data[i];

                if (!data.can_be_summoned)
                {
                    continue;
                }

                if (data.spawn_min_dlvl <= max_dlvl_spawned)
                {
                    summon_bucket.push_back(ActorId(i));
                }
            }
        }
    }

    if (summon_bucket.empty())
    {
        TRACE << "No elligible monsters found for spawning" << std::endl;

        ASSERT(false);

        return;
    }

#ifndef NDEBUG
    for (const auto id : summon_bucket)
    {
        ASSERT(actor_data::data[(size_t)id].can_be_summoned);
    }
#endif // NDEBUG

    const ActorId mon_id = rnd::element(summon_bucket);

    Actor* leader = nullptr;

    bool did_player_summon_hostile = false;

    if (caster->is_player())
    {
        did_player_summon_hostile = rnd::one_in(summon_hostile_one_in_n);

        leader =
            did_player_summon_hostile ?
            nullptr :
            caster;
    }
    else // Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;

        leader = caster_leader ? caster_leader : caster;
    }

    const auto mon_summoned =
        actor_factory::spawn(summon_pos,
                              {mon_id},
                              MakeMonAware::yes,
                              leader);

    if (mon_summoned.empty())
    {
        return;
    }

    Mon* const mon = mon_summoned[0];

    mon->prop_handler().apply(
        new PropSummoned(PropTurns::indefinite));

    mon->prop_handler().apply(
        new PropWaiting(PropTurns::specific, 2));

    if (map::player->can_see_actor(*mon))
    {
        msg_log::add(text_format::first_to_upper(mon->name_a()) + " appears!");

        if (did_player_summon_hostile)
        {
            msg_log::add("It is hostile!",
                         clr_msg_note,
                         true,
                         MorePromptOnMsg::yes);
        }
    }

    // Player cannot see monster
    else if (caster->is_player())
    {
        msg_log::add("I sense a new presence.");
    }
}

std::vector<std::string> SpellSummonMon::descr_specific(
    const SpellSkill skill) const
{
    (void)skill;

    return
    {
        "Summons a creature to do the caster's bidding. A more skilled "
        "sorcerer summons beings of greater might and rarity.",

        summon_warning_str
    };
}

bool SpellSummonMon::allow_mon_cast_now(Mon& mon) const
{
    return
        mon.tgt_ &&
        (mon.is_tgt_seen_ || rnd::one_in(20));
}

// -----------------------------------------------------------------------------
// Healing
// -----------------------------------------------------------------------------
void SpellHeal::run_effect(Actor* const caster,
                           const SpellSkill skill) const
{
    (void)skill;

    if ((int)skill >= (int)SpellSkill::expert)
    {
        caster->prop_handler().end_prop(PropId::infected);
        caster->prop_handler().end_prop(PropId::diseased);
        caster->prop_handler().end_prop(PropId::weakened);
    }

    if (skill == SpellSkill::master)
    {
        caster->prop_handler().end_prop(PropId::blind);
        caster->prop_handler().end_prop(PropId::poisoned);

        if (caster->is_player())
        {
            Prop* const wound_prop =
                map::player->prop_handler().prop(PropId::wound);

            if (wound_prop)
            {
                auto* const wound = static_cast<PropWound*>(wound_prop);

                wound->heal_one_wound();
            }
        }
    }

    const int nr_hp_restored = 8 + (int)skill * 8;

    caster->restore_hp(nr_hp_restored);
}

bool SpellHeal::allow_mon_cast_now(Mon& mon) const
{
    return mon.hp() < mon.hp_max(true);
}

std::vector<std::string> SpellHeal::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    const int nr_hp_restored = 8 + (int)skill * 8;

    descr.push_back(
        "Restores " + std::to_string(nr_hp_restored) + " Hit Points.");

    if (skill == SpellSkill::expert)
    {
        descr.push_back(
            "Cures infections, disease, and weakening.");
    }
    else if (skill == SpellSkill::master)
    {
        descr.push_back(
            "Cures infections, disease, weakening, blindness, and poisoning.");

        descr.push_back(
            "Heals one wound.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Mi-go hypnosis
// -----------------------------------------------------------------------------
void SpellMiGoHypno::run_effect(Actor* const caster,
                                const SpellSkill skill) const
{
    (void)skill;

    ASSERT(!caster->is_player());

    Actor* caster_used = caster;

    auto* const mon = static_cast<Mon*>(caster_used);

    Actor* target = mon->tgt_;

    ASSERT(target);

    ASSERT(mon->is_tgt_seen_);

    // Spell resistance?
    if (target->has_prop(PropId::r_spell))
    {
        on_resist(*target);

        // Spell reflection?
        if (target->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*target))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            std::swap(caster_used, target);
        }
        else // No spell reflection, just abort
        {
            return;
        }
    }

    if (target->is_player())
    {
        msg_log::add("There is a sharp droning in my head!");
    }

    if (rnd::coin_toss())
    {
        Prop* const prop = new PropFainted(PropTurns::specific,
                                           rnd::range(2, 10));

        target->prop_handler().apply(prop);
    }
    else
    {
        msg_log::add("I feel dizzy.");
    }
}

bool SpellMiGoHypno::allow_mon_cast_now(Mon& mon) const
{
    return
        mon.tgt_ &&
        mon.is_tgt_seen_ &&
        mon.tgt_->is_player();
}

// -----------------------------------------------------------------------------
// Immolation
// -----------------------------------------------------------------------------
void SpellBurn::run_effect(Actor* const caster,
                           const SpellSkill skill) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;

    auto* const mon = static_cast<Mon*>(caster_used);

    Actor* target = mon->tgt_;

    ASSERT(target);

    ASSERT(mon->is_tgt_seen_);

    // Spell resistance?
    if (target->has_prop(PropId::r_spell))
    {
        on_resist(*target);

        // Spell reflection?
        if (target->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*target))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            std::swap(caster_used, target);
        }
        else // No spell reflection, just abort
        {
            return;
        }
    }

    std::string target_str = "me";

    if (!target->is_player())
    {
        target_str = target->name_the();
    }

    if (map::player->can_see_actor(*target))
    {
        msg_log::add("Flames are rising around " + target_str + "!");
    }

    const int nr_turns = 2 + (int)skill;

    Prop* const prop = new PropBurning(PropTurns::specific, nr_turns);

    target->prop_handler().apply(prop);
}

bool SpellBurn::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Deafening
// -----------------------------------------------------------------------------
void SpellDeafen::run_effect(Actor* const caster,
                             const SpellSkill skill) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;

    auto* const mon = static_cast<Mon*>(caster_used);

    Actor* target = mon->tgt_;

    ASSERT(target);

    ASSERT(mon->is_tgt_seen_);

    // Spell resistance?
    if (target->has_prop(PropId::r_spell))
    {
        on_resist(*target);

        // Spell reflection?
        if (target->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*target))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            std::swap(caster_used, target);
        }
        else // No spell reflection, just abort
        {
            return;
        }
    }

    const int nr_turns = 75 + (int)skill * 75;

    Prop* const prop = new PropDeaf(PropTurns::specific, nr_turns);

    target->prop_handler().apply(prop);
}

bool SpellDeafen::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.is_tgt_seen_;
}

// -----------------------------------------------------------------------------
// Transmutation
// -----------------------------------------------------------------------------
void SpellTransmut::run_effect(Actor* const caster,
                               const SpellSkill skill) const
{
    (void)caster;

    const P& p = map::player->pos;

    auto& cell = map::cells[p.x][p.y];

    Item* item_before = cell.item;

    if (!item_before)
    {
        msg_log::add("There is a vague change in the air.");

        return;
    }

    // Player is standing on an item

    //
    // Get information on the existing item(s)
    //
    const bool is_stackable_before = item_before->data().is_stackable;

    const int nr_items_before =
        is_stackable_before ?
        item_before->nr_items_ :
        1;

    const auto item_type_before = item_before->data().type;

    int melee_wpn_plus = 0;

    melee_wpn_plus =
        (item_type_before == ItemType::melee_wpn) ?
        item_before->melee_dmg_plus_ :
        0;

    const auto id_before = item_before->id();

    std::string item_name_before = "The ";

    if (nr_items_before > 1)
    {
        item_name_before += item_before->name(ItemRefType::plural);
    }
    else // Single item
    {
        item_name_before += item_before->name(ItemRefType::plain);
    }

    //
    // Remove the existing item(s)
    //
    delete cell.item;

    cell.item = nullptr;

    if (cell.is_seen_by_player)
    {
        msg_log::add(item_name_before + " disappears.",
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);
    }

    //
    // Determine which item(s) to spawn, if any
    //
    int pct_chance_per_item = (int)skill * 20;

    std::vector<ItemId> id_bucket;

    // Converting a potion?
    if (item_type_before == ItemType::potion)
    {
        pct_chance_per_item += 50;

        for (size_t item_id = 0; (ItemId)item_id != ItemId::END; ++item_id)
        {
            if ((ItemId)item_id == id_before)
            {
                continue;
            }

            const auto& d = item_data::data[item_id];

            if (d.type == ItemType::potion)
            {
                id_bucket.push_back((ItemId)item_id);
            }
        }
    }
    // Converting a scroll?
    else if (item_type_before == ItemType::scroll)
    {
        pct_chance_per_item += 50;

        for (size_t item_id = 0; (ItemId)item_id != ItemId::END; ++item_id)
        {
            if ((ItemId)item_id == id_before)
            {
                continue;
            }

            const auto& d = item_data::data[item_id];

            if (d.type == ItemType::scroll)
            {
                id_bucket.push_back((ItemId)item_id);
            }
        }
    }
    // Converting a melee weapon (with at least one "plus")?
    else if ((item_type_before == ItemType::melee_wpn) &&
             (melee_wpn_plus >= 1))
    {
        pct_chance_per_item += (melee_wpn_plus * 5);

        for (size_t item_id = 0; (ItemId)item_id != ItemId::END; ++item_id)
        {
            const auto& d = item_data::data[item_id];

            if ((d.type == ItemType::potion) ||
                (d.type == ItemType::scroll))
            {
                id_bucket.push_back((ItemId)item_id);
            }
        }
    }

    ItemId id_new = ItemId::END;

    if (!id_bucket.empty())
    {
        // Which item?
        id_new = rnd::element(id_bucket);
    }

    int nr_items_new = 0;

    // How many items?
    for (int i = 0; i < nr_items_before; ++i)
    {
        if (rnd::percent(pct_chance_per_item))
        {
            ++nr_items_new;
        }
    }

    if ((id_new == ItemId::END) ||
        (nr_items_new < 1))
    {
        msg_log::add("Nothing appears.");

        return;
    }

    // OK, items are good, and player succeeded the rolls etc

    //
    // Spawn new item(s)
    //
    auto* const item_new =
        item_factory::mk_item_on_floor(id_new, map::player->pos);

    if (item_new->data().is_stackable)
    {
        item_new->nr_items_ = nr_items_new;
    }

    if (cell.is_seen_by_player)
    {
        const std::string item_name_new =
            text_format::first_to_upper(
                item_new->name(ItemRefType::plural));

        msg_log::add(item_name_new + " appears.");
    }
}

std::vector<std::string> SpellTransmut::descr_specific(
    const SpellSkill skill) const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Attempts to convert items (stand over an item when casting). "
        "The spell has a certain chance per item to succeed based on skill "
        "level and item type. On failure, the item is destroyed.");

    const int skill_bon = (int)skill * 20;

    const int chance_per_pot = skill_bon + 50;

    const int chance_per_scroll = skill_bon + 50;

    const int chance_per_wpn_plus = 5;

    const int chance_wpn_plus_1 = skill_bon + chance_per_wpn_plus;
    const int chance_wpn_plus_2 = skill_bon + chance_per_wpn_plus * 2;
    const int chance_wpn_plus_3 = skill_bon + chance_per_wpn_plus * 3;

    descr.push_back(
        "Converts Potions to a different type, with " +
        std::to_string(chance_per_pot) +
        "% chance per item.");

    descr.push_back(
        "Converts Manuscripts to a different type, with " +
        std::to_string(chance_per_scroll) +
        "% chance per item.");

    descr.push_back(
        "Melee weapons with at least +1 damage (not counting any damage bonus "
        "from skills) are converted to a Potion or Manuscript, with " +
        std::to_string(chance_wpn_plus_1) + "% chance for a +1 weapon, " +
        std::to_string(chance_wpn_plus_2) + "% chance for a +2 weapon, " +
        std::to_string(chance_wpn_plus_3) + "% chance for a +3 weapon, etc.");

    return descr;
}
