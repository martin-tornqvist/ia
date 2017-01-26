#include "actor.hpp"

#include "init.hpp"

#include "io.hpp"
#include "game_time.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "fov.hpp"
#include "msg_log.hpp"
#include "feature_trap.hpp"
#include "drop.hpp"
#include "explosion.hpp"
#include "game.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "item.hpp"
#include "marker.hpp"
#include "look.hpp"

Actor::Actor() :
    pos             (),
    delay_          (0),
    state_          (ActorState::alive),
    hp_             (-1),
    hp_max_         (-1),
    spi_            (-1),
    spi_max_        (-1),
    lair_pos_       (),
    prop_handler_   (nullptr),
    data_           (nullptr),
    inv_            (nullptr) {}

Actor::~Actor()
{
    // Free all items owning actors
    for (Item* item : inv_->backpack_)
    {
        item->clear_actor_carrying();
    }

    for (auto& slot : inv_->slots_)
    {
        if (slot.item)
        {
            slot.item->clear_actor_carrying();
        }
    }

    delete inv_;
    delete prop_handler_;
}

bool Actor::has_prop(const PropId id) const
{
    return prop_handler_->has_prop(id);
}

int Actor::ability(const AbilityId id, const bool is_affected_by_props) const
{
    return data_->ability_vals.val(id, is_affected_by_props, *this);
}

ActionResult Actor::roll_sneak(const Actor& other) const
{
    const int sneak_skill = ability(AbilityId::stealth, true);

    const int search_mod =
        other.is_player() ?
        other.ability(AbilityId::searching, true) :
        0;

    const int dist = king_dist(pos, other.pos);

    // Distance  Sneak bonus
    // ----------------------
    // 1         -20
    // 2           0
    // 3          20
    // 4          40
    // 5          60
    // 6          80
    const int dist_mod = std::min((dist - 2) * 20, 80);

    const Cell& cell = map::cells[pos.x][pos.y];

    const int lgt_mod =
        cell.is_lit ?
        -40 : 0;

    const int drk_mod =
        (cell.is_dark && ! cell.is_lit) ?
        40 : 0;

    const int sneak_tot =
        sneak_skill
        - search_mod
        + dist_mod
        + lgt_mod
        + drk_mod;

    const auto result = ability_roll::roll(sneak_tot, this);

    return result;
}

int Actor::hp_max(const bool with_modifiers) const
{
    int result = hp_max_;

    if (with_modifiers)
    {
        result = prop_handler_->affect_max_hp(result);

        result = std::max(1, result);
    }

    return result;
}

int Actor::speed_pct() const
{
    int ret = data_->speed_pct;

    // "Slowed" gives speed penalty
    if (prop_handler_->has_prop(PropId::slowed))
    {
        ret -= 50;
    }

    // "Hasted" gives speed bonus
    if (prop_handler_->has_prop(PropId::hasted))
    {
        ret += 100;
    }

    // "Frenzied" gives speed bonus
    if (prop_handler_->has_prop(PropId::frenzied))
    {
        ret += 100;
    }

    if (is_player())
    {
        if (player_bon::traits[(size_t)Trait::dexterous])
        {
            ret += 10;
        }

        if (player_bon::traits[(size_t)Trait::lithe])
        {
            ret += 10;
        }
    }

    const int min_speed = 20;

    ret = std::max(min_speed, ret);

    return ret;
}

void Actor::place(const P& pos_, ActorDataT& actor_data)
{
    pos = pos_;

    data_ = &actor_data;

    state_ = ActorState::alive;

    hp_ = hp_max_ = data_->hp;

    spi_ = spi_max_ = data_->spi;

    lair_pos_ = pos;

    inv_ = new Inventory(this);

    prop_handler_ = new PropHandler(this);

    prop_handler_->init_natural_props();

    if (data_->id != ActorId::player)
    {
        mk_start_items();
    }

    place_hook();
}

