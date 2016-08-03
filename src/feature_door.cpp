#include "feature_door.hpp"

#include "init.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "feature_data.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "postmortem.hpp"
#include "player_bon.hpp"
#include "render.hpp"
#include "map_parsing.hpp"

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(const P& feature_pos,
           const Rigid* const mimic_feature,
           DoorSpawnState spawn_state) :
    Rigid                   (feature_pos),
    mimic_feature_          (mimic_feature),
    nr_spikes_              (0),
    is_open_                (false),
    is_stuck_               (false),
    is_secret_              (false),
    is_handled_externally_  (false),
    matl_                   (Matl::wood)
{
    if (spawn_state == DoorSpawnState::any)
    {
        //NOTE: The chances below are just generic "default" behavior for random
        //      doors placed wherever. Doors may be explicitly set to other
        //      states elsewhere during map generation (e.g. set to secret to
        //      hide an optional branch of the map).

        const int pct_secret        = std::min(50, (map::dlvl - 1) * 5);
        const int stuck_one_in_n    = 24;

        if (rnd::percent(pct_secret))
        {
            if (rnd::one_in(stuck_one_in_n))
            {
                spawn_state = DoorSpawnState::secret_and_stuck;
            }
            else //Not stuck
            {
                spawn_state = DoorSpawnState::secret;
            }
        }
        else //Not secret
        {
            Fraction chance_open(3, 4);

            if (chance_open.roll())
            {
                spawn_state = DoorSpawnState::open;
            }
            else //Closed
            {
                if (rnd::one_in(stuck_one_in_n))
                {
                    spawn_state = DoorSpawnState::stuck;
                }
                else //Not stuck
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
        ASSERT(false && "Should not happen");
        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = false;
    }
}

Door::~Door()
{
    if (mimic_feature_)
    {
        delete mimic_feature_;
    }
}

void Door::on_hit(const DmgType dmg_type, const DmgMethod dmg_method, Actor* const actor)
{
    if (dmg_type == DmgType::physical)
    {
        if (dmg_method == DmgMethod::forced)
        {
            //---------------------------------------------------------------------- FORCED
            (void)actor;
            map::put(new RubbleLow(pos_));
        }

        //---------------------------------------------------------------------- SHOTGUN
        if (dmg_method == DmgMethod::shotgun)
        {
            (void)actor;

            if (!is_open_)
            {
                switch (matl_)
                {
                case Matl::wood:
                    if (rnd::fraction(7, 10))
                    {
                        if (map::is_pos_seen_by_player(pos_))
                        {
                            const std::string a = is_secret_ ? "A" : "The";
                            msg_log::add(a + " door is blown to splinters!");
                        }

                        map::put(new RubbleLow(pos_));
                    }
                    break;

                case Matl::empty:
                case Matl::cloth:
                case Matl::fluid:
                case Matl::plant:
                case Matl::stone:
                case Matl::metal:
                    break;
                }
            }
        }

        //---------------------------------------------------------------------- EXPLOSION
        if (dmg_method == DmgMethod::explosion)
        {
            (void)actor;
            //TODO
        }

        //---------------------------------------------------------------------- HEAVY BLUNT
        if (dmg_method == DmgMethod::blunt_heavy)
        {
            ASSERT(actor);

            switch (matl_)
            {
            case Matl::wood:
            {
                Fraction destr_chance(6, 10);

                if (actor == map::player)
                {
                    if (player_bon::traits[size_t(Trait::tough)])
                    {
                        destr_chance.num += 2;
                    }

                    if (player_bon::traits[size_t(Trait::rugged)])
                    {
                        destr_chance.num += 2;
                    }

                    if (destr_chance.roll())
                    {

                    }
                }
                else //Is monster
                {
                    if (destr_chance.roll())
                    {

                    }
                }
            } break;

            case Matl::empty:
            case Matl::cloth:
            case Matl::fluid:
            case Matl::plant:
            case Matl::stone:
            case Matl::metal:
                break;
            }
        }

        //---------------------------------------------------------------------- KICK
        if (dmg_method == DmgMethod::kick)
        {
            ASSERT(actor);

            const bool is_player    = actor == map::player;
            const bool is_cell_seen = map::is_pos_seen_by_player(pos_);
            const bool is_weak      = actor->has_prop(PropId::weakened);

            switch (matl_)
            {
            case Matl::wood:
            {
                if (is_player)
                {
                    Fraction destr_chance(4 - nr_spikes_, 18);

                    destr_chance.num = std::max(1, destr_chance.num);

                    if (player_bon::traits[(size_t)Trait::tough])
                    {
                        destr_chance.num += 2;
                    }

                    if (player_bon::traits[(size_t)Trait::rugged])
                    {
                        destr_chance.num += 2;
                    }

                    if (actor->has_prop(PropId::frenzied))
                    {
                        destr_chance.num += 4;
                    }

                    if (is_weak)
                    {
                        destr_chance.num = 0;
                    }

                    //Cap numerator to denominator
                    destr_chance.num = std::min(destr_chance.num, destr_chance.den);

                    if (destr_chance.num > 0)
                    {
                        if (destr_chance.roll())
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
                                    msg_log::add("A door crashes open!");
                                }
                                else
                                {
                                    msg_log::add("The door crashes open!");
                                }
                            }
                            else //Cell not seen
                            {
                                msg_log::add("I feel a door crashing open!");
                            }

                            map::put(new RubbleLow(pos_));
                        }
                        else //Not broken
                        {
                            const SfxId sfx = is_secret_ ? SfxId::END : SfxId::door_bang;

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
                    else //No chance of success
                    {
                        if (is_cell_seen && !is_secret_)
                        {
                            Snd snd("",
                                    SfxId::door_bang,
                                    IgnoreMsgIfOriginSeen::no,
                                    pos_,
                                    actor,
                                    SndVol::low,
                                    AlertsMon::yes);

                            snd_emit::run(snd);

                            msg_log::add("It seems futile.",
                                         clr_msg_note,
                                         false,
                                         MorePromptOnMsg::yes);
                        }
                    }
                }
                else //Not player
                {
                    Fraction destr_chance(10 - (nr_spikes_ * 3), 100);

                    destr_chance.num = std::max(1, destr_chance.num);

                    if (is_weak)
                    {
                        destr_chance.num = 0;
                    }

                    if (destr_chance.roll())
                    {
                        Snd snd("I hear a door crashing open!",
                                SfxId::door_break,
                                IgnoreMsgIfOriginSeen::yes,
                                pos_,
                                actor,
                                SndVol::high,
                                AlertsMon::no);

                        snd_emit::run(snd);

                        if (map::player->can_see_actor(*actor))
                        {
                            msg_log::add("The door crashes open!");
                        }
                        else if (is_cell_seen)
                        {
                            msg_log::add("A door crashes open!");
                        }

                        map::put(new RubbleLow(pos_));
                    }
                    else //Not broken
                    {
                        Snd snd("I hear a loud banging on a door.",
                                SfxId::door_bang,
                                IgnoreMsgIfOriginSeen::no,
                                pos_,
                                actor,
                                SndVol::low,
                                AlertsMon::no);

                        snd_emit::run(snd);
                    }
                }

            }
            break;

            case Matl::metal:
                if (is_player && is_cell_seen && !is_secret_)
                {
                    msg_log::add("It seems futile.",
                                 clr_msg_note,
                                 false,
                                 MorePromptOnMsg::yes);
                }
                break;

            case Matl::empty:
            case Matl::cloth:
            case Matl::fluid:
            case Matl::plant:
            case Matl::stone:
                break;
            }
        }
    }

    //---------------------------------------------------------------------- FIRE
    if (dmg_method == DmgMethod::elemental)
    {
        if (dmg_type == DmgType::fire)
        {
            (void)actor;

            if (matl_ == Matl::wood)
            {
                try_start_burning(true);
            }
        }
    }

    //-------------------------------------- OLD

//    if(method == DmgMethod::kick) {
//      int skill_value_bash = 0;
//
//      if(!is_basher_weak) {
//        if(is_player) {
//          const int bon   = player_bon::traits[size_t(Trait::tough)] ? 20 : 0;
//          skill_value_bash  = 40 + bon - min(58, nr_spikes_ * 20);
//        } else {
//          skill_value_bash  = 10 - min(9, nr_spikes_ * 3);
//        }
//      }
//      const bool is_door_smashed =
//        (type_ == DoorType::metal || is_basher_weak) ? false :
//        rnd::percent() < skill_value_bash;
//
//      if(is_player && !is_secret_ && (type_ == DoorType::metal || is_basher_weak)) {
//        msg_log::add("It seems futile.");
//      }
//    }
//
//    if(is_door_smashed) {
//      TRACE << "Bash successful" << std::endl;
//      const bool is_secret_before = is_secret_;
//      is_stuck_  = false;
//      is_secret_ = false;
//      is_open_   = true;
//      if(is_player) {
//        Snd snd("", SfxId::door_break, IgnoreMsgIfOriginSeen::yes, pos_,
//                &actor, SndVol::low, AlertsMon::yes);
//        snd_emit::run(snd);
//        if(!actor.prop_handler().allow_see()) {
//          msg_log::add("I feel a door crashing open!");
//        } else {
//          if(is_secret_before) {
//            msg_log::add("A door crashes open!");
//          } else {
//            msg_log::add("The door crashes open!");
//          }
//        }
//      } else {
//        Snd snd("I hear a door crashing open!",
//                SfxId::door_break, IgnoreMsgIfOriginSeen::yes, pos_, &actor,
//                SndVol::high, AlertsMon::no);
//        snd_emit::run(snd);
//        if(map::player->can_see_actor(actor)) {
//          msg_log::add("The door crashes open!");
//        } else if(map::cells[pos_.x][pos_.y].is_seen_by_player) {
//          msg_log::add("A door crashes open!");
//        }
//      }
//    } else {
//      if(is_player) {
//        const SfxId sfx = is_secret_ ? SfxId::END : SfxId::door_bang;
//        Snd snd("", sfx, IgnoreMsgIfOriginSeen::yes, actor.pos,
//                &actor, SndVol::low, AlertsMon::yes);
//        snd_emit::run(snd);
//      } else {
//        //Emitting the sound from the actor instead of the door, because the
//        //sound message should be received even if the door is seen
//        Snd snd("I hear a loud banging on a door.",
//                SfxId::door_bang, IgnoreMsgIfOriginSeen::yes, actor.pos,
//                &actor, SndVol::low, AlertsMon::no);
//        snd_emit::run(snd);
//        if(map::player->can_see_actor(actor)) {
//          msg_log::add(actor.name_the() + " bashes at a door!");
//        }
//      }
//    }
//  }
}

WasDestroyed Door::on_finished_burning()
{
    if (map::is_pos_seen_by_player(pos_))
    {
        msg_log::add("The door burns down.");
    }

    RubbleLow* const rubble = new RubbleLow(pos_);
    rubble->set_has_burned();
    map::put(rubble);
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

    if (actor.has_prop(PropId::ethereal) || actor.has_prop(PropId::ooze))
    {
        return true;
    }

    return is_open_;
}

bool Door::is_los_passable() const
{
    return is_open_;
}

bool Door::is_projectile_passable() const
{
    return is_open_;
}

bool Door::is_smoke_passable() const
{
    return is_open_;
}


std::string Door::name(const Article article) const
{
    if (is_secret_)
    {
        return mimic_feature_->name(article);
    }

    std::string ret = "";

    if (burn_state() == BurnState::burning)
    {
        ret = article == Article::a ? "a " : "the ";
        ret += "burning ";
    }
    else
    {
        ret = article == Article::a ? (is_open_ ? "an " : "a ") : "the ";
    }

    ret += is_open_ ? "open " : "closed ";
    ret += matl_ == Matl::wood ? "wooden " : "metal ";

    return ret + "door";
}

Clr Door::clr_default() const
{
    if (is_secret_)
    {
        return mimic_feature_->clr();
    }
    else
    {
        switch (matl_)
        {
        case Matl::wood:
            return clr_brown_drk;
            break;

        case Matl::metal:
            return clr_gray;
            break;

        case Matl::empty:
        case Matl::cloth:
        case Matl::fluid:
        case Matl::plant:
        case Matl::stone:
            return clr_yellow;
            break;
        }
    }

    ASSERT(false && "Failed to get door color");
    return clr_gray;
}

char Door::glyph() const
{
    return is_secret_ ? mimic_feature_->glyph() : (is_open_ ? 39 : '+');
}

TileId Door::tile() const
{
    return is_secret_ ? mimic_feature_->tile() :
           (is_open_ ? TileId::door_open : TileId::door_closed);
}

Matl Door::matl() const
{
    return is_secret_ ? mimic_feature_->matl() : matl_;
}

void Door::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (is_secret_)
        {
            //Print messages as if this was a wall

            if (map::cells[pos_.x][pos_.y].is_seen_by_player)
            {
                TRACE << "Player bumped into secret door, with vision in cell" << std::endl;
                msg_log::add(feature_data::data(FeatureId::wall).msg_on_player_blocked);
            }
            else //Not seen by player
            {
                TRACE << "Player bumped into secret door, without vision in cell" << std::endl;
                msg_log::add(feature_data::data(FeatureId::wall).msg_on_player_blocked_blind);
            }

            return;
        }

        if (!is_open_)
        {
            try_open(&actor_bumping);
        }
    }
}

