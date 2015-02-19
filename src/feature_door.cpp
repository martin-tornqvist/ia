#include "feature_door.hpp"

#include "init.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "feature_data.hpp"
#include "map.hpp"
#include "log.hpp"
#include "postmortem.hpp"
#include "player_bon.hpp"
#include "render.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"

using namespace std;

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(const Pos& pos, const Rigid* const mimic_feature, Door_spawn_state spawn_state) :
    Rigid         (pos),
    mimic_feature_ (mimic_feature),
    nr_spikes_     (0)
{

    is_handled_externally_ = false;

    const int ROLL = Rnd::percent();

    const Door_spawn_state door_state = spawn_state == Door_spawn_state::any ?
                                     (ROLL < 5  ? Door_spawn_state::secret_and_stuck  :
                                      ROLL < 40 ? Door_spawn_state::secret          :
                                      ROLL < 50 ? Door_spawn_state::stuck           :
                                      ROLL < 75 ? Door_spawn_state::open            :
                                      Door_spawn_state::closed)                     :
                                     spawn_state;

    switch (Door_spawn_state(door_state))
    {
    case Door_spawn_state::open:
    {
        is_open_   = true;
        is_stuck_  = false;
        is_secret_ = false;
    } break;

    case Door_spawn_state::closed:
    {
        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = false;
    } break;

    case Door_spawn_state::stuck:
    {
        is_open_   = false;
        is_stuck_  = true;
        is_secret_ = false;
    } break;

    case Door_spawn_state::secret:
    {
        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = true;
    } break;

    case Door_spawn_state::secret_and_stuck:
    {
        is_open_   = false;
        is_stuck_  = true;
        is_secret_ = true;
    } break;

    case Door_spawn_state::any:
    {
        assert(false && "Should not happen");
        is_open_   = false;
        is_stuck_  = false;
        is_secret_ = false;
    }
    }

    matl_ = Matl::wood;
}

Door::~Door()
{
    if (mimic_feature_) {delete mimic_feature_;}
}