void Actor::on_std_turn_common()
{
    // Do light damage if in lit cell
    if (map::cells[pos.x][pos.y].is_lit)
    {
        hit(1, DmgType::light);
    }

    if (is_alive())
    {
        // Slowly decrease current HP/spirit if above max
        const int decr_above_max_n_turns = 7;

        const bool decr_this_turn =
            (game_time::turn_nr() % decr_above_max_n_turns) == 0;

        if ((hp() > hp_max(true)) && decr_this_turn)
        {
            --hp_;
        }

        if ((spi() > spi_max()) && decr_this_turn)
        {
            --spi_;
        }

        // Regenerate spirit
        int regen_spi_n_turns = 20;

        if (is_player())
        {
            if (player_bon::traits[(size_t)Trait::strong_spirit])
            {
                regen_spi_n_turns -= 4;
            }

            if (player_bon::traits[(size_t)Trait::mighty_spirit])
            {
                regen_spi_n_turns -= 4;
            }
        }
        else // Is monster
        {
            // Monsters regen spirit very quickly, so spell casters doesn't
            // suddenly get completely handicapped
            regen_spi_n_turns = 2;
        }

        const bool regen_spi_this_turn =
            (game_time::turn_nr() % regen_spi_n_turns) == 0;

        if (regen_spi_this_turn)
        {
            restore_spi(1, false, Verbosity::silent);
        }
    }

    on_std_turn();
}

void Actor::teleport()
{
    bool blocked[map_w][map_h];

    map_parsers::BlocksActor(*this, ParseActors::yes)
        .run(blocked);

    std::vector<P> pos_bucket;
    to_vec(blocked, false, pos_bucket);

    if (pos_bucket.empty())
    {
        return;
    }

    if (!is_player() && map::player->can_see_actor(*this))
    {
        msg_log::add(name_the() + " suddenly disappears!");
    }

    P tgt_pos = pos_bucket[rnd::range(0, pos_bucket.size() - 1)];

    bool player_has_tele_control = false;

    if (is_player())
    {
        // Teleport control?
        if (prop_handler_->has_prop(PropId::tele_ctrl) &&
            !prop_handler_->has_prop(PropId::confused))
        {
            player_has_tele_control = true;

            /*
            auto chance_of_tele_success = [](const P & tgt)
            {
                const int dist = king_dist(map::player->pos, tgt);
                return constr_in_range(25, 100 - dist, 95);
            };

            auto on_marker_at_pos =
                [chance_of_tele_success](const P & p,
                                         CellOverlay overlay[map_w][map_h])
            {
                (void)overlay;

                msg_log::clear();
                look::print_location_info_msgs(p);

                const int chance_pct = chance_of_tele_success(p);

                msg_log::add(std::to_string(chance_pct) + "% chance of success.");

                msg_log::add("[enter] to teleport here");
                msg_log::add(cancel_info_str_no_space);
            };

            auto on_key_press = [](const P & p, const InputData & key_data)
            {
                (void)p;

                if (key_data.key == SDLK_RETURN)
                {
                    msg_log::clear();
                    return MarkerDone::yes;
                }

                return MarkerDone::no;
            };
            */

            msg_log::add("I have the power to control teleportation.",
                         clr_white,
                         false,
                         MorePromptOnMsg::yes);

            /*
            const P marker_tgt_pos = marker::run(MarkerUsePlayerTgt::no,
                                                 on_marker_at_pos,
                                                 on_key_press,
                                                 MarkerShowBlocked::no);

            if (blocked[marker_tgt_pos.x][marker_tgt_pos.y])
            {
                // Blocked
                msg_log::add("Something is blocking me...",
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);
            }
            else if (rnd::percent(chance_of_tele_success(marker_tgt_pos)))
            {
                // Success
                tgt_pos = marker_tgt_pos;
            }
            else // Distance roll failed
            {
                msg_log::add("I failed to go there...",
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);
            }
            */
        }
    }
    else // Is a monster
    {
        static_cast<Mon*>(this)->player_aware_of_me_counter_ = 0;
    }

    // If actor was held by a spider web, destroy it

    // TODO: If something like a bear trap is implemented, the code below
    //       needs to be adapted to consider other "hold" type traps
    Rigid* const rigid = map::cells[pos.x][pos.y].rigid;

    if (rigid->id() == FeatureId::trap)
    {
        Trap* const trap = static_cast<Trap*>(rigid);

        if (trap->type() == TrapId::web &&
            trap->is_holding_actor())
        {
            trap->destroy();
        }
    }

    pos = tgt_pos;

    map::update_vision();

    const auto player_seen_actors =
        map::player->seen_actors();

    for (Actor* const actor : player_seen_actors)
    {
        static_cast<Mon*>(actor)->set_player_aware_of_me();
    }

    if (is_player() &&
        !player_has_tele_control)
    {
        msg_log::add("I suddenly find myself in a different location!");

        prop_handler_->try_add(new PropConfused(PropTurns::specific, 8));
    }
}

