#include "actor_mon.hpp"

#include <vector>
#include <cassert>

#include "init.hpp"
#include "item.hpp"
#include "actor_player.hpp"
#include "game_time.hpp"
#include "attack.hpp"
#include "reload.hpp"
#include "inventory.hpp"
#include "feature_trap.hpp"
#include "feature_mob.hpp"
#include "properties.hpp"
#include "render.hpp"
#include "sound.hpp"
#include "utils.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "map_parsing.hpp"
#include "ai.hpp"
#include "line_calc.hpp"
#include "gods.hpp"
#include "item_factory.hpp"
#include "actor_factory.hpp"
#include "knockback.hpp"
#include "explosion.hpp"
#include "popup.hpp"
#include "fov.hpp"
#include "text_format.hpp"

Mon::Mon() :
    Actor                       (),
    aware_counter_              (0),
    player_aware_of_me_counter_ (0),
    is_msg_mon_in_view_printed_ (false),
    last_dir_moved_             (Dir::center),
    spell_cool_down_cur_        (0),
    is_roaming_allowed_         (true),
    is_sneaking_                (false),
    leader_                     (nullptr),
    tgt_                        (nullptr),
    waiting_                    (false),
    shock_caused_cur_           (0.0),
    has_given_xp_for_spotting_  (false) {}

Mon::~Mon()
{
    for (Spell* const spell : spells_known_)
    {
        delete spell;
    }
}

void Mon::on_actor_turn()
{
    if (aware_counter_ > 0)
    {
        --aware_counter_;
    }
}

//TODO: Some of the things done in this function should probably be moved to
//"on_actor_turn()" instead. The purpose of this function ("act()") is only to
//tell the actor to "do something".
void Mon::act()
{
#ifndef NDEBUG
    //Sanity check - verify that monster is not outside the map
    if (!utils::is_pos_inside_map(pos, false))
    {
        TRACE << "Monster outside map" << std::endl;
        assert(false);
    }

    //Sanity check - verify that monster's leader does not have a leader (never allowed)
    if (
        leader_ && !is_actor_my_leader(map::player) &&  //Has a leader which is a monster?
        static_cast<Mon*>(leader_)->leader_)            //Leader has a leader?
    {
        TRACE << "Two (or more) steps of leader is never allowed" << std::endl;
        assert(false);
    }
#endif // NDEBUG

    if (aware_counter_ <= 0 && !is_actor_my_leader(map::player))
    {
        waiting_ = !waiting_;

        if (waiting_)
        {
            game_time::tick();
            return;
        }
    }
    else //Is aware, or player is leader
    {
        waiting_ = false;
    }

    //Pick a target
    std::vector<Actor*> tgt_bucket;

    if (prop_handler_->has_prop(Prop_id::conflict))
    {
        //Monster is conflicted (e.g. by player ring/amulet)
        tgt_bucket = game_time::actors;

        bool hard_blocked_los[MAP_W][MAP_H];

        const Rect fov_rect = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       hard_blocked_los,
                       Map_parse_mode::overwrite,
                       fov_rect);

        //Remove self and all unseen actors from vector
        for (auto it = begin(tgt_bucket); it != end(tgt_bucket);)
        {
            if (*it == this || !can_see_actor(**it, hard_blocked_los))
            {
                tgt_bucket.erase(it);
            }
            else //Is a seen actor
            {
                ++it;
            }
        }
    }
    else //Not conflicted
    {
        seen_foes(tgt_bucket);

        //If not aware, remove player from target bucket
        if (aware_counter_ <= 0)
        {
            for (auto it = begin(tgt_bucket); it != end(tgt_bucket); ++it)
            {
                if (*it == map::player)
                {
                    tgt_bucket.erase(it);
                    break;
                }
            }
        }
    }

    tgt_ = utils::random_closest_actor(pos, tgt_bucket);

    if (spell_cool_down_cur_ != 0)
    {
        --spell_cool_down_cur_;
    }

    if (aware_counter_ > 0)
    {
        is_roaming_allowed_ = true;

        if (leader_)
        {
            if (leader_->is_alive() && !is_actor_my_leader(map::player))
            {
                static_cast<Mon*>(leader_)->aware_counter_ =
                    leader_->data().nr_turns_aware;
            }
        }
        else //Monster does not have a leader
        {
            if (is_alive() && rnd::one_in(12))
            {
                speak_phrase(Alerts_mon::no);
            }
        }
    }

    is_sneaking_ = !is_actor_my_leader(map::player)         &&
                   ability(Ability_id::stealth, true) > 0   &&
                   !map::player->can_see_actor(*this);

    //Array used for AI purposes, e.g. to prevent tactically bad positions,
    //or prevent certain monsters from walking on a certain type of cells, etc.
    //This is checked in all AI movement functions. Cells set to true are
    //totally forbidden for the monster to move into.
    bool ai_special_blockers[MAP_W][MAP_H];
    ai::info::set_special_blocked_cells(*this, ai_special_blockers);

    //------------------------------ SPECIAL MONSTER ACTIONS
    //                               (ZOMBIES RISING, WORMS MULTIPLYING...)
    if (leader_ != map::player && (tgt_ == nullptr || tgt_ == map::player))
    {
        if (on_act() == Did_action::yes)
        {
            return;
        }
    }

    //------------------------------ COMMON ACTIONS
    //                               (MOVING, ATTACKING, CASTING SPELLS...)

    //Looking is as an action if monster was not aware before, and became aware from looking.
    //(This is to give the monsters some reaction time, and not instantly attack)
    if (
        data_->ai[size_t(Ai_id::looks)] &&
        leader_ != map::player          &&
        (tgt_ == nullptr || tgt_ == map::player))
    {
        if (ai::info::look_become_player_aware(*this))
        {
            return;
        }
    }

    if (data_->ai[size_t(Ai_id::makes_room_for_friend)] && tgt_ == map::player)
    {
        if (ai::action::make_room_for_friend(*this))
        {
            return;
        }
    }

    if (rnd::one_in(5))
    {
        if (ai::action::try_cast_random_spell(*this))
        {
            return;
        }
    }

    if (data_->ai[size_t(Ai_id::attacks)] && tgt_)
    {
        if (try_attack(*tgt_))
        {
            return;
        }
    }

    if (ai::action::try_cast_random_spell(*this))
    {
        return;
    }

    int erratic_move_pct = int(data_->erratic_move_pct);

    //Never move erratically if frenzied
    if (prop_handler_->has_prop(Prop_id::frenzied))
    {
        erratic_move_pct = 0;
    }

    //Move less erratically if allied to player
    if (is_actor_my_leader(map::player))
    {
        erratic_move_pct /= 2;
    }

    //Move more erratically if confused
    if (prop_handler_->has_prop(Prop_id::confused))
    {
        erratic_move_pct *= 2;
    }

    utils::set_constr_in_range(0, erratic_move_pct, 95);

    //Occasionally move erratically
    if (
        data_->ai[size_t(Ai_id::moves_to_random_when_unaware)] &&
        rnd::percent(erratic_move_pct))
    {
        if (ai::action::move_to_random_adj_cell(*this))
        {
            return;
        }
    }

    const bool IS_TERRIFIED = prop_handler_->has_prop(Prop_id::terrified);

    if (data_->ai[size_t(Ai_id::moves_to_tgt_when_los)] && !IS_TERRIFIED)
    {
        if (ai::action::move_to_tgt_simple(*this))
        {
            return;
        }
    }

    std::vector<P> path;

    if (
        data_->ai[size_t(Ai_id::paths_to_tgt_when_aware)]   &&
        leader_ != map::player                              &&
        !IS_TERRIFIED)
    {
        ai::info::try_set_path_to_player(*this, path);
    }

    if (leader_ != map::player)
    {
        if (ai::action::handle_closed_blocking_door(*this, path))
        {
            return;
        }
    }

    if (ai::action::step_path(*this, path))
    {
        return;
    }

    if (data_->ai[size_t(Ai_id::moves_to_leader)] && !IS_TERRIFIED)
    {
        ai::info::try_set_path_to_leader(*this, path);

        if (ai::action::step_path(*this, path))
        {
            return;
        }
    }

    if (
        data_->ai[size_t(Ai_id::moves_to_lair)] &&
        leader_ != map::player                  &&
        (tgt_ == nullptr || tgt_ == map::player))
    {
        if (ai::action::step_to_lair_if_los(*this, lair_pos_))
        {
            return;
        }
        else //No LOS to lair
        {
            //Try to use pathfinder to travel to lair
            ai::info::try_set_path_to_lair_if_no_los(*this, path, lair_pos_);

            if (ai::action::step_path(*this, path))
            {
                return;
            }
        }
    }

    //When unaware, move randomly
    if (data_->ai[size_t(Ai_id::moves_to_random_when_unaware)])
    {
        if (ai::action::move_to_random_adj_cell(*this))
        {
            return;
        }
    }

    //No action could be performed, just let someone else act
    game_time::tick();
}

