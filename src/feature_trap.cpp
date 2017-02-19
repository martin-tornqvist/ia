#include "feature_trap.hpp"

#include <algorithm>

#include "init.hpp"
#include "feature_data.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "item.hpp"
#include "drop.hpp"
#include "postmortem.hpp"
#include "explosion.hpp"
#include "popup.hpp"
#include "actor_mon.hpp"
#include "inventory.hpp"
#include "sound.hpp"
#include "actor_factory.hpp"
#include "io.hpp"
#include "player_bon.hpp"
#include "item_factory.hpp"
#include "attack.hpp"

// -----------------------------------------------------------------------------
// Trap
// -----------------------------------------------------------------------------
Trap::Trap(const P& feature_pos,
           Rigid* const mimic_feature,
           TrapId id) :
    Rigid                   (feature_pos),
    mimic_feature_          (mimic_feature),
    is_hidden_              (true),
    nr_turns_until_trigger_ (-1),
    trap_impl_              (nullptr)
{
    ASSERT(id != TrapId::END);

    auto* const rigid_here = map::cells[feature_pos.x][feature_pos.y].rigid;

    if (!rigid_here->can_have_rigid())
    {
        TRACE << "Cannot place trap on feature id: " << (int)rigid_here->id()
              << std::endl
              << "Trap id: " << (int)id
              << std::endl;

        ASSERT(false);
        return;
    }

    auto try_place_trap_or_discard = [&](const TrapId trap_id)
    {
        auto* impl = mk_trap_impl_from_id(trap_id);

        auto valid = impl->on_place();

        if (valid == TrapPlacementValid::yes)
        {
            trap_impl_ = impl;
        }
        else // Placement not valid
        {
            delete impl;
        }
    };

    if (id == TrapId::any)
    {
        // Attempt to set a trap implementation until a valid one is picked
        while (true)
        {
            const auto random_id = TrapId(rnd::range(0, (int)TrapId::END - 1));

            try_place_trap_or_discard(random_id);

            if (trap_impl_)
            {
                // Trap placement is good!
                break;
            }
        }
    }
    else // Make a specific trap type
    {
        // NOTE: This may fail, in which case we have no trap implementation.
        // The trap creator is responsible for handling this situation.
        try_place_trap_or_discard(id);
    }
}

Trap::~Trap()
{
    delete trap_impl_;
    delete mimic_feature_;
}

TrapImpl* Trap::mk_trap_impl_from_id(const TrapId trap_id)
{
    switch (trap_id)
    {
    case TrapId::dart:
        return new TrapDart(pos_, this);
        break;

    case TrapId::spear:
        return new TrapSpear(pos_, this);
        break;

    case TrapId::gas_confusion:
        return new TrapGasConfusion(pos_, this);
        break;

    case TrapId::gas_paralyze:
        return new TrapGasParalyzation(pos_, this);
        break;

    case TrapId::gas_fear:
        return new TrapGasFear(pos_, this);
        break;

    case TrapId::blinding:
        return new TrapBlindingFlash(pos_, this);
        break;

    case TrapId::teleport:
        return new TrapTeleport(pos_, this);
        break;

    case TrapId::summon:
        return new TrapSummonMon(pos_, this);
        break;

    case TrapId::spi_drain:
        return new TrapSpiDrain(pos_, this);
        break;

    case TrapId::smoke:
        return new TrapSmoke(pos_, this);
        break;

    case TrapId::fire:
        return new TrapFire(pos_, this);
        break;

    case TrapId::alarm:
        return new TrapAlarm(pos_, this);
        break;

    case TrapId::web:
        return new TrapWeb(pos_, this); break;

    default:
        return nullptr;
    }
}

void Trap::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

TrapId Trap::type() const
{
    ASSERT(trap_impl_);
    return trap_impl_->type_;
}

bool Trap::is_holding_actor() const
{
    ASSERT(trap_impl_);

    return trap_impl_->is_holding_actor();
}

bool Trap::is_magical() const
{
    ASSERT(trap_impl_);

    return trap_impl_->is_magical();
}

void Trap::on_new_turn_hook()
{
    if (nr_turns_until_trigger_ > 0)
    {
        --nr_turns_until_trigger_;

        TRACE_VERBOSE << "Number of turns until trigger: "
                      << nr_turns_until_trigger_ << std::endl;

        if (nr_turns_until_trigger_ == 0)
        {
            // NOTE: This will reset number of turns until triggered
            trigger_trap(nullptr);
        }
    }
}