TileId Actor::tile() const
{
    if (is_corpse())
    {
        return TileId::corpse2;
    }

    return data_->tile;
}

char Actor::glyph() const
{
    if (is_corpse())
    {
        return '&';
    }

    return data_->glyph;
}

bool Actor::restore_hp(const int hp_restored,
                       const bool is_allowed_above_max,
                       const Verbosity verbosity)
{
    bool is_hp_gained = is_allowed_above_max;
    const int dif_from_max = hp_max(true) - hp_restored;

    // If hp is below limit, but restored hp will push it over the limit, hp is
    // set to max.
    if (!is_allowed_above_max &&
        hp() > dif_from_max &&
        hp() < hp_max(true))
    {
        hp_ = hp_max(true);
        is_hp_gained = true;
    }

    // If hp is below limit, and restored hp will NOT push it over the limit -
    // restored hp is added to current.
    if (is_allowed_above_max || hp() <= dif_from_max)
    {
        hp_ += hp_restored;
        is_hp_gained = true;
    }

    if (verbosity == Verbosity::verbose && is_hp_gained)
    {
        if (is_player())
        {
            msg_log::add("I feel healthier!", clr_msg_good);
        }
        else // Is a monster
        {
            if (map::player->can_see_actor(*this))
            {
                msg_log::add(data_->name_the + " looks healthier.");
            }
        }
    }

    return is_hp_gained;
}

bool Actor::restore_spi(const int spi_restored,
                        const bool is_allowed_above_max,
                        const Verbosity verbosity)
{
    // Maximum allowed level to increase spirit to
    // * If we allow above max, we can raise spirit "infinitely"
    // * Otherwise we cap to max spirit, or current spirit, whichever is higher
    const int limit =
        is_allowed_above_max ?
        INT_MAX :
        std::max(spi_, spi_max());

    const int spi_before = spi_;

    spi_ = std::min(spi_ + spi_restored, limit);

    const bool is_spi_gained = spi_ > spi_before;

    if (verbosity == Verbosity::verbose &&
        is_spi_gained)
    {
        if (is_player())
        {
            msg_log::add("I feel more spirited!", clr_msg_good);
        }
        else
        {
            if (map::player->can_see_actor(*this))
            {
                msg_log::add(data_->name_the + " looks more spirited.");
            }
        }
    }

    return is_spi_gained;
}

void Actor::set_hp_and_spi_to_max()
{
    hp_ = hp_max(true);
    spi_ = spi_max();
}

void Actor::change_max_hp(const int change, const Verbosity verbosity)
{
    hp_max_ = std::max(1, hp_max_ + change);

    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            if (change > 0)
            {
                msg_log::add("I feel more vigorous!", clr_msg_good);
            }
            else if (change < 0)
            {
                msg_log::add("I feel frailer!", clr_msg_bad);
            }
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                if (change > 0)
                {
                    msg_log::add(name_the() + " looks more vigorous.");
                }
                else if (change < 0)
                {
                    msg_log::add(name_the() + " looks frailer.");
                }
            }
        }
    }
}

void Actor::change_max_spi(const int change, const Verbosity verbosity)
{
    spi_max_ = std::max(1, spi_max_ + change);

    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            if (change > 0)
            {
                msg_log::add("My spirit is stronger!", clr_msg_good);
            }
            else if (change < 0)
            {
                msg_log::add("My spirit is weaker!", clr_msg_bad);
            }
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                if (change > 0)
                {
                    msg_log::add(name_the() + " appears to grow in spirit.");
                }
                else if (change < 0)
                {
                    msg_log::add(name_the() + " appears to shrink in spirit.");
                }
            }
        }
    }
}