bool Mon::can_see_actor(const Actor& other, const bool hard_blocked_los[MAP_W][MAP_H]) const
{
    if (this == &other || !other.is_alive())
    {
        return true;
    }

    //Outside FOV range?
    if (!fov::is_in_fov_range(pos, other.pos))
    {
        //Other actor is outside FOV range
        return false;
    }

    //Monster allied to player looking at other monster which is hidden?
    if (
        is_actor_my_leader(map::player) &&
        !other.is_player()              &&
        static_cast<const Mon*>(&other)->is_sneaking_)
    {
        return false;
    }

    //Monster is blind?
    if (!prop_handler_->allow_see())
    {
        return false;
    }

    const Los_result los = fov::check_cell(pos, other.pos, hard_blocked_los);

    //LOS blocked hard (e.g. a wall)?
    if (los.is_blocked_hard)
    {
        return false;
    }

    const bool CAN_SEE_INVIS = has_prop(Prop_id::see_invis);

    //Actor is invisible, and monster cannot see invisible?
    if (other.has_prop(Prop_id::invis) && !CAN_SEE_INVIS)
    {
        return false;
    }

    bool        HAS_INFRAVIS                = has_prop(Prop_id::infravis);
    const bool  IS_OTHER_INFRA_VISIBLE      = other.data().is_infra_visible;

    const bool  CAN_SEE_ACTOR_WITH_INFRAVIS = HAS_INFRAVIS && IS_OTHER_INFRA_VISIBLE;

    const bool  CAN_SEE_OTHER_IN_DRK        = CAN_SEE_INVIS || CAN_SEE_ACTOR_WITH_INFRAVIS;

    //Blocked by darkness, and not seeing actor with infravision?
    if (los.is_blocked_by_drk && !CAN_SEE_OTHER_IN_DRK)
    {
        return false;
    }

    //OK, all checks passed, actor can bee seen!
    return true;
}

void Mon::on_std_turn()
{
    on_std_turn_hook();
}

void Mon::on_hit(int& dmg,
                 const Dmg_type dmg_type,
                 const Dmg_method method,
                 const Allow_wound allow_wound)
{
    (void)dmg;
    (void)dmg_type;
    (void)method;
    (void)allow_wound;

    aware_counter_ = data_->nr_turns_aware;
}

void Mon::move(Dir dir)
{
#ifndef NDEBUG

    if (dir == Dir::END)
    {
        TRACE << "Illegal direction parameter" << std::endl;
        assert(false);
    }

    if (!utils::is_pos_inside_map(pos, false))
    {
        TRACE << "Monster outside map" << std::endl;
        assert(false);
    }

#endif // NDEBUG

    prop_handler().affect_move_dir(pos, dir);

    //Trap affects leaving?
    if (dir != Dir::center)
    {
        auto* f = map::cells[pos.x][pos.y].rigid;

        if (f->id() == Feature_id::trap)
        {
            dir = static_cast<Trap*>(f)->actor_try_leave(*this, dir);

            if (dir == Dir::center)
            {
                TRACE_VERBOSE << "Monster move prevented by trap" << std::endl;
                game_time::tick();
                return;
            }
        }
    }

    // Movement direction is stored for AI purposes
    last_dir_moved_ = dir;

    const P tgt_cell(pos + dir_utils::offset(dir));

    if (dir != Dir::center && utils::is_pos_inside_map(tgt_cell, false))
    {
        pos = tgt_cell;

        //Bump features in target cell (i.e. to trigger traps)
        std::vector<Mob*> mobs;
        game_time::mobs_at_pos(pos, mobs);

        for (auto* m : mobs) {m->bump(*this);}

        map::cells[pos.x][pos.y].rigid->bump(*this);
    }

    game_time::tick();
}

void Mon::hear_sound(const Snd& snd)
{
    if (is_alive() && snd.is_alerting_mon())
    {
        become_aware(false);
    }
}

void Mon::speak_phrase(const Alerts_mon alerts_others)
{
    const bool IS_SEEN_BY_PLAYER = map::player->can_see_actor(*this);

    const std::string msg = IS_SEEN_BY_PLAYER ?
                            aggro_phrase_mon_seen() :
                            aggro_phrase_mon_hidden();

    const Sfx_id sfx = IS_SEEN_BY_PLAYER ?
                       aggro_sfx_mon_seen() :
                       aggro_sfx_mon_hidden();

    Snd snd(msg,
            sfx,
            Ignore_msg_if_origin_seen::no,
            pos,
            this,
            Snd_vol::low,
            alerts_others);

    snd_emit::run(snd);
}

void Mon::become_aware(const bool IS_FROM_SEEING)
{
    if (is_alive())
    {
        const int AWARENESS_CNT_BEFORE  = aware_counter_;
        aware_counter_                  = data_->nr_turns_aware;

        if (AWARENESS_CNT_BEFORE <= 0)
        {
            if (IS_FROM_SEEING && map::player->can_see_actor(*this))
            {
                map::player->update_fov();
                render::draw_map_and_interface(true);
                msg_log::add(name_the() + " sees me!");
            }

            if (rnd::coin_toss())
            {
                speak_phrase(Alerts_mon::yes);
            }
        }
    }
}

void Mon::set_player_aware_of_me(const int DURATION_FACTOR)
{
    const int LOWER             = 4 * DURATION_FACTOR;
    const int UPPER             = 6 * DURATION_FACTOR;
    const int ROLL              = rnd::range(LOWER, UPPER);
    player_aware_of_me_counter_ = std::max(player_aware_of_me_counter_, ROLL);
}

