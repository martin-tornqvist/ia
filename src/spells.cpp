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
#include "player_spells.hpp"
#include "item_scroll.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "line_calc.hpp"
#include "sdl_base.hpp"
#include "player_bon.hpp"
#include "game.hpp"
#include "explosion.hpp"

namespace
{

const int summon_hostile_one_in_n = 7;

const int warlock_multi_cast_chance_pct = 25;

} // namespace

namespace spell_handling
{

Spell* random_spell_for_mon()
{
    std::vector<SpellId> bucket;

    for (int i = 0; i < (int)SpellId::END; ++i)
    {
        Spell* const spell = mk_spell_from_id(SpellId(i));

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
    case SpellId::enfeeble_mon:
        return new SpellEnfeebleMon;

    case SpellId::disease:
        return new SpellDisease;

    case SpellId::darkbolt:
        return new SpellDarkbolt;

    case SpellId::aza_wrath:
        return new SpellAzaWrath;

    case SpellId::summon:
        return new SpellSummonMon;

    case SpellId::heal_self:
        return new SpellHealSelf;

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

    case SpellId::det_items:
        return new SpellDetItems;

    case SpellId::det_traps:
        return new SpellDetTraps;

    case SpellId::det_mon:
        return new SpellDetMon;

    case SpellId::opening:
        return new SpellOpening;

    case SpellId::sacr_life:
        return new SpellSacrLife;

    case SpellId::sacr_spi:
        return new SpellSacrSpi;

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

    case SpellId::END:
        break;
    }

    ASSERT(false && "No spell found for ID");
    return nullptr;
}

} // spell_handling

Range Spell::spi_cost(Actor* const caster) const
{
    int cost_max = max_spi_cost();

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

        //
        // Traits reducing the cost of specific spells
        //

        //
        // TODO: Shouldn't this be done by calling virtual functions?
        //
        bool is_warlock =
            player_bon::traits[(size_t)Trait::warlock];

        bool is_seer =
            player_bon::traits[(size_t)Trait::seer];

        bool is_summoner =
            player_bon::traits[(size_t)Trait::summoner];

        switch (id())
        {
        case SpellId::darkbolt:
            if (is_warlock)
            {
                --cost_max;
            }
            break;

        case SpellId::aza_wrath:
            if (is_warlock)
            {
                --cost_max;
            }
            break;

        case SpellId::mayhem:
            if (is_warlock)
            {
                --cost_max;
            }
            break;

        case SpellId::det_mon:
            if (is_seer)
            {
                --cost_max;
            }
            break;

        case SpellId::det_items:
            if (is_seer)
            {
                cost_max -= 3;
            }
            break;

        case SpellId::det_traps:
            if (is_seer)
            {
                cost_max -= 3;
            }
            break;

        case SpellId::summon:
            if (is_summoner)
            {
                --cost_max;
            }
            break;

        case SpellId::pest:
            if (is_summoner)
            {
                --cost_max;
            }
            break;

        case SpellId::pharaoh_staff:
            if (is_summoner)
            {
                --cost_max;
            }
            break;

        default:
            break;
        }

        const bool is_blood_sorc =
            player_bon::traits[(size_t)Trait::blood_sorc];

        if (is_blood_sorc)
        {
            --cost_max;
        }

        if (caster->has_prop(PropId::blessed))
        {
            --cost_max;
        }

        if (caster->has_prop(PropId::cursed))
        {
            cost_max += 3;
        }
    } // Is player, and use modified cost

    cost_max = std::max(1, cost_max);

    const int cost_min = std::max(1, (cost_max + 1) / 2);

    return Range(cost_min, cost_max);
}

void Spell::cast(Actor* const caster,
                 const bool is_intrinsic) const
{
    TRACE_FUNC_BEGIN;

    ASSERT(caster);

    const bool allow_cast =
        caster->prop_handler().allow_cast_spell(Verbosity::verbose);

    if (!allow_cast)
    {
        return;
    }

    if (caster->is_player())
    {
        TRACE << "Player casting spell" << std::endl;

        const ShockSrc shock_src =
            is_intrinsic ?
            ShockSrc::cast_intr_spell :
            ShockSrc::use_strange_item;

        const int shock_value = is_intrinsic ? shock_lvl_intr_cast() : 10;

        map::player->incr_shock(shock_value, shock_src);

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
                mon_name = mon->name_the();
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

        mon->spell_cooldown_current_ = mon->data().spell_cooldown_turns;
    }

    if (is_intrinsic)
    {
        const Range cost = spi_cost(caster);

        caster->hit_spi(cost.roll(), Verbosity::silent);
    }

    if (caster->is_alive())
    {
        run_effect(caster);
    }

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

    if (is_player)
    {
        target.prop_handler().end_prop(PropId::r_spell);
    }
}