ActorDied Actor::hit(int dmg,
                     const DmgType dmg_type,
                     const DmgMethod method,
                     const AllowWound allow_wound)
{
    const int hp_pct_before = (hp() * 100) / hp_max(true);

    if (state_ == ActorState::destroyed)
    {
        TRACE_FUNC_END_VERBOSE;

        return ActorDied::no;
    }

    // Damage type is "light", and actor is not light sensitive?
    if (dmg_type == DmgType::light &&
        !prop_handler_->has_prop(PropId::lgt_sens))
    {
        return ActorDied::no;
    }

    if (is_player())
    {
        map::player->interrupt_actions();
    }

    // Damage to corpses
    // NOTE: Corpse is automatically destroyed if damage is high enough,
    //       otherwise it is destroyed with a random chance
    if (is_corpse() && !is_player())
    {
        ASSERT(data_->can_leave_corpse);

        const int dmg_threshold = (hp_max(true) * 2) / 3;

        if (dmg >= dmg_threshold ||
            rnd::fraction(3, 4))
        {
            if (method == DmgMethod::kick)
            {
                Snd snd("*Crack!*",
                        SfxId::hit_corpse_break,
                        IgnoreMsgIfOriginSeen::yes,
                        pos,
                        nullptr,
                        SndVol::low,
                        AlertsMon::yes);

                snd_emit::run(snd);
            }

            state_ = ActorState::destroyed;

            if (is_humanoid())
            {
                map::mk_gore(pos);
            }

            if (map::cells[pos.x][pos.y].is_seen_by_player)
            {
                const std::string corpse_name = corpse_name_the();

                ASSERT(!corpse_name.empty());

                msg_log::add(corpse_name_the() + " is destroyed.");
            }
        }
        else // Not destroyed
        {
            if (method == DmgMethod::kick)
            {
                Snd snd("*Thud*",
                        SfxId::hit_medium,
                        IgnoreMsgIfOriginSeen::yes,
                        pos,
                        nullptr,
                        SndVol::low,
                        AlertsMon::yes);

                snd_emit::run(snd);
            }
        }

        return ActorDied::no;
    }

    if (dmg_type == DmgType::spirit)
    {
        return hit_spi(dmg);
    }

    // Property resists?
    const auto verbosity =
        is_alive() ?
        Verbosity::verbose :
        Verbosity::silent;

    if (prop_handler_->try_resist_dmg(dmg_type, verbosity))
    {
        return ActorDied::no;
    }

    // Filter damage through worn armor
    dmg = std::max(1, dmg);

    if (dmg_type == DmgType::physical &&
        is_humanoid())
    {
        Armor* armor =
            static_cast<Armor*>(inv_->item_in_slot(SlotId::body));

        if (armor)
        {
            TRACE_VERBOSE << "Has armor, running hit on armor" << std::endl;

            dmg = armor->take_dur_hit_and_get_reduced_dmg(dmg);

            if (armor->is_destroyed())
            {
                TRACE << "Armor was destroyed" << std::endl;

                if (is_player())
                {
                    const std::string armor_name =
                        armor->name(ItemRefType::plain, ItemRefInf::none);

                    msg_log::add("My " + armor_name + " is torn apart!",
                                 clr_msg_note);
                }

                inv_->remove_item_in_slot(SlotId::body, true);

                armor = nullptr;
            }
        }
    }

    on_hit(dmg,
           dmg_type,
           method,
           allow_wound);

    prop_handler_->on_hit();

    // TODO: Perhaps allow zero damage?
    dmg = std::max(1, dmg);

    if (!(is_player() && config::is_bot_playing()))
    {
        hp_ -= dmg;
    }

    if (hp() <= 0)
    {
        const bool is_on_bottomless =
            map::cells[pos.x][pos.y].rigid->is_bottomless();

        //
        // Destroy the corpse if the damage is either:
        //
        // * Above a threshold relative to the actor's maximum hit points, or
        // * Above a fixed value threshold
        //
        // The purpose of the first case is to make it very likely that small
        // creatures like rats are destroyed.
        //
        // The purpose of the second point is that powerful attacks like
        // explosions should always destroy the corpse, even if the creature
        // has a very high pool of hit points.
        //

        const int dmg_threshold_relative = (hp_max(true) * 3) / 2;

        const int dmg_threshold_absolute = 14;

        const bool is_destroyed =
            !data_->can_leave_corpse ||
            is_on_bottomless ||
            (dmg >= dmg_threshold_relative) ||
            (dmg >= dmg_threshold_absolute);

        die(is_destroyed,       // Is destroyed
            !is_on_bottomless,  // Allow gore
            !is_on_bottomless); // Allow dropping items

        return ActorDied::yes;
    }

    // HP is greater than 0

    const int hp_pct_after = (hp() * 100) / hp_max(true);

    const int hp_warn_lvl = 25;

    if (is_player() &&
        hp_pct_before > hp_warn_lvl &&
        hp_pct_after <= hp_warn_lvl)
    {
        msg_log::add("-LOW HP WARNING!-",
                     clr_msg_bad,
                     true,
                     MorePromptOnMsg::yes);
    }

    return ActorDied::no;
}