bool Mon::try_attack(Actor& defender)
{
    if (
        state_ != Actor_state::alive ||
        (aware_counter_ <= 0 && leader_ != map::player))
    {
        return false;
    }

    Ai_avail_attacks_data my_avail_attacks;
    avail_attacks(defender, my_avail_attacks);

    const Ai_att_data att = choose_att(my_avail_attacks);

    if (!att.wpn)
    {
        return false;
    }

    if (att.is_melee)
    {
        if (att.wpn->data().melee.is_melee_wpn)
        {
            attack::melee(this, pos, defender, *att.wpn);
            return true;
        }

        return false;
    }

    if (att.wpn->data().ranged.is_ranged_wpn)
    {
        if (my_avail_attacks.is_reload_needed)
        {
            reload::try_reload(*this, att.wpn);

            return true;
        }

        //Check if friend is in the way (with a small chance to ignore this)
        bool is_blocked_by_friend = false;

        if (rnd::fraction(4, 5))
        {
            std::vector<P> line;
            line_calc::calc_new_line(pos, defender.pos, true, 9999, false, line);

            for (P& line_pos : line)
            {
                if (line_pos != pos && line_pos != defender.pos)
                {
                    Actor* const actor_here = utils::actor_at_pos(line_pos);

                    if (actor_here)
                    {
                        is_blocked_by_friend = true;
                        break;
                    }
                }
            }
        }

        if (is_blocked_by_friend)
        {
            return false;
        }

        const int NR_TURNS_NO_RANGED = data_->ranged_cooldown_turns;

        Prop_disabled_ranged* ranged_cooldown_prop =
            new Prop_disabled_ranged(Prop_turns::specific, NR_TURNS_NO_RANGED);

        prop_handler_->try_add(ranged_cooldown_prop);

        attack::ranged(this, pos, defender.pos, *att.wpn);

        return true;
    }

    return false;
}

void Mon::avail_attacks(Actor& defender, Ai_avail_attacks_data& dst)
{
    if (prop_handler_->allow_attack(Verbosity::silent))
    {
        dst.is_melee = utils::is_pos_adj(pos, defender.pos, false);

        Wpn* wpn = nullptr;
        const size_t nr_intrinsics = inv_->intrinsics_size();

        if (dst.is_melee)
        {
            if (prop_handler_->allow_attack_melee(Verbosity::silent))
            {
                //Melee weapon in wielded slot?
                wpn = static_cast<Wpn*>(inv_->item_in_slot(Slot_id::wpn));

                if (wpn)
                {
                    if (wpn->data().melee.is_melee_wpn)
                    {
                        dst.weapons.push_back(wpn);
                    }
                }

                //Intrinsic melee attacks?
                for (size_t i = 0; i < nr_intrinsics; ++i)
                {
                    wpn = static_cast<Wpn*>(inv_->intrinsic_in_element(i));

                    if (wpn->data().melee.is_melee_wpn)
                    {
                        dst.weapons.push_back(wpn);
                    }
                }
            }
        }
        else //Ranged attack
        {
            if (prop_handler_->allow_attack_ranged(Verbosity::silent))
            {
                //Ranged weapon in wielded slot?
                wpn = static_cast<Wpn*>(inv_->item_in_slot(Slot_id::wpn));

                if (wpn)
                {
                    if (wpn->data().ranged.is_ranged_wpn)
                    {
                        dst.weapons.push_back(wpn);

                        //Check if reload time instead
                        if (
                            wpn->nr_ammo_loaded_ == 0 &&
                            !wpn->data().ranged.has_infinite_ammo)
                        {
                            if (inv_->has_ammo_for_firearm_in_inventory())
                            {
                                dst.is_reload_needed = true;
                            }
                        }
                    }
                }

                //Intrinsic ranged attacks?
                for (size_t i = 0; i < nr_intrinsics; ++i)
                {
                    wpn = static_cast<Wpn*>(inv_->intrinsic_in_element(i));

                    if (wpn->data().ranged.is_ranged_wpn)
                    {
                        dst.weapons.push_back(wpn);
                    }
                }
            }
        }
    }
}

Ai_att_data Mon::choose_att(const Ai_avail_attacks_data& mon_avail_attacks)
{
    Ai_att_data att(nullptr, mon_avail_attacks.is_melee);

    if (mon_avail_attacks.weapons.empty())
    {
        return att;
    }

    const size_t IDX = rnd::range(0, mon_avail_attacks.weapons.size() - 1);

    att.wpn = mon_avail_attacks.weapons[IDX];

    return att;
}

bool Mon::is_leader_of(const Actor* const actor) const
{
    if (!actor || actor->is_player())
        return false;

    //Actor is a monster
    return static_cast<const Mon*>(actor)->leader_ == this;
}

bool Mon::is_actor_my_leader(const Actor* const actor) const
{
    return leader_ == actor;
}

int Mon::group_size()
{
    const Actor* const group_leader = leader_ ? leader_ : this;

    int ret = 1; //Starting at one to include leader

    for (const Actor* const actor : game_time::actors)
    {
        if (actor->is_actor_my_leader(group_leader))
        {
            ++ret;
        }
    }

    return ret;
}

//--------------------------------------------------------- SPECIFIC MONSTERS
std::string Cultist::cultist_phrase()
{
    std::vector<std::string> phrase_bucket;

    const God* const god = gods::cur_god();

    if (god && rnd::coin_toss())
    {
        const std::string name   = god->name();
        const std::string descr  = god->descr();
        phrase_bucket.push_back(name + " save us!");
        phrase_bucket.push_back(descr + " will save us!");
        phrase_bucket.push_back(name + ", guide us!");
        phrase_bucket.push_back(descr + " guides us!");
        phrase_bucket.push_back("For " + name + "!");
        phrase_bucket.push_back("For " + descr + "!");
        phrase_bucket.push_back("Blood for " + name + "!");
        phrase_bucket.push_back("Blood for " + descr + "!");
        phrase_bucket.push_back("Perish for " + name + "!");
        phrase_bucket.push_back("Perish for " + descr + "!");
        phrase_bucket.push_back("In the name of " + name + "!");
    }
    else
    {
        phrase_bucket.push_back("Apigami!");
        phrase_bucket.push_back("Bhuudesco invisuu!");
        phrase_bucket.push_back("Bhuuesco marana!");
        phrase_bucket.push_back("Crudux cruo!");
        phrase_bucket.push_back("Cruento paashaeximus!");
        phrase_bucket.push_back("Cruento pestis shatruex!");
        phrase_bucket.push_back("Cruo crunatus durbe!");
        phrase_bucket.push_back("Cruo lokemundux!");
        phrase_bucket.push_back("Cruo stragara-na!");
        phrase_bucket.push_back("Gero shay cruo!");
        phrase_bucket.push_back("In marana domus-bhaava crunatus!");
        phrase_bucket.push_back("Caecux infirmux!");
        phrase_bucket.push_back("Malax sayti!");
        phrase_bucket.push_back("Marana pallex!");
        phrase_bucket.push_back("Marana malax!");
        phrase_bucket.push_back("Pallex ti!");
        phrase_bucket.push_back("Peroshay bibox malax!");
        phrase_bucket.push_back("Pestis Cruento!");
        phrase_bucket.push_back("Pestis cruento vilomaxus pretiacruento!");
        phrase_bucket.push_back("Pretaanluxis cruonit!");
        phrase_bucket.push_back("Pretiacruento!");
        phrase_bucket.push_back("Stragar-Naya!");
        phrase_bucket.push_back("Vorox esco marana!");
        phrase_bucket.push_back("Vilomaxus!");
        phrase_bucket.push_back("Prostragaranar malachtose!");
        phrase_bucket.push_back("Apigami!");
    }

    return phrase_bucket[rnd::range(0, phrase_bucket.size() - 1)];
}

