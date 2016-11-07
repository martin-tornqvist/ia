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

namespace
{

const int summon_hostile_one_in_n = 7;

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

    case SpellId::knock_back:
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

    case SpellId::cloud_minds:
        return new SpellCloudMinds;

    case SpellId::frenzy:
        return new SpellFrenzy;

    case SpellId::bless:
        return new SpellBless;

    case SpellId::mi_go_hypno:
        return new SpellMiGoHypno;

    case SpellId::burn:
        return new SpellBurn;

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

Range Spell::spi_cost(const bool is_base_cost_only, Actor* const caster) const
{
    int cost_max = max_spi_cost();

    if (
        caster == map::player &&
        !is_base_cost_only)
    {
        // The spell is more expensive the less skill the player has in it
        const int skill_pct = player_spells::spell_skill_pct(id());

        cost_max += (cost_max * (100 - skill_pct)) / 100;

        // Standing next to an altar makes it cheaper
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

        // Traits reducing the cost of specific spells
        // TODO: Shouldn't this be done by calling virtual functions?
        bool is_warlock     = player_bon::traits[(size_t)Trait::warlock];
        bool is_seer        = player_bon::traits[(size_t)Trait::seer];
        bool is_summoner    = player_bon::traits[(size_t)Trait::summoner];

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

        if (caster->has_prop(PropId::blessed))
        {
            --cost_max;
        }

        if (caster->has_prop(PropId::cursed))
        {
            cost_max += 3;
        }
    } // Is player, and use modified cost

    cost_max            = std::max(1, cost_max);
    const int cost_min  = std::max(1, (cost_max + 1) / 2);

    return Range(cost_min, cost_max);
}

SpellEffectNoticed Spell::cast(Actor* const caster,
                               const bool is_intrinsic,
                               const bool is_base_cost_only) const
{
    TRACE_FUNC_BEGIN;

    ASSERT(caster);

    const bool allow_cast =
        caster->prop_handler().allow_cast_spell(Verbosity::verbose);

    if (allow_cast)
    {
        if (caster->is_player())
        {
            TRACE << "Player casting spell" << std::endl;

            const ShockSrc shock_src = is_intrinsic ?
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
                    mon_name = mon->data().is_humanoid ? "Someone" : "Something";
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
            const Range cost = spi_cost(is_base_cost_only, caster);

            caster->hit_spi(cost.roll(), Verbosity::silent);
        }

        SpellEffectNoticed is_noticed = SpellEffectNoticed::no;

        if (caster->is_alive())
        {
            is_noticed = cast_impl(caster);
        }

        game_time::tick();
        TRACE_FUNC_END;
        return is_noticed;
    }

    TRACE_FUNC_END;
    return SpellEffectNoticed::no;
}

void Spell::on_resist(Actor& target) const
{
    const bool is_player        = target.is_player();
    const bool player_see_tgt   = map::player->can_see_actor(target);

    if (player_see_tgt)
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
        target.prop_handler().end_prop(PropId::rSpell);
    }

}