void Trap::trigger_start(const Actor* actor)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    ASSERT(trap_impl_);

    if (actor == map::player)
    {
        // Reveal trap if triggered by player stepping on it
        reveal(Verbosity::silent);

        map::player->update_fov();

        states::draw();
    }

    if (is_magical())
    {
        // TODO: Play sfx for magic traps (if player)
    }
    else // Not magical
    {
        if (type() != TrapId::web)
        {
            std::string msg = "I hear a click.";

            auto more_prompt_on_msg = MorePromptOnMsg::no;

            if (actor == map::player)
            {
                // If player is triggering, make the message a bit more
                // "foreboding"
                msg += "..";

                more_prompt_on_msg = MorePromptOnMsg::yes;
            }

            // TODO: Make a sound effect for this
            Snd snd(msg,
                    SfxId::END,
                    IgnoreMsgIfOriginSeen::no,
                    pos_,
                    nullptr,
                    SndVol::low,
                    AlertsMon::yes,
                    more_prompt_on_msg);

            snd_emit::run(snd);
        }
    }

    // Get a randomized value for number of remaining turns
    const Range turns_range = trap_impl_->nr_turns_range_to_trigger();
    const int rnd_nr_turns = turns_range.roll();

    // Set number of remaining turns to the randomized value if number of turns
    // was not already set, or if the new value will make it trigger sooner.
    if (
        nr_turns_until_trigger_ == -1 ||
        rnd_nr_turns < nr_turns_until_trigger_)
    {
        nr_turns_until_trigger_ = rnd_nr_turns;
    }

    TRACE_VERBOSE << "nr_turns_until_trigger_: "
                  << nr_turns_until_trigger_
                  << std::endl;

    ASSERT(nr_turns_until_trigger_ > -1);

    // If number of remaining turns is zero, trigger immediately
    if (nr_turns_until_trigger_ == 0)
    {
        // NOTE: This will reset number of turns until triggered
        trigger_trap(nullptr);
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap::bump(Actor& actor_bumping)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const ActorDataT& d = actor_bumping.data();

    if (actor_bumping.has_prop(PropId::ethereal) ||
        actor_bumping.has_prop(PropId::flying))
    {
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool actor_can_see = actor_bumping.prop_handler().allow_see();

    const std::string trap_name = trap_impl_->title();

    //
    // TODO: Reimplement something affecting chance of success, e.g.
    //       dexterous traits
    //

    const int dodge_skill = 50;

    if (actor_bumping.is_player())
    {
        TRACE_VERBOSE << "Player bumping" << std::endl;

        int avoid_skill = 30 + dodge_skill;

        if (is_hidden_)
        {
            avoid_skill = std::max(10, avoid_skill / 2);
        }

        const auto result = ability_roll::roll(avoid_skill, &actor_bumping);

        if (result >= ActionResult::success)
        {
            if (!is_hidden_ && actor_can_see)
            {
                map::player->update_fov();

                states::draw();

                msg_log::add("I avoid a " +
                             trap_name +
                             ".",
                             clr_msg_good);
            }
        }
        else // Failed to avoid
        {
            trigger_start(&actor_bumping);
        }
    }
    else // Is a monster
    {
        TRACE_VERBOSE << "Monster bumping trap" << std::endl;

        if (d.actor_size == ActorSize::humanoid && !d.is_spider)
        {
            TRACE_VERBOSE << "Humanoid monster bumping" << std::endl;
            Mon* const mon = static_cast<Mon*>(&actor_bumping);

            if (mon->aware_of_player_counter_ > 0)
            {
                TRACE_VERBOSE << "Monster eligible for triggering trap"
                              << std::endl;

                const bool is_actor_seen_by_player =
                    map::player->can_see_actor(actor_bumping);

                const int avoid_skill = 60 + dodge_skill;

                const auto result =
                    ability_roll::roll(avoid_skill, &actor_bumping);

                if (result >= ActionResult::success)
                {
                    if (!is_hidden_ && is_actor_seen_by_player)
                    {
                        const std::string actor_name = actor_bumping.name_the();

                        msg_log::add(actor_name +
                                     " avoids a " +
                                     trap_name +
                                     ".");
                    }
                }
                else // Failed to avoid
                {
                    trigger_start(&actor_bumping);
                }
            }
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap::disarm()
{
    // Abort if trap is hidden
    if (is_hidden())
    {
        msg_log::add(msg_disarm_no_trap);

        states::draw();

        return;
    }

    // Spider webs are automatically destroyed if wielding machete
    bool is_auto_succeed = false;

    if (type() == TrapId::web)
    {
        Item* item = map::player->inv().item_in_slot(SlotId::wpn);

        if (item)
        {
            is_auto_succeed = item->id() == ItemId::machete;
        }
    }

    const bool is_occultist = player_bon::bg() == Bg::occultist;

    if (is_magical() && !is_occultist)
    {
        msg_log::add("I do not know how to dispel magic traps.");
        return;
    }

    const bool is_blessed = map::player->has_prop(PropId::blessed);
    const bool is_cursed = map::player->has_prop(PropId::cursed);

    int disarm_num = 6;
    const int disarm_den = 10;

    if (is_blessed)
    {
        disarm_num += 3;
    }

    if (is_cursed)
    {
        disarm_num -= 3;
    }

    set_constr_in_range(1, disarm_num, disarm_den - 1);

    const bool is_disarmed = is_auto_succeed ||
                             rnd::fraction(disarm_num, disarm_den);

    if (is_disarmed)
    {
        msg_log::add(trap_impl_->disarm_msg());
    }
    else // Not disarmed
    {
        msg_log::add(trap_impl_->disarm_fail_msg());

        states::draw();

        const int trigger_one_in_n = is_blessed ? 9 : is_cursed ? 2 : 6;

        if (rnd::one_in(trigger_one_in_n))
        {
            map::player->pos = pos_;

            trigger_trap(map::player);
        }
    }

    game_time::tick();

    if (is_disarmed)
    {
        destroy();
    }
}

void Trap::destroy()
{
    ASSERT(mimic_feature_);

    // Magical traps and webs simply "dissapear" (place their mimic feature),
    // and mechanical traps puts rubble.

    if (is_magical() || type() == TrapId::web)
    {
        Rigid* const f_tmp = mimic_feature_;

        mimic_feature_ = nullptr;

        // NOTE: This call destroys the object!
        map::put(f_tmp);
    }
    else // "Mechanical" trap
    {
        map::put(new RubbleLow(pos_));
    }
}

DidTriggerTrap Trap::trigger_trap(Actor* const actor)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    (void)actor;

    TRACE_VERBOSE << "Name of trap triggering: "
                  << trap_impl_->title()
                  << std::endl;

    nr_turns_until_trigger_ = -1;

    TRACE_VERBOSE << "Calling trigger in trap implementation" << std::endl;

    trap_impl_->trigger();

    // NOTE: This object may now be deleted (e.g. a web was torn down)!

    TRACE_FUNC_END_VERBOSE;
    return DidTriggerTrap::yes;
}

void Trap::reveal(const Verbosity verbosity)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (!is_hidden_)
    {
        return;
    }

    is_hidden_ = false;

    // Destroy any corpse on the trap
    for (Actor* actor : game_time::actors)
    {
        if (actor->pos == pos_ && actor->is_corpse())
        {
            actor->state_ = ActorState::destroyed;
        }
    }

    clear_gore();

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        states::draw();

        if (verbosity == Verbosity::verbose)
        {
            std::string msg = "";

            const std::string trap_name = trap_impl_->title();

            if (pos_ == map::player->pos)
            {
                msg += "There is a " + trap_name + " here!";
            }
            else // Trap is not at player position
            {
                msg = "I spot a " + trap_name + ".";
            }

            msg_log::add(msg);
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

std::string Trap::name(const Article article) const
{
    if (is_hidden_)
    {
        return mimic_feature_->name(article);
    }
    else // Not hidden
    {
        return (article == Article::a ? "a " : "the ") + trap_impl_->title();
    }
}

Clr Trap::clr_default() const
{
    return is_hidden_ ? mimic_feature_->clr() : trap_impl_->clr();
}

Clr Trap::clr_bg_default() const
{
    const auto* const item = map::cells[pos_.x][pos_.y].item;

    if (is_hidden_ || !item)
    {
        return clr_black;
    }
    else
    {
        return trap_impl_->clr();
    }
}

char Trap::glyph() const
{
    return is_hidden_ ? mimic_feature_->glyph() : trap_impl_->glyph();
}

TileId Trap::tile() const
{
    return is_hidden_ ? mimic_feature_->tile() : trap_impl_->tile();
}

Dir Trap::actor_try_leave(Actor& actor, const Dir dir)
{
    return trap_impl_->actor_try_leave(actor, dir);
}

Matl Trap::matl() const
{
    return is_hidden_ ? mimic_feature_->matl() : data().matl_type;
}

// -----------------------------------------------------------------------------
// Trap implementations
// -----------------------------------------------------------------------------
TrapDart::TrapDart(P pos, Trap* const base_trap) :
    MechTrapImpl                (pos, TrapId::dart, base_trap),
    is_poisoned_                (map::dlvl >= dlvl_harder_traps &&
                                 rnd::one_in(3)),
    dart_origin_                (),
    is_dart_origin_destroyed_   (false) {}

TrapPlacementValid TrapDart::on_place()
{
    auto offsets = dir_utils::cardinal_list;

    random_shuffle(begin(offsets), end(offsets));

    const int nr_steps_min = 2;
    const int nr_steps_max = fov_std_radi_int;

    auto trap_plament_valid = TrapPlacementValid::no;

    for (const P& d : offsets)
    {
        P p = pos_;

        for (int i = 0; i <= nr_steps_max; ++i)
        {
            p += d;

            const Rigid* const rigid = map::cells[p.x][p.y].rigid;
            const bool is_wall = rigid->id() == FeatureId::wall;
            const bool is_passable = rigid->is_projectile_passable();

            if (!is_passable && (i < nr_steps_min || !is_wall))
            {
                // We are blocked too early - OR - blocked by a rigid feature
                // other than a wall. Give up on this direction.
                break;
            }

            if (i >= nr_steps_min && is_wall)
            {
                // This is a good origin!
                dart_origin_ = p;
                trap_plament_valid = TrapPlacementValid::yes;
                break;
            }
        }

        if (trap_plament_valid == TrapPlacementValid::yes)
        {
            // A valid origin has been found

            if (rnd::fraction(2, 3))
            {
                map::mk_gore(pos_);
                map::mk_blood(pos_);
            }

            break;
        }
    }

    return trap_plament_valid;
}

void TrapDart::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    ASSERT(dart_origin_.x == pos_.x || dart_origin_.y == pos_.y);
    ASSERT(dart_origin_ != pos_);

    const auto& origin_cell = map::cells[dart_origin_.x][dart_origin_.y];

    if (origin_cell.rigid->id() != FeatureId::wall)
    {
        // NOTE: This is permanently set from now on
        is_dart_origin_destroyed_ = true;
    }

    if (is_dart_origin_destroyed_)
    {
        return;
    }

    // Aim target is the wall on the other side of the map
    P aim_pos = dart_origin_;

    if (dart_origin_.x == pos_.x)
    {
        aim_pos.y = dart_origin_.y > pos_.y ? 0 : (map_h - 1);
    }
    else // Dart origin is on same vertial line as the trap
    {
        aim_pos.x = dart_origin_.x > pos_.x ? 0 : (map_w - 1);
    }

    if (origin_cell.is_seen_by_player)
    {
        const std::string name = origin_cell.rigid->name(Article::the);

        msg_log::add("A dart is launched from " + name + "!");
    }

    // Make a temporary dart weapon
    Wpn* wpn = nullptr;

    if (is_poisoned_)
    {
        wpn = static_cast<Wpn*>(item_factory::mk(ItemId::trap_dart_poison));
    }
    else // Not poisoned
    {
        wpn = static_cast<Wpn*>(item_factory::mk(ItemId::trap_dart));
    }

    // Fire!
    attack::ranged(nullptr, dart_origin_, aim_pos, *wpn);

    delete wpn;

    TRACE_FUNC_END_VERBOSE;
}

TrapSpear::TrapSpear(P pos, Trap* const base_trap) :
    MechTrapImpl                (pos, TrapId::spear, base_trap),
    is_poisoned_                ((map::dlvl >= dlvl_harder_traps) &&
                                 rnd::one_in(4)),
    spear_origin_               (),
    is_spear_origin_destroyed_  (false) {}

TrapPlacementValid TrapSpear::on_place()
{
    auto offsets = dir_utils::cardinal_list;

    random_shuffle(begin(offsets), end(offsets));

    auto trap_plament_valid = TrapPlacementValid::no;

    for (const P& d : offsets)
    {
        const P p = pos_ + d;

        const Rigid* const rigid = map::cells[p.x][p.y].rigid;

        const bool is_wall = rigid->id() == FeatureId::wall;

        const bool is_passable = rigid->is_projectile_passable();

        if (is_wall && !is_passable)
        {
            // This is a good origin!
            spear_origin_ = p;
            trap_plament_valid = TrapPlacementValid::yes;

            if (rnd::fraction(2, 3))
            {
                map::mk_gore(pos_);
                map::mk_blood(pos_);
            }

            break;
        }
    }

    return trap_plament_valid;
}

void TrapSpear::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    ASSERT(spear_origin_.x == pos_.x || spear_origin_.y == pos_.y);
    ASSERT(spear_origin_ != pos_);

    const auto& origin_cell = map::cells[spear_origin_.x][spear_origin_.y];

    if (origin_cell.rigid->id() != FeatureId::wall)
    {
        // NOTE: This is permanently set from now on
        is_spear_origin_destroyed_ = true;
    }

    if (is_spear_origin_destroyed_)
    {
        return;
    }

    if (origin_cell.is_seen_by_player)
    {
        const std::string name = origin_cell.rigid->name(Article::the);

        msg_log::add("A spear shoots out from " + name + "!");
    }

    // Is anyone standing on the trap now?
    Actor* const actor_on_trap = map::actor_at_pos(pos_);

    if (actor_on_trap)
    {
        // Make a temporary spear weapon
        Wpn* wpn = nullptr;

        if (is_poisoned_)
        {
            wpn =
                static_cast<Wpn*>(
                    item_factory::mk(
                        ItemId::trap_spear_poison));
        }
        else // Not poisoned
        {
            wpn =
                static_cast<Wpn*>(
                    item_factory::mk(
                        ItemId::trap_spear));
        }

        // Attack!
        attack::melee(nullptr, spear_origin_, *actor_on_trap, *wpn);

        delete wpn;
    }

    TRACE_FUNC_BEGIN_VERBOSE;
}

void TrapGasConfusion::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of gas is released from a vent in the floor!");
    }

    Snd snd("I hear a burst of gas.",
            SfxId::gas,
            IgnoreMsgIfOriginSeen::yes,
            pos_,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   -1,
                   ExplExclCenter::no,
                   {new PropConfused(PropTurns::std)},
                   &clr_magenta);

    TRACE_FUNC_END_VERBOSE;
}

