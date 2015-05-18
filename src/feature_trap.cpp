#include "feature_trap.hpp"

#include <algorithm>
#include <assert.h>

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

using namespace std;

//------------------------------------------------------------- TRAP
Trap::Trap(const Pos& feature_pos, const Rigid* const mimic_feature, Trap_id type) :
    Rigid(feature_pos),
    mimic_feature_(mimic_feature),
    is_hidden_(true),
    specific_trap_(nullptr)
{
    assert(type != Trap_id::END);

    assert(map::cells[feature_pos.x][feature_pos.y].rigid->can_have_rigid());

    if (type == Trap_id::any)
    {
        set_specific_trap_from_id(Trap_id(rnd::range(0, int(Trap_id::END) - 1)));
    }
    else
    {
        set_specific_trap_from_id(type);
    }

    assert(specific_trap_);
}

Trap::~Trap()
{
    assert(specific_trap_);
    delete specific_trap_;
    delete mimic_feature_;
}

void Trap::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                  Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Trap::set_specific_trap_from_id(const Trap_id trap_id)
{
    switch (trap_id)
    {
    case Trap_id::dart:
        specific_trap_ = new Trap_dart(pos_);
        break;

    case Trap_id::spear:
        specific_trap_ = new Trap_spear(pos_);
        break;

    case Trap_id::gas_confusion:
        specific_trap_ = new Trap_gas_confusion(pos_);
        break;

    case Trap_id::gas_paralyze:
        specific_trap_ = new Trap_gas_paralyzation(pos_);
        break;

    case Trap_id::gas_fear:
        specific_trap_ = new Trap_gas_fear(pos_);
        break;

    case Trap_id::blinding:
        specific_trap_ = new Trap_blinding_flash(pos_);
        break;

    case Trap_id::teleport:
        specific_trap_ = new Trap_teleport(pos_);
        break;

    case Trap_id::summon:
        specific_trap_ = new Trap_summon_mon(pos_);
        break;

    case Trap_id::smoke:
        specific_trap_ = new Trap_smoke(pos_);
        break;

    case Trap_id::alarm:
        specific_trap_ = new Trap_alarm(pos_);
        break;

    case Trap_id::web:
        specific_trap_ = new Trap_web(pos_); break;

    default:
        specific_trap_ = nullptr;
        break;
    }
}

Trap_id Trap::trap_type() const {return specific_trap_->trap_type_;}

bool Trap::is_magical() const {return specific_trap_->is_magical();}

void Trap::trigger_on_purpose(Actor& actor_triggering)
{
    const Ability_roll_result DODGE_RESULT = fail_small;
    specific_trap_->trigger(actor_triggering, DODGE_RESULT);
}