ActorDied Actor::hit_spi(const int dmg, const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            msg_log::add("My spirit is drained!", clr_msg_bad);
        }
    }

    prop_handler_->on_hit();

    if (!is_player() || !config::is_bot_playing())
    {
        spi_ = std::max(0, spi_ - dmg);
    }

    if (spi() <= 0)
    {
        if (is_player())
        {
            msg_log::add("All my spirit is depleted, I am devoid of life!",
                         clr_msg_bad);
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                msg_log::add(name_the() + " has no spirit left!");
            }
        }

        const bool is_on_bottomless =
            map::cells[pos.x][pos.y].rigid->is_bottomless();

        const bool is_destroyed =
            !data_->can_leave_corpse ||
            is_on_bottomless;

        die(is_destroyed, false, true);
        return ActorDied::yes;
    }

    // SP is greater than 0

    return ActorDied::no;
}

void Actor::die(const bool is_destroyed,
                const bool allow_gore,
                const bool allow_drop_items)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    ASSERT(data_->can_leave_corpse || is_destroyed);

    // Check all monsters and unset this actor as leader
    for (Actor* other : game_time::actors)
    {
        if (other != this &&
            !other->is_player() &&
            is_leader_of(other))
        {
            static_cast<Mon*>(other)->leader_ = nullptr;
        }
    }

    bool can_player_see_dying_actor = true;

    if (!is_player())
    {
        // If this monster is player's target, unset the target
        if (map::player->tgt_ == this)
        {
            map::player->tgt_ = nullptr;
        }

        TRACE_VERBOSE << "Printing death message" << std::endl;

        if (map::player->can_see_actor(*this))
        {
            can_player_see_dying_actor = true;

            const std::string msg = death_msg();

            if (!msg.empty())
            {
                msg_log::add(msg);
            }
        }
    }

    if (is_destroyed)
    {
        state_ = ActorState::destroyed;
    }
    else // Not destroyed
    {
        state_ = ActorState::corpse;
    }

    if (!is_player())
    {
        // This is a monster

        if (is_humanoid())
        {
            TRACE_VERBOSE << "Emitting death sound" << std::endl;

            Snd snd("I hear agonized screaming.",
                    SfxId::END,
                    IgnoreMsgIfOriginSeen::yes,
                    pos,
                    this,
                    SndVol::high,
                    AlertsMon::no);

            snd_emit::run(snd);
        }

        if (allow_drop_items)
        {
            item_drop::drop_all_characters_items(*this);
        }
    }

    if (is_destroyed)
    {
        if (data_->can_bleed && allow_gore)
        {
            map::mk_gore(pos);
            map::mk_blood(pos);
        }
    }
    else // Not destroyed
    {
        if (!is_player())
        {
            P new_pos;

            auto* feature_here = map::cells[pos.x][pos.y].rigid;

            // TODO: this should be decided with a floodfill instead
            if (!feature_here->can_have_corpse())
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        new_pos = pos + P(dx, dy);
                        feature_here = map::cells[pos.x + dx][pos.y + dy].rigid;

                        if (feature_here->can_have_corpse())
                        {
                            pos.set(new_pos);
                            dx = 9999;
                            dy = 9999;
                        }
                    }
                }
            }
        }
    }

    on_death();

    prop_handler_->on_death(can_player_see_dying_actor);

    if (!is_player())
    {
        game::on_mon_killed(*this);
        static_cast<Mon*>(this)->leader_ = nullptr;
    }

    TRACE_FUNC_END_VERBOSE;
}