void Cultist::mk_start_items()
{
    //If we are on a low-ish dlvl, let the vast majority of cultists carry pistols
    const bool  IS_LOW_DLVL   = map::dlvl < 4;

    const int   PISTOL        = IS_LOW_DLVL   ? 20 : 6;
    const int   PUMP_SHOTGUN  = PISTOL        + 3;
    const int   SAWN_SHOTGUN  = PUMP_SHOTGUN  + 3;
    const int   MG            = SAWN_SHOTGUN  + 1;

    const int   TOT           = MG;
    const int   RND           = map::dlvl == 0 ? PISTOL : rnd::range(1, TOT);

    if (RND <= PISTOL)
    {
        Item*   item            = item_factory::mk(Item_id::pistol);
        Wpn*    wpn             = static_cast<Wpn*>(item);
        const int AMMO_CAP      = wpn->data().ranged.max_ammo;
        wpn->nr_ammo_loaded_    = rnd::range(AMMO_CAP / 4, AMMO_CAP);

        inv_->put_in_slot(Slot_id::wpn, item);

        if (rnd::one_in(6))
        {
            inv_->put_in_backpack(item_factory::mk(Item_id::pistol_mag));
        }
    }
    else if (RND <= PUMP_SHOTGUN)
    {
        Item*   item            = item_factory::mk(Item_id::pump_shotgun);
        Wpn*    wpn             = static_cast<Wpn*>(item);
        const int AMMO_CAP      = wpn->data().ranged.max_ammo;
        wpn->nr_ammo_loaded_    = rnd::range(AMMO_CAP / 4, AMMO_CAP);

        inv_->put_in_slot(Slot_id::wpn, item);

        if (rnd::one_in(4))
        {
            item            = item_factory::mk(Item_id::shotgun_shell);
            item->nr_items_ = rnd::range(1, 8);
            inv_->put_in_backpack(item);
        }
    }
    else if (RND <= SAWN_SHOTGUN)
    {
        inv_->put_in_slot(Slot_id::wpn, item_factory::mk(Item_id::sawed_off));

        if (rnd::one_in(4))
        {
            Item* item      = item_factory::mk(Item_id::shotgun_shell);
            item->nr_items_ = rnd::range(1, 8);
            inv_->put_in_backpack(item);
        }
    }
    else //Machine gun
    {
        //Number of machine gun bullets loaded needs to be a multiple of the number of
        //projectiles fired in each burst
        Item*       item        = item_factory::mk(Item_id::machine_gun);
        Wpn* const  wpn         = static_cast<Wpn*>(item);
        const int   CAP_SCALED  = wpn->data().ranged.max_ammo / NR_MG_PROJECTILES;
        const int   MIN_SCALED  = CAP_SCALED / 4;
        wpn->nr_ammo_loaded_    = rnd::range(MIN_SCALED, CAP_SCALED) * NR_MG_PROJECTILES;
        inv_->put_in_slot(Slot_id::wpn, item);
    }

    if (rnd::one_in(8))
    {
        inv_->put_in_backpack(item_factory::mk_random_scroll_or_potion(true, true));
    }

    if (rnd::one_in(12))
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

void Cultist_electric::mk_start_items()
{
    Item*       item        = item_factory::mk(Item_id::mi_go_gun);
    Wpn*        wpn         = static_cast<Wpn*>(item);
    const int   AMMO_CAP    = wpn->data().ranged.max_ammo;

    wpn->nr_ammo_loaded_     = rnd::range(AMMO_CAP / 4, AMMO_CAP);

    inv_->put_in_slot(Slot_id::wpn, item);

    if (rnd::one_in(5))
    {
        inv_->put_in_backpack(item_factory::mk_random_scroll_or_potion(true, true));
    }

    if (rnd::one_in(3))
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

void Cultist_spike_gun::mk_start_items()
{
    Item*       item        = item_factory::mk(Item_id::spike_gun);
    Wpn*        wpn         = static_cast<Wpn*>(item);
    const int   AMMO_CAP    = wpn->data().ranged.max_ammo;
    wpn->nr_ammo_loaded_    = rnd::range(AMMO_CAP / 4, AMMO_CAP);

    inv_->put_in_slot(Slot_id::wpn, item);

    if (rnd::one_in(4))
    {
        item = item_factory::mk(Item_id::iron_spike);
        item->nr_items_ = rnd::range(4, 12);
        inv_->put_in_backpack(item);
    }
}

void Cultist_priest::mk_start_items()
{
    Item* item = item_factory::mk(Item_id::dagger);
    item->melee_dmg_plus_ = 2;
    inv_->put_in_slot(Slot_id::wpn, item);

    inv_->put_in_backpack(item_factory::mk_random_scroll_or_potion(true, true));
    inv_->put_in_backpack(item_factory::mk_random_scroll_or_potion(true, true));

    const int NR_SPELLS = 3;

    for (int i = 0; i < NR_SPELLS; ++i)
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

void Fire_hound::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::fire_hound_breath));
    inv_->put_in_intrinsics(item_factory::mk(Item_id::fire_hound_bite));
}

void Zuul::place_hook()
{
    if (actor_data::data[size_t(Actor_id::zuul)].nr_left_allowed_to_spawn > 0)
    {
        //NOTE: Do not call die() here - that would have side effects such as player
        //getting XP. Instead, simply set the dead state to destroyed.
        state_                      = Actor_state::destroyed;

        Actor* actor                = actor_factory::mk(Actor_id::cultist_priest, pos);
        auto& priest_prop_handler   = actor->prop_handler();

        auto* poss_by_zuul_prop     = new Prop_poss_by_zuul(Prop_turns::indefinite);

        priest_prop_handler.try_add(poss_by_zuul_prop);
        actor->restore_hp(999, false, Verbosity::silent);
    }
}

void Zuul::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::zuul_bite));
}

Did_action Vortex::on_act()
{
    if (!is_alive())
    {
        return Did_action::no;
    }

    if (pull_cooldown > 0)
    {
        --pull_cooldown;
    }

    if (aware_counter_ <= 0 || pull_cooldown > 0)
    {
        return Did_action::no;
    }

    const P& player_pos = map::player->pos;

    if (
        !utils::is_pos_adj(pos, player_pos, true) &&
        rnd::one_in(3))
    {
        TRACE << "Vortex attempting to pull player" << std::endl;

        const P   delta                 = player_pos - pos;
        P         knock_back_from_pos   = player_pos;

        if (delta.x >  1)
        {
            knock_back_from_pos.x++;
        }

        if (delta.x < -1)
        {
            knock_back_from_pos.x--;
        }

        if (delta.y >  1)
        {
            knock_back_from_pos.y++;
        }

        if (delta.y < -1)
        {
            knock_back_from_pos.y--;
        }

        if (knock_back_from_pos != player_pos)
        {
            TRACE << "Good pos found to knockback player from (";
            TRACE << knock_back_from_pos.x << ",";
            TRACE << knock_back_from_pos.y << ")" << std::endl;
            TRACE << "Player position: ";
            TRACE << player_pos.x << "," << player_pos.y << ")" << std::endl;

            bool blocked_los[MAP_W][MAP_H];

            const Rect fov_rect = fov::get_fov_rect(pos);

            map_parse::run(cell_check::Blocks_los(),
                           blocked_los,
                           Map_parse_mode::overwrite,
                           fov_rect);

            if (can_see_actor(*(map::player), blocked_los))
            {
                TRACE << "Is seeing player" << std::endl;

                set_player_aware_of_me();

                if (map::player->can_see_actor(*this))
                {
                    msg_log::add("The Vortex pulls me!");
                }
                else
                {
                    msg_log::add("A powerful wind is pulling me!");
                }

                TRACE << "Attempt pull (knockback)" << std::endl;

                //TODO: There should be a sfx here

                knock_back::try_knock_back(*(map::player),
                                           knock_back_from_pos,
                                           false,
                                           false);

                pull_cooldown = 2;

                game_time::tick();

                return Did_action::yes;
            }
        }
    }

    return Did_action::no;
}