void Door::reveal(const bool allow_message)
{
    if (is_secret_)
    {
        is_secret_ = false;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            render::draw_map_state();

            if (allow_message)
            {
                msg_log::add("A secret is revealed.");
                render::draw_map_state();
            }
        }
    }
}

void Door::player_try_spot_hidden()
{
    if (is_secret_)
    {
        const int player_skill = map::player->ability(AbilityId::searching, true);

        if (ability_roll::roll(player_skill, map::player) >= success)
        {
            reveal(true);
        }
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

    //Door is in correct state for spiking (known, closed)
    ++nr_spikes_;
    is_stuck_ = true;

    if (is_player)
    {
        if (!tryer_is_blind)
        {
            msg_log::add("I jam the door with a spike.");
        }
        else
        {
            msg_log::add("I jam a door with a spike.");
        }
    }

    game_time::tick();
    return true;
}

void Door::try_close(Actor* actor_trying)
{
    const bool is_player        = actor_trying == map::player;
    const bool tryer_is_blind   = !actor_trying->prop_handler().allow_see();

    const bool player_see_tryer = is_player ? true :
                                  map::player->can_see_actor(*actor_trying);

    bool is_closable = true;

    if (is_handled_externally_)
    {
        if (is_player)
        {
            msg_log::add(
                "This door refuses to be closed, perhaps it is handled elsewhere?");
            render::draw_map_state();
        }

        return;
    }

    //Already closed?
    if (is_closable && !is_open_)
    {
        is_closable = false;

        if (is_player)
        {
            if (tryer_is_blind)
            {
                msg_log::add("I find nothing there to close.");
            }
            else //Can see
            {
                msg_log::add("I see nothing there to close.");
            }
        }
    }

    //Blocked?
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

        if (isblocked_by_actor || map::cells[pos_.x][pos_.y].item)
        {
            is_closable = false;

            if (is_player)
            {
                if (tryer_is_blind)
                {
                    msg_log::add("Something is blocking the door.");
                }
                else //Can see
                {
                    msg_log::add("The door is blocked.");
                }
            }
        }
    }

    if (is_closable)
    {
        //Door is in correct state for closing (open, working, not blocked)

        if (!tryer_is_blind)
        {
            is_open_ = false;

            if (is_player)
            {
                Snd snd("", SfxId::door_close, IgnoreMsgIfOriginSeen::yes, pos_,
                        actor_trying, SndVol::low, AlertsMon::yes);
                snd_emit::run(snd);
                msg_log::add("I close the door.");
            }
            else //Is a monster closing
            {
                Snd snd("I hear a door closing.",
                        SfxId::door_close, IgnoreMsgIfOriginSeen::yes, pos_, actor_trying,
                        SndVol::low, AlertsMon::no);
                snd_emit::run(snd);

                if (player_see_tryer)
                {
                    msg_log::add(actor_trying->name_the() + " closes a door.");
                }
            }
        }
        else //Cannot see
        {
            if (rnd::percent() < 50)
            {
                is_open_ = false;

                if (is_player)
                {
                    Snd snd("", SfxId::door_close, IgnoreMsgIfOriginSeen::yes, pos_,
                            actor_trying, SndVol::low, AlertsMon::yes);
                    snd_emit::run(snd);
                    msg_log::add("I fumble with a door and succeed to close it.");
                }
                else //Monster closing
                {
                    Snd snd("I hear a door closing.",
                            SfxId::door_close, IgnoreMsgIfOriginSeen::yes, pos_, actor_trying,
                            SndVol::low, AlertsMon::no);
                    snd_emit::run(snd);

                    if (player_see_tryer)
                    {
                        msg_log::add(actor_trying->name_the() +
                                     "fumbles about and succeeds to close a door.");
                    }
                }
            }
            else //Fail to close
            {
                if (is_player)
                {
                    msg_log::add("I fumble blindly with a door and fail to close it.");
                }
                else //Monster failing to close
                {
                    if (player_see_tryer)
                    {
                        msg_log::add(actor_trying->name_the() +
                                     " fumbles blindly and fails to close a door.");
                    }
                }
            }
        }
    }

    //TODO: It doesn't seem like a turn is spent if player is blind and fail to close the door?
    if (!is_open_ && is_closable)
    {
        game_time::tick();
    }
}