void Door::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::physical)
    {
        if (dmg_method == Dmg_method::forced)
        {
            //---------------------------------------------------------------------- FORCED
            (void)actor;
            Map::put(new Rubble_low(pos_));
        }

        //---------------------------------------------------------------------- SHOTGUN
        if (dmg_method == Dmg_method::shotgun)
        {
            (void)actor;
            if (!is_open_)
            {
                switch (matl_)
                {
                case Matl::wood:
                {
                    if (Rnd::fraction(7, 10))
                    {
                        if (Map::is_pos_seen_by_player(pos_))
                        {
                            const string a = is_secret_ ? "A" : "The";
                            Log::add_msg(a + " door is blown to splinters!");
                        }
                        Map::put(new Rubble_low(pos_));
                    }
                } break;

                case Matl::empty:
                case Matl::cloth:
                case Matl::fluid:
                case Matl::plant:
                case Matl::stone:
                case Matl::metal: {} break;
                }
            }
        }

        //---------------------------------------------------------------------- EXPLOSION
        if (dmg_method == Dmg_method::explosion)
        {
            (void)actor;
            //TODO
        }

        //---------------------------------------------------------------------- HEAVY BLUNT
        if (dmg_method == Dmg_method::blunt_heavy)
        {
            assert(actor);
            switch (matl_)
            {
            case Matl::wood:
            {
                Fraction destr_chance(6, 10);
                if (actor == Map::player)
                {
                    if (Player_bon::traits[int(Trait::tough)])
                    {
                        destr_chance.numerator += 2;
                    }
                    if (Player_bon::traits[int(Trait::rugged)])
                    {
                        destr_chance.numerator += 2;
                    }
                    if (Player_bon::traits[int(Trait::unbreakable)])
                    {
                        destr_chance.numerator += 2;
                    }

                    if (Rnd::fraction(destr_chance))
                    {

                    }
                }
                else
                {
                    if (Rnd::fraction(destr_chance))
                    {

                    }
                }
            } break;

            case Matl::empty:
            case Matl::cloth:
            case Matl::fluid:
            case Matl::plant:
            case Matl::stone:
            case Matl::metal: {} break;
            }
        }

        //---------------------------------------------------------------------- KICK
        if (dmg_method == Dmg_method::kick)
        {
            assert(actor);

            const bool IS_PLAYER    = actor == Map::player;
            const bool IS_CELL_SEEN = Map::is_pos_seen_by_player(pos_);

            bool props[size_t(Prop_id::END)];
            if (actor) {actor->get_prop_handler().get_prop_ids(props);}

            const bool IS_WEAK = props[int(Prop_id::weakened)];

            switch (matl_)
            {
            case Matl::wood:
            {
                if (IS_PLAYER)
                {
                    Fraction destr_chance(4 - nr_spikes_, 10);
                    destr_chance.numerator = max(1, destr_chance.numerator);

                    if (Player_bon::traits[int(Trait::tough)])
                    {
                        destr_chance.numerator += 2;
                    }
                    if (Player_bon::traits[int(Trait::rugged)])
                    {
                        destr_chance.numerator += 2;
                    }
                    if (Player_bon::traits[int(Trait::unbreakable)])
                    {
                        destr_chance.numerator += 2;
                    }

                    if (IS_WEAK) {destr_chance.numerator = 0;}

                    if (destr_chance.numerator > 0)
                    {
                        if (Rnd::fraction(destr_chance))
                        {
                            Snd snd("", Sfx_id::door_break, Ignore_msg_if_origin_seen::yes, pos_,
                                    actor, Snd_vol::low, Alerts_mon::yes);
                            Snd_emit::emit_snd(snd);
                            if (IS_CELL_SEEN)
                            {
                                if (is_secret_)
                                {
                                    Log::add_msg("A door crashes open!");
                                }
                                else
                                {
                                    Log::add_msg("The door crashes open!");
                                }
                            }
                            else
                            {
                                Log::add_msg("I feel a door crashing open!");
                            }
                            Map::put(new Rubble_low(pos_));
                        }
                        else //Not broken
                        {
                            const Sfx_id sfx = is_secret_ ? Sfx_id::END : Sfx_id::door_bang;
                            Snd_emit::emit_snd({"", sfx, Ignore_msg_if_origin_seen::no, pos_, actor,
                                              Snd_vol::low, Alerts_mon::yes
                                             });
                        }
                    }
                    else //No chance of success
                    {
                        if (IS_CELL_SEEN && !is_secret_)
                        {
                            Snd_emit::emit_snd({"", Sfx_id::door_bang, Ignore_msg_if_origin_seen::no, pos_,
                                              actor, Snd_vol::low, Alerts_mon::yes
                                             });
                            Log::add_msg("It seems futile.", clr_msg_note, false, true);
                        }
                    }
                }
                else //Not player
                {
                    Fraction destr_chance(10 - (nr_spikes_ * 3), 100);
                    destr_chance.numerator = max(1, destr_chance.numerator);

                    if (IS_WEAK) {destr_chance.numerator = 0;}

                    if (Rnd::fraction(destr_chance))
                    {
                        Snd snd("I hear a door crashing open!",
                                Sfx_id::door_break, Ignore_msg_if_origin_seen::yes, pos_, actor,
                                Snd_vol::high, Alerts_mon::no);
                        Snd_emit::emit_snd(snd);
                        if (Map::player->can_see_actor(*actor, nullptr))
                        {
                            Log::add_msg("The door crashes open!");
                        }
                        else if (IS_CELL_SEEN)
                        {
                            Log::add_msg("A door crashes open!");
                        }
                        Map::put(new Rubble_low(pos_));
                    }
                    else //Not broken
                    {
                        Snd snd("I hear a loud banging on a door.",
                                Sfx_id::door_bang, Ignore_msg_if_origin_seen::no, pos_,
                                actor, Snd_vol::low, Alerts_mon::no);
                        Snd_emit::emit_snd(snd);
                    }
                }

            } break;

            case Matl::metal:
            {
                if (IS_PLAYER && IS_CELL_SEEN && !is_secret_)
                {
                    Log::add_msg("It seems futile.", clr_msg_note, false, true);
                }
            } break;

            case Matl::empty:
            case Matl::cloth:
            case Matl::fluid:
            case Matl::plant:
            case Matl::stone: {} break;
            }
        }
    }

    //---------------------------------------------------------------------- FIRE
    if (dmg_method == Dmg_method::elemental)
    {
        if (dmg_type == Dmg_type::fire)
        {
            (void)actor;
            if (matl_ == Matl::wood)
            {
                try_start_burning(true);
            }
        }
    }

    //-------------------------------------- OLD