void Dust_vortex::on_death()
{
    //TODO: Make a sound effect for this
    Snd snd("",
            Sfx_id::END,
            Ignore_msg_if_origin_seen::yes,
            pos,
            this,
            Snd_vol::high,
            Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run(pos,
                   Expl_type::apply_prop,
                   Expl_src::misc,
                   Emit_expl_snd::no,
                   0,
                   new Prop_blind(Prop_turns::std),
                   &clr_gray);
}

void Dust_vortex::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::dust_vortex_engulf));
}

void Fire_vortex::on_death()
{
    //TODO: Make a sound effect for this
    Snd snd("",
            Sfx_id::END,
            Ignore_msg_if_origin_seen::yes,
            pos,
            this,
            Snd_vol::high,
            Alerts_mon::yes);

    snd_emit::run(snd);

    explosion::run(pos,
                   Expl_type::apply_prop,
                   Expl_src::misc,
                   Emit_expl_snd::no,
                   0,
                   new Prop_burning(Prop_turns::std),
                   &clr_red_lgt);
}

void Fire_vortex::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::fire_vortex_engulf));
}

Did_action Ghost::on_act()
{
    if (
        is_alive()                                      &&
        aware_counter_ > 0                              &&
        utils::is_pos_adj(pos, map::player->pos, false) &&
        rnd::one_in(4))
    {
        set_player_aware_of_me();

        const bool          PLAYER_SEES_ME  = map::player->can_see_actor(*this);
        const std::string   name            = PLAYER_SEES_ME ? name_the() : "It";

        msg_log::add(name + " reaches for me...");

        const int DODGE_SKILL = map::player->ability(Ability_id::dodge_att, true);

        const Ability_roll_result roll_result = ability_roll::roll(DODGE_SKILL, map::player);

        const bool PLAYER_DODGES = roll_result >= success;

        if (PLAYER_DODGES)
        {
            msg_log::add("I dodge!", clr_msg_good);
        }
        else
        {
            map::player->prop_handler().try_add(new Prop_slowed(Prop_turns::std));
        }

        game_time::tick();

        return Did_action::yes;
    }

    return Did_action::no;
}

void Ghost::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ghost_claw));
}

void Phantasm::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::phantasm_sickle));
}

void Wraith::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::wraith_claw));
    spells_known_.push_back(spell_handling::random_spell_for_mon());
    spells_known_.push_back(spell_handling::random_spell_for_mon());
}

void Mi_go::mk_start_items()
{
    Item*       item        = item_factory::mk(Item_id::mi_go_gun);
    Wpn*        wpn         = static_cast<Wpn*>(item);
    const int   AMMO_CAP    = wpn->data().ranged.max_ammo;

    wpn->nr_ammo_loaded_ = rnd::range(AMMO_CAP / 4, AMMO_CAP);

    inv_->put_in_slot(Slot_id::wpn, item);

    if (id() == Actor_id::mi_go)
    {
        inv_->put_in_intrinsics(item_factory::mk(Item_id::mi_go_sting));
    }
    else if (id() == Actor_id::mi_go_commander)
    {
        inv_->put_in_intrinsics(item_factory::mk(Item_id::mi_go_commander_sting));

        if (rnd::one_in(3))
        {
            inv_->put_in_slot(Slot_id::body, item_factory::mk(Item_id::armor_mi_go));
        }
    }

    if (rnd::one_in(9))
    {
        inv_->put_in_backpack(item_factory::mk(Item_id::mi_go_gun_ammo));
    }

    spells_known_.push_back(new Spell_teleport);
    spells_known_.push_back(new Spell_mi_go_hypno);
    spells_known_.push_back(new Spell_heal_self);

    if (rnd::coin_toss())
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

void Sentry_drone::mk_start_items()
{
    spells_known_.push_back(new Spell_teleport);
    spells_known_.push_back(new Spell_heal_self);
    spells_known_.push_back(new Spell_darkbolt);
    spells_known_.push_back(new Spell_burn);
}

void Flying_polyp::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::polyp_tentacle));
}

void Greater_polyp::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::greater_polyp_tentacle));
}

void Rat::mk_start_items()
{
    Item* item = nullptr;

    if (rnd::percent() < 15)
    {
        item = item_factory::mk(Item_id::rat_bite_diseased);
    }
    else
    {
        item = item_factory::mk(Item_id::rat_bite);
    }

    inv_->put_in_intrinsics(item);
}

void Rat_thing::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::rat_thing_bite));
}

void Brown_jenkin::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::brown_jenkin_bite));
    spells_known_.push_back(new Spell_teleport);
}

void Shadow::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::shadow_claw));
}

void Invis_stalker::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::invis_stalker_claw));
}

void Ghoul::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ghoul_claw));
}

void Ghoul::place_hook()
{
    //If player is Ghoul, then Ghouls are allied to player
    if (player_bon::bg() == Bg::ghoul)
    {
        leader_ = map::player;
    }
}

Did_action Ghoul::on_act()
{
    if (!is_alive())
    {
        return Did_action::no;
    }

    if (rnd::coin_toss())
    {
        const auto did_action = try_eat_corpse();

        if (did_action == Did_action::yes)
        {
            game_time::tick();

            return did_action;
        }
    }

    return Did_action::no;
}

void Mummy::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::mummy_maul));

    spells_known_.push_back(spell_handling::mk_spell_from_id(Spell_id::disease));

    const int NR_SPELLS = 3;

    for (int i = 0; i < NR_SPELLS; ++i)
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

Did_action Mummy::on_act()
{
    //TODO: Below is an implementation for mummies turning friendly if player is wielding
    //the Staff of the Pharoh. It is commented out at least until after v17.0 is released.
    //It is probably too powerful and unbalanced. Perhaps add this power as a separate
    //spell to the Staff (something like "Pharohs Command")?

//    if (
//        !is_alive()          ||
//        data_->is_unique     ||
//        aware_counter_ <= 0  ||
//        is_actor_my_leader(map::player))
//    {
//        return false;
//    }
//
//    const Item* const player_wpn = map::player->inv().item_in_slot(Slot_id::wpn);
//
//    if (player_wpn && player_wpn->id() == Item_id::pharaoh_staff)
//    {
//        bool blocked_los[MAP_W][MAP_H];
//        map_parse::run(cell_check::Blocks_los(), blocked_los);
//
//        if (can_see_actor(*map::player, blocked_los))
//        {
//            if (map::player->can_see_actor(*this))
//            {
//                const std::string name = name_the();
//
//                msg_log::add(name + " bows before me.", clr_white, false, true);
//            }
//
//            leader_ = map::player;
//
//            game_time::tick();
//            return true;
//        }
//    }

    return Did_action::no;
}

void Mummy_croc_head::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::croc_head_mummy_spear));
}

void Mummy_unique::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::mummy_maul));

    spells_known_.push_back(spell_handling::mk_spell_from_id(Spell_id::disease));

    spells_known_.push_back(spell_handling::random_spell_for_mon());
    spells_known_.push_back(spell_handling::random_spell_for_mon());
    spells_known_.push_back(spell_handling::random_spell_for_mon());
}