void TrapGasParalyzation::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of gas is released from a vent in the floor!");
    }

    Snd snd("I hear a burst of gas.",
            SfxId::gas,
            IgnoreMsgIfOriginSeen::yes,
            pos_,
            nullptr,
            SndVol::low, AlertsMon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   -1,
                   ExplExclCenter::no,
                   {new PropParalyzed(PropTurns::std)},
                   &clr_magenta);

    TRACE_FUNC_END_VERBOSE;
}

void TrapGasFear::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of gas is released from a vent in the floor!");
    }

    Snd snd("I hear a burst of gas.",
            SfxId::gas,
            IgnoreMsgIfOriginSeen::yes,
            pos_,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   -1,
                   ExplExclCenter::no,
                   {new PropTerrified(PropTurns::std)},
                   &clr_magenta);

    TRACE_FUNC_END_VERBOSE;
}

void TrapBlindingFlash::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("There is an intense flash of light!");
    }

    explosion::run(pos_,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   -1,
                   ExplExclCenter::no,
                   {new PropBlind(PropTurns::std)},
                   &clr_yellow);

    TRACE_FUNC_END_VERBOSE;
}

void TrapTeleport::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = map::actor_at_pos(pos_);

    ASSERT(actor_here);

    if (!actor_here)
    {
        // Should never happen
        return;
    }

    const bool is_player = actor_here->is_player();

    const bool can_see = actor_here->prop_handler().allow_see();

    const bool can_player_see_actor = map::player->can_see_actor(*actor_here);

    const std::string actor_name = actor_here->name_the();

    const bool is_hidden = base_trap_->is_hidden();

    if (is_player)
    {
        map::update_vision();

        if (can_see)
        {
            std::string msg = "A beam of light shoots out from";

            if (!is_hidden)
            {
                msg += " a curious shape on";
            }

            msg += " the floor!";

            msg_log::add(msg);
        }
        else // Cannot see
        {
            msg_log::add("I feel a peculiar energy around me!");
        }
    }
    else // Is a monster
    {
        if (can_player_see_actor)
        {
            msg_log::add("A beam shoots out under " + actor_name + ".");
        }
    }

    actor_here->teleport();

    TRACE_FUNC_END_VERBOSE;
}