void Trap::bump(Actor& actor_bumping)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const Actor_data_t& d = actor_bumping.data();

    bool props[size_t(Prop_id::END)];
    actor_bumping.prop_handler().prop_ids(props);

    if (!props[int(Prop_id::ethereal)] && !props[int(Prop_id::flying)])
    {
        const bool    IS_PLAYER             = actor_bumping.is_player();
        const bool    ACTOR_CAN_SEE         = actor_bumping.prop_handler().allow_see();
        Ability_vals&  abilities             = actor_bumping.data().ability_vals;
        const int     DODGE_SKILL           = abilities.val(Ability_id::dodge_trap, true,
                                              actor_bumping);
        const int     BASE_CHANCE_TO_AVOID  = 30;

        const string  trap_name              = specific_trap_->title();

        if (IS_PLAYER)
        {
            TRACE_VERBOSE << "Player bumping" << endl;
            int chance_to_avoid = BASE_CHANCE_TO_AVOID + DODGE_SKILL;

            if (is_hidden_)
            {
                chance_to_avoid = max(10, chance_to_avoid / 2);
            }

            const Ability_roll_result result = ability_roll::roll(chance_to_avoid);

            if (result >= success_small)
            {
                if (!is_hidden_ && ACTOR_CAN_SEE)
                {
                    map::player->update_fov();
                    render::draw_map_and_interface();
                    msg_log::add("I avoid a " + trap_name + ".", clr_msg_good, false, true);
                }
            }
            else //Failed to avoid
            {
                map::player->update_fov();
                render::draw_map_and_interface();
                trigger_trap(&actor_bumping);
            }
        }
        else //Is a monster
        {
            TRACE_VERBOSE << "Monster bumping trap" << endl;

            if (d.actor_size == Actor_size::humanoid && !d.is_spider)
            {
                TRACE_VERBOSE << "Humanoid monster bumping" << endl;
                Mon* const mon = static_cast<Mon*>(&actor_bumping);

                if (mon->aware_counter_ > 0 && !mon->is_stealth_)
                {
                    TRACE_VERBOSE << "Monster eligible for triggering trap" << endl;

                    const bool IS_ACTOR_SEEN_BY_PLAYER =
                        map::player->can_see_actor(actor_bumping, nullptr);

                    const int CHANCE_TO_AVOID = BASE_CHANCE_TO_AVOID + DODGE_SKILL;
                    const Ability_roll_result result = ability_roll::roll(CHANCE_TO_AVOID);

                    if (result >= success_small)
                    {
                        if (!is_hidden_ && IS_ACTOR_SEEN_BY_PLAYER)
                        {
                            const string actor_name = actor_bumping.name_the();
                            msg_log::add(actor_name + " avoids a " + trap_name + ".");
                        }
                    }
                    else
                    {
                        trigger_trap(&actor_bumping);
                    }
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
        Item* item = map::player->inv().item_in_slot(Slot_id::wielded);

        if (item)
        {
            is_auto_succeed = item->id() == Item_id::machete;
        }
    }

    const bool IS_OCCULTIST   = player_bon::bg() == Bg::occultist;

    if (is_magical() && !IS_OCCULTIST)
    {
        msg_log::add("I do not know how to dispel magic traps.");
        return;
    }

    bool props[size_t(Prop_id::END)];
    map::player->prop_handler().prop_ids(props);

    const bool IS_BLESSED = props[int(Prop_id::blessed)];
    const bool IS_CURSED  = props[int(Prop_id::cursed)];

    int       disarm_numerator     = 6;
    const int DISARM_DENOMINATOR  = 10;

    if (IS_BLESSED)  disarm_numerator += 3;

    if (IS_CURSED)   disarm_numerator -= 3;

    set_constr_in_range(1, disarm_numerator, DISARM_DENOMINATOR - 1);

    const bool IS_DISARMED = is_auto_succeed ||
                             rnd::fraction(disarm_numerator, DISARM_DENOMINATOR);

    if (IS_DISARMED)
    {
        msg_log::add(specific_trap_->disarm_msg());
    }
    else //Not disarmed
    {
        msg_log::add(specific_trap_->disarm_fail_msg());

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
        else
        {
            map::put(new Rubble_low(pos_));
        }
    }
}

Did_trigger_trap Trap::trigger_trap(Actor* const actor)
{
    TRACE_FUNC_BEGIN;

    assert(actor);

    TRACE << "Specific trap is " << specific_trap_->title() << endl;

    const Actor_data_t& d = actor->data();

    TRACE << "Actor triggering is " << d.name_a << endl;

    const int DODGE_SKILL = d.ability_vals.val(Ability_id::dodge_trap, true, *actor);

    TRACE << "Actor dodge skill is " << DODGE_SKILL << endl;

    if (actor->is_player())
    {
        TRACE_VERBOSE << "Player triggering trap" << endl;
        const Ability_roll_result DODGE_RESULT = ability_roll::roll(DODGE_SKILL);
        reveal(false);
        TRACE_VERBOSE << "Calling trigger" << endl;
        specific_trap_->trigger(*actor, DODGE_RESULT);
    }
    else
    {
        TRACE_VERBOSE << "Monster triggering trap" << endl;
        const bool IS_ACTOR_SEEN_BY_PLAYER  = map::player->can_see_actor(*actor, nullptr);
        const Ability_roll_result dodge_result = ability_roll::roll(DODGE_SKILL);

        if (IS_ACTOR_SEEN_BY_PLAYER)
        {
            reveal(false);
        }

        TRACE_VERBOSE << "Calling trigger" << endl;
        specific_trap_->trigger(*actor, dodge_result);
    }

    TRACE_FUNC_END;
    return Did_trigger_trap::yes;
}

void Trap::reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    is_hidden_ = false;

    //Destroy any corpse on the trap
    for (Actor* actor : game_time::actors_)
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
            const string trap_name = specific_trap_->title();
            msg_log::add("I spot a " + trap_name + ".", clr_msg_note, false, true);
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap::player_try_spot_hidden()
{
    if (is_hidden_)
    {
        const auto& abilities = map::player->data().ability_vals;

        const int SKILL =
            abilities.val(Ability_id::searching, true, *(map::player));

        if (ability_roll::roll(SKILL) >= success_small)
        {
            reveal(true);
        }
    }
}

string Trap::name(const Article article) const
{
    if (is_hidden_)
    {
        return mimic_feature_->name(article);
    }
    else
    {
        return (article == Article::a ? "a " : "the ") + specific_trap_->title();
    }
}

Clr Trap::clr_() const
{
    return is_hidden_ ? mimic_feature_->clr() : specific_trap_->clr();
}

Clr Trap::clr_bg_() const
{
    const auto* const item = map::cells[pos_.x][pos_.y].item;

    if (is_hidden_ || !item)
    {
        return clr_black;
    }
    else
    {
        return specific_trap_->clr();
    }
}

char Trap::glyph() const
{
    return is_hidden_ ? mimic_feature_->glyph() : specific_trap_->glyph();
}

Tile_id Trap::tile() const
{
    return is_hidden_ ? mimic_feature_->tile() : specific_trap_->tile();
}

Dir Trap::actor_try_leave(Actor& actor, const Dir dir)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    TRACE_FUNC_END_VERBOSE;
    return specific_trap_->actor_try_leave(actor, dir);
}

Matl Trap::matl() const
{
    return is_hidden_ ? mimic_feature_->matl() : data().matl_type;
}

//------------------------------------------------------------- SPECIFIC TRAPS
Trap_dart::Trap_dart(Pos pos) :
    Specific_trap_base(pos, Trap_id::dart), is_poisoned(false)
{
    is_poisoned = map::dlvl >= MIN_DLVL_HARDER_TRAPS && rnd::coin_toss();
}

void Trap_dart::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    const bool IS_PLAYER = &actor == map::player;
    const bool CAN_SEE = actor.prop_handler().allow_see();
    const bool CAN_PLAYER_SEE_ACTOR = map::player->can_see_actor(actor, nullptr);
    const string actor_name = actor.name_the();

    //Dodge?
    if (dodge_result >= success_small)
    {
        if (IS_PLAYER)
        {
            if (CAN_SEE)
            {
                msg_log::add("I dodge a dart!", clr_msg_good, false, true);
            }
            else
            {
                msg_log::add("I feel a mechanism trigger and quickly leap aside!",
                             clr_msg_good, false, true);
            }
        }
        else if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add(actor_name +  " dodges a dart!");
        }
    }
    else //Dodge failed
    {
        //Trap misses?
        if (rnd::one_in(4))
        {
            if (IS_PLAYER)
            {
                if (CAN_SEE)
                {
                    msg_log::add("A dart barely misses me!", clr_msg_good, false, true);
                }
                else
                {
                    msg_log::add(
                        "A mechanism triggers, I hear something barely missing me!",
                        clr_msg_good, false, true);
                }
            }
            else if (CAN_PLAYER_SEE_ACTOR)
            {
                msg_log::add("A dart barely misses " + actor_name + "!");
            }
        }
        else //Dodge failed, and trap hits actor
        {
            if (IS_PLAYER)
            {
                if (CAN_SEE)
                {
                    msg_log::add("I am hit by a dart!", clr_msg_bad, false, true);
                }
                else
                {
                    msg_log::add(
                        "A mechanism triggers, I feel a needle piercing my skin!",
                        clr_msg_bad, false, true);
                }
            }
            else if (CAN_PLAYER_SEE_ACTOR)
            {
                msg_log::add(actor_name + " is hit by a dart!", clr_msg_good);
            }

            const int DMG = rnd::dice(1, 8);
            actor.hit(DMG, Dmg_type::physical);

            if (actor.is_alive() && is_poisoned)
            {
                actor.prop_handler().try_apply_prop(new Prop_poisoned(Prop_turns::std));
            }
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

Trap_spear::Trap_spear(Pos pos) :
    Specific_trap_base(pos, Trap_id::spear), is_poisoned(false)
{
    is_poisoned = map::dlvl >= MIN_DLVL_HARDER_TRAPS && rnd::one_in(4);
}

void Trap_spear::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const bool IS_PLAYER = &actor == map::player;
    const bool CAN_SEE = actor.prop_handler().allow_see();
    const bool CAN_PLAYER_SEE_ACTOR = map::player->can_see_actor(actor, nullptr);
    const string actor_name = actor.name_the();

    //Dodge?
    if (dodge_result >= success_small)
    {
        if (IS_PLAYER)
        {
            if (CAN_SEE)
            {
                msg_log::add("I dodge a spear!", clr_msg_good, false, true);
            }
            else
            {
                msg_log::add("I feel a mechanism trigger and quickly leap aside!",
                             clr_msg_good, false, true);
            }
        }
        else if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add(actor_name +  " dodges a spear!");
        }
    }
    else //Dodge failed
    {
        //Trap misses?
        if (rnd::one_in(4))
        {
            if (IS_PLAYER)
            {
                if (CAN_SEE)
                {
                    msg_log::add("A spear barely misses me!", clr_msg_good, false, true);
                }
                else
                {
                    msg_log::add("A mechanism triggers, I hear a *swoosh*!",
                                 clr_msg_good, false, true);
                }
            }
            else if (CAN_PLAYER_SEE_ACTOR)
            {
                msg_log::add("A spear barely misses " + actor_name + "!");
            }
        }
        else //Dodge failed and trap hits
        {
            if (IS_PLAYER)
            {
                if (CAN_SEE)
                {
                    msg_log::add("I am hit by a spear!", clr_msg_bad, false, true);
                }
                else
                {
                    msg_log::add("A mechanism triggers, something sharp pierces my skin!",
                                 clr_msg_bad, false, true);
                }
            }
            else if (CAN_PLAYER_SEE_ACTOR)
            {
                msg_log::add(actor_name + " is hit by a spear!", clr_msg_good);
            }

            const int DMG = rnd::dice(2, 6);
            actor.hit(DMG, Dmg_type::physical);

            if (actor.is_alive() && is_poisoned)
            {
                actor.prop_handler().try_apply_prop(new Prop_poisoned(Prop_turns::std));
            }
        }
    }

    TRACE_FUNC_BEGIN_VERBOSE;
}