void Door::try_open(Actor* actor_trying)
{
    TRACE_FUNC_BEGIN;
    const bool is_player        = actor_trying == map::player;
    const bool player_see_door  = map::cells[pos_.x][pos_.y].is_seen_by_player;

    const bool player_see_tryer = is_player ? true :
                                  map::player->can_see_actor(*actor_trying);

    if (is_handled_externally_)
    {
        if (is_player)
        {
            msg_log::add("I see no way to open this door, perhaps it is opened elsewhere.");
            render::draw_map_state();
        }

        return;
    }

    if (is_stuck_)
    {
        TRACE << "Is stuck" << std::endl;

        if (is_player)
        {
            msg_log::add("The door seems to be stuck.");
        }

    }
    else //Not stuck
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
                msg_log::add("I open the door.");
            }
            else //Is monster
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
                    msg_log::add(actor_trying->name_the() + " opens a door.");
                }
                else if (player_see_door)
                {
                    msg_log::add("I see a door opening.");
                }
            }
        }
        else //Tryer is blind
        {
            if (rnd::percent() < 50)
            {
                TRACE << "Tryer is blind, but open succeeded anyway" << std::endl;
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

                    msg_log::add("I fumble with a door and succeed to open it.");
                }
                else //Is monster
                {
                    Snd snd("I hear something open a door clumsily.",
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
                                     "fumbles about and succeeds to open a door.");
                    }
                    else if (player_see_door)
                    {
                        msg_log::add("I see a door open clumsily.");
                    }
                }
            }
            else //Failed to open
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

                    msg_log::add("I fumble blindly with a door and fail to open it.");
                }
                else //Is monster
                {
                    //Emitting the sound from the actor instead of the door, because the
                    //sound message should be received even if the door is seen
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
                                     " fumbles blindly and fails to open a door.");
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
            reveal(true);
        }

        TRACE << "Calling game_time::end_turn_of_current_actor()" << std::endl;
        game_time::tick();
    }
}

DidOpen Door::open(Actor* const actor_opening)
{
    (void)actor_opening;

    is_open_   = true;
    is_secret_ = false;
    is_stuck_  = false;
    return DidOpen::yes;
}
