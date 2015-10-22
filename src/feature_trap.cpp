#include "feature_trap.hpp"

#include <algorithm>
#include <cassert>

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
#include "render.hpp"
#include "utils.hpp"
#include "player_bon.hpp"
#include "item_factory.hpp"
#include "attack.hpp"
#include "dungeon_master.hpp"

namespace
{

Alerts_mon is_trigger_snd_alerting_monsters(const Actor* const actor)
{
    if (actor && !actor->is_actor_my_leader(map::player))
    {
        //Is a monster, and is hostile to player - other monsters are not alerted
        return Alerts_mon::no;
    }

    //Is triggered by player, a monster allied to the player, or not by an actor - alert monsters.
    return Alerts_mon::yes;
}

} // namespace

//------------------------------------------------------------- TRAP
Trap::Trap(const P& feature_pos, const Rigid* const mimic_feature, Trap_id id) :
    Rigid                   (feature_pos),
    mimic_feature_          (mimic_feature),
    is_hidden_              (true),
    nr_turns_until_trigger_ (-1),
    trap_impl_              (nullptr)
{
    assert(id != Trap_id::END);

    auto* const rigid_here = map::cells[feature_pos.x][feature_pos.y].rigid;

    if (!rigid_here->can_have_rigid())
    {
        TRACE << "Cannot place trap on feature id: " << int(rigid_here->id()) << std::endl
              << "Trap id: " << int(id) << std::endl;
        assert(false);
        return;
    }

    auto try_place_trap_or_discard = [&](const Trap_id trap_id)
    {
        auto* impl = mk_trap_impl_from_id(trap_id);

        auto valid = impl->on_place();

        if (valid == Trap_placement_valid::yes)
        {
            trap_impl_ = impl;
        }
        else //Placement not valid
        {
            delete impl;
        }
    };

    if (id == Trap_id::any)
    {
        //Attempt to set a trap implementation until a valid one is picked
        while (true)
        {
            const auto random_id = Trap_id(rnd::range(0, int(Trap_id::END) - 1));

            try_place_trap_or_discard(random_id);

            if (trap_impl_)
            {
                //Trap placement is good!
                break;
            }
        }
    }
    else //Make a specific trap type
    {
        //NOTE: This may fail, in which case we have no trap implementation.
        //The trap creator is responsible for handling this situation.
        try_place_trap_or_discard(id);
    }
}

Trap::~Trap()
{
    delete trap_impl_;
    delete mimic_feature_;
}

Trap_impl* Trap::mk_trap_impl_from_id(const Trap_id trap_id) const
{
    switch (trap_id)
    {
    case Trap_id::dart:
        return new Trap_dart(pos_, this);
        break;

    case Trap_id::spear:
        return new Trap_spear(pos_, this);
        break;

    case Trap_id::gas_confusion:
        return new Trap_gas_confusion(pos_, this);
        break;

    case Trap_id::gas_paralyze:
        return new Trap_gas_paralyzation(pos_, this);
        break;

    case Trap_id::gas_fear:
        return new Trap_gas_fear(pos_, this);
        break;

    case Trap_id::blinding:
        return new Trap_blinding_flash(pos_, this);
        break;

    case Trap_id::teleport:
        return new Trap_teleport(pos_, this);
        break;

    case Trap_id::summon:
        return new Trap_summon_mon(pos_, this);
        break;

    case Trap_id::spi_drain:
        return new Trap_spi_drain(pos_, this);
        break;

    case Trap_id::smoke:
        return new Trap_smoke(pos_, this);
        break;

    case Trap_id::fire:
        return new Trap_fire(pos_, this);
        break;

    case Trap_id::alarm:
        return new Trap_alarm(pos_, this);
        break;

    case Trap_id::web:
        return new Trap_web(pos_, this); break;

    default:
        return nullptr;
    }
}

void Trap::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

Trap_id Trap::trap_type() const
{
    assert(trap_impl_);
    return trap_impl_->trap_type_;
}

bool Trap::is_magical() const
{
    assert(trap_impl_);

    return trap_impl_->is_magical();
}

void Trap::on_new_turn_hook()
{
    if (nr_turns_until_trigger_ > 0)
    {
        --nr_turns_until_trigger_;

        TRACE_VERBOSE << "Number of turns until trigger: " << nr_turns_until_trigger_ << std::endl;

        if (nr_turns_until_trigger_ == 0)
        {
            //NOTE: This will reset number of turns  until triggered
            trigger_trap(nullptr);
        }
    }
}