//    if(method == Dmg_method::kick) {
//      int skill_value_bash = 0;
//
//      if(!IS_BASHER_WEAK) {
//        if(IS_PLAYER) {
//          const int BON   = Player_bon::traits[int(Trait::tough)] ? 20 : 0;
//          skill_value_bash  = 40 + BON - min(58, nr_spikes_ * 20);
//        } else {
//          skill_value_bash  = 10 - min(9, nr_spikes_ * 3);
//        }
//      }
//      const bool IS_DOOR_SMASHED =
//        (type_ == Door_type::metal || IS_BASHER_WEAK) ? false :
//        Rnd::percent() < skill_value_bash;
//
//      if(IS_PLAYER && !is_secret_ && (type_ == Door_type::metal || IS_BASHER_WEAK)) {
//        Log::add_msg("It seems futile.");
//      }
//    }
//
//    if(IS_DOOR_SMASHED) {
//      TRACE << "Bash successful" << endl;
//      const bool IS_SECRET_BEFORE = is_secret_;
//      is_stuck_  = false;
//      is_secret_ = false;
//      is_open_   = true;
//      if(IS_PLAYER) {
//        Snd snd("", Sfx_id::door_break, Ignore_msg_if_origin_seen::yes, pos_,
//                &actor, Snd_vol::low, Alerts_mon::yes);
//        Snd_emit::emit_snd(snd);
//        if(!actor.get_prop_handler().allow_see()) {
//          Log::add_msg("I feel a door crashing open!");
//        } else {
//          if(IS_SECRET_BEFORE) {
//            Log::add_msg("A door crashes open!");
//          } else {
//            Log::add_msg("The door crashes open!");
//          }
//        }
//      } else {
//        Snd snd("I hear a door crashing open!",
//                Sfx_id::door_break, Ignore_msg_if_origin_seen::yes, pos_, &actor,
//                Snd_vol::high, Alerts_mon::no);
//        Snd_emit::emit_snd(snd);
//        if(Map::player->can_see_actor(actor, nullptr)) {
//          Log::add_msg("The door crashes open!");
//        } else if(Map::cells[pos_.x][pos_.y].is_seen_by_player) {
//          Log::add_msg("A door crashes open!");
//        }
//      }
//    } else {
//      if(IS_PLAYER) {
//        const Sfx_id sfx = is_secret_ ? Sfx_id::END : Sfx_id::door_bang;
//        Snd snd("", sfx, Ignore_msg_if_origin_seen::yes, actor.pos,
//                &actor, Snd_vol::low, Alerts_mon::yes);
//        Snd_emit::emit_snd(snd);
//      } else {
//        //Emitting the sound from the actor instead of the door, because the
//        //sound message should be received even if the door is seen
//        Snd snd("I hear a loud banging on a door.",
//                Sfx_id::door_bang, Ignore_msg_if_origin_seen::yes, actor.pos,
//                &actor, Snd_vol::low, Alerts_mon::no);
//        Snd_emit::emit_snd(snd);
//        if(Map::player->can_see_actor(actor, nullptr)) {
//          Log::add_msg(actor.get_name_the() + " bashes at a door!");
//        }
//      }
//    }
//  }
}

Was_destroyed Door::on_finished_burning()
{
    if (Map::is_pos_seen_by_player(pos_))
    {
        Log::add_msg("The door burns down.");
    }
    Rubble_low* const rubble = new Rubble_low(pos_);
    rubble->set_has_burned();
    Map::put(rubble);
    return Was_destroyed::yes;
}