std::vector<std::string> Spell::descr() const
{
    auto ret = descr_specific();

    const int skill = map::player->spell_skill(id());

    if (can_be_improved_with_skill())
    {
        ret.push_back("Skill level: " + std::to_string(skill) + "%");
    }

    return ret;
}

// -----------------------------------------------------------------------------
// Darkbolt
// -----------------------------------------------------------------------------
void SpellDarkbolt::run_effect(Actor* const caster) const
{
    if (caster == map::player &&
        player_bon::traits[(size_t)Trait::warlock] &&
        rnd::percent(warlock_multi_cast_chance_pct))
    {
        run_effect(caster);

        if (!caster->is_alive())
        {
            return;
        }
    }

    Actor* target = nullptr;

    const auto seen_actors = caster->seen_foes();

    if (seen_actors.empty())
    {
        if (caster->is_player())
        {
            msg_log::add(
                "A dark sphere materializes, but quickly fizzles out.");
        }

        return;
    }

    target = map::random_closest_actor(caster->pos, seen_actors);

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
            run_effect(target);
        }

        return;
    }

    std::vector<P> line;

    line_calc::calc_new_line(caster->pos,
                             target->pos,
                             true,
                             999,
                             false,
                             line);

    states::draw();

    const size_t line_size = line.size();

    for (size_t i = 1; i < line_size; ++i)
    {
        const P& p = line[i];

        if (config::is_tiles_mode())
        {
            io::draw_tile(TileId::blast1, Panel::map, p, clr_magenta);
        }
        else
        {
            io::draw_glyph('*', Panel::map, p, clr_magenta);
        }

        io::update_screen();

        sdl_base::sleep(config::delay_projectile_draw());
    }

    io::draw_blast_at_cells({target->pos}, clr_magenta);

    Clr msg_clr = clr_msg_good;

    std::string str_begin = "I am";

    if (target->is_player())
    {
        msg_clr = clr_msg_bad;
    }
    else // Target is monster
    {
        str_begin = target->name_the() + " is";

        if (map::player->is_leader_of(target))
        {
            msg_clr = clr_white;
        }
    }

    if (map::player->can_see_actor(*target))
    {
        msg_log::add(str_begin + " struck by a blast!", msg_clr);
    }

    int skill = caster->spell_skill(id());

    const int dmg = (skill / 10) + 1;

    target->hit(dmg,
             DmgType::physical,
             DmgMethod::END,
             AllowWound::no);

    if (skill >= 100 &&
        target->state() == ActorState::alive)
    {
        const int elec_dmg = rnd::dice(1, 8);

        target->hit(elec_dmg,
                 DmgType::electric,
                 DmgMethod::END,
                 AllowWound::no);
    }

    if (skill >= 20 &&
        target->state() == ActorState::alive)
    {
        target->prop_handler().try_add(
            new PropParalyzed(PropTurns::specific, 2));
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

std::vector<std::string> SpellDarkbolt::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Siphons power from some infernal dimension, which is then focused "
        "into a bolt hurled towards a target with great force.");

    descr.push_back(
        "The conjured bolt has some will on its own - the caster determine "
        "determine exactly which creature will be struck.");

    const int skill = map::player->spell_skill(id());

    const int dmg = (skill / 10) + 1;

    descr.push_back(
        "The impact does " +
        std::to_string(dmg) +
        " damage.");

    if (skill >= 20)
    {
        descr.push_back("The target is paralyzed.");
    }

    if (skill >= 100)
    {
        descr.push_back(
            "Does 1d8 electric damage in addition to the normal damage.");
    }

    return descr;
}

bool SpellDarkbolt::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(3);
}