// -----------------------------------------------------------------------------
// Darkbolt
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellDarkbolt::cast_impl(Actor* const caster) const
{
    Actor* tgt = nullptr;

    std::vector<Actor*> seen_actors;
    caster->seen_foes(seen_actors);

    if (seen_actors.empty())
    {
        return SpellEffectNoticed::no;
    }

    tgt = map::random_closest_actor(caster->pos, seen_actors);

    // Spell reflection?
    if (tgt->has_prop(PropId::spell_reflect))
    {
        if (map::player->can_see_actor(*tgt))
        {
            msg_log::add(spell_reflect_msg,
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
        }

        return cast_impl(tgt);
    }

    // Spell resistance?
    if (tgt->has_prop(PropId::rSpell))
    {
        on_resist(*tgt);

        return SpellEffectNoticed::yes;
    }

    std::vector<P> line;

    line_calc::calc_new_line(caster->pos,
                             tgt->pos,
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

    io::draw_blast_at_cells({tgt->pos}, clr_magenta);

    bool        is_warlock_charged  = false;
    Clr         msg_clr             = clr_msg_good;
    std::string str_begin           = "I am";

    if (tgt->is_player())
    {
        msg_clr = clr_msg_bad;
    }
    else // Target is monster
    {
        str_begin = tgt->name_the() + " is";

        if (map::player->is_leader_of(tgt))
        {
            msg_clr = clr_white;
        }
    }

    if (caster->is_player())
    {
        is_warlock_charged = caster->has_prop(PropId::warlock_charged);
    }

    if (map::player->can_see_actor(*tgt))
    {
        msg_log::add(str_begin + " struck by a blast!", msg_clr);
    }

    tgt->prop_handler().try_add(new PropParalyzed(PropTurns::specific, 2));

    Range dmg_range(4, 10);
    const int dmg = is_warlock_charged ? dmg_range.max : dmg_range.roll();

    tgt->hit(dmg, DmgType::physical, DmgMethod::END, AllowWound::no);

    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            tgt->pos,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);

    return SpellEffectNoticed::yes;
}

bool SpellDarkbolt::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(3);
}

// -----------------------------------------------------------------------------
// Azathoths wrath
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellAzaWrath::cast_impl(Actor* const caster) const
{
    Range dmg_range(4, 8);
    bool  is_warlock_charged = false;

    std::vector<Actor*> tgts;
    caster->seen_foes(tgts);

    if (tgts.empty())
    {
        return SpellEffectNoticed::no;
    }

    // This point reached means targets are available
    if (caster->is_player())
    {
        is_warlock_charged = caster->has_prop(PropId::warlock_charged);
    }

    io::draw_blast_at_seen_actors(tgts, clr_red_lgt);

    for (Actor* const tgt : tgts)
    {
        // Spell reflection?
        if (tgt->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*tgt))
            {
                msg_log::add(spell_reflect_msg,
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);
            }

            cast_impl(tgt);
            continue;
        }

        // Spell resistance?
        if (tgt->has_prop(PropId::rSpell))
        {
            on_resist(*tgt);

            continue;
        }

        std::string     str_begin   = "I am";
        Clr             msg_clr     = clr_msg_good;

        if (tgt->is_player())
        {
            msg_clr = clr_msg_bad;
        }
        else // Target is monster
        {
            str_begin = tgt->name_the() + " is";

            if (map::player->is_leader_of(tgt))
            {
                msg_clr = clr_white;
            }
        }

        if (map::player->can_see_actor(*tgt))
        {
            msg_log::add(str_begin + " struck by a roaring blast!", msg_clr);
        }

        tgt->prop_handler().try_add(new PropParalyzed(PropTurns::specific, 2));

        const int dmg = is_warlock_charged ? dmg_range.max : dmg_range.roll();

        tgt->hit(dmg, DmgType::physical, DmgMethod::END, AllowWound::no);

        Snd snd("",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                tgt->pos,
                nullptr,
                SndVol::high, AlertsMon::yes);

        snd_emit::run(snd);
    }

    return SpellEffectNoticed::yes;
}

bool SpellAzaWrath::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(3);
}