bool Door::can_move_cmn() const {return is_open_;}

bool Door::can_move(const bool actor_prop_ids[size_t(Prop_id::END)]) const
{
    if (is_open_)
    {
        return true;
    }

    if (actor_prop_ids[int(Prop_id::ethereal)] || actor_prop_ids[int(Prop_id::ooze)])
    {
        return true;
    }

    return is_open_;
}

bool Door::is_los_passable()     const {return is_open_;}
bool Door::is_projectile_passable() const {return is_open_;}
bool Door::is_smoke_passable()      const {return is_open_;}

string Door::get_name(const Article article) const
{
    if (is_secret_) {return mimic_feature_->get_name(article);}

    string ret = "";

    if (get_burn_state() == Burn_state::burning)
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

Clr Door::get_clr_() const
{
    if (is_secret_)
    {
        return mimic_feature_->get_clr();
    }
    else
    {
        switch (matl_)
        {
        case Matl::wood:      return clr_brown_drk; break;
        case Matl::metal:     return clr_gray;     break;
        case Matl::empty:
        case Matl::cloth:
        case Matl::fluid:
        case Matl::plant:
        case Matl::stone:     return clr_yellow;   break;
        }
    }
    assert(false && "Failed to get door color");
    return clr_gray;
}

char Door::get_glyph() const
{
    return is_secret_ ? mimic_feature_->get_glyph() : (is_open_ ? 39 : '+');
}

Tile_id Door::get_tile() const
{
    return is_secret_ ? mimic_feature_->get_tile() :
           (is_open_ ? Tile_id::door_open : Tile_id::door_closed);
}

Matl Door::get_matl() const
{
    return is_secret_ ? mimic_feature_->get_matl() : matl_;
}

void Door::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (is_secret_)
        {
            if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
            {
                TRACE << "Player bumped into secret door, with vision in cell" << endl;
                Log::add_msg("That way is blocked.");
            }
            else
            {
                TRACE << "Player bumped into secret door, without vision in cell" << endl;
                Log::add_msg("I bump into something.");
            }
            return;
        }

        if (!is_open_) {try_open(&actor_bumping);}
    }
}

void Door::reveal(const bool ALLOW_MESSAGE)
{
    if (is_secret_)
    {
        is_secret_ = false;
        if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            Render::draw_map_and_interface();
            if (ALLOW_MESSAGE)
            {
                Log::add_msg("A secret is revealed.");
                Render::draw_map_and_interface();
            }
        }
    }
}

void Door::player_try_spot_hidden()
{
    if (is_secret_)
    {
        const int PLAYER_SKILL = Map::player->get_data().ability_vals.get_val(
                                     Ability_id::searching, true, *(Map::player));
        if (Ability_roll::roll(PLAYER_SKILL) >= success_small) {reveal(true);}
    }
}

bool Door::try_spike(Actor* actor_trying)
{
    const bool IS_PLAYER = actor_trying == Map::player;
    const bool TRYER_IS_BLIND = !actor_trying->get_prop_handler().allow_see();

    if (is_secret_ || is_open_) {return false;}

    //Door is in correct state for spiking (known, closed)
    nr_spikes_++;
    is_stuck_ = true;

    if (IS_PLAYER)
    {
        if (!TRYER_IS_BLIND)
        {
            Log::add_msg("I jam the door with a spike.");
        }
        else
        {
            Log::add_msg("I jam a door with a spike.");
        }
    }
    Game_time::tick();
    return true;
}