Did_action Khephren::on_act()
{
    if (is_alive() && aware_counter_ > 0 && !has_summoned_locusts)
    {
        bool blocked[MAP_W][MAP_H];

        const Rect fov_rect = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       blocked,
                       Map_parse_mode::overwrite,
                       fov_rect);

        if (can_see_actor(*(map::player), blocked))
        {
            map_parse::run(cell_check::Blocks_move_cmn(true), blocked);

            const int SPAWN_AFTER_X = map::player->pos.x + FOV_STD_RADI_INT + 1;

            for (int y = 0; y  < MAP_H; ++y)
            {
                for (int x = 0; x <= SPAWN_AFTER_X; ++x)
                {
                    blocked[x][y] = true;
                }
            }

            std::vector<P> free_cells;
            utils::mk_vector_from_bool_map(false, blocked, free_cells);

            sort(begin(free_cells), end(free_cells), Is_closer_to_pos(pos));

            const size_t NR_OF_SPAWNS = 15;

            if (free_cells.size() >= NR_OF_SPAWNS + 1)
            {
                msg_log::add("Khephren calls a plague of Locusts!");
                map::player->incr_shock(Shock_lvl::heavy, Shock_src::misc);

                for (size_t i = 0; i < NR_OF_SPAWNS; ++i)
                {
                    Actor* const actor  = actor_factory::mk(Actor_id::locust, free_cells[0]);
                    Mon* const mon      = static_cast<Mon*>(actor);
                    mon->aware_counter_ = 999;
                    mon->leader_        = leader_ ? leader_ : this;

                    free_cells.erase(begin(free_cells));
                }

                render::draw_map_and_interface();
                has_summoned_locusts = true;
                game_time::tick();
                return Did_action::yes;
            }
        }
    }

    return Did_action::no;
}

void Deep_one::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::deep_one_javelin_att));
    inv_->put_in_intrinsics(item_factory::mk(Item_id::deep_one_spear_att));
}

void Ape::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ape_maul));
}

Did_action Ape::on_act()
{
    if (frenzy_cool_down_ > 0)
    {
        --frenzy_cool_down_;
    }

    if (
        frenzy_cool_down_ <= 0      &&
        tgt_                        &&
        (hp() <= hp_max(true) / 2))
    {
        frenzy_cool_down_ = 30;

        const int NR_FRENZY_TURNS = rnd::range(4, 6);

        prop_handler_->try_add(new Prop_frenzied(Prop_turns::specific, NR_FRENZY_TURNS));
    }

    return Did_action::no;
}

void Raven::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::raven_peck));
}

void Giant_bat::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::giant_bat_bite));
}

void Byakhee::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::byakhee_claw));
}

void Giant_mantis::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::giant_mantis_claw));
}

void Chthonian::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::chthonian_bite));
}

void Death_fiend::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::death_fiend_claw));

    spells_known_.push_back(new Spell_paralyze_mon);
    spells_known_.push_back(new Spell_slow_mon);
    spells_known_.push_back(new Spell_terrify_mon);
}

void Hunting_horror::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::hunting_horror_bite));
}

Did_action Keziah_mason::on_act()
{
    if (is_alive() && aware_counter_ > 0 && !has_summoned_jenkin)
    {
        bool blocked_los[MAP_W][MAP_H];

        const Rect fov_rect = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       blocked_los,
                       Map_parse_mode::overwrite,
                       fov_rect);

        if (can_see_actor(*(map::player), blocked_los))
        {
            map_parse::run(cell_check::Blocks_move_cmn(true), blocked_los);

            std::vector<P> line;
            line_calc::calc_new_line(pos, map::player->pos, true, 9999, false, line);

            const int LINE_SIZE = line.size();

            for (int i = 0; i < LINE_SIZE; ++i)
            {
                const P c = line[i];

                if (!blocked_los[c.x][c.y])
                {
                    //TODO: Use the generalized summoning functionality
                    set_player_aware_of_me();

                    msg_log::add("Keziah summons Brown Jenkin!");

                    Actor* const actor      = actor_factory::mk(Actor_id::brown_jenkin, c);

                    Mon* jenkin             = static_cast<Mon*>(actor);

                    render::draw_map_and_interface();

                    has_summoned_jenkin     = true;
                    jenkin->aware_counter_  = 999;
                    jenkin->leader_         = leader_ ? leader_ : this;
                    game_time::tick();
                    return Did_action::yes;
                }
            }
        }
    }

    return Did_action::no;
}

void Keziah_mason::mk_start_items()
{
    spells_known_.push_back(new Spell_teleport);
    spells_known_.push_back(new Spell_heal_self);
    spells_known_.push_back(new Spell_summon_mon);
    spells_known_.push_back(new Spell_pest);
    spells_known_.push_back(new Spell_darkbolt);
    spells_known_.push_back(spell_handling::random_spell_for_mon());

    for (int i = rnd::range(2, 3); i > 0; --i)
    {
        inv_->put_in_backpack(item_factory::mk_random_scroll_or_potion(true, true));
    }
}

void Leng_elder::on_std_turn_hook()
{
    if (is_alive())
    {
        aware_counter_ = 100;

        if (has_given_item_to_player_)
        {
            bool blocked_los[MAP_W][MAP_H];

            const Rect fov_rect = fov::get_fov_rect(pos);

            map_parse::run(cell_check::Blocks_los(),
                           blocked_los,
                           Map_parse_mode::overwrite,
                           fov_rect);

            if (can_see_actor(*map::player, blocked_los))
            {
                if (nr_turns_to_hostile_ <= 0)
                {
                    msg_log::add("I am ripped to pieces!!!", clr_msg_bad);
                    map::player->hit(999, Dmg_type::pure);
                }
                else
                {
                    --nr_turns_to_hostile_;
                }
            }
        }
        else //Has not given item to player
        {
            const bool IS_PLAYER_SEE_ME = map::player->can_see_actor(*this);
            const bool IS_PLAYER_ADJ    = utils::is_pos_adj(pos, map::player->pos, false);

            if (IS_PLAYER_SEE_ME && IS_PLAYER_ADJ)
            {
                msg_log::add("I perceive a cloaked figure standing before me...",
                             clr_white, false, More_prompt_on_msg::yes);

                msg_log::add("It is the Elder Hierophant of the Leng monastery, ");

                msg_log::add("the High Priest Not to Be Described.",
                             clr_white, false, More_prompt_on_msg::yes);

                popup::show_msg("", true, "");

                //auto& inv = map::player->inv();
                //TODO: Which item to give?
                //inv.put_in_backpack(item_factory::mk(Item_id::hideous_mask));

                has_given_item_to_player_ = true;
                nr_turns_to_hostile_     = rnd::range(9, 11);
            }
        }
    }
}

void Leng_elder::mk_start_items()
{

}

void Ooze::on_std_turn_hook()
{
    if (is_alive())
    {
        restore_hp(1, false, Verbosity::silent);
    }
}

void Ooze_black::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ooze_black_spew_pus));
}

void Ooze_clear::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ooze_clear_spew_pus));
}

void Ooze_putrid::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ooze_putrid_spew_pus));
}

void Ooze_poison::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::ooze_poison_spew_pus));
}

void Color_oo_space::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::color_oo_space_touch));
}

Did_action Color_oo_space::on_act()
{
    Rigid* r = map::cells[pos.x][pos.y].rigid;

    r->corrupt_color();

    return Did_action::no;
}

Clr Color_oo_space::clr()
{
    Clr clr = clr_magenta_lgt;

    clr.r = rnd::range(40, 255);
    clr.g = rnd::range(40, 255);
    clr.b = rnd::range(40, 255);

    return clr;
}

void Color_oo_space::on_std_turn_hook()
{
    if (is_alive())
    {
        restore_hp(1, false, Verbosity::silent);

        if (map::player->can_see_actor(*this))
        {
            map::player->prop_handler().try_add(new Prop_confused(Prop_turns::std));
        }
    }
}

Did_action Spider::on_act()
{
    return Did_action::no;
}

void Green_spider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::green_spider_bite));
}

void White_spider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::white_spider_bite));
}

void Red_spider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::red_spider_bite));
}

