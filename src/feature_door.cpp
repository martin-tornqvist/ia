#include "feature_door.hpp"

#include "init.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "feature_data.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "postmortem.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"

Door::Door(const P& feature_pos,
           const Rigid* const mimic_feature,
           DoorType type,
           DoorSpawnState spawn_state) :
    Rigid                   (feature_pos),
    mimic_feature_          (mimic_feature),
    nr_spikes_              (0),
    is_open_                (false),
    is_stuck_               (false),
    is_secret_              (false),
    is_handled_externally_  (false),
    type_                   (type)
{
    //
    // Gates should never be secret
    //
    ASSERT(!(type_ == DoorType::gate &&
             mimic_feature));

    ASSERT(!(type_ == DoorType::gate &&
             (spawn_state == DoorSpawnState::secret ||
              spawn_state == DoorSpawnState::secret_and_stuck)));

    if (spawn_state == DoorSpawnState::any)
    {
        // NOTE: The chances below are just generic default behavior for random
        //       doors placed wherever. Doors may be explicitly set to other
        //       states elsewhere during map generation (e.g. set to secret to
        //       hide an optional branch of the map).

        const int pct_secret =
            type_ == DoorType::gate ?
            0 :
            std::min(80, (map::dlvl - 1) * 10);

        const int stuck_one_in_n = 28;

        if (rnd::percent(pct_secret))
        {
            if (rnd::one_in(stuck_one_in_n))
            {
                spawn_state = DoorSpawnState::secret_and_stuck;
            }
            else // Not stuck
            {
                spawn_state = DoorSpawnState::secret;
            }
        }
        else // Not secret
        {
            Fraction chance_open(3, 4);

            if (chance_open.roll())
            {
                spawn_state = DoorSpawnState::open;
            }
            else // Closed
            {
                if (rnd::one_in(stuck_one_in_n))
                {
                    spawn_state = DoorSpawnState::stuck;
                }
                else // Not stuck
                {
                    spawn_state = DoorSpawnState::closed;
                }
            }
        }
    }

    switch (DoorSpawnState(spawn_state))
    {
    case DoorSpawnState::open:
        is_open_   = true;
        is_stuck_  = false;
        is_secret_ = false;
        break;

    case DoorSpawnState::closed:
        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = false;
        break;

    case DoorSpawnState::stuck:
        is_open_   = false;
        is_stuck_  = true;
        is_secret_ = false;
        break;

    case DoorSpawnState::secret:
        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = true;
        break;

    case DoorSpawnState::secret_and_stuck:
        is_open_   = false;
        is_stuck_  = true;
        is_secret_ = true;
        break;

    case DoorSpawnState::any:
        ASSERT(false);

        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = false;
    }

} // Door

Door::~Door()
{
    if (mimic_feature_)
    {
        delete mimic_feature_;
    }
}