void Door::try_close(Actor* actor_trying)
{
    const bool IS_PLAYER = actor_trying == Map::player;
    const bool TRYER_IS_BLIND = !actor_trying->get_prop_handler().allow_see();
    //const bool PLAYER_SEE_DOOR    = Map::player_vision[pos_.x][pos_.y];
    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_los(), blocked);

    const bool PLAYER_SEE_TRYER =
        IS_PLAYER ? true :
        Map::player->can_see_actor(*actor_trying, blocked);

    bool is_closable = true;

    if (is_handled_externally_)
    {
        if (IS_PLAYER)
        {
            Log::add_msg(
                "This door refuses to be closed, perhaps it is handled elsewhere?");
            Render::draw_map_and_interface();
        }
        return;
    }

    //Already closed?
    if (is_closable)
    {
        if (!is_open_)
        {
            is_closable = false;
            if (IS_PLAYER)
            {
                if (!TRYER_IS_BLIND)
                    Log::add_msg("I see nothing there to close.");
                else Log::add_msg("I find nothing there to close.");
            }
        }
    }

    //Blocked?
    if (is_closable)
    {
        bool isblocked_by_actor = false;
        for (Actor* actor : Game_time::actors_)
        {
            if (actor->pos == pos_)
            {
                isblocked_by_actor = true;
                break;
            }
        }
        if (isblocked_by_actor || Map::cells[pos_.x][pos_.y].item)
        {
            is_closable = false;
            if (IS_PLAYER)
            {
                if (!TRYER_IS_BLIND)
                {
                    Log::add_msg("The door is blocked.");
                }
                else
                {
                    Log::add_msg("Something is blocking the door.");
                }
            }
        }
    }

    if (is_closable)
    {
        //Door is in correct state for closing (open, working, not blocked)

        if (!TRYER_IS_BLIND)
        {
            is_open_ = false;
            if (IS_PLAYER)
            {
                Snd snd("", Sfx_id::door_close, Ignore_msg_if_origin_seen::yes, pos_,
                        actor_trying, Snd_vol::low, Alerts_mon::yes);
                Snd_emit::emit_snd(snd);
                Log::add_msg("I close the door.");
            }
            else
            {
                Snd snd("I hear a door closing.",
                        Sfx_id::door_close, Ignore_msg_if_origin_seen::yes, pos_, actor_trying,
                        Snd_vol::low, Alerts_mon::no);
                Snd_emit::emit_snd(snd);
                if (PLAYER_SEE_TRYER)
                {
                    Log::add_msg(actor_trying->get_name_the() + " closes a door.");
                }
            }
        }
        else
        {
            if (Rnd::percent() < 50)
            {
                is_open_ = false;
                if (IS_PLAYER)
                {
                    Snd snd("", Sfx_id::door_close, Ignore_msg_if_origin_seen::yes, pos_,
                            actor_trying, Snd_vol::low, Alerts_mon::yes);
                    Snd_emit::emit_snd(snd);
                    Log::add_msg("I fumble with a door and succeed to close it.");
                }
                else
                {
                    Snd snd("I hear a door closing.",
                            Sfx_id::door_close, Ignore_msg_if_origin_seen::yes, pos_, actor_trying,
                            Snd_vol::low, Alerts_mon::no);
                    Snd_emit::emit_snd(snd);
                    if (PLAYER_SEE_TRYER)
                    {
                        Log::add_msg(actor_trying->get_name_the() +
                                    "fumbles about and succeeds to close a door.");
                    }
                }
            }
            else
            {
                if (IS_PLAYER)
                {
                    Log::add_msg(
                        "I fumble blindly with a door and fail to close it.");
                }
                else
                {
                    if (PLAYER_SEE_TRYER)
                    {
                        Log::add_msg(actor_trying->get_name_the() +
                                    " fumbles blindly and fails to close a door.");
                    }
                }
            }
        }
    }

    if (!is_open_ && is_closable) {Game_time::tick();}
}