void Trap_gas_confusion::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    (void)dodge_result;

    const bool IS_PLAYER = &actor == map::player;
    const bool CAN_SEE = actor.prop_handler().allow_see();
    const bool CAN_PLAYER_SEE_ACTOR = map::player->can_see_actor(actor, nullptr);
    const string actor_name = actor.name_the();

    if (IS_PLAYER)
    {
        if (CAN_SEE)
        {
            msg_log::add("I am hit by a burst of gas!", clr_white, false, true);
        }
        else
        {
            msg_log::add("A mechanism triggers, I am hit by a burst of gas!", clr_white,
                         false, true);
        }
    }
    else if (CAN_PLAYER_SEE_ACTOR)
    {
        msg_log::add(actor_name + " is hit by a burst of gas!");
    }

    auto explosion_clr = clr();

    explosion::run_explosion_at(pos_, Expl_type::apply_prop, Expl_src::misc, 0, Sfx_id::END,
                                new Prop_confused(Prop_turns::std), &explosion_clr);
    TRACE_FUNC_END_VERBOSE;
}

void Trap_gas_paralyzation::trigger(Actor& actor,  const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    (void)dodge_result;

    const bool IS_PLAYER = &actor == map::player;
    const bool CAN_SEE = actor.prop_handler().allow_see();
    const bool CAN_PLAYER_SEE_ACTOR = map::player->can_see_actor(actor, nullptr);
    const string actor_name = actor.name_the();

    if (IS_PLAYER)
    {
        if (CAN_SEE)
        {
            msg_log::add("I am hit by a burst of gas!", clr_white, false, true);
        }
        else
        {
            msg_log::add("A mechanism triggers, I am hit by a burst of gas!", clr_white,
                         false, true);
        }
    }
    else if (CAN_PLAYER_SEE_ACTOR)
    {
        msg_log::add(actor_name + " is hit by a burst of gas!");
    }

    Clr explosion_clr = clr();
    explosion::run_explosion_at(pos_, Expl_type::apply_prop, Expl_src::misc, 0, Sfx_id::END,
                                new Prop_paralyzed(Prop_turns::std), &explosion_clr) ;
    TRACE_FUNC_END_VERBOSE;
}