void Shadow_spider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::shadow_spider_bite));
}

void Leng_spider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::leng_spider_bite));
}

void Pit_viper::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::pit_viper_bite));
}

void Spitting_cobra::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::spitting_cobra_bite));
    inv_->put_in_intrinsics(item_factory::mk(Item_id::spitting_cobra_spit));
}

void Black_mamba::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::black_mamba_bite));
}

void Wolf::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::wolf_bite));
}

void Worm_mass::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::worm_mass_bite));
}

void Worm_mass::on_death()
{
    const int PCT_SPLIT = 47;

    if (rnd::percent(PCT_SPLIT) && (game_time::actors.size() < MAX_NR_ACTORS_ON_MAP))
    {
        std::vector<Actor_id> worm_ids(2, id());

        //If dying worm has a leader, set that worm as leader for spawned worms
        Actor* leader = leader_ ? leader_ : nullptr;

        std::vector<Mon*> summoned;

        actor_factory::summon(pos,
                              worm_ids,
                              Make_mon_aware::yes,
                              leader,
                              &summoned,
                              Verbosity::silent);

        assert(summoned.size() == 2);

        //If no leader has been set yet, assign the first worm as leader of the second
        if (!leader && summoned.size() >= 2)
        {
            summoned[1]->leader_ = summoned[0];
        }
    }
}

void Mind_worms::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::mind_worms_bite));
}

Did_action Giant_locust::on_act()
{
    if (
        is_alive()                                      &&
        aware_counter_ > 0                              &&
        game_time::actors.size() < MAX_NR_ACTORS_ON_MAP &&
        rnd::one_in(spawn_new_one_in_n))
    {
        bool blocked[MAP_W][MAP_H];

        map_parse::run(cell_check::Blocks_actor(*this, true),
                       blocked,
                       Map_parse_mode::overwrite,
                       Rect(pos - 1, pos + 1));

        for (const P& d : dir_utils::dir_list)
        {
            const P p_adj(pos + d);

            if (!blocked[p_adj.x][p_adj.y])
            {
                Actor* const actor = actor_factory::mk(data_->id, p_adj);

                Giant_locust* const locust = static_cast<Giant_locust*>(actor);

                spawn_new_one_in_n += 3;

                locust->spawn_new_one_in_n  = spawn_new_one_in_n;
                locust->aware_counter_      = aware_counter_;
                locust->leader_             = leader_ ? leader_ : this;

                game_time::tick();

                return Did_action::yes;
            }
        }
    }

    return Did_action::no;
}

void Giant_locust::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::giant_locust_bite));
}

Did_action Lord_of_shadows::on_act()
{
    return Did_action::no;
}

void Lord_of_shadows::mk_start_items()
{

}

Did_action Lord_of_spiders::on_act()
{
    if (is_alive() && aware_counter_ > 0 && rnd::coin_toss())
    {
        const P player_pos = map::player->pos;

        if (map::player->can_see_actor(*this))
        {
            msg_log::add(data_->spell_cast_msg);
        }

        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (rnd::fraction(3, 4))
                {

                    const P p(player_pos + P(dx, dy));
                    const auto* const feature_here = map::cells[p.x][p.y].rigid;

                    if (feature_here->can_have_rigid())
                    {
                        auto& d = feature_data::data(feature_here->id());
                        const auto* const mimic = static_cast<Rigid*>(d.mk_obj(p));
                        Trap* const f = new Trap(p, mimic, Trap_id::web);
                        map::put(f);
                        f->reveal(false);
                    }
                }
            }
        }
    }

    return Did_action::no;
}

void Lord_of_spiders::mk_start_items()
{

}

Did_action Lord_of_spirits::on_act()
{
    return Did_action::no;
}

void Lord_of_spirits::mk_start_items()
{

}

Did_action Lord_of_pestilence::on_act()
{
    return Did_action::no;
}

void Lord_of_pestilence::mk_start_items()
{

}

Did_action Zombie::on_act()
{
    return try_resurrect();
}

Did_action Zombie::try_resurrect()
{
    if (!is_corpse() || has_resurrected)
    {
        return Did_action::no;
    }

    const int MIN_NR_TURNS_UNTIL_RISE   = 3;
    const int RISE_ONE_IN_N_TURNS       = 8;

    if (dead_turn_counter < MIN_NR_TURNS_UNTIL_RISE)
    {
        ++dead_turn_counter;
    }
    else if (rnd::one_in(RISE_ONE_IN_N_TURNS))
    {
        Actor* actor = utils::actor_at_pos(pos);

        if (!actor)
        {
            state_  = Actor_state::alive;
            hp_     = (hp_max(true) * 3) / 4;
            glyph_  = data_->glyph;
            tile_   = data_->tile;
            clr_    = data_->color;

            has_resurrected = true;

            data_->nr_kills--;

            if (map::cells[pos.x][pos.y].is_seen_by_player)
            {
                msg_log::add(corpse_name_the() + " rises again!!", clr_white, true);

                map::player->incr_shock(Shock_lvl::some, Shock_src::see_mon);
            }

            aware_counter_ = data_->nr_turns_aware * 2;

            game_time::tick();

            return Did_action::yes;
        }
    }

    return Did_action::no;
}

void Zombie::on_death()
{
    //If resurrected once and has corpse, blow up the corpse
    if (has_resurrected && is_corpse())
    {
        state_ = Actor_state::destroyed;
        map::mk_blood(pos);
        map::mk_gore(pos);
    }

    //If corpse is destroyed, occasionally spawn Zombie parts. Spawning is only allowed if the
    //corpse is not destroyed "too hard" (e.g. by a near explosion or a sledge hammer). This also
    //serves to reward heavy weapons, since they will more often prevent spawning nasty stuff.

    const int SUMMON_ONE_IN_N = 7;

    if (state_ == Actor_state::destroyed && hp_ > -10 && rnd::one_in(SUMMON_ONE_IN_N))
    {
        Actor_id id_to_spawn = Actor_id::END;

        //With a small chance, spawn a Floating Head, otherwise spawn Hands or Intestines
        const int ROLL = rnd::one_in(50) ? 3 : rnd::range(1, 2);

        const std::string my_name = name_the();

        std::string spawn_msg = "";

        if (ROLL == 1)
        {
            id_to_spawn = Actor_id::crawling_hand;

            spawn_msg = "The hand of " + my_name + " comes off and starts crawling around!";
        }
        else if (ROLL == 2)
        {
            id_to_spawn = Actor_id::crawling_intestines;

            spawn_msg = "The intestines of " + my_name + " starts crawling around!";
        }
        else
        {
            id_to_spawn = Actor_id::floating_head;

            spawn_msg = "The head of " + my_name + " starts floating around!";
        }

        if (map::cells[pos.x][pos.y].is_seen_by_player)
        {
            assert(!spawn_msg.empty());

            msg_log::add(spawn_msg);

            map::player->incr_shock(Shock_lvl::some, Shock_src::see_mon);
        }

        assert(id_to_spawn != Actor_id::END);

        actor_factory::summon(pos,
        {id_to_spawn},
        Make_mon_aware::yes,
        nullptr,
        nullptr,
        Verbosity::silent);

        render::draw_map_and_interface();
    }
}

void Zombie_claw::mk_start_items()
{
    Item* item = nullptr;

    if (rnd::percent() < 20)
    {
        item = item_factory::mk(Item_id::zombie_claw_diseased);
    }
    else //Not diseased
    {
        item = item_factory::mk(Item_id::zombie_claw);
    }

    inv_->put_in_intrinsics(item);
}

void Zombie_axe::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::zombie_axe));
}