// -----------------------------------------------------------------------------
// Mayhem
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellMayhem::cast_impl(Actor* const caster) const
{
    const bool is_player = caster->is_player();

    if (map::player->can_see_actor(*caster))
    {
        std::string caster_name = is_player ? "me" : caster->name_the();
        msg_log::add("Destruction rages around " + caster_name + "!");
    }

    const P& caster_pos = caster->pos;

    const int nr_sweeps = 5;
    const int radi      = fov_std_radi_int;

    const int x0 = std::max(1, caster_pos.x - radi);
    const int y0 = std::max(1, caster_pos.y - radi);
    const int x1 = std::min(map_w - 1, caster_pos.x + radi) - 1;
    const int y1 = std::min(map_h - 1, caster_pos.y + radi) - 1;

    for (int i = 0; i < nr_sweeps; ++i)
    {
        for (int y = y0; y <= y1; ++y)
        {
            for (int x = x0; x <= x1; ++x)
            {
                bool is_adj_to_walkable_cell = false;

                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        const Rigid* const f = map::cells[x + dx][y + dy].rigid;

                        if (f->can_move_cmn()) {is_adj_to_walkable_cell = true;}
                    }
                }

                if (is_adj_to_walkable_cell && rnd::one_in(8))
                {
                    map::cells[x][y].rigid->hit(DmgType::physical, DmgMethod::explosion);
                }
            }
        }
    }

    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            auto* const f = map::cells[x][y].rigid;

            if (f->can_have_blood() && rnd::one_in(10))
            {
                f->mk_bloody();
            }

            if (rnd::one_in(4))
            {
                f->hit(DmgType::fire, DmgMethod::elemental, nullptr);
            }
        }
    }

    std::vector<Actor*> seen_foes;
    caster->seen_foes(seen_foes);

    for (auto* tgt : seen_foes)
    {
        // Spell reflection?
        if (tgt->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*tgt))
            {
                msg_log::add(spell_reflect_msg,
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);
            }

            cast_impl(tgt);
            continue;
        }

        // Spell resistance?
        if (tgt->has_prop(PropId::rSpell))
        {
            on_resist(*tgt);

            continue;
        }

        tgt->prop_handler().try_add(new PropBurning(PropTurns::std));
    }

    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            caster_pos,
            nullptr,
            SndVol::high,
            AlertsMon::yes);

    snd_emit::run(snd);

    return SpellEffectNoticed::yes;
}

bool SpellMayhem::allow_mon_cast_now(Mon& mon) const
{
    return
        mon.aware_counter_ > 0 &&
        rnd::coin_toss() &&
        (mon.tgt_ || rnd::one_in(20));
}

// -----------------------------------------------------------------------------
// Pestilence
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellPest::cast_impl(Actor* const caster) const
{
    const size_t nr_mon = rnd::range(7, 10);

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

    std::vector<Mon*> mon_summoned;

    actor_factory::summon(caster->pos, {nr_mon, ActorId::rat},
                          MakeMonAware::yes,
                          leader,
                          &mon_summoned);

    bool is_any_seen_by_player = false;

    for (Mon* const mon : mon_summoned)
    {
        if (map::player->can_see_actor(*mon))
        {
            is_any_seen_by_player = true;
        }
    }

    if (is_any_seen_by_player)
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
            msg_log::add("They are hostile!", clr_msg_note, true, MorePromptOnMsg::yes);
        }

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

bool SpellPest::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_         &&
           rnd::one_in(3)   &&
           (mon.tgt_ || rnd::one_in(30));
}

// -----------------------------------------------------------------------------
// Animate weapons
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellAnimWpns::cast_impl(Actor* const caster) const
{
    bool is_any_animated = false;

    if (caster->is_player())
    {
        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                Cell& cell = map::cells[x][y];

                Item* const item = cell.item;

                if (
                    cell.is_seen_by_player  &&
                    item                    &&
                    item->data().type == ItemType::melee_wpn)
                {
                    cell.item = nullptr;

                    const P p(x, y);

                    std::vector<Mon*> summoned;

                    actor_factory::summon(p,
                                          std::vector<ActorId>(1, ActorId::animated_wpn),
                                          MakeMonAware::no,
                                          map::player,
                                          &summoned,
                                          Verbosity::silent);

                    ASSERT(summoned.size() == 1);

                    Mon* const anim_wpn = summoned[0];

                    Inventory& inv = anim_wpn->inv();

                    ASSERT(!inv.item_in_slot(SlotId::wpn));

                    inv.put_in_slot(SlotId::wpn, item);

                    const std::string item_name = item->name(ItemRefType::plain,
                                                  ItemRefInf::yes,
                                                  ItemRefAttInf::none);

                    msg_log::add("The " + item_name + " rises into thin air!");

                    is_any_animated = true;
                }
            }
        }
    }

    return is_any_animated ?
           SpellEffectNoticed::yes : SpellEffectNoticed::no;
}