void Trap_gas_fear::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    (void)dodge_result;

    const bool IS_PLAYER = &actor == map::player;
    const bool CAN_SEE = actor.prop_handler().allow_see();
    const bool CAN_PLAYER_SEE_ACTOR = map::player->can_see_actor(actor, nullptr);
    const string actor_name = actor.name_the();

    if (IS_PLAYER)
    {
        if (CAN_SEE)
        {
            msg_log::add("I am hit by a burst of gas!", clr_white, false, true);
        }
        else
        {
            msg_log::add("A mechanism triggers, I am hit by a burst of gas!", clr_white,
                         false, true);
        }
    }
    else if (CAN_PLAYER_SEE_ACTOR)
    {
        msg_log::add(actor_name + " is hit by a burst of gas!");
    }

    Clr explosion_clr = clr();
    explosion::run_explosion_at(pos_, Expl_type::apply_prop, Expl_src::misc, 0, Sfx_id::END,
                                new Prop_terrified(Prop_turns::std), &explosion_clr);
    TRACE_FUNC_END_VERBOSE;
}

void Trap_blinding_flash::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    const bool IS_PLAYER = &actor == map::player;
    const bool CAN_SEE = actor.prop_handler().allow_see();
    const bool CAN_PLAYER_SEE_ACTOR = map::player->can_see_actor(actor, nullptr);
    const string actor_name = actor.name_the();

    //Dodge?
    if (dodge_result >= success_small)
    {
        if (IS_PLAYER)
        {
            if (CAN_SEE)
            {
                msg_log::add("I cover my eyes just in time to avoid an intense flash!",
                             clr_msg_good, false, true);
            }
            else
            {
                msg_log::add("I feel a mechanism trigger!", clr_msg_good, false, true);
            }
        }
        else if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add(actor_name + " covers from a blinding flash!");
        }
    }
    else //Dodge failed
    {
        if (IS_PLAYER)
        {
            if (CAN_SEE)
            {
                msg_log::add("A sharp flash of light pierces my eyes!", clr_white, false,
                             true);
                actor.prop_handler().try_apply_prop(new Prop_blind(Prop_turns::std));
            }
            else
            {
                msg_log::add("I feel a mechanism trigger!", clr_white, false, true);
            }
        }
        else if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add(actor_name + " is hit by a flash of blinding light!");
            actor.prop_handler().try_apply_prop(new Prop_blind(Prop_turns::std));
        }
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap_teleport::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    (void)dodge_result;

    const bool    IS_PLAYER             = &actor == map::player;
    const bool    CAN_SEE               = actor.prop_handler().allow_see();
    const bool    CAN_PLAYER_SEE_ACTOR  = map::player->can_see_actor(actor, nullptr);
    const string  actor_name             = actor.name_the();

    if (IS_PLAYER)
    {
        map::player->update_fov();

        if (CAN_SEE)
        {
            msg_log::add("A beam of light shoots out from a curious shape on the floor!",
                         clr_white, false, true);
        }
        else
        {
            msg_log::add("I feel a peculiar energy around me!", clr_white, false, true);
        }
    }
    else
    {
        if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add("A beam shoots out under " + actor_name + ".");
        }
    }

    actor.teleport();
    TRACE_FUNC_END_VERBOSE;
}