// -----------------------------------------------------------------------------
// Azathoths wrath
// -----------------------------------------------------------------------------
void SpellAzaWrath::run_effect(Actor* const caster) const
{
    if (caster == map::player &&
        player_bon::traits[(size_t)Trait::warlock] &&
        rnd::percent(warlock_multi_cast_chance_pct))
    {
        run_effect(caster);

        if (!caster->is_alive())
        {
            return;
        }
    }

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
                run_effect(target);
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
            str_begin = target->name_the() + " is";

            if (map::player->is_leader_of(target))
            {
                msg_clr = clr_white;
            }
        }

        if (map::player->can_see_actor(*target))
        {
            msg_log::add(str_begin + " struck by a roaring blast!", msg_clr);
        }

        int skill = caster->spell_skill(id());

        int dmg = (skill / 12) + 1;

        target->hit(dmg,
                    DmgType::physical,
                    DmgMethod::END,
                    AllowWound::no);

        if (skill >= 100 &&
            target->state() == ActorState::alive)
        {
            const int elec_dmg = rnd::dice(1, 6);

            target->hit(elec_dmg,
                     DmgType::electric,
                     DmgMethod::END,
                     AllowWound::no);
        }

        if (skill >= 20 &&
            target->state() == ActorState::alive)
        {
            target->prop_handler().try_add(
                new PropParalyzed(PropTurns::specific, 2));
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

std::vector<std::string> SpellAzaWrath::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Channels the destructive force of Azathoth unto all visible enemies.");

    const int skill = map::player->spell_skill(id());

    int dmg = (skill / 12) + 1;

    descr.push_back(
        "The spell does " +
        std::to_string(dmg) +
        " damage per creature.");

    if (skill >= 20)
    {
        descr.push_back("The targets are paralyzed.");
    }

    if (skill >= 100)
    {
        descr.push_back(
            "Does 1d6 electric damage in addition to the normal damage.");
    }

    return descr;
}

bool SpellAzaWrath::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(3);
}