void Door::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    if (dmg_method == DmgMethod::forced)
    {
        //
        // Forced
        //
        map::put(new RubbleLow(pos_));

        map::update_vision();

        return;
    }

    if (dmg_type == DmgType::physical)
    {
        //
        // Shotgun
        //
        if (dmg_method == DmgMethod::shotgun)
        {
            if (!is_open_)
            {
                switch (type_)
                {
                case DoorType::wood:
                case DoorType::gate:
                    if (rnd::fraction(7, 10))
                    {
                        if (map::is_pos_seen_by_player(pos_))
                        {
                            const std::string a =
                                is_secret_ ?
                                "A " : "The ";

                            msg_log::add(a +
                                         base_name_short() +
                                         " is blown to pieces!");
                        }

                        map::put(new RubbleLow(pos_));

                        map::update_vision();

                        return;
                    }
                    break;

                case DoorType::metal:
                    break;
                }
            }
        }

        //
        // Explosion
        //
        if (dmg_method == DmgMethod::explosion)
        {
            //
            //TODO
            //
        }

        //
        // Heavy blunt
        //
        if (dmg_method == DmgMethod::blunt_heavy)
        {
            ASSERT(actor);

            // switch (type_)
            // {
            // case DoorType::wood:
            // case DoorType::gate:
            // {
            //     Fraction destr_chance(6, 10);

            //     if (actor == map::player)
            //     {
            //         if (player_bon::traits[(size_t)Trait::tough])
            //         {
            //             destr_chance.num += 2;
            //         }

            //         if (player_bon::traits[(size_t)Trait::rugged])
            //         {
            //             destr_chance.num += 2;
            //         }

            //         if (destr_chance.roll())
            //         {

            //         }
            //     }
            //     else //Is monster
            //     {
            //         if (destr_chance.roll())
            //         {

            //         }
            //     }
            // } break;

            // case DoorType::metal:
            //     break;
            // }

        } // blunt_heavy

        //
        // Kick
        //
        if (dmg_method == DmgMethod::kick)
        {
            ASSERT(actor);

            const bool is_player = actor == map::player;

            const bool is_cell_seen = map::is_pos_seen_by_player(pos_);

            const bool is_weak = actor->has_prop(PropId::weakened);

            switch (type_)
            {
            case DoorType::wood:
            case DoorType::gate:
            {
                if (is_player)
                {
                    int destr_chance_pct = 25 - (nr_spikes_ * 3);

                    destr_chance_pct = std::max(1, destr_chance_pct);

                    if (player_bon::traits[(size_t)Trait::tough])
                    {
                        destr_chance_pct += 15;
                    }

                    if (player_bon::traits[(size_t)Trait::rugged])
                    {
                        destr_chance_pct += 15;
                    }

                    if (actor->has_prop(PropId::frenzied))
                    {
                        destr_chance_pct += 30;
                    }

                    if (is_weak)
                    {
                        destr_chance_pct = 0;
                    }

                    destr_chance_pct = std::min(100, destr_chance_pct);

                    if (destr_chance_pct > 0)
                    {
                        if (rnd::percent(destr_chance_pct))
                        {
                            Snd snd("",
                                    SfxId::door_break,
                                    IgnoreMsgIfOriginSeen::yes,
                                    pos_,
                                    actor,
                                    SndVol::low,
                                    AlertsMon::yes);

                            snd_emit::run(snd);

                            if (is_cell_seen)
                            {
                                if (is_secret_)
                                {
                                    msg_log::add("A " +
                                                 base_name_short() +
                                                 " crashes open!");
                                }
                                else
                                {
                                    msg_log::add("The " +
                                                 base_name_short() +
                                                 " crashes open!");
                                }
                            }
                            else // Cell not seen
                            {
                                msg_log::add("I feel a door crashing open!");
                            }

                            map::put(new RubbleLow(pos_));

                            map::update_vision();
                        }
                        else // Not destroyed
                        {
                            const SfxId sfx =
                                is_secret_ ?
                                SfxId::END : SfxId::door_bang;

                            Snd snd("",
                                    sfx,
                                    IgnoreMsgIfOriginSeen::no,
                                    pos_,
                                    actor,
                                    SndVol::low,
                                    AlertsMon::yes);

                            snd_emit::run(snd);
                        }
                    }
                    else // No chance of success
                    {
                        if (is_cell_seen && !is_secret_)
                        {
                            Snd snd("",
                                    SfxId::door_bang,
                                    IgnoreMsgIfOriginSeen::no,
                                    actor->pos,
                                    actor,
                                    SndVol::low,
                                    AlertsMon::yes);

                            snd_emit::run(snd);

                            msg_log::add("It seems futile.");
                        }
                    }
                }
                else // Is monster
                {
                    int destr_chance_pct = 8 - (nr_spikes_ * 2);

                    destr_chance_pct = std::max(1, destr_chance_pct);

                    if (is_weak)
                    {
                        destr_chance_pct = 0;
                    }

                    if (rnd::percent(destr_chance_pct))
                    {
                        //
                        // NOTE: When it's a monster bashing down the door, we
                        //       make the sound alert other monsters - since
                        //       causes a nicer AI behavior (everyone near the
                        //       door understands that it's time to run inside)
                        //
                        Snd snd("I hear a door crashing open!",
                                SfxId::door_break,
                                IgnoreMsgIfOriginSeen::yes,
                                pos_,
                                actor,
                                SndVol::high,
                                AlertsMon::yes);

                        snd_emit::run(snd);

                        if (map::player->can_see_actor(*actor))
                        {
                            msg_log::add("The " +
                                         base_name_short() +
                                         " crashes open!");
                        }
                        else if (is_cell_seen)
                        {
                            msg_log::add("A " +
                                         base_name_short() +
                                         " crashes open!");
                        }

                        map::put(new RubbleLow(pos_));

                        map::update_vision();
                    }
                    else // Not destroyed
                    {
                        Snd snd("I hear a loud banging.",
                                SfxId::door_bang,
                                IgnoreMsgIfOriginSeen::yes,
                                actor->pos,
                                actor,
                                SndVol::high,
                                AlertsMon::no);

                        snd_emit::run(snd);
                    }
                }

            }
            break; // wood, gate

            case DoorType::metal:
            {
                if (is_player &&
                    is_cell_seen &&
                    !is_secret_)
                {
                    msg_log::add("It seems futile.",
                                 clr_msg_note,
                                 false,
                                 MorePromptOnMsg::yes);
                }
            }
            break; // metal

            } // Door type switch

        } // kick

    } // physical damage

    //
    // Fire
    //
    if (dmg_method == DmgMethod::elemental &&
        dmg_type == DmgType::fire &&
        matl() == Matl::wood)
    {
        try_start_burning(true);
    }

} // on_hit