void Trap_summon_mon::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    (void)dodge_result;

    const bool IS_PLAYER = &actor == map::player;
    TRACE_VERBOSE << "Is player: " << IS_PLAYER << endl;

    if (!IS_PLAYER)
    {
        TRACE_VERBOSE << "Not triggered by player" << endl;
        TRACE_FUNC_END_VERBOSE;
        return;
    }

    const bool CAN_SEE = actor.prop_handler().allow_see();
    TRACE_VERBOSE << "Actor can see: " << CAN_SEE << endl;

    const string actor_name = actor.name_the();
    TRACE_VERBOSE << "Actor name: " << actor_name << endl;

    map::player->incr_shock(5, Shock_src::misc);
    map::player->update_fov();

    if (CAN_SEE)
    {
        msg_log::add("A beam of light shoots out from a curious shape on the floor!",
                     clr_white, false, true);
    }
    else
    {
        msg_log::add("I feel a peculiar energy around me!", clr_white, false, true);
    }

    TRACE << "Finding summon candidates" << endl;
    vector<Actor_id> summon_bucket;

    for (int i = 0; i < int(Actor_id::END); ++i)
    {
        const Actor_data_t& data = actor_data::data[i];

        if (data.can_be_summoned && data.spawn_min_dLVL <= map::dlvl + 3)
        {
            summon_bucket.push_back(Actor_id(i));
        }
    }

    const int NR_ELEMENTS = summon_bucket.size();
    TRACE << "Nr candidates: " << NR_ELEMENTS << endl;

    if (NR_ELEMENTS == 0)
    {
        TRACE << "No eligible candidates found" << endl;
    }
    else
    {
        const int ELEMENT = rnd::range(0, NR_ELEMENTS - 1);
        const Actor_id actor_id_to_summon = summon_bucket[ELEMENT];
        TRACE << "Actor id: " << int(actor_id_to_summon) << endl;

        actor_factory::summon(pos_, vector<Actor_id>(1, actor_id_to_summon), true);
        TRACE << "Monster was summoned" << endl;
    }

    TRACE_FUNC_END_VERBOSE;
}