// -----------------------------------------------------------------------------
// Mayhem
// -----------------------------------------------------------------------------
void SpellMayhem::run_effect(Actor* const caster) const
{
    if (caster == map::player &&
        player_bon::traits[(size_t)Trait::warlock] &&
        rnd::percent(warlock_multi_cast_chance_pct))
    {
        run_effect(caster);

        if (!caster->is_alive())
        {
            return;
        }
    }

    const bool is_player = caster->is_player();

    if (map::player->can_see_actor(*caster))
    {
        std::string caster_name = is_player ? "me" : caster->name_the();

        msg_log::add("Destruction rages around " + caster_name + "!");
    }

    const P& caster_pos = caster->pos;

    const int radi = fov_std_radi_int - 2;

    const int x0 = std::max(1, caster_pos.x - radi);
    const int y0 = std::max(1, caster_pos.y - radi);
    const int x1 = std::min(map_w - 1, caster_pos.x + radi) - 1;
    const int y1 = std::min(map_h - 1, caster_pos.y + radi) - 1;

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

    const int skill = caster->spell_skill(id());

    int nr_expl = (skill / 10) + 3;

    if (skill >= 100)
    {
        nr_expl += 3;
    }

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

    // Let's add some extra chaos, put blood and set stuff on fire
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            auto* const f = map::cells[x][y].rigid;

            if (f->can_have_blood() &&
                rnd::one_in(10))
            {
                f->mk_bloody();
            }

            if (P(x, y) != caster->pos &&
                rnd::one_in(4))
            {
                f->hit(1, // Doesn't matter
                       DmgType::fire,
                       DmgMethod::elemental,
                       nullptr);
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

std::vector<std::string> SpellMayhem::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back("Blasts the surrounding area with explosions.");

    const int skill = map::player->spell_skill(id());

    int nr_expl = (skill / 10) + 3;

    if (skill >= 100)
    {
        nr_expl += 3;
    }

    descr.push_back("Conjures " + std::to_string(nr_expl) + " explosion(s).");

    return descr;
}

bool SpellMayhem::allow_mon_cast_now(Mon& mon) const
{
    return
        mon.aware_of_player_counter_ > 0 &&
        rnd::coin_toss() &&
        (mon.tgt_ || rnd::one_in(20));
}

// -----------------------------------------------------------------------------
// Pestilence
// -----------------------------------------------------------------------------
void SpellPest::run_effect(Actor* const caster) const
{
    const int skill = caster->spell_skill(id());

    size_t nr_mon = (skill / 10) + 2;

    Actor* leader = nullptr;

    bool did_player_summon_hostile = false;

    if (caster->is_player())
    {
        const int n =
            summon_hostile_one_in_n *
            (player_bon::traits[(size_t)Trait::summoner] ? 2 : 1);

        did_player_summon_hostile = rnd::one_in(n);

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
        actor_factory::spawn(caster->pos,
                             {nr_mon, ActorId::rat},
                             MakeMonAware::yes,
                             leader);

    bool is_any_seen_by_player = false;

    for (Mon* const mon : mon_summoned)
    {
        mon->prop_handler().try_add(
            new PropSummoned(PropTurns::indefinite));

        if (map::player->can_see_actor(*mon))
        {
            is_any_seen_by_player = true;
        }

        // Haste the rats if 100% skill
        if (skill >= 100)
        {
            mon->prop_handler().try_add(
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

std::vector<std::string> SpellPest::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back("A pack of rats appear around the caster.");

    const int skill = map::player->spell_skill(id());

    size_t nr_mon = (skill / 10) + 2;

    descr.push_back("Summons " + std::to_string(nr_mon) + " rats.");

    if (skill >= 100)
    {
        descr.push_back("The rats are Hasted (+100% speed).");
    }

    descr.push_back(summon_warning_str);

    return descr;
}

bool SpellPest::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ &&
           rnd::one_in(3) &&
           (mon.tgt_ || rnd::one_in(30));
}

// -----------------------------------------------------------------------------
// Animate weapons
// -----------------------------------------------------------------------------
void SpellAnimWpns::run_effect(Actor* const caster) const
{
    bool is_any_animated = false;

    if (!caster->is_player())
    {
        return;
    }

    const int skill = caster->spell_skill(id());

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Cell& cell = map::cells[x][y];

            Item* const item = cell.item;

            if (cell.is_seen_by_player &&
                item &&
                (item->data().type == ItemType::melee_wpn))
            {
                cell.item = nullptr;

                const P p(x, y);

                const auto summoned =
                    actor_factory::spawn(
                        p,
                        {1, ActorId::animated_wpn},
                        MakeMonAware::no,
                        map::player);

                ASSERT(summoned.size() == 1);

                Mon* const anim_wpn = summoned[0];

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

                if (skill >= 50)
                {
                    anim_wpn->prop_handler().try_add(
                        new PropSeeInvis(PropTurns::indefinite),
                        PropSrc::intr,
                        true,
                        Verbosity::silent);
                }

                if (skill >= 100)
                {
                    anim_wpn->prop_handler().try_add(
                        new PropHasted(PropTurns::indefinite),
                        PropSrc::intr,
                        true,
                        Verbosity::silent);
                }

                is_any_animated = true;
            }
        }
    }

    if (!is_any_animated)
    {
        msg_log::add(
            "The dust and gravel on the ground starts shooting everywhere.");
    }
}

std::vector<std::string> SpellAnimWpns::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Infuses lifeless weapons with a spirit of their own, causing "
        "them to rise up into the air and protect their master (for a "
        "while). It is only possible to animate basic melee weapons "
        "however - \"modern\" mechanisms such as pistols or machine guns "
        "are far too complex.");

    const int skill = map::player->spell_skill(id());

    if (skill >= 50)
    {
        descr.push_back("The weapon can see invisible creatures.");
    }

    if (skill >= 100)
    {
        descr.push_back("The weapon is also Hasted (+100% speed).");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Pharaoh staff
// -----------------------------------------------------------------------------
void SpellPharaohStaff::run_effect(Actor* const caster) const
{
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
        const int n = summon_hostile_one_in_n *
                      (player_bon::traits[(size_t)Trait::summoner] ? 2 : 1);

        did_player_summon_hostile = rnd::one_in(n);
        leader = did_player_summon_hostile ? nullptr : caster;
    }
    else // Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;
        leader = caster_leader ? caster_leader : caster;
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

    const Mon* const mon = summoned_mon[0];

    if (map::player->can_see_actor(*mon))
    {
        msg_log::add(mon->name_a() + " appears!");

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

std::vector<std::string> SpellPharaohStaff::descr_specific() const
{
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
    return
        mon.tgt_ &&
        rnd::one_in(4);
}

// -----------------------------------------------------------------------------
// Detect items
// -----------------------------------------------------------------------------
void SpellDetItems::run_effect(Actor* const caster) const
{
    (void)caster;

    const int skill = caster->spell_skill(id());

    const int range =
        (skill >= 100) ?
        100 :
        (3 + (skill / 10));

    const int orig_x = map::player->pos.x;
    const int orig_y = map::player->pos.y;

    const int x0 = std::max(0, orig_x - range);
    const int y0 = std::max(0, orig_y - range);
    const int x1 = std::min(map_w - 1, orig_x + range);
    const int y1 = std::min(map_h - 1, orig_y + range);

    std::vector<P> items_revealed_cells;

    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            auto& cell = map::cells[x][y];

            if (cell.item)
            {
                cell.is_seen_by_player = true;

                cell.is_explored = true;

                items_revealed_cells.push_back(P(x, y));
            }
        }
    }

    if (items_revealed_cells.empty())
    {
        msg_log::add("I sense that there are no items nearby.");
    }
    else // Items detected
    {
        // To update the render data
        states::draw();

        map::player->update_fov();

        states::draw();

        io::draw_blast_at_cells(items_revealed_cells, clr_white);

        if (items_revealed_cells.size() == 1)
        {
            msg_log::add("An item is revealed to me.");
        }
        else // > 1 items detected
        {
            msg_log::add("Some items are revealed to me.");
        }
    }
}

std::vector<std::string> SpellDetItems::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Reveals the presence of items in the surrounding area.");

    const int skill = map::player->spell_skill(id());

    int range;

    if (skill < 100)
    {
        range = (3 + (skill / 10));

        descr.push_back("The spell has a range of " +
                        std::to_string(range) +
                        " cells.");
    }
    else // Skill >= 100
    {
        descr.push_back("Reveals all items on the map.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Detect traps
// -----------------------------------------------------------------------------
void SpellDetTraps::run_effect(Actor* const caster) const
{
    (void)caster;

    const int skill = caster->spell_skill(id());

    const int range =
        (skill >= 100) ?
        100 :
        (3 + (skill / 10));

    const int orig_x = map::player->pos.x;
    const int orig_y = map::player->pos.y;

    const int x0 = std::max(0, orig_x - range);
    const int y0 = std::max(0, orig_y - range);
    const int x1 = std::min(map_w - 1, orig_x + range);
    const int y1 = std::min(map_h - 1, orig_y + range);

    std::vector<P> traps_revealed_cells;

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            if (map::cells[x][y].is_seen_by_player)
            {
                auto* const f = map::cells[x][y].rigid;

                if (f->id() == FeatureId::trap)
                {
                    auto* const trap = static_cast<Trap*>(f);

                    trap->reveal(Verbosity::silent);

                    traps_revealed_cells.push_back(P(x, y));
                }
            }
        }
    }

    if (traps_revealed_cells.empty())
    {
        msg_log::add("I sense that there are no traps nearby.");
    }
    else // Traps detected
    {
        map::player->update_fov();

        states::draw();

        io::draw_blast_at_cells(traps_revealed_cells, clr_white);

        if (traps_revealed_cells.size() == 1)
        {
            msg_log::add("A hidden trap is revealed to me.");
        }

        if (traps_revealed_cells.size() > 1)
        {
            msg_log::add("Some hidden traps are revealed to me.");
        }
    }
}

std::vector<std::string> SpellDetTraps::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Reveals the presence of traps in the surrounding area.");

    const int skill = map::player->spell_skill(id());

    int range;

    if (skill < 100)
    {
        range = (3 + (skill / 10));

        descr.push_back("The spell has a range of " +
                        std::to_string(range) +
                        " cells.");
    }
    else // Skill >= 100
    {
        descr.push_back("Reveals all traps on the map.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Detect monsters
// -----------------------------------------------------------------------------
void SpellDetMon::run_effect(Actor* const caster) const
{
    (void)caster;

    const int skill = caster->spell_skill(id());

    const int range =
        (skill >= 100) ?
        100 :
        (3 + (skill / 10));

    bool is_seer = player_bon::traits[(size_t)Trait::seer];

    const int multiplier = 20 * (is_seer ? 2 : 1);

    bool did_detect = false;

    for (Actor* actor : game_time::actors)
    {
        if (!actor->is_player() &&
            king_dist(caster->pos, actor->pos) <= range)
        {
            static_cast<Mon*>(actor)->set_player_aware_of_me(multiplier);

            did_detect = true;
        }
    }

    if (!did_detect)
    {
        msg_log::add("I sense that there are no creatures nearby.");
    }
}

std::vector<std::string> SpellDetMon::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Reveals the presence of creatures in the surrounding area.");

    const int skill = map::player->spell_skill(id());

    int range;

    if (skill < 100)
    {
        range = (3 + (skill / 10));

        descr.push_back("The spell has a range of " +
                        std::to_string(range) +
                        " cells.");
    }
    else // Skill >= 100
    {
        descr.push_back("Reveals all creatures on the map.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Opening
// -----------------------------------------------------------------------------
void SpellOpening::run_effect(Actor* const caster) const
{
    (void)caster;

    const int skill = caster->spell_skill(id());

    const int range = (1 + (skill / 20));

    const int orig_x = map::player->pos.x;
    const int orig_y = map::player->pos.y;

    const int x0 = std::max(0, orig_x - range);
    const int y0 = std::max(0, orig_y - range);
    const int x1 = std::min(map_w - 1, orig_x + range);
    const int y1 = std::min(map_h - 1, orig_y + range);

    bool is_any_opened = false;

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            const auto& cell = map::cells[x][y];

            DidOpen did_open = cell.rigid->open(nullptr);

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

std::vector<std::string> SpellOpening::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back("Opens all locks, lids and doors.");

    const int skill = map::player->spell_skill(id());

    const int range = (1 + (skill / 20));

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

    return descr;
}

// -----------------------------------------------------------------------------
// Sacrifice life
// -----------------------------------------------------------------------------
void SpellSacrLife::run_effect(Actor* const caster) const
{
    (void)caster;

    // Convert every 2 hp to 1 SPI

    const int player_hp_cur = map::player->hp();

    if (player_hp_cur > 2)
    {
        const int hp_drained = ((player_hp_cur - 1) / 2) * 2;

        map::player->hit(hp_drained, DmgType::pure);

        map::player->restore_spi(hp_drained, true);

        const int skill = caster->spell_skill(id());

        if (skill >= 50)
        {
            SpellAzaWrath spell;

            spell.run_effect(caster);
        }

        if (skill >= 100)
        {
            SpellMayhem  spell;

            spell.run_effect(caster);
        }
    }
    else // Not enough HP
    {
        msg_log::add("I feel like I have very little to offer.");
    }
}

std::vector<std::string> SpellSacrLife::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Brings the caster to the brink of death in order to restore the "
        "spirit. The amount restored is proportional to the life "
        "sacrificed.");

    const int skill = map::player->spell_skill(id());

    if (skill >= 50)
    {
        SpellAzaWrath spell;

        const std::string name = spell.name();

        descr.push_back(
            "If any health was sacrificed, the spell \"" + name + "\""
            "is also cast (at the casters skill level for that spell).");
    }

    if (skill >= 100)
    {
        SpellMayhem spell;

        const std::string name = spell.name();

        descr.push_back("Also casts the spell \"" + name + "\".");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Sacrifice spirit
// -----------------------------------------------------------------------------
void SpellSacrSpi::run_effect(Actor* const caster) const
{
    (void)caster;

    // Convert all spi to HP

    const int player_spi_cur = map::player->spi();

    if (player_spi_cur > 0)
    {
        const int hp_drained = player_spi_cur - 1;

        map::player->hit_spi(hp_drained);

        map::player->restore_hp(hp_drained);

        std::vector<PropId> props_can_heal;

        const int skill = caster->spell_skill(id());

        if (skill >= 50)
        {
            props_can_heal.push_back(PropId::poisoned);
        }

        if (skill >= 100)
        {
            props_can_heal.push_back(PropId::blind);
            props_can_heal.push_back(PropId::infected);
            props_can_heal.push_back(PropId::diseased);
            props_can_heal.push_back(PropId::weakened);
        }

        for (PropId prop_id : props_can_heal)
        {
            caster->prop_handler().end_prop(prop_id);
        }
    }
    else // Not enough spirit
    {
        msg_log::add("I feel like I have very little to offer.");
    }
}

std::vector<std::string> SpellSacrSpi::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Brings the caster to the brink of spiritual death in order to "
        "restore health. The amount restored is proportional to the "
        "spirit sacrificed.");

    const int skill = map::player->spell_skill(id());

    if (skill >= 100)
    {
        descr.push_back(
            "If any spirit was sacrificed, cures blindness, poisoning, "
            "infections, disease, and weakening.");
    }
    // skill < 100
    else if (skill >= 50)
    {
        descr.push_back("If any spirit was sacrificed, cures poisoning.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Ghoul frenzy
// -----------------------------------------------------------------------------
void SpellFrenzy::run_effect(Actor* const caster) const
{
    const int nr_turns = rnd::range(12, 18);

    PropFrenzied* frenzy = new PropFrenzied(PropTurns::specific, nr_turns);

    caster->prop_handler().try_add(frenzy);
}

std::vector<std::string> SpellFrenzy::descr_specific() const
{
    return
    {
        "Incites a great rage in the caster, which will charge their "
        "enemies with a terrible, uncontrollable fury."
    };
}

// -----------------------------------------------------------------------------
// Bless
// -----------------------------------------------------------------------------
void SpellBless::run_effect(Actor* const caster) const
{
    const int skill = caster->spell_skill(id());

    const int nr_turns = 7 + (skill * 7);

    caster->prop_handler().try_add(
        new PropBlessed(PropTurns::specific, nr_turns));
}

std::vector<std::string> SpellBless::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back("Bends reality in favor of the caster.");

    const int skill = map::player->spell_skill(id());

    const int nr_turns = 7 + (skill * 7);

    descr.push_back(
        "The spell lasts " +
        std::to_string(nr_turns) +
        " turns.");

    return descr;
}

// -----------------------------------------------------------------------------
// Light
// -----------------------------------------------------------------------------
void SpellLight::run_effect(Actor* const caster) const
{
    caster->prop_handler().try_add(new PropRadiant(PropTurns::std));

    const int skill = caster->spell_skill(id());

    if (skill >= 50)
    {
        std::vector<Prop*> props;

        props.push_back(new PropBlind(PropTurns::std));

        if (skill >= 100)
        {
            props.push_back(new PropBurning(PropTurns::std));
        }

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

std::vector<std::string> SpellLight::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back("Illuminates the area around the caster.");

    const int skill = map::player->spell_skill(id());

    if (skill >= 50)
    {
        descr.push_back(
            "On casting, causes a blinding flash centered on the caster "
            "(but not affecting the caster itself).");
    }

    if (skill >= 100)
    {
        descr.push_back(
            "The flash also sets creatures on fire.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Teleport
// -----------------------------------------------------------------------------
void SpellTeleport::run_effect(Actor* const caster) const
{
    const int skill = caster->spell_skill(id());

    if (skill >= 50)
    {
        const int nr_turns =
            skill >= 100 ?
            8 :
            3;

        caster->prop_handler().try_add(
            new PropInvisible(PropTurns::specific, nr_turns));
    }

    caster->teleport();
}

bool SpellTeleport::allow_mon_cast_now(Mon& mon) const
{
    const bool is_low_hp = mon.hp() <= (mon.hp_max(true) / 2);

    return
        (mon.aware_of_player_counter_ > 0) &&
        is_low_hp &&
        rnd::fraction(3, 4);
}

std::vector<std::string> SpellTeleport::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back("Instantly moves the caster to a different position.");

    const int skill = map::player->spell_skill(id());

    if (skill >= 50)
    {
        const int nr_turns =
            skill >= 100 ?
            8 :
            3;

        descr.push_back(
            "On teleporting, the caster is invisible for " +
            std::to_string(nr_turns) +
            " turns.");
    }

    return descr;
}

// -----------------------------------------------------------------------------
// Resistance
// -----------------------------------------------------------------------------
void SpellRes::run_effect(Actor* const caster) const
{
    const int skill = caster->spell_skill(id());

    int nr_turns = 10 + (skill / 4);

    if (skill >= 100)
    {
        nr_turns += 50;
    }

    PropHandler& prop_hlr = caster->prop_handler();

    prop_hlr.try_add(new PropRFire(PropTurns::specific, nr_turns));
    prop_hlr.try_add(new PropRElec(PropTurns::specific, nr_turns));
}

std::vector<std::string> SpellRes::descr_specific() const
{
    std::vector<std::string> descr;

    const int skill = map::player->spell_skill(id());

    int nr_turns = 10 + (skill / 4);

    if (skill >= 100)
    {
        nr_turns += 50;
    }

    descr.push_back(
        "The caster is completely shielded from fire and electricity.");

    descr.push_back("The spell has a duration of " +
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
        mon.tgt_ &&
        rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Knockback
// -----------------------------------------------------------------------------
void SpellKnockBack::run_effect(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Clr msg_clr = clr_msg_good;
    std::string target_str = "me";
    Actor* caster_used = caster;
    Actor* target = static_cast<Mon*>(caster_used)->tgt_;
    ASSERT(target);

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
    return mon.tgt_ && rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Enfeeble
// -----------------------------------------------------------------------------
void SpellEnfeebleMon::run_effect(Actor* const caster) const
{
    PropId prop_id = PropId::END;

    const int rnd = rnd::range(1, 4);

    switch (rnd)
    {
    case 1: prop_id = PropId::slowed;
        break;

    case 2: prop_id = PropId::weakened;
        break;

    case 3: prop_id = PropId::paralyzed;
        break;

    case 4: prop_id = PropId::terrified;
        break;

    default:
        ASSERT(false);
        break;
    }

    auto targets = caster->seen_foes();

    if (targets.empty())
    {
        switch (prop_id)
        {
        case PropId::slowed:
            msg_log::add("The bugs on the ground suddenly move very slowly.");
            break;

        case PropId::weakened:
            msg_log::add("The bugs on the ground suddenly move very feebly.");
            break;

        case PropId::paralyzed:
            msg_log::add("The bugs on the ground suddenly freeze.");
            break;

        case PropId::terrified:
            msg_log::add("The bugs on the ground suddenly scatter away.");
            break;

        default:
            ASSERT(false);
            break;
        }

        return;
    }

    //
    // There are targets available
    //

    const int skill = caster->spell_skill(id());

    // If low skill, only target one monster
    if (skill < 50)
    {
        Actor* target = map::random_closest_actor(caster->pos, targets);

        targets.clear();

        targets.push_back(target);
    }

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
                run_effect(target);
            }

            continue;
        }

        Prop* const prop = prop_handler.mk_prop(prop_id, PropTurns::std);

        // Double prop turns if max skill
        if (skill >= 100)
        {
            prop->set_nr_turns_left(prop->nr_turns_left() * 2);
        }

        prop_handler.try_add(prop);
    }
}

std::vector<std::string> SpellEnfeebleMon::descr_specific() const
{
    std::vector<std::string> descr;

    descr.push_back(
        "Attempts to physically or mentally enfeeble others.");

    const int skill = map::player->spell_skill(id());

    if (skill >= 50)
    {
        descr.push_back("Affects all visible enemies.");
    }

    if (skill >= 100)
    {
        descr.push_back("Effect duration is doubled.");
    }

    return descr;
}

bool SpellEnfeebleMon::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(4);
}

// -----------------------------------------------------------------------------
// Disease
// -----------------------------------------------------------------------------
void SpellDisease::run_effect(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;
    Actor* target = static_cast<Mon*>(caster_used)->tgt_;

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

    target->prop_handler().try_add(new PropDiseased(PropTurns::std));
}

bool SpellDisease::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Summon monster
// -----------------------------------------------------------------------------
void SpellSummonMon::run_effect(Actor* const caster) const
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

    const int skill = caster->spell_skill(id());

    //
    // NOTE: There are summonable monsters spawning from dlvl 0 (wolves), so
    //       it's not a problem if the dlvl limit is 0
    //

    int max_dlvl_spawned = (skill * dlvl_last) / 100;

    // If it's a monster doing the summoning, also don't allow a too far
    // out of depth monsters to be spawned
    if (!caster->is_player())
    {
        const int nr_dlvls_ood_allowed = 3;

        max_dlvl_spawned =
            std::min(max_dlvl_spawned,
                     map::dlvl + nr_dlvls_ood_allowed);
    }

    for (int i = 0; i < (int)ActorId::END; ++i)
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

    if (summon_bucket.empty())
    {
        TRACE << "No elligible monsters found for spawning" << std::endl;

        ASSERT(false);

        return;
    }

    const ActorId mon_id = rnd::element(summon_bucket);

    Actor* leader = nullptr;

    bool did_player_summon_hostile = false;

    if (caster->is_player())
    {
        const int n = summon_hostile_one_in_n *
            (player_bon::traits[(size_t)Trait::summoner] ? 2 : 1);

        did_player_summon_hostile = rnd::one_in(n);

        leader = did_player_summon_hostile ? nullptr : caster;
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

    Mon* const mon = mon_summoned[0];

    mon->prop_handler().try_add(
        new PropSummoned(PropTurns::indefinite));

    if (map::player->can_see_actor(*mon))
    {
        msg_log::add(mon->name_a() + " appears!");

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

std::vector<std::string> SpellSummonMon::descr_specific() const
{
    return
    {
        "Summons a creature to do the caster's bidding. A more skilled "
        "sorcerer summons beings of greater might and rarity.",

        summon_warning_str
    };
}

bool SpellSummonMon::allow_mon_cast_now(Mon& mon) const
{
    // NOTE: Checking awareness instead of target, to allow summoning even with
    //       broken LOS
    return (mon.aware_of_player_counter_ > 0) &&
           rnd::coin_toss() &&
           (mon.tgt_ || rnd::one_in(23));
}

// -----------------------------------------------------------------------------
// Heal self
// -----------------------------------------------------------------------------
void SpellHealSelf::run_effect(Actor* const caster) const
{
    // The spell effect is noticed if any hit points were restored
    caster->restore_hp(999);
}

bool SpellHealSelf::allow_mon_cast_now(Mon& mon) const
{
    return
        (mon.hp() < mon.hp_max(true)) &&
        rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Mi-go hypnosis
// -----------------------------------------------------------------------------
void SpellMiGoHypno::run_effect(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;
    Actor* target = static_cast<Mon*>(caster_used)->tgt_;

    ASSERT(target);

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

        target->prop_handler().try_add(prop);
    }
    else
    {
        msg_log::add("I feel dizzy.");
    }
}

bool SpellMiGoHypno::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ &&
           mon.tgt_->is_player() &&
           rnd::one_in(3);
}

// -----------------------------------------------------------------------------
// Immolation
// -----------------------------------------------------------------------------
void SpellBurn::run_effect(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;
    Actor* target = static_cast<Mon*>(caster_used)->tgt_;

    ASSERT(target);

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

    const int skill = caster->spell_skill(id());

    const int nr_turns = 1 + (skill / 20);

    Prop* const prop = new PropBurning(PropTurns::specific, nr_turns);

    target->prop_handler().try_add(prop);
}

bool SpellBurn::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(4);
}

// -----------------------------------------------------------------------------
// Deafening
// -----------------------------------------------------------------------------
void SpellDeafen::run_effect(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;

    Actor* target = static_cast<Mon*>(caster_used)->tgt_;

    ASSERT(target);

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

    const int skill = caster->spell_skill(id());

    const int nr_turns = 100 + skill;

    Prop* const prop = new PropDeaf(PropTurns::specific, nr_turns);

    target->prop_handler().try_add(prop);
}

bool SpellDeafen::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(4);
}