std::string Actor::death_msg() const
{
    return name_the() + " dies.";
}

DidAction Actor::try_eat_corpse()
{
    const bool actor_is_player = is_player();

    PropWound* wound = nullptr;

    if (actor_is_player)
    {
        Prop* prop = prop_handler_->prop(PropId::wound);

        if (prop)
        {
            wound = static_cast<PropWound*>(prop);
        }
    }

    if (hp() >= hp_max(true) && !wound)
    {
        // Not "hungry"
        return DidAction::no;
    }

    Actor* corpse = nullptr;

    // Check all corpses here, if this is the player eating, stop at any corpse
    // which is prioritized for bashing (Zombies)
    for (Actor* const actor : game_time::actors)
    {
        if (actor->pos == pos && actor->state() == ActorState::corpse)
        {
            corpse = actor;

            if (actor_is_player && actor->data().prio_corpse_bash)
            {
                break;
            }
        }
    }

    if (corpse)
    {
        const int corpse_max_hp = corpse->hp_max(false);
        const int destr_one_in_n = constr_in_range(1, corpse_max_hp / 4, 8);
        const bool is_destroyed = rnd::one_in(destr_one_in_n);
        const std::string corpse_name = corpse->corpse_name_the();

        Snd snd("I hear ripping and chewing.",
                SfxId::bite,
                IgnoreMsgIfOriginSeen::yes,
                pos,
                this,
                SndVol::low,
                AlertsMon::no,
                MorePromptOnMsg::no);

        snd_emit::run(snd);

        if (actor_is_player)
        {
            msg_log::add("I feed on " + corpse_name + ".");

            if (is_destroyed)
            {
                msg_log::add("There is nothing left to eat.");
            }
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string name = name_the();

                msg_log::add(name + " feeds on " + corpse_name + ".");
            }
        }

        if (is_destroyed)
        {
            corpse->state_ = ActorState::destroyed;
            map::mk_gore(pos);
            map::mk_blood(pos);
        }

        // Heal
        on_feed();

        return DidAction::yes;
    }

    return DidAction::no;
}

void Actor::on_feed()
{
    const int hp_restored = rnd::range(3, 5);

    restore_hp(hp_restored, false, Verbosity::silent);

    if (is_player())
    {
        Prop* const prop = prop_handler_->prop(PropId::wound);

        if (prop && rnd::one_in(6))
        {
            PropWound* const wound = static_cast<PropWound*>(prop);

            wound->heal_one_wound();
        }
    }
}

void Actor::add_light(bool light_map[map_w][map_h]) const
{
    if (state_ == ActorState::alive && prop_handler_->has_prop(PropId::radiant))
    {
        // TODO: Much of the code below is duplicated from
        // ActorPlayer::add_light_hook(), some refactoring is needed.

        bool hard_blocked[map_w][map_h];

         const R fov_lmt = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
            .run(hard_blocked,
                 MapParseMode::overwrite,
                 fov_lmt);

        LosResult fov[map_w][map_h];

        fov::run(pos, hard_blocked, fov);

        for (int y = fov_lmt.p0.y; y <= fov_lmt.p1.y; ++y)
        {
            for (int x = fov_lmt.p0.x; x <= fov_lmt.p1.x; ++x)
            {
                if (!fov[x][y].is_blocked_hard)
                {
                    light_map[x][y] = true;
                }
            }
        }
    }
    else if (prop_handler_->has_prop(PropId::burning))
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                light_map[pos.x + dx][pos.y + dy] = true;
            }
        }
    }

    add_light_hook(light_map);
}

bool Actor::is_player() const
{
    return this == map::player;
}