WasDestroyed Door::on_finished_burning()
{
    if (map::is_pos_seen_by_player(pos_))
    {
        msg_log::add("The door burns down.");
    }

    RubbleLow* const rubble = new RubbleLow(pos_);

    rubble->set_has_burned();

    map::put(rubble);

    map::update_vision();

    return WasDestroyed::yes;
}

bool Door::can_move_cmn() const
{
    return is_open_;
}

bool Door::can_move(Actor& actor) const
{
    if (is_open_)
    {
        return true;
    }

    if (actor.has_prop(PropId::ethereal) ||
        actor.has_prop(PropId::ooze))
    {
        return true;
    }

    return is_open_;
}

bool Door::is_los_passable() const
{
    return is_open_ || (type_ == DoorType::gate);
}

bool Door::is_projectile_passable() const
{
    return is_open_ || (type_ == DoorType::gate);
}

bool Door::is_smoke_passable() const
{
    return is_open_ || (type_ == DoorType::gate);
}

std::string Door::base_name() const
{
    std::string ret = "";

    switch (type_)
    {
    case DoorType::wood:
        ret = "wooden door";
        break;

    case DoorType::metal:
        ret = "metal door";
        break;

    case DoorType::gate:
        ret = "barred gate";
        break;
    }

    return ret;
}

std::string Door::base_name_short() const
{
    std::string ret = "";

    switch (type_)
    {
    case DoorType::wood:
        ret = "door";
        break;

    case DoorType::metal:
        ret = "door";
        break;

    case DoorType::gate:
        ret = "barred gate";
        break;
    }

    return ret;
}

std::string Door::name(const Article article) const
{
    if (is_secret_)
    {
        ASSERT(type_ != DoorType::gate);
        ASSERT(mimic_feature_);

        return mimic_feature_->name(article);
    }

    std::string a = "";

    std::string mod = "";

    if (burn_state() == BurnState::burning)
    {
        a =
            article == Article::a ?
            "a " : "the ";

        mod = "burning ";
    }

    if (is_open_)
    {
        if (a.empty())
        {
            a =
                article == Article::a ?
                "an " : "the ";
        }

        mod += "open " ;
    }

    if (a.empty())
    {
        a =
            article == Article::a ?
            "a " : "the ";
    }

    return a + mod + base_name();

} // name

Clr Door::clr_default() const
{
    if (is_secret_)
    {
        return mimic_feature_->clr();
    }
    else
    {
        switch (type_)
        {
        case DoorType::wood:
            return clr_brown_drk;
            break;

        case DoorType::metal:
            return clr_white;
            break;

        case DoorType::gate:
            return clr_gray;
            break;
        }
    }

    ASSERT(false);

    return clr_gray;
}