void TrapSummonMon::trigger()
{
    TRACE_FUNC_BEGIN;

    Actor* const actor_here = map::actor_at_pos(pos_);

    ASSERT(actor_here);

    if (!actor_here)
    {
        // Should never happen
        return;
    }

    const bool is_player = actor_here->is_player();
    const bool is_hidden = base_trap_->is_hidden();

    TRACE_VERBOSE << "Is player: " << is_player << std::endl;

    if (!is_player)
    {
        TRACE_VERBOSE << "Not triggered by player" << std::endl;
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool can_see = actor_here->prop_handler().allow_see();
    TRACE_VERBOSE << "Actor can see: " << can_see << std::endl;

    const std::string actor_name = actor_here->name_the();
    TRACE_VERBOSE << "Actor name: " << actor_name << std::endl;

    map::player->update_fov();

    if (can_see)
    {
        std::string msg = "A beam of light shoots out from";

        if (!is_hidden)
        {
            msg += " a curious shape on";
        }

        msg += " the floor!";

        msg_log::add(msg);
    }
    else // Cannot see
    {
        msg_log::add("I feel a peculiar energy around me!");
    }

    TRACE << "Finding summon candidates" << std::endl;
    std::vector<ActorId> summon_bucket;

    for (int i = 0; i < (int)ActorId::END; ++i)
    {
        const ActorDataT& data = actor_data::data[i];

        if (data.can_be_summoned && data.spawn_min_dlvl <= map::dlvl + 3)
        {
            summon_bucket.push_back(ActorId(i));
        }
    }

    if (summon_bucket.empty())
    {
        TRACE_VERBOSE << "No eligible candidates found" << std::endl;
    }
    else // Eligible monsters found
    {
        const size_t idx = rnd::range(0, summon_bucket.size() - 1);

        const ActorId id_to_summon = summon_bucket[idx];

        TRACE_VERBOSE << "Actor id: " << int(id_to_summon) << std::endl;

        const auto summoned =
            actor_factory::spawn(pos_,
                                  {1, id_to_summon},
                                  MakeMonAware::yes,
                                  nullptr);

        ASSERT(summoned.size() == 1);

        TRACE_VERBOSE << "Monster was summoned" << std::endl;

        Mon* const mon = summoned[0];

        if (map::player->can_see_actor(*mon))
        {
            mon->set_player_aware_of_me();

            states::draw();

            msg_log::add(mon->name_a() + " appears!");
        }

        map::player->incr_shock(5, ShockSrc::misc);
    }

    TRACE_FUNC_END;
}

void TrapSpiDrain::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = map::actor_at_pos(pos_);

    ASSERT(actor_here);

    if (!actor_here)
    {
        // Should never happen
        return;
    }

    const bool is_player = actor_here->is_player();
    const bool is_hidden = base_trap_->is_hidden();

    TRACE_VERBOSE << "Is player: " << is_player << std::endl;

    if (!is_player)
    {
        TRACE_VERBOSE << "Not triggered by player" << std::endl;
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool can_see = actor_here->prop_handler().allow_see();

    TRACE_VERBOSE << "Actor can see: " << can_see << std::endl;

    const std::string actor_name = actor_here->name_the();

    TRACE_VERBOSE << "Actor name: " << actor_name << std::endl;

    if (can_see)
    {
        std::string msg = "A beam of light shoots out from";

        if (!is_hidden)
        {
            msg += " a curious shape on";
        }

        msg += " the floor!";

        msg_log::add(msg);
    }
    else // Cannot see
    {
        msg_log::add("I feel a peculiar energy around me!");
    }

    TRACE << "Draining player spirit" << std::endl;

    // Never let spirit draining traps insta-kill the player
    const int player_spi = map::player->spi();

    const int spi_drained = player_spi - 1;

    if (spi_drained > 0)
    {
        map::player->hit_spi(spi_drained);
    }
    else
    {
        msg_log::add("I feel somewhat drained.");
    }

    TRACE_FUNC_END_VERBOSE;
}

void TrapSmoke::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of smoke is released from a vent in the floor!");
    }

    Snd snd("I hear a burst of gas.",
            SfxId::gas,
            IgnoreMsgIfOriginSeen::yes,
            pos_,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);

    explosion::run_smoke_explosion_at(pos_);

    TRACE_FUNC_END_VERBOSE;
}