void Trap_smoke::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    (void)dodge_result;

    const bool    IS_PLAYER             = &actor == map::player;
    const bool    CAN_SEE               = actor.prop_handler().allow_see();
    const bool    CAN_PLAYER_SEE_ACTOR  = map::player->can_see_actor(actor, nullptr);
    const string  actor_name             = actor.name_the();

    if (IS_PLAYER)
    {
        if (CAN_SEE)
        {
            msg_log::add("Suddenly the air is thick with smoke!", clr_white, false, true);
        }
        else
        {
            msg_log::add("A mechanism triggers, the air is thick with smoke!", clr_white,
                         false, true);
        }
    }
    else
    {
        if (CAN_PLAYER_SEE_ACTOR)
        {
            msg_log::add("Suddenly the air around " + actor_name + " is thick with smoke!");
        }
    }

    explosion::run_smoke_explosion_at(pos_);
    TRACE_FUNC_END_VERBOSE;
}

void Trap_alarm::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;
    (void)dodge_result;

    Ignore_msg_if_origin_seen msg_ignore_rule;

    if (map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("An alarm sounds!", clr_white, false, true);
        msg_ignore_rule = Ignore_msg_if_origin_seen::yes;
    }
    else
    {
        msg_ignore_rule = Ignore_msg_if_origin_seen::no;
    }

    Snd snd("I hear an alarm sounding!", Sfx_id::END, msg_ignore_rule, pos_,
            &actor, Snd_vol::high, Alerts_mon::yes);
    snd_emit::emit_snd(snd);
    TRACE_FUNC_END_VERBOSE;
}