char Door::glyph() const
{
    if (is_secret_)
    {
        ASSERT(type_ != DoorType::gate);
        ASSERT(mimic_feature_);

        return mimic_feature_->glyph();
    }
    else // Not secret
    {
        return is_open_ ? 39 : '+';
    }
}

TileId Door::tile() const
{
    TileId ret = TileId::empty;

    if (is_secret_)
    {
        ASSERT(type_ != DoorType::gate);
        ASSERT(mimic_feature_);

        ret = mimic_feature_->tile();
    }
    else // Not secret
    {
        switch (type_)
        {
        case DoorType::wood:
        case DoorType::metal:
        {
            ret =
                is_open_ ?
                TileId::door_open :
                TileId::door_closed;
        }
        break;

        case DoorType::gate:
        {
            ret =
                is_open_ ?
                TileId::gate_open :
                TileId::gate_closed;
        }
        break;
        }
    }

    return ret;
}

Matl Door::matl() const
{
    switch (type_)
    {
    case DoorType::wood:
        return Matl::wood;
        break;

    case DoorType::metal:
    case DoorType::gate:
        return Matl::metal;
        break;
    }

    ASSERT(false);

    return Matl::wood;
}

void Door::bump(Actor& actor_bumping)
{
    if (!actor_bumping.is_player())
    {
        return;
    }

    if (is_secret_)
    {
        ASSERT(type_ != DoorType::gate);

        // Print messages as if this was a wall

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            TRACE << "Player bumped into secret door, "
                  << "with vision in cell" << std::endl;

            msg_log::add(feature_data::data(FeatureId::wall).
                         msg_on_player_blocked);
        }
        else // Not seen by player
        {
            TRACE << "Player bumped into secret door, "
                  << "without vision in cell" << std::endl;

            msg_log::add(feature_data::data(FeatureId::wall).
                         msg_on_player_blocked_blind);
        }

        return;
    }

    if (!is_open_)
    {
        try_open(&actor_bumping);
    }

} // bump

void Door::reveal(const Verbosity verbosity)
{
    if (!is_secret_)
    {
        return;
    }

    is_secret_ = false;

    if (verbosity == Verbosity::verbose &&
        map::cells[pos_.x][pos_.y].is_seen_by_player)
    {
        msg_log::add("A secret is revealed.");
    }
}

bool Door::try_jam(Actor* actor_trying)
{
    const bool is_player = actor_trying == map::player;

    const bool tryer_is_blind = !actor_trying->prop_handler().allow_see();

    if (is_secret_ || is_open_)
    {
        return false;
    }

    // Door is in correct state for spiking (known, closed)
    ++nr_spikes_;
    is_stuck_ = true;

    if (is_player)
    {
        std::string a =
            tryer_is_blind ?
            "a " : "the ";

        msg_log::add("I jam " +
                     a +
                     base_name_short() +
                     " with a spike.");
    }

    game_time::tick();
    return true;
}