void Door::try_open(Actor* actor_trying)
{
    TRACE_FUNC_BEGIN;
    const bool IS_PLAYER        = actor_trying == Map::player;
    const bool TRYER_IS_BLIND   = !actor_trying->get_prop_handler().allow_see();
    const bool PLAYER_SEE_DOOR  = Map::cells[pos_.x][pos_.y].is_seen_by_player;
    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_los(), blocked);

    const bool PLAYER_SEE_TRYER =
        IS_PLAYER ? true : Map::player->can_see_actor(*actor_trying, blocked);

    if (is_handled_externally_)
    {
        if (IS_PLAYER)
        {
            Log::add_msg(
                "I see no way to open this door, perhaps it is opened elsewhere?");
            Render::draw_map_and_interface();
        }
        return;
    }

    if (is_stuck_)
    {
        TRACE << "Is stuck" << endl;

        if (IS_PLAYER)
        {
            Log::add_msg("The door seems to be stuck.");
        }

    }
    else
    {
        TRACE << "Is not stuck" << endl;
        if (!TRYER_IS_BLIND)
        {
            TRACE << "Tryer can see, opening" << endl;
            is_open_ = true;
            if (IS_PLAYER)
            {
                Snd snd("", Sfx_id::door_open, Ignore_msg_if_origin_seen::yes, pos_,
                        actor_trying, Snd_vol::low, Alerts_mon::yes);
                Snd_emit::emit_snd(snd);
                Log::add_msg("I open the door.");
            }
            else
            {
                Snd snd("I hear a door open.", Sfx_id::door_open,
                        Ignore_msg_if_origin_seen::yes, pos_, actor_trying, Snd_vol::low,
                        Alerts_mon::no);
                Snd_emit::emit_snd(snd);
                if (PLAYER_SEE_TRYER)
                {
                    Log::add_msg(actor_trying->get_name_the() + " opens a door.");
                }
                else if (PLAYER_SEE_DOOR)
                {
                    Log::add_msg("I see a door opening.");
                }
            }
        }
        else
        {
            if (Rnd::percent() < 50)
            {
                TRACE << "Tryer is blind, but open succeeded anyway" << endl;
                is_open_ = true;
                if (IS_PLAYER)
                {
                    Snd snd("", Sfx_id::door_open, Ignore_msg_if_origin_seen::yes, pos_,
                            actor_trying, Snd_vol::low, Alerts_mon::yes);
                    Snd_emit::emit_snd(snd);
                    Log::add_msg("I fumble with a door and succeed to open it.");
                }
                else
                {
                    Snd snd("I hear something open a door clumsily.", Sfx_id::door_open,
                            Ignore_msg_if_origin_seen::yes, pos_, actor_trying, Snd_vol::low,
                            Alerts_mon::no);
                    Snd_emit::emit_snd(snd);
                    if (PLAYER_SEE_TRYER)
                    {
                        Log::add_msg(actor_trying->get_name_the() +
                                    "fumbles about and succeeds to open a door.");
                    }
                    else if (PLAYER_SEE_DOOR)
                    {
                        Log::add_msg("I see a door open clumsily.");
                    }
                }
            }
            else
            {
                TRACE << "Tryer is blind, and open failed" << endl;
                if (IS_PLAYER)
                {
                    Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes, pos_,
                            actor_trying, Snd_vol::low, Alerts_mon::yes);
                    Snd_emit::emit_snd(snd);
                    Log::add_msg("I fumble blindly with a door and fail to open it.");
                }
                else
                {
                    //Emitting the sound from the actor instead of the door, because the
                    //sound message should be received even if the door is seen
                    Snd snd("I hear something attempting to open a door.", Sfx_id::END,
                            Ignore_msg_if_origin_seen::yes, actor_trying->pos, actor_trying,
                            Snd_vol::low, Alerts_mon::no);
                    Snd_emit::emit_snd(snd);
                    if (PLAYER_SEE_TRYER)
                    {
                        Log::add_msg(actor_trying->get_name_the() +
                                    " fumbles blindly and fails to open a door.");
                    }
                }
                Game_time::tick();
            }
        }
    }

    if (is_open_)
    {
        TRACE << "Open was successful" << endl;
        if (is_secret_)
        {
            TRACE << "Was secret, now revealing" << endl;
            reveal(true);
        }
        TRACE << "Calling Game_time::end_turn_of_cur_actor()" << endl;
        Game_time::tick();
    }
}

Did_open Door::open(Actor* const actor_opening)
{
    (void)actor_opening;

    is_open_   = true;
    is_secret_ = false;
    is_stuck_  = false;
    return Did_open::yes;
}