void Trap_web::trigger(Actor& actor, const Ability_roll_result dodge_result)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    (void)dodge_result;

    is_holding_actor_ = true;

    const bool    IS_PLAYER             = &actor == map::player;
    const bool    CAN_SEE               = actor.prop_handler().allow_see();
    const bool    CAN_PLAYER_SEE_ACTOR  = map::player->can_see_actor(actor, nullptr);
    const string  actor_name             = actor.name_the();

    if (IS_PLAYER)
    {
        TRACE << "Checking if player has machete" << endl;
        Inventory& player_inv = map::player->inv();
        Item* item_wielded = player_inv.item_in_slot(Slot_id::wielded);
        bool has_machete = false;

        if (item_wielded) {has_machete = item_wielded->data().id == Item_id::machete;}

        if (has_machete)
        {
            if (CAN_SEE)
            {
                msg_log::add("I cut down a spider web with my machete.");
            }
            else
            {
                msg_log::add("I cut down a sticky mass of threads with my machete.");
            }

            TRACE << "Destroyed by Machete, placing floor" << endl;
            map::put(new Floor(pos_));
        }
        else //Does not wield machete
        {
            if (CAN_SEE)
            {
                msg_log::add("I am entangled in a spider web!", clr_white, false, true);
            }
            else
            {
                msg_log::add("I am entangled in a sticky mass of threads!", clr_white,
                             false, true);
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
        TRACE_VERBOSE << "Not holding actor, returning current direction" << endl;
        return dir;
    }

    TRACE_VERBOSE << "Is holding actor" << endl;

    const bool    IS_PLAYER             = &actor == map::player;
    const bool    PLAYER_CAN_SEE        = map::player->prop_handler().allow_see();
    const bool    PLAYER_CAN_SEE_ACTOR  = map::player->can_see_actor(actor, nullptr);
    const string  actor_name             = actor.name_the();

    TRACE_VERBOSE << "Name of actor held: " << actor_name << endl;

    //TODO: reimplement something affecting chance of success?

    if (rnd::one_in(4))
    {
        TRACE_VERBOSE << "Actor succeeded to break free" << endl;

        is_holding_actor_ = false;

        if (IS_PLAYER)
        {
            msg_log::add("I break free.");
        }
        else
        {
            if (PLAYER_CAN_SEE_ACTOR)
            {
                msg_log::add(actor_name + " breaks free from a spiderweb.");
            }
        }

        if (rnd::one_in(2))
        {
            if ((IS_PLAYER && PLAYER_CAN_SEE) || (!IS_PLAYER && PLAYER_CAN_SEE_ACTOR))
            {
                msg_log::add("The web is destroyed.");
            }

            TRACE_VERBOSE << "Web destroyed, placing floor and returning center direction"
                          << endl;
            map::put(new Floor(pos_));
            return Dir::center;
        }
    }
    else //Failed to break free
    {
        if (IS_PLAYER)
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