void Bloated_zombie::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::bloated_zombie_punch));
    inv_->put_in_intrinsics(item_factory::mk(Item_id::bloated_zombie_spit));
}

Did_action Major_clapham_lee::on_act()
{
    if (try_resurrect() == Did_action::yes)
    {
        return Did_action::yes;
    }

    if (is_alive() && aware_counter_ > 0 && !has_summoned_tomb_legions)
    {
        bool blocked_los[MAP_W][MAP_H];

        const Rect fov_rect = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(), blocked_los, Map_parse_mode::overwrite, fov_rect);

        if (can_see_actor(*(map::player), blocked_los))
        {
            set_player_aware_of_me();

            msg_log::add("Major Clapham Lee calls forth his Tomb-Legions!");

            std::vector<Actor_id> mon_ids = {Actor_id::dean_halsey};

            const int NR_OF_EXTRA_SPAWNS = 4;

            for (int i = 0; i < NR_OF_EXTRA_SPAWNS; ++i)
            {
                const int ZOMBIE_TYPE = rnd::range(1, 3);

                Actor_id mon_id = Actor_id::zombie;

                switch (ZOMBIE_TYPE)
                {
                case 1:
                    mon_id = Actor_id::zombie;
                    break;

                case 2:
                    mon_id = Actor_id::zombie_axe;
                    break;

                case 3:
                    mon_id = Actor_id::bloated_zombie;
                    break;
                }

                mon_ids.push_back(mon_id);
            }

            actor_factory::summon(pos, mon_ids, Make_mon_aware::yes, this);

            render::draw_map_and_interface();

            has_summoned_tomb_legions = true;

            map::player->incr_shock(Shock_lvl::heavy, Shock_src::misc);

            game_time::tick();

            return Did_action::yes;
        }
    }

    return Did_action::no;
}

void Crawling_intestines::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::crawling_intestines_strangle));
}

void Crawling_hand::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::crawling_hand_strangle));
}

void Thing::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::thing_strangle));
    spells_known_.push_back(new Spell_teleport);
}

void Floating_head::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::floating_head_bite));
}

Did_action Floating_head::on_act()
{
    if (is_alive() && rnd::one_in(12))
    {
        bool blocked_los[MAP_W][MAP_H];

        const Rect fov_rect = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       blocked_los,
                       Map_parse_mode::overwrite,
                       fov_rect);

        if (can_see_actor(*map::player, blocked_los))
        {
            const std::string name = name_the();

            const bool PLAYER_SEE_ME = map::player->can_see_actor(*this);

            std::string snd_msg = PLAYER_SEE_ME ? name : "Someone";

            snd_msg += " spews forth a litany of curses.";

            Snd snd(snd_msg,
                    Sfx_id::END,
                    Ignore_msg_if_origin_seen::no,
                    pos,
                    this,
                    Snd_vol::high,
                    Alerts_mon::no);

            snd_emit::run(snd);

            Prop* const prop = new Prop_cursed(Prop_turns::std);

            map::player->prop_handler().try_add(prop, Prop_src::intr);

            return Did_action::yes;
        }
    }

    return Did_action::no;
}

Did_action Mold::on_act()
{
    if (
        is_alive()                                      &&
        game_time::actors.size() < MAX_NR_ACTORS_ON_MAP &&
        rnd::one_in(spawn_new_one_in_n))
    {
        bool blocked[MAP_W][MAP_H];

        map_parse::run(cell_check::Blocks_actor(*this, true),
                       blocked,
                       Map_parse_mode::overwrite,
                       Rect(pos - 1, pos + 1));

        for (const P& d : dir_utils::dir_list)
        {
            const P adj_pos(pos + d);

            if (!blocked[adj_pos.x][adj_pos.y])
            {
                Actor* const  actor     = actor_factory::mk(data_->id, adj_pos);
                Mold* const   mold      = static_cast<Mold*>(actor);
                mold->aware_counter_    = aware_counter_;
                mold->leader_           = leader_ ? leader_ : this;
                game_time::tick();
                return Did_action::yes;
            }
        }
    }

    return Did_action::no;
}

void Mold::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::mold_spores));
}

void Gas_spore::on_death()
{
    explosion::run(pos, Expl_type::expl);
}

The_high_priest::The_high_priest() :
    Mon                 (),
    has_greeted_player_ (false) {}

void The_high_priest::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(Item_id::the_high_priest_claw));

    spells_known_.push_back(new Spell_summon_mon());
    spells_known_.push_back(new Spell_burn());
    spells_known_.push_back(new Spell_paralyze_mon());
    spells_known_.push_back(new Spell_mi_go_hypno());
    spells_known_.push_back(new Spell_pest());
    spells_known_.push_back(new Spell_knock_back());
}

void The_high_priest::on_death()
{
    msg_log::add("The ground rumbles...",
                 clr_white,
                 false,
                 More_prompt_on_msg::yes);

    const P stair_pos(MAP_W - 2, 11);

    map::put(new Stairs(stair_pos));
    map::put(new Rubble_low(stair_pos - P(1, 0)));

    map::player->update_fov();
    render::draw_map_and_interface();
}

void The_high_priest::on_std_turn_hook()
{

}

Did_action The_high_priest::on_act()
{
    if (!is_alive())
    {
        return Did_action::no;
    }

    if (!has_greeted_player_)
    {
        map::player->update_fov();
        render::draw_map_and_interface();

        msg_log::add("A booming voice echoes through the halls.",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        audio::play(Sfx_id::boss_voice1);

        has_greeted_player_ = true;
        aware_counter_      = data_->nr_turns_aware;
    }

    if (rnd::coin_toss())
    {
        map::cells[pos.x][pos.y].rigid->mk_bloody();
        map::cells[pos.x][pos.y].rigid->try_put_gore();
    }

    return Did_action::no;
}

Animated_wpn::Animated_wpn() :
    Mon                     (),
    nr_turns_until_drop_    (rnd::range(75, 125)) {}

std::string Animated_wpn::name_the() const
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    assert(item);

    const std::string name = item->name(Item_ref_type::plain,
                                        Item_ref_inf::yes,
                                        Item_ref_att_inf::none);

    return "The floating " + name;
}

std::string Animated_wpn::name_a() const
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    assert(item);

    const std::string name = item->name(Item_ref_type::plain,
                                        Item_ref_inf::yes,
                                        Item_ref_att_inf::none);

    return "A floating " + name;
}

char Animated_wpn::glyph() const
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    assert(item);

    return item->glyph();
}

Clr Animated_wpn::clr()
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    assert(item);

    return item->clr();
}

Tile_id Animated_wpn::tile() const
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    assert(item);

    return item->tile();
}

std::string Animated_wpn::descr() const
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    assert(item);

    std::string name = item->name(Item_ref_type::a,
                                  Item_ref_inf::yes,
                                  Item_ref_att_inf::none);

    text_format::first_to_upper(name);

    return name + ", floating through the air as if wielded by some invisible hand.";
}

std::string Animated_wpn::death_msg() const
{
    Item* item = inv_->item_in_slot(Slot_id::wpn);

    if (!item)
    {
        assert(false);

        //Release build robustness
        return "";
    }

    const std::string name = item->name(Item_ref_type::plain,
                                        Item_ref_inf::yes,
                                        Item_ref_att_inf::none);

    return "The " + name + " suddenly becomes lifeless and drops down.";
}

void Animated_wpn::on_std_turn_hook()
{
    if (!is_alive())
    {
        return;
    }

    if (nr_turns_until_drop_ <= 0)
    {
        die(true, false, true);
    }
    else //Not yet time to die
    {
        --nr_turns_until_drop_;
    }
}