void Door::try_close(Actor* actor_trying)
{
    const bool is_player = actor_trying == map::player;

    const bool tryer_is_blind = !actor_trying->prop_handler().allow_see();

    const bool player_see_tryer =
        is_player ?
        true :
        map::player->can_see_actor(*actor_trying);

    bool is_closable = true;

    if (is_handled_externally_)
    {
        if (is_player)
        {
            msg_log::add("This " +
                         base_name_short() +
                         " cannot be closed.");

            msg_log::add("Perhaps it is handled elsewhere.");
        }

        return;
    }

    // Already closed?
    if (is_closable && !is_open_)
    {
        is_closable = false;

        if (is_player)
        {
            if (tryer_is_blind)
            {
                msg_log::add("I find nothing there to close.");
            }
            else // Can see
            {
                msg_log::add("I see nothing there to close.");
            }
        }
    }

    // Blocked?
    if (is_closable)
    {
        bool isblocked_by_actor = false;

        for (Actor* actor : game_time::actors)
        {
            if (actor->pos == pos_)
            {
                isblocked_by_actor = true;
                break;
            }
        }

        if (isblocked_by_actor ||
            map::cells[pos_.x][pos_.y].item)
        {
            is_closable = false;

            if (is_player)
            {
                if (tryer_is_blind)
                {
                    msg_log::add("Something is blocking the " +
                                 base_name_short() +
                                 ".");
                }
                else // Can see
                {
                    msg_log::add("The " +
                                 base_name_short() +
                                 " is blocked.");
                }
            }
        }
    }

    if (is_closable)
    {
        // Door is in correct state for closing (open, working, not blocked)

        if (tryer_is_blind)
        {
            if (rnd::coin_toss())
            {
                is_open_ = false;

                if (is_player)
                {
                    Snd snd("",
                            SfxId::door_close,
                            IgnoreMsgIfOriginSeen::yes,
                            pos_,
                            actor_trying,
                            SndVol::low,
                            AlertsMon::yes);

                    snd_emit::run(snd);

                    msg_log::add("I fumble with a " +
                                 base_name_short() +
                                 ", but manage to close it.");
                }
                else // Monster closing
                {
                    Snd snd("I hear a door closing.",
                            SfxId::door_close,
                            IgnoreMsgIfOriginSeen::yes,
                            pos_,
                            actor_trying,
                            SndVol::low,
                            AlertsMon::no);

                    snd_emit::run(snd);

                    if (player_see_tryer)
                    {
                        msg_log::add(actor_trying->name_the() +
                                     "fumbles, but manages to close a " +
                                     base_name_short() +
                                     ".");
                    }
                }
            }
            else // Fail to close
            {
                if (is_player)
                {
                    msg_log::add("I fumble blindly with a " +
                                 base_name_short() +
                                 ", and fail to close it.");
                }
                else // Monster failing to close
                {
                    if (player_see_tryer)
                    {
                        msg_log::add(actor_trying->name_the() +
                                     " fumbles blindly, and fails to close a " +
                                     base_name_short() +
                                     ".");
                    }
                }
            }
        }
        else // Can see
        {
            is_open_ = false;

            if (is_player)
            {
                Snd snd("",
                        SfxId::door_close,
                        IgnoreMsgIfOriginSeen::yes,
                        pos_,
                        actor_trying,
                        SndVol::low,
                        AlertsMon::yes);

                snd_emit::run(snd);

                msg_log::add("I close the " +
                             base_name_short() +
                             ".");
            }
            else // Is a monster closing
            {
                Snd snd("I hear a door closing.",
                        SfxId::door_close,
                        IgnoreMsgIfOriginSeen::yes,
                        pos_,
                        actor_trying,
                        SndVol::low,
                        AlertsMon::no);

                snd_emit::run(snd);

                if (player_see_tryer)
                {
                    msg_log::add(actor_trying->name_the() +
                                 " closes a " +
                                 base_name_short() +
                                 ".");
                }
            }
        }
    }

    // TODO: It doesn't seem like a turn is spent if player is blind and fails
    //       to close the door?
    if (!is_open_ && is_closable)
    {
        game_time::tick();
    }

    if (!is_open_)
    {
        map::update_vision();
    }

} // try_close