// -----------------------------------------------------------------------------
// Pharaoh staff
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellPharaohStaff::cast_impl(Actor* const caster) const
{
    // First check for a friendly mummy and heal it (as per the spell description)
    for (Actor* const actor : game_time::actors)
    {
        const auto actor_id = actor->data().id;

        const bool is_actor_id_ok = actor_id == ActorId::mummy ||
                                    actor_id == ActorId::croc_head_mummy;

        if (is_actor_id_ok && caster->is_leader_of(actor))
        {
            actor->restore_hp(999);
            return SpellEffectNoticed::yes;
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

    std::vector<Mon*> summoned_mon;

    const auto actor_id = rnd::coin_toss() ? ActorId::mummy : ActorId::croc_head_mummy;

    actor_factory::summon(caster->pos, {actor_id},
                          MakeMonAware::yes,
                          leader,
                          &summoned_mon);

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

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

bool SpellPharaohStaff::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(4);
}

// -----------------------------------------------------------------------------
// Detect items
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellDetItems::cast_impl(Actor* const caster) const
{
    (void)caster;

    const int radi = fov_std_radi_int + 3;
    const int orig_x = map::player->pos.x;
    const int orig_y = map::player->pos.y;
    const int x0 = std::max(0, orig_x - radi);
    const int y0 = std::max(0, orig_y - radi);
    const int x1 = std::min(map_w - 1, orig_x + radi);
    const int y1 = std::min(map_h - 1, orig_y + radi);

    std::vector<P> items_revealed_cells;

    for (int y = y0; y < y1; ++y)
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

    if (!items_revealed_cells.empty())
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

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

// -----------------------------------------------------------------------------
// Detect traps
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellDetTraps::cast_impl(Actor* const caster) const
{
    (void)caster;

    std::vector<P> traps_revealed_cells;

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (map::cells[x][y].is_seen_by_player)
            {
                auto* const f = map::cells[x][y].rigid;

                if (f->id() == FeatureId::trap)
                {
                    auto* const trap = static_cast<Trap*>(f);
                    trap->reveal(false);
                    traps_revealed_cells.push_back(P(x, y));
                }
            }
        }
    }

    if (!traps_revealed_cells.empty())
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

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

// -----------------------------------------------------------------------------
// Detect monsters
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellDetMon::cast_impl(Actor* const caster) const
{
    (void)caster;

    bool is_seer = player_bon::traits[(size_t)Trait::seer];

    const int multiplier  = 20 * (is_seer ? 2 : 1);

    SpellEffectNoticed is_noticed = SpellEffectNoticed::no;

    for (Actor* actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            static_cast<Mon*>(actor)->set_player_aware_of_me(multiplier);
            is_noticed = SpellEffectNoticed::yes;
        }
    }

    if (is_noticed == SpellEffectNoticed::yes)
    {
        msg_log::add("I detect creatures.");
    }

    return is_noticed;
}

// -----------------------------------------------------------------------------
// Opening
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellOpening::cast_impl(Actor* const caster) const
{
    (void)caster;

    bool is_any_opened = false;

    for (int y = 1; y < map_h - 1; ++y)
    {
        for (int x = 1; x < map_w - 1; ++x)
        {
            const auto& cell = map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                DidOpen did_open = cell.rigid->open(nullptr);

                if (did_open == DidOpen::yes)
                {
                    is_any_opened = true;
                }
            }
        }
    }

    if (!is_any_opened)
    {
        return SpellEffectNoticed::no;
    }

    map::update_vision();

    return SpellEffectNoticed::yes;
}