void TrapFire::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("Flames burst out from a vent in the floor!");
    }

    Snd snd("I hear a burst of flames.",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            pos_,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   -1,
                   ExplExclCenter::no,
                   {new PropBurning(PropTurns::std)});

    TRACE_FUNC_END_VERBOSE;
}

void TrapAlarm::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("An alarm sounds!");
    }

    Snd snd("I hear an alarm sounding!",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            pos_,
            nullptr,
            SndVol::high,
            AlertsMon::yes);

    snd_emit::run(snd);

    TRACE_FUNC_END_VERBOSE;
}

void TrapWeb::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = map::actor_at_pos(pos_);

    ASSERT(actor_here);

    if (!actor_here)
    {
        // Should never happen
        return;
    }

    is_holding_actor_ = true;

    const bool is_player = actor_here->is_player();
    const bool can_see = actor_here->prop_handler().allow_see();
    const bool can_player_see_actor = map::player->can_see_actor(*actor_here);
    const std::string actor_name = actor_here->name_the();

    if (is_player)
    {
        TRACE << "Checking if player has machete" << std::endl;

        const auto& player_inv = map::player->inv();

        Item* const item_wielded = player_inv.item_in_slot(SlotId::wpn);

        const bool has_machete =
            item_wielded &&
            (item_wielded->data().id == ItemId::machete);

        if (has_machete)
        {
            if (can_see)
            {
                msg_log::add("I cut down a spider web with my machete.");
            }
            else
            {
                msg_log::add(
                    "I cut down a sticky mass of threads with my machete.");
            }

            TRACE << "Destroyed by Machete" << std::endl;
            base_trap_->destroy();
        }
        else // Not wielding machete
        {
            if (can_see)
            {
                msg_log::add("I am entangled in a spider web!");
            }
            else // Cannot see
            {
                msg_log::add("I am entangled in a sticky mass of threads!");
            }
        }
    }
    else // Is a monster
    {
        if (can_player_see_actor)
        {
            msg_log::add(actor_name + " is entangled in a huge spider web!");
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

Dir TrapWeb::actor_try_leave(Actor& actor, const Dir dir)
{
    if (!is_holding_actor_)
    {
        TRACE_VERBOSE << "Not holding actor, returning current direction"
                      << std::endl;
        return dir;
    }

    TRACE_VERBOSE << "Is holding actor" << std::endl;

    const bool player_can_see = map::player->prop_handler().allow_see();
    const bool player_can_see_actor = map::player->can_see_actor(actor);
    const std::string actor_name = actor.name_the();

    TRACE_VERBOSE << "Name of actor held: " << actor_name << std::endl;

    // TODO: reimplement something affecting chance of success?

    if (rnd::one_in(4))
    {
        TRACE_VERBOSE << "Actor succeeded to break free" << std::endl;

        is_holding_actor_ = false;

        if (actor.is_player())
        {
            msg_log::add("I break free.");
        }
        else // Is monster
        {
            if (player_can_see_actor)
            {
                msg_log::add(actor_name + " breaks free from a spiderweb.");
            }
        }

        if (rnd::one_in(2))
        {
            if ((actor.is_player() && player_can_see) ||
                (!actor.is_player() && player_can_see_actor))
            {
                msg_log::add("The web is destroyed.");
            }

            TRACE_VERBOSE << "Web destroyed, returning center direction"
                          << std::endl;

            base_trap_->destroy();
        }
    }
    else // Failed to break free
    {
        if (actor.is_player())
        {
            msg_log::add("I struggle to break free.");
        }
        else if (player_can_see_actor)
        {
            msg_log::add(actor_name + " struggles to break free.");
        }
    }

    return Dir::center;
}