void Trap::trigger_start(const Actor* actor)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    assert(trap_impl_);

    if (actor == map::player)
    {
        map::player->update_fov();
        render::draw_map_and_interface();
    }

    if (is_magical())
    {
        //TODO: Play sfx for magic traps (if player)
    }
    else //Not magical
    {
        More_prompt_on_msg more_prompt_on_msg = More_prompt_on_msg::no;

        if (trap_type() != Trap_id::web)
        {
            std::string msg = "I hear a click.";

            if (actor == map::player)
            {
                //If player is triggering, make the message a bit more "foreboding"
                msg += "..";

                more_prompt_on_msg = More_prompt_on_msg::yes;
            }

            //TODO: Make a sound effect for this
            Snd snd(msg, Sfx_id::END, Ignore_msg_if_origin_seen::no, pos_, nullptr, Snd_vol::low,
                    Alerts_mon::yes, more_prompt_on_msg);

            snd_emit::run(snd);
        }
    }

    //Get a randomized value for number of remaining turns
    const Range turns_range     = trap_impl_->nr_turns_range_to_trigger();
    const int   RND_NR_TURNS    = rnd::range(turns_range);

    //Set number of remaining turns to the randomized value if number of turns
    //was not already set, or if the new value will make it trigger sooner.
    if (nr_turns_until_trigger_ == -1 || RND_NR_TURNS < nr_turns_until_trigger_)
    {
        nr_turns_until_trigger_ = RND_NR_TURNS;
    }

    TRACE_VERBOSE << "nr_turns_until_trigger_: " << nr_turns_until_trigger_ << std::endl;

    assert(nr_turns_until_trigger_ > -1);

    //If number of remaining turns is zero, trigger immediately
    if (nr_turns_until_trigger_ == 0)
    {
        //NOTE: This will reset number of turns  until triggered
        trigger_trap(nullptr);
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap::bump(Actor& actor_bumping)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const Actor_data_t& d = actor_bumping.data();

    if (actor_bumping.has_prop(Prop_id::ethereal) || actor_bumping.has_prop(Prop_id::flying))
    {
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool          ACTOR_CAN_SEE   = actor_bumping.prop_handler().allow_see();
    Ability_vals&       abilities       = actor_bumping.data().ability_vals;
    const std::string   trap_name       = trap_impl_->title();

    const int DODGE_SKILL = abilities.val(Ability_id::dodge_trap, true, actor_bumping);

    if (actor_bumping.is_player())
    {
        TRACE_VERBOSE << "Player bumping" << std::endl;

        int avoid_skill = 30 + DODGE_SKILL;

        if (is_hidden_)
        {
            avoid_skill = std::max(10, avoid_skill / 2);
        }

        const Ability_roll_result result = ability_roll::roll(avoid_skill, &actor_bumping);

        if (result >= success)
        {
            if (!is_hidden_ && ACTOR_CAN_SEE)
            {
                map::player->update_fov();

                render::draw_map_and_interface();

                msg_log::add("I avoid a " + trap_name + ".", clr_msg_good, false,
                             More_prompt_on_msg::yes);
            }
        }
        else //Failed to avoid
        {
            trigger_start(&actor_bumping);
        }
    }
    else //Is a monster
    {
        TRACE_VERBOSE << "Monster bumping trap" << std::endl;

        if (d.actor_size == Actor_size::humanoid && !d.is_spider)
        {
            TRACE_VERBOSE << "Humanoid monster bumping" << std::endl;
            Mon* const mon = static_cast<Mon*>(&actor_bumping);

            if (mon->aware_counter_ > 0 && !mon->is_sneaking_)
            {
                TRACE_VERBOSE << "Monster eligible for triggering trap" << std::endl;

                const bool IS_ACTOR_SEEN_BY_PLAYER =
                    map::player->can_see_actor(actor_bumping);

                const int AVOID_SKILL = 60 + DODGE_SKILL;

                const Ability_roll_result result = ability_roll::roll(AVOID_SKILL, &actor_bumping);

                if (result >= success)
                {
                    if (!is_hidden_ && IS_ACTOR_SEEN_BY_PLAYER)
                    {
                        const std::string actor_name = actor_bumping.name_the();

                        msg_log::add(actor_name + " avoids a " + trap_name + ".");
                    }
                }
                else //Failed to avoid
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
    //Abort if trap is hidden
    if (is_hidden())
    {
        msg_log::add(msg_disarm_no_trap);
        render::draw_map_and_interface();
        return;
    }

    //Spider webs are automatically destroyed if wielding machete
    bool is_auto_succeed = false;

    if (trap_type() == Trap_id::web)
    {
        Item* item = map::player->inv().item_in_slot(Slot_id::wpn);

        if (item)
        {
            is_auto_succeed = item->id() == Item_id::machete;
        }
    }

    const bool IS_OCCULTIST = player_bon::bg() == Bg::occultist;

    if (is_magical() && !IS_OCCULTIST)
    {
        msg_log::add("I do not know how to dispel magic traps.");
        return;
    }

    const bool IS_BLESSED = map::player->has_prop(Prop_id::blessed);
    const bool IS_CURSED  = map::player->has_prop(Prop_id::cursed);

    int         disarm_numerator    = 6;
    const int   DISARM_DENOMINATOR  = 10;

    if (IS_BLESSED)
    {
        disarm_numerator += 3;
    }

    if (IS_CURSED)
    {
        disarm_numerator -= 3;
    }

    utils::set_constr_in_range(1, disarm_numerator, DISARM_DENOMINATOR - 1);

    const bool IS_DISARMED = is_auto_succeed ||
                             rnd::fraction(disarm_numerator, DISARM_DENOMINATOR);

    if (IS_DISARMED)
    {
        msg_log::add(trap_impl_->disarm_msg());

        dungeon_master::incr_player_xp(XP_FOR_DISRM_TRAP);
    }
    else //Not disarmed
    {
        msg_log::add(trap_impl_->disarm_fail_msg());

        render::draw_map_and_interface();

        const int TRIGGER_ONE_IN_N = IS_BLESSED ? 9 : IS_CURSED ? 2 : 6;

        if (rnd::one_in(TRIGGER_ONE_IN_N))
        {
            if (trap_type() == Trap_id::web)
            {
                map::player->pos = pos_;
            }

            trigger_trap(map::player);
        }
    }

    game_time::tick();

    if (IS_DISARMED)
    {
        if (is_magical() || trap_type() == Trap_id::web)
        {
            map::put(new Floor(pos_));
        }
        else //"Mechanical" trap
        {
            map::put(new Rubble_low(pos_));
        }
    }
}

Did_trigger_trap Trap::trigger_trap(Actor* const actor)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    (void)actor;

    TRACE_VERBOSE << "Name of trap triggering: " << trap_impl_->title() << std::endl;

    nr_turns_until_trigger_ = -1;

    TRACE_VERBOSE << "Calling trigger in trap implementation" << std::endl;

    trap_impl_->trigger();

    //NOTE: This object may now be deleted (e.g. a web was torn down)!

    TRACE_FUNC_END_VERBOSE;
    return Did_trigger_trap::yes;
}

void Trap::reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    is_hidden_ = false;

    //Destroy any corpse on the trap
    for (Actor* actor : game_time::actors)
    {
        if (actor->pos == pos_ && actor->is_corpse())
        {
            actor->state_ = Actor_state::destroyed;
        }
    }

    clear_gore();

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        render::draw_map_and_interface();

        if (PRINT_MESSSAGE_WHEN_PLAYER_SEES)
        {
            std::string msg = "";

            const std::string trap_name = trap_impl_->title();

            if (pos_ == map::player->pos)
            {
                msg += "There is a " + trap_name + " here!";
            }
            else //Trap is not at player position
            {
                msg = "I spot a " + trap_name + ".";
            }

            msg_log::add(msg, clr_msg_note, false, More_prompt_on_msg::yes);
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap::player_try_spot_hidden()
{
    if (is_hidden_)
    {
        const auto& abilities = map::player->data().ability_vals;

        const int SKILL = abilities.val(Ability_id::searching, true, *(map::player));

        if (ability_roll::roll(SKILL, map::player) >= success)
        {
            reveal(true);
        }
    }
}

std::string Trap::name(const Article article) const
{
    if (is_hidden_)
    {
        return mimic_feature_->name(article);
    }
    else //Not hidden
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

Tile_id Trap::tile() const
{
    return is_hidden_ ? mimic_feature_->tile() : trap_impl_->tile();
}

Dir Trap::actor_try_leave(Actor& actor, const Dir dir)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    TRACE_FUNC_END_VERBOSE;
    return trap_impl_->actor_try_leave(actor, dir);
}

Matl Trap::matl() const
{
    return is_hidden_ ? mimic_feature_->matl() : data().matl_type;
}

//------------------------------------------------------------- TRAP IMPLEMENTATIONS
Trap_dart::Trap_dart(P pos, const Trap* const base_trap) :
    Mech_trap_impl              (pos, Trap_id::dart, base_trap),
    is_poisoned_                (map::dlvl >= MIN_DLVL_HARDER_TRAPS && rnd::one_in(3)),
    dart_origin_                (),
    is_dart_origin_destroyed_   (false) {}

Trap_placement_valid Trap_dart::on_place()
{
    auto offsets = dir_utils::cardinal_list;

    random_shuffle(begin(offsets), end(offsets));

    const int NR_STEPS_MIN = 2;
    const int NR_STEPS_MAX = FOV_STD_RADI_INT;

    auto trap_plament_valid = Trap_placement_valid::no;

    for (const P& d : offsets)
    {
        P p = pos_;

        for (int i = 0; i <= NR_STEPS_MAX; ++i)
        {
            p += d;

            const Rigid* const  rigid       = map::cells[p.x][p.y].rigid;
            const bool          IS_WALL     = rigid->id() == Feature_id::wall;
            const bool          IS_PASSABLE = rigid->is_projectile_passable();

            if (!IS_PASSABLE && (i < NR_STEPS_MIN || !IS_WALL))
            {
                //We are blocked too early - OR - blocked by a rigid feature other than a wall.
                //Give up on this direction.
                break;
            }

            if (i >= NR_STEPS_MIN && IS_WALL)
            {
                //This is a good origin!
                dart_origin_ = p;
                trap_plament_valid = Trap_placement_valid::yes;
                break;
            }
        }

        if (trap_plament_valid == Trap_placement_valid::yes)
        {
            //A valid origin has been found

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

void Trap_dart::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    assert(dart_origin_.x == pos_.x || dart_origin_.y == pos_.y);
    assert(dart_origin_ != pos_);

    const auto& origin_cell = map::cells[dart_origin_.x][dart_origin_.y];

    if (origin_cell.rigid->id() != Feature_id::wall)
    {
        is_dart_origin_destroyed_ = true; //NOTE: This is permanently set from now on
    }

    if (is_dart_origin_destroyed_)
    {
        return;
    }

    //Aim target is the wall on the other side of the map
    P aim_pos = dart_origin_;

    if (dart_origin_.x == pos_.x)
    {
        aim_pos.y = dart_origin_.y > pos_.y ? 0 : (MAP_H - 1);
    }
    else //Dart origin is on same vertial line as the trap
    {
        aim_pos.x = dart_origin_.x > pos_.x ? 0 : (MAP_W - 1);
    }

    if (origin_cell.is_seen_by_player)
    {
        const std::string name = origin_cell.rigid->name(Article::the);

        msg_log::add("A dart is launched from " + name + "!");
    }

    //Make a temporary dart weapon
    Wpn* wpn = nullptr;

    if (is_poisoned_)
    {
        wpn = static_cast<Wpn*>(item_factory::mk(Item_id::trap_dart_poison));
    }
    else //Not poisoned
    {
        wpn = static_cast<Wpn*>(item_factory::mk(Item_id::trap_dart));
    }

    //Fire!
    attack::ranged(nullptr, dart_origin_, aim_pos, *wpn);

    delete wpn;

    TRACE_FUNC_END_VERBOSE;
}

Trap_spear::Trap_spear(P pos, const Trap* const base_trap) :
    Mech_trap_impl              (pos, Trap_id::spear, base_trap),
    is_poisoned_                (map::dlvl >= MIN_DLVL_HARDER_TRAPS && rnd::one_in(4)),
    spear_origin_               (),
    is_spear_origin_destroyed_  (false) {}

Trap_placement_valid Trap_spear::on_place()
{
    auto offsets = dir_utils::cardinal_list;

    random_shuffle(begin(offsets), end(offsets));

    auto trap_plament_valid = Trap_placement_valid::no;

    for (const P& d : offsets)
    {
        const P p = pos_ + d;

        const Rigid* const  rigid       = map::cells[p.x][p.y].rigid;
        const bool          IS_WALL     = rigid->id() == Feature_id::wall;
        const bool          IS_PASSABLE = rigid->is_projectile_passable();

        if (IS_WALL && !IS_PASSABLE)
        {
            //This is a good origin!
            spear_origin_ = p;
            trap_plament_valid = Trap_placement_valid::yes;

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

void Trap_spear::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    assert(spear_origin_.x == pos_.x || spear_origin_.y == pos_.y);
    assert(spear_origin_ != pos_);

    const auto& origin_cell = map::cells[spear_origin_.x][spear_origin_.y];

    if (origin_cell.rigid->id() != Feature_id::wall)
    {
        is_spear_origin_destroyed_ = true; //NOTE: This is permanently set from now on
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

    //Is anyone standing on the trap now?
    Actor* const actor_on_trap = utils::actor_at_pos(pos_);

    if (actor_on_trap)
    {
        //Make a temporary spear weapon
        Wpn* wpn = nullptr;

        if (is_poisoned_)
        {
            wpn = static_cast<Wpn*>(item_factory::mk(Item_id::trap_spear_poison));
        }
        else //Not poisoned
        {
            wpn = static_cast<Wpn*>(item_factory::mk(Item_id::trap_spear));
        }

        //Attack!
        attack::melee(nullptr, spear_origin_, *actor_on_trap, *wpn);

        delete wpn;
    }

    TRACE_FUNC_BEGIN_VERBOSE;
}

void Trap_gas_confusion::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of gas is released from a vent in the floor!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        msg_log::more_prompt();
    }

    Snd snd("I hear a burst of gas.",
            Sfx_id::gas,
            Ignore_msg_if_origin_seen::yes,
            pos_,
            nullptr,
            Snd_vol::low,
            Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   Expl_type::apply_prop,
                   Expl_src::misc,
                   Emit_expl_snd::no,
                   -1,
                   new Prop_confused(Prop_turns::std),
                   &clr_magenta);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_gas_paralyzation::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of gas is released from a vent in the floor!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        msg_log::more_prompt();
    }

    Snd snd("I hear a burst of gas.",
            Sfx_id::gas,
            Ignore_msg_if_origin_seen::yes,
            pos_,
            nullptr,
            Snd_vol::low, Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   Expl_type::apply_prop,
                   Expl_src::misc,
                   Emit_expl_snd::no,
                   -1,
                   new Prop_paralyzed(Prop_turns::std),
                   &clr_magenta);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_gas_fear::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of gas is released from a vent in the floor!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        msg_log::more_prompt();
    }

    Snd snd("I hear a burst of gas.",
            Sfx_id::gas,
            Ignore_msg_if_origin_seen::yes,
            pos_,
            nullptr,
            Snd_vol::low,
            Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   Expl_type::apply_prop,
                   Expl_src::misc,
                   Emit_expl_snd::no,
                   -1,
                   new Prop_terrified(Prop_turns::std),
                   &clr_magenta);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_blinding_flash::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("There is an intense flash of light!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        msg_log::more_prompt();
    }

    explosion::run(pos_, Expl_type::apply_prop, Expl_src::misc, Emit_expl_snd::no,
                   -1, new Prop_blind(Prop_turns::std), &clr_yellow);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_teleport::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = utils::actor_at_pos(pos_);

    assert(actor_here);

    if (!actor_here)
    {
        //Should never happen
        return;
    }

    const bool          IS_PLAYER               = actor_here->is_player();
    const bool          CAN_SEE                 = actor_here->prop_handler().allow_see();
    const bool          CAN_PLAYER_SEE_ACTOR    = map::player->can_see_actor(*actor_here);
    const std::string   actor_name              = actor_here->name_the();
    const bool          IS_HIDDEN               = base_trap_->is_hidden();

    if (IS_PLAYER)
    {
        map::player->update_fov();

        if (CAN_SEE)
        {
            std::string msg = "A beam of light shoots out from";

            if (!IS_HIDDEN)
            {
                msg += " a curious shape on";
            }

            msg += " the floor!";

            msg_log::add(msg, clr_white, false, More_prompt_on_msg::yes);
        }
        else //Cannot see
        {
            msg_log::add("I feel a peculiar energy around me!", clr_white, false,
                         More_prompt_on_msg::yes);
        }
    }
    else //Is a monster
    {
        if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add("A beam shoots out under " + actor_name + ".");
        }
    }

    actor_here->teleport();

    TRACE_FUNC_END_VERBOSE;
}

void Trap_summon_mon::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = utils::actor_at_pos(pos_);

    assert(actor_here);

    if (!actor_here)
    {
        //Should never happen
        return;
    }

    const bool IS_PLAYER = actor_here->is_player();
    const bool IS_HIDDEN = base_trap_->is_hidden();

    TRACE_VERBOSE << "Is player: " << IS_PLAYER << std::endl;

    if (!IS_PLAYER)
    {
        TRACE_VERBOSE << "Not triggered by player" << std::endl;
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool CAN_SEE = actor_here->prop_handler().allow_see();
    TRACE_VERBOSE << "Actor can see: " << CAN_SEE << std::endl;

    const std::string actor_name = actor_here->name_the();
    TRACE_VERBOSE << "Actor name: " << actor_name << std::endl;

    map::player->incr_shock(5, Shock_src::misc);
    map::player->update_fov();

    if (CAN_SEE)
    {
        std::string msg = "A beam of light shoots out from";

        if (!IS_HIDDEN)
        {
            msg += " a curious shape on";
        }

        msg += " the floor!";

        msg_log::add(msg, clr_white, false, More_prompt_on_msg::yes);
    }
    else //Cannot see
    {
        msg_log::add("I feel a peculiar energy around me!", clr_white, false,
                     More_prompt_on_msg::yes);
    }

    TRACE << "Finding summon candidates" << std::endl;
    std::vector<Actor_id> summon_bucket;

    for (int i = 0; i < int(Actor_id::END); ++i)
    {
        const Actor_data_t& data = actor_data::data[i];

        if (data.can_be_summoned && data.spawn_min_dlvl <= map::dlvl + 3)
        {
            summon_bucket.push_back(Actor_id(i));
        }
    }

    const size_t NR_ELEMENTS = summon_bucket.size();
    TRACE << "Nr candidates: " << NR_ELEMENTS << std::endl;

    if (NR_ELEMENTS == 0)
    {
        TRACE << "No eligible candidates found" << std::endl;
    }
    else //Eligible candidates found
    {
        const int       ELEMENT         = rnd::range(0, NR_ELEMENTS - 1);
        const Actor_id  id_to_summon    = summon_bucket[ELEMENT];
        TRACE << "Actor id: " << int(id_to_summon) << std::endl;

        actor_factory::summon(pos_, {1, id_to_summon}, Make_mon_aware::yes);
        TRACE << "Monster was summoned" << std::endl;
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap_spi_drain::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = utils::actor_at_pos(pos_);

    assert(actor_here);

    if (!actor_here)
    {
        //Should never happen
        return;
    }

    const bool IS_PLAYER = actor_here->is_player();
    const bool IS_HIDDEN = base_trap_->is_hidden();

    TRACE_VERBOSE << "Is player: " << IS_PLAYER << std::endl;

    if (!IS_PLAYER)
    {
        TRACE_VERBOSE << "Not triggered by player" << std::endl;
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool CAN_SEE = actor_here->prop_handler().allow_see();
    TRACE_VERBOSE << "Actor can see: " << CAN_SEE << std::endl;

    const std::string actor_name = actor_here->name_the();
    TRACE_VERBOSE << "Actor name: " << actor_name << std::endl;

    if (CAN_SEE)
    {
        std::string msg = "A beam of light shoots out from";

        if (!IS_HIDDEN)
        {
            msg += " a curious shape on";
        }

        msg += " the floor!";

        msg_log::add(msg,
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);
    }
    else //Cannot see
    {
        msg_log::add("I feel a peculiar energy around me!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);
    }

    TRACE << "Draining player spirit" << std::endl;

    //Allow draining more than starting spirit if this is DLVL depth after harder traps
    const bool ALLOW_OVER_START_SPI = map::dlvl >= MIN_DLVL_HARDER_TRAPS;
    const int D                     = SPI_PER_LVL * 2;
    const int MAX                   = PLAYER_START_SPI + (ALLOW_OVER_START_SPI ? D : -D);
    const int SPI_DRAINED           = rnd::range(1, MAX);

    map::player->hit_spi(SPI_DRAINED);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_smoke::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A burst of smoke is released from a vent in the floor!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        msg_log::more_prompt();
    }

    Snd snd("I hear a burst of gas.",
            Sfx_id::gas,
            Ignore_msg_if_origin_seen::yes,
            pos_,
            nullptr,
            Snd_vol::low,
            Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run_smoke_explosion_at(pos_);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_fire::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("Flames burst out from a vent in the floor!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        msg_log::more_prompt();
    }

    Snd snd("I hear a burst of flames.",
            Sfx_id::END,
            Ignore_msg_if_origin_seen::yes,
            pos_,
            nullptr,
            Snd_vol::low,
            Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run(pos_,
                   Expl_type::apply_prop,
                   Expl_src::misc,
                   Emit_expl_snd::no,
                   -1,
                   new Prop_burning(Prop_turns::std));

    TRACE_FUNC_END_VERBOSE;
}

void Trap_alarm::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("An alarm sounds!",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);
    }

    Snd snd("I hear an alarm sounding!",
            Sfx_id::END,
            Ignore_msg_if_origin_seen::yes,
            pos_,
            nullptr,
            Snd_vol::high,
            Alerts_mon::yes);

    snd_emit::run(snd);

    TRACE_FUNC_END_VERBOSE;
}

void Trap_web::trigger()
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Actor* const actor_here = utils::actor_at_pos(pos_);

    assert(actor_here);

    if (!actor_here)
    {
        //Should never happen
        return;
    }

    is_holding_actor_ = true;

    const bool          IS_PLAYER               = actor_here->is_player();
    const bool          CAN_SEE                 = actor_here->prop_handler().allow_see();
    const bool          CAN_PLAYER_SEE_ACTOR    = map::player->can_see_actor(*actor_here);
    const std::string   actor_name              = actor_here->name_the();

    if (IS_PLAYER)
    {
        TRACE << "Checking if player has machete" << std::endl;
        const auto& player_inv      = map::player->inv();
        Item* const item_wielded    = player_inv.item_in_slot(Slot_id::wpn);
        const bool  HAS_MACHETE     = item_wielded && item_wielded->data().id == Item_id::machete;

        if (HAS_MACHETE)
        {
            if (CAN_SEE)
            {
                msg_log::add("I cut down a spider web with my machete.");
            }
            else
            {
                msg_log::add("I cut down a sticky mass of threads with my machete.");
            }

            TRACE << "Destroyed by Machete, placing floor" << std::endl;
            map::put(new Floor(pos_));
        }
        else //Not wielding machete
        {
            if (CAN_SEE)
            {
                msg_log::add("I am entangled in a spider web!", clr_white, false,
                             More_prompt_on_msg::yes);
            }
            else //Cannot see
            {
                msg_log::add("I am entangled in a sticky mass of threads!", clr_white,
                             false, More_prompt_on_msg::yes);
            }
        }
    }
    else //Is a monster
    {
        if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add(actor_name + " is entangled in a huge spider web!");
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

Dir Trap_web::actor_try_leave(Actor& actor, const Dir dir)
{
    if (!is_holding_actor_)
    {
        TRACE_VERBOSE << "Not holding actor, returning current direction" << std::endl;
        return dir;
    }

    TRACE_VERBOSE << "Is holding actor" << std::endl;

    const bool          PLAYER_CAN_SEE          = map::player->prop_handler().allow_see();
    const bool          PLAYER_CAN_SEE_ACTOR    = map::player->can_see_actor(actor);
    const std::string   actor_name              = actor.name_the();

    TRACE_VERBOSE << "Name of actor held: " << actor_name << std::endl;

    //TODO: reimplement something affecting chance of success?

    if (rnd::one_in(4))
    {
        TRACE_VERBOSE << "Actor succeeded to break free" << std::endl;

        is_holding_actor_ = false;

        if (actor.is_player())
        {
            msg_log::add("I break free.");
        }
        else //Is monster
        {
            if (PLAYER_CAN_SEE_ACTOR)
            {
                msg_log::add(actor_name + " breaks free from a spiderweb.");
            }
        }

        if (rnd::one_in(2))
        {
            if (
                (actor.is_player()  && PLAYER_CAN_SEE) ||
                (!actor.is_player() && PLAYER_CAN_SEE_ACTOR))
            {
                msg_log::add("The web is destroyed.");
            }

            TRACE_VERBOSE << "Web destroyed, placing floor and returning center direction"
                          << std::endl;
            map::put(new Floor(pos_));
            return Dir::center;
        }
    }
    else //Failed to break free
    {
        if (actor.is_player())
        {
            msg_log::add("I struggle to break free.");
        }
        else if (PLAYER_CAN_SEE_ACTOR)
        {
            msg_log::add(actor_name + " struggles to break free.");
        }
    }

    return Dir::center;
}