// -----------------------------------------------------------------------------
// Sacrifice life
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellSacrLife::cast_impl(Actor* const caster) const
{
    (void)caster;

    // Convert every 2 hp to 1 SPI

    const int player_hp_cur = map::player->hp();

    if (player_hp_cur > 2)
    {
        const int hp_drained = ((player_hp_cur - 1) / 2) * 2;

        map::player->hit(hp_drained, DmgType::pure);

        map::player->restore_spi(hp_drained, true);

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

// -----------------------------------------------------------------------------
// Sacrifice spirit
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellSacrSpi::cast_impl(Actor* const caster) const
{
    (void)caster;

    // Convert all spi to HP

    const int player_spi_cur = map::player->spi();

    if (player_spi_cur > 0)
    {
        const int hp_drained = player_spi_cur - 1;

        map::player->hit_spi(hp_drained);

        map::player->restore_hp(hp_drained);

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

// -----------------------------------------------------------------------------
// Cloud minds
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellCloudMinds::cast_impl(Actor* const caster) const
{
    (void)caster;

    msg_log::add("I vanish from the minds of my enemies.");

    for (Actor* actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            Mon* const mon = static_cast<Mon*>(actor);
            mon->aware_counter_ = 0;
        }
    }

    return SpellEffectNoticed::yes;
}

// -----------------------------------------------------------------------------
// Ghoul frenzy
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellFrenzy::cast_impl(Actor* const caster) const
{
    const int nr_turns = rnd::range(12, 18);

    PropFrenzied* frenzy = new PropFrenzied(PropTurns::specific, nr_turns);

    caster->prop_handler().try_add(frenzy);

    return SpellEffectNoticed::yes;
}

// -----------------------------------------------------------------------------
// Bless
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellBless::cast_impl(Actor* const caster) const
{
    caster->prop_handler().try_add(new PropBlessed(PropTurns::std));

    return SpellEffectNoticed::yes;
}

// -----------------------------------------------------------------------------
// Light
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellLight::cast_impl(Actor* const caster) const
{
    caster->prop_handler().try_add(new PropRadiant(PropTurns::std));
    return SpellEffectNoticed::yes;
}

// -----------------------------------------------------------------------------
// Teleport
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellTeleport::cast_impl(Actor* const caster) const
{
    caster->teleport();

    return SpellEffectNoticed::yes;
}

bool SpellTeleport::allow_mon_cast_now(Mon& mon) const
{
    const bool is_low_hp = mon.hp() <= (mon.hp_max(true) / 2);

    return (mon.aware_counter_ > 0) && is_low_hp && rnd::fraction(3, 4);
}

// -----------------------------------------------------------------------------
// Resistance
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellRes::cast_impl(Actor* const caster) const
{
    const int duration = 20;

    PropHandler& prop_hlr = caster->prop_handler();

    prop_hlr.try_add(new PropRFire(PropTurns::specific, duration));
    prop_hlr.try_add(new PropRElec(PropTurns::specific, duration));

    return SpellEffectNoticed::yes;
}

bool SpellRes::allow_mon_cast_now(Mon& mon) const
{
    const bool has_rfire = mon.prop_handler().has_prop(PropId::rFire);
    const bool has_relec = mon.prop_handler().has_prop(PropId::rElec);

    return (!has_rfire || !has_relec) && mon.tgt_ && rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Knockback
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellKnockBack::cast_impl(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Clr             msg_clr     = clr_msg_good;
    std::string     tgt_str     = "me";
    Actor*          caster_used = caster;
    Actor*          tgt         = static_cast<Mon*>(caster_used)->tgt_;
    ASSERT(tgt);

    // Spell reflection?
    if (tgt->has_prop(PropId::spell_reflect))
    {
        if (map::player->can_see_actor(*tgt))
        {
            msg_log::add(spell_reflect_msg,
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
        }

        std::swap(caster_used, tgt);
    }

    // Spell resistance?
    if (tgt->has_prop(PropId::rSpell))
    {
        on_resist(*tgt);

        return SpellEffectNoticed::yes;
    }

    if (tgt->is_player())
    {
        msg_clr = clr_msg_bad;
    }
    else // Target is monster
    {
        tgt_str = tgt->name_the();

        if (map::player->is_leader_of(tgt)) {msg_clr = clr_white;}
    }

    if (map::player->can_see_actor(*tgt))
    {
        msg_log::add("A force pushes " + tgt_str + "!", msg_clr);
    }

    knock_back::try_knock_back(*tgt, caster->pos, false);

    return SpellEffectNoticed::no;
}

bool SpellKnockBack::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Enfeeble
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellEnfeebleMon::cast_impl(Actor* const caster) const
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

    std::vector<Actor*> tgts;
    caster->seen_foes(tgts);

    if (tgts.empty())
    {
        return SpellEffectNoticed::no;
    }

    io::draw_blast_at_seen_actors(tgts, clr_magenta);

    for (Actor* const tgt : tgts)
    {
        PropHandler& prop_handler = tgt->prop_handler();

        // Spell reflection?
        if (tgt->has_prop(PropId::spell_reflect))
        {
            if (map::player->can_see_actor(*tgt))
            {
                msg_log::add(spell_reflect_msg,
                             clr_text,
                             false,
                             MorePromptOnMsg::yes);
            }

            cast_impl(tgt);
            continue;
        }

        // Spell resistance?
        if (tgt->has_prop(PropId::rSpell))
        {
            on_resist(*tgt);

            continue;
        }

        Prop* const prop = prop_handler.mk_prop(prop_id, PropTurns::std);
        prop_handler.try_add(prop);
    }

    return SpellEffectNoticed::yes;
}

bool SpellEnfeebleMon::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(4);
}

// -----------------------------------------------------------------------------
// Disease
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellDisease::cast_impl(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;
    Actor* tgt        = static_cast<Mon*>(caster_used)->tgt_;

    // Spell reflection?
    if (tgt->has_prop(PropId::spell_reflect))
    {
        if (map::player->can_see_actor(*tgt))
        {
            msg_log::add(spell_reflect_msg,
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
        }

        std::swap(caster_used, tgt);
    }

    // Spell resistance?
    if (tgt->has_prop(PropId::rSpell))
    {
        on_resist(*tgt);

        return SpellEffectNoticed::yes;
    }

    std::string actor_name = "me";

    if (!tgt->is_player())
    {
        actor_name = tgt->name_the();
    }

    if (map::player->can_see_actor(*tgt))
    {
        msg_log::add("A horrible disease is starting to afflict " + actor_name + "!");
    }

    tgt->prop_handler().try_add(new PropDiseased(PropTurns::std));
    return SpellEffectNoticed::no;
}

bool SpellDisease::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::coin_toss();
}

// -----------------------------------------------------------------------------
// Summon monster
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellSummonMon::cast_impl(Actor* const caster) const
{
    // Try to summon a creature inside the player's FOV (inside the standard range), in a
    // free visible cell. If no such cell is available, instead summon near the caster.

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksMoveCmn(true), blocked);

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
        std::vector<P> free_cells_vector;
        to_vec(blocked, false, free_cells_vector);

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

    for (int i = 0; i < (int)ActorId::END; ++i)
    {
        const ActorDataT& data = actor_data::data[i];

        if (data.can_be_summoned)
        {
            // Method for finding eligible monsters depends on if player or
            // monster is casting.
            int dlvl_max = -1;

            if (caster->is_player())
            {
                // Compare player CVL with monster's allowed spawning DLVL.
                const int player_clvl = game::clvl();

                const int player_clvl_pct =
                    (player_clvl * 100) / player_max_clvl;

                dlvl_max = (player_clvl_pct * dlvl_last) / 100;
            }
            else // Caster is monster
            {
                // Compare caster and summoned monster's allowed spawning DLVL.
                dlvl_max = caster->data().spawn_min_dlvl;
            }

            if (data.spawn_min_dlvl <= dlvl_max)
            {
                summon_bucket.push_back(ActorId(i));
            }
        }
    }

    if (summon_bucket.empty())
    {
        TRACE << "No elligible monsters found for spawning" << std::endl;
        ASSERT(false);
        return SpellEffectNoticed::no;
    }

    const ActorId   mon_id                      = rnd::element(summon_bucket);
    Actor*          leader                      = nullptr;
    bool            did_player_summon_hostile   = false;

    if (caster->is_player())
    {
        const int n = summon_hostile_one_in_n *
            (player_bon::traits[(size_t)Trait::summoner] ? 2 : 1);

        did_player_summon_hostile   = rnd::one_in(n);
        leader                      = did_player_summon_hostile ? nullptr : caster;
    }
    else // Caster is monster
    {
        Actor* const caster_leader  = static_cast<Mon*>(caster)->leader_;
        leader                      = caster_leader ? caster_leader : caster;
    }

    std::vector<Mon*> mon_summoned;

    actor_factory::summon(summon_pos, {mon_id},
                          MakeMonAware::yes,
                          leader,
                          &mon_summoned);

    Mon* const mon = mon_summoned[0];

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

        return SpellEffectNoticed::yes;
    }

    return SpellEffectNoticed::no;
}

bool SpellSummonMon::allow_mon_cast_now(Mon& mon) const
{
    // NOTE: Checking awareness instead of target, to allow summoning even with broken LOS
    return (mon.aware_counter_ > 0) &&
           rnd::coin_toss()         &&
           (mon.tgt_ || rnd::one_in(23));
}

// -----------------------------------------------------------------------------
// Heal self
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellHealSelf::cast_impl(Actor* const caster) const
{
    // The spell effect is noticed if any hit points were restored
    const bool is_any_hp_healed = caster->restore_hp(999);

    return is_any_hp_healed ? SpellEffectNoticed::yes : SpellEffectNoticed::no;
}

bool SpellHealSelf::allow_mon_cast_now(Mon& mon) const
{
    return mon.hp() < mon.hp_max(true);
}

// -----------------------------------------------------------------------------
// Mi-go hypnosis
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellMiGoHypno::cast_impl(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used  = caster;
    Actor* tgt          = static_cast<Mon*>(caster_used)->tgt_;

    ASSERT(tgt);

    // Spell reflection?
    if (tgt->has_prop(PropId::spell_reflect))
    {
        if (map::player->can_see_actor(*tgt))
        {
            msg_log::add(spell_reflect_msg,
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
        }

        std::swap(caster_used, tgt);
    }

    // Spell resistance?
    if (tgt->has_prop(PropId::rSpell))
    {
        on_resist(*tgt);

        return SpellEffectNoticed::yes;
    }

    if (tgt->is_player())
    {
        msg_log::add("There is a sharp droning in my head!");
    }

    if (rnd::coin_toss())
    {
        Prop* const prop = new PropFainted(PropTurns::specific, rnd::range(2, 10));
        tgt->prop_handler().try_add(prop);
    }
    else
    {
        msg_log::add("I feel dizzy.");
    }

    return SpellEffectNoticed::yes;
}

bool SpellMiGoHypno::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_                 &&
           mon.tgt_->is_player()    &&
           rnd::one_in(3);
}

// -----------------------------------------------------------------------------
// Immolation
// -----------------------------------------------------------------------------
SpellEffectNoticed SpellBurn::cast_impl(Actor* const caster) const
{
    ASSERT(!caster->is_player());

    Actor* caster_used = caster;
    Actor* tgt        = static_cast<Mon*>(caster_used)->tgt_;

    ASSERT(tgt);

    // Spell reflection?
    if (tgt->has_prop(PropId::spell_reflect))
    {
        if (map::player->can_see_actor(*tgt))
        {
            msg_log::add(spell_reflect_msg,
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
        }

        std::swap(caster_used, tgt);
    }

    // Spell resistance?
    if (tgt->has_prop(PropId::rSpell))
    {
        on_resist(*tgt);

        return SpellEffectNoticed::yes;
    }

    std::string tgt_str = "me";

    if (!tgt->is_player())
    {
        tgt_str = tgt->name_the();
    }

    if (map::player->can_see_actor(*tgt))
    {
        msg_log::add("Flames are rising around " + tgt_str + "!");
    }

    Prop* const prop = new PropBurning(PropTurns::specific, rnd::range(3, 4));
    tgt->prop_handler().try_add(prop);

    return SpellEffectNoticed::yes;
}

bool SpellBurn::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && rnd::one_in(4);
}