void Door::try_open(Actor* actor_trying)
{
    TRACE_FUNC_BEGIN;

    const bool is_player = actor_trying == map::player;

    const bool player_see_door = map::cells[pos_.x][pos_.y].is_seen_by_player;

    const bool player_see_tryer =
        is_player ?
        true :
        map::player->can_see_actor(*actor_trying);

    if (is_handled_externally_)
    {
        if (is_player)
        {
            msg_log::add("I see no way to open the " +
                         base_name_short() +
                         ".");

            msg_log::add("Perhaps it is opened elsewhere.");
        }

        return;
    }

    if (is_stuck_)
    {
        TRACE << "Is stuck" << std::endl;

        if (is_player)
        {
            msg_log::add("The " +
                         base_name_short() +
                         " seems to be stuck.");
        }
    }
    else // Not stuck
    {
        TRACE << "Is not stuck" << std::endl;

        const bool tryer_can_see = actor_trying->prop_handler().allow_see();

        if (tryer_can_see)
        {
            TRACE << "Tryer can see, opening" << std::endl;
            is_open_ = true;

            if (is_player)
            {
                Snd snd("",
                        SfxId::door_open,
                        IgnoreMsgIfOriginSeen::yes,
                        pos_,
                        actor_trying,
                        SndVol::low,
                        AlertsMon::yes);

                snd_emit::run(snd);

                msg_log::add("I open the " +
                             base_name_short() +
                             ".");
            }
            else // Is monster
            {
                Snd snd("I hear a door open.",
                        SfxId::door_open,
                        IgnoreMsgIfOriginSeen::yes,
                        pos_,
                        actor_trying,
                        SndVol::low,
                        AlertsMon::no);

                snd_emit::run(snd);

                if (player_see_tryer)
                {
                    msg_log::add(actor_trying->name_the() +
                                 " opens a " +
                                 base_name_short() +
                                 ".");
                }
                else if (player_see_door)
                {
                    msg_log::add("I see a " +
                                 base_name_short() +
                                 " opening.");
                }
            }
        }
        else // Tryer is blind
        {
            if (rnd::coin_toss())
            {
                TRACE << "Tryer is blind, but open succeeded anyway"
                      << std::endl;

                is_open_ = true;

                if (is_player)
                {
                    Snd snd("",
                            SfxId::door_open,
                            IgnoreMsgIfOriginSeen::yes,
                            pos_,
                            actor_trying,
                            SndVol::low,
                            AlertsMon::yes);

                    snd_emit::run(snd);

                    msg_log::add("I fumble with a " +
                                 base_name_short() +
                                 ", but finally manage to open it.");
                }
                else // Is monster
                {
                    Snd snd("I hear something open a door awkwardly.",
                            SfxId::door_open,
                            IgnoreMsgIfOriginSeen::yes,
                            pos_,
                            actor_trying,
                            SndVol::low,
                            AlertsMon::no);

                    snd_emit::run(snd);

                    if (player_see_tryer)
                    {
                        msg_log::add(actor_trying->name_the() +
                                     "fumbles, but manages to open a " +
                                     base_name_short() +
                                     ".");
                    }
                    else if (player_see_door)
                    {
                        msg_log::add("I see a " +
                                     base_name_short() +
                                     " open awkwardly.");
                    }
                }
            }
            else // Failed to open
            {
                TRACE << "Tryer is blind, and open failed" << std::endl;

                if (is_player)
                {
                    Snd snd("",
                            SfxId::END,
                            IgnoreMsgIfOriginSeen::yes,
                            pos_,
                            actor_trying,
                            SndVol::low,
                            AlertsMon::yes);

                    snd_emit::run(snd);

                    msg_log::add("I fumble blindly with a " +
                                 base_name_short() +
                                 ", and fail to open it.");
                }
                else // Is monster
                {
                    // Emitting the sound from the actor instead of the door,
                    // because the sound message should be received even if the
                    // door is seen
                    Snd snd("I hear something attempting to open a door.",
                            SfxId::END,
                            IgnoreMsgIfOriginSeen::yes,
                            actor_trying->pos,
                            actor_trying,
                            SndVol::low,
                            AlertsMon::no);

                    snd_emit::run(snd);

                    if (player_see_tryer)
                    {
                        msg_log::add(actor_trying->name_the() +
                                     " fumbles blindly, and fails to open a " +
                                     base_name_short() +
                                     ".");
                    }
                }

                game_time::tick();
            }
        }
    }

    if (is_open_)
    {
        TRACE << "Open was successful" << std::endl;

        if (is_secret_)
        {
            TRACE << "Was secret, now revealing" << std::endl;
            reveal(Verbosity::verbose);
        }

        game_time::tick();

        map::update_vision();
    }

} // try_open

DidOpen Door::open(Actor* const actor_opening)
{
    (void)actor_opening;

    is_open_ = true;

    is_secret_= false;

    is_stuck_ = false;

    return DidOpen::yes;
}
