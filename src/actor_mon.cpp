#include "actor_mon.hpp"

#include <vector>

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
#include "io.hpp"
#include "sound.hpp"
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
    spell_cooldown_current_     (0),
    is_roaming_allowed_         (true),
    is_sneaking_                (false),
    leader_                     (nullptr),
    tgt_                        (nullptr),
    waiting_                    (false) {}

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

// TODO: Some of the things done in this function should probably be moved to
//       "on_actor_turn()" instead. The purpose of this function ("act()") is
//       only to tell the actor to "do something".
void Mon::act()
{
#ifndef NDEBUG
    // Sanity check - verify that monster is not outside the map
    if (!map::is_pos_inside_map(pos, false))
    {
        TRACE << "Monster outside map" << std::endl;
        ASSERT(false);
    }

    // Sanity check - verify that monster's leader does not have a leader
    // (never allowed)
    if (leader_ &&                              // Has leader?
        !is_actor_my_leader(map::player) &&     // Leader is monster?
        static_cast<Mon*>(leader_)->leader_)    // Leader has a leader?
    {
        TRACE << "Two (or more) steps of leader is never allowed" << std::endl;
        ASSERT(false);
    }
#endif // NDEBUG

    if (aware_counter_ <= 0 &&
        !is_actor_my_leader(map::player))
    {
        waiting_ = !waiting_;

        if (waiting_)
        {
            game_time::tick();
            return;
        }
    }
    else // Is aware, or player is leader
    {
        waiting_ = false;
    }

    // Pick a target
    std::vector<Actor*> tgt_bucket;

    if (prop_handler_->has_prop(PropId::conflict))
    {
        // Monster is conflicted (e.g. by player ring/amulet)
        tgt_bucket = game_time::actors;

        bool hard_blocked_los[map_w][map_h];

        const R fov_rect = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
            .run(hard_blocked_los,
                 MapParseMode::overwrite,
                 fov_rect);

        // Remove self and all unseen actors from vector
        for (auto it = begin(tgt_bucket); it != end(tgt_bucket);)
        {
            if (*it == this || !can_see_actor(**it, hard_blocked_los))
            {
                tgt_bucket.erase(it);
            }
            else // Is a seen actor
            {
                ++it;
            }
        }
    }
    else // Not conflicted
    {
        seen_foes(tgt_bucket);

        // If not aware, remove player from target bucket
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

    tgt_ = map::random_closest_actor(pos, tgt_bucket);

    if (spell_cooldown_current_ != 0)
    {
        --spell_cooldown_current_;
    }

    if (aware_counter_ > 0)
    {
        is_roaming_allowed_ = true;

        if (leader_ && leader_->is_alive())
        {
            // Monster has a living leader

            if (!is_actor_my_leader(map::player))
            {
                // Make leader aware
                Mon* const leader_mon = static_cast<Mon*>(leader_);

                leader_mon->aware_counter_ =
                    std::max(leader_mon->data().nr_turns_aware,
                             leader_mon->aware_counter_);
            }
        }
        else // Monster does not have a living leader
        {
            if (is_alive() && rnd::one_in(12))
            {
                speak_phrase(AlertsMon::no);
            }
        }
    }

    is_sneaking_ =
        !is_actor_my_leader(map::player) &&
        (ability(AbilityId::stealth, true) > 0) &&
        !map::player->can_see_actor(*this);

    // Array used for AI purposes, e.g. to prevent tactically bad positions, or
    // prevent certain monsters from walking on a certain type of cells, etc.
    // This is checked in all AI movement functions. Cells set to true are
    // totally forbidden for the monster to move into.
    bool ai_special_blockers[map_w][map_h];

    ai::info::set_special_blocked_cells(*this, ai_special_blockers);

    // -------------------------------------------------------------------------
    // Special monster actions (e.g. zombies rising)
    // -------------------------------------------------------------------------
    if (leader_ != map::player &&
        (tgt_ == nullptr || tgt_ == map::player))
    {
        if (on_act() == DidAction::yes)
        {
            return;
        }
    }

    // -------------------------------------------------------------------------
    // Common actions (moving, attacking, casting spells, etc)
    // -------------------------------------------------------------------------

    // NOTE: Looking is as an action if monster was not aware before, and became
    //       aware from looking. (This is to give the monsters some reaction
    //       time, and not instantly attack)
    if (data_->ai[(size_t)AiId::looks] &&
        leader_ != map::player &&
        (tgt_ == nullptr || tgt_ == map::player))
    {
        if (ai::info::look_become_player_aware(*this))
        {
            return;
        }
    }

    if (data_->ai[(size_t)AiId::makes_room_for_friend] &&
        leader_ != map::player &&
        tgt_ == map::player)
    {
        if (ai::action::make_room_for_friend(*this))
        {
            return;
        }
    }

    // Cast instead of attacking?
    if (rnd::one_in(5))
    {
        if (ai::action::try_cast_random_spell(*this))
        {
            return;
        }
    }

    if (data_->ai[(size_t)AiId::attacks] && tgt_)
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

    int erratic_move_pct = (int)data_->erratic_move_pct;

    // Never move erratically if frenzied
    if (prop_handler_->has_prop(PropId::frenzied))
    {
        erratic_move_pct = 0;
    }

    // Move less erratically if allied to player
    if (is_actor_my_leader(map::player))
    {
        erratic_move_pct /= 2;
    }

    // Move more erratically if confused
    if (prop_handler_->has_prop(PropId::confused) &&
        erratic_move_pct > 0)
    {
        erratic_move_pct += 50;
    }

    set_constr_in_range(0, erratic_move_pct, 95);

    // Occasionally move erratically
    if (data_->ai[(size_t)AiId::moves_to_random_when_unaware] &&
        rnd::percent(erratic_move_pct))
    {
        if (ai::action::move_to_random_adj_cell(*this))
        {
            return;
        }
    }

    const bool is_terrified = prop_handler_->has_prop(PropId::terrified);

    if (data_->ai[(size_t)AiId::moves_to_tgt_when_los] &&
        !is_terrified)
    {
        if (ai::action::move_to_tgt_simple(*this))
        {
            return;
        }
    }

    std::vector<P> path;

    if (data_->ai[(size_t)AiId::paths_to_tgt_when_aware] &&
        leader_ != map::player &&
        !is_terrified)
    {
        ai::info::find_path_to_player(*this, path);
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

    if (data_->ai[(size_t)AiId::moves_to_leader] && !is_terrified)
    {
        ai::info::find_path_to_leader(*this, path);

        if (ai::action::step_path(*this, path))
        {
            return;
        }
    }

    if (data_->ai[(size_t)AiId::moves_to_lair]  &&
        leader_ != map::player &&
        (tgt_ == nullptr || tgt_ == map::player))
    {
        if (ai::action::step_to_lair_if_los(*this, lair_pos_))
        {
            return;
        }
        else // No LOS to lair
        {
            // Try to use pathfinder to travel to lair
            ai::info::find_path_to_lair_if_no_los(*this, path, lair_pos_);

            if (ai::action::step_path(*this, path))
            {
                return;
            }
        }
    }

    // When unaware, move randomly
    if (data_->ai[(size_t)AiId::moves_to_random_when_unaware])
    {
        if (ai::action::move_to_random_adj_cell(*this))
        {
            return;
        }
    }

    // No action could be performed, just let someone else act
    game_time::tick();
}

bool Mon::can_see_actor(const Actor& other,
                        const bool hard_blocked_los[map_w][map_h]) const
{
    if (this == &other || !other.is_alive())
    {
        return true;
    }

    // Outside FOV range?
    if (!fov::is_in_fov_range(pos, other.pos))
    {
        // Other actor is outside FOV range
        return false;
    }

    // Monster allied to player looking at other monster which is hidden?
    if (is_actor_my_leader(map::player) &&
        !other.is_player() &&
        static_cast<const Mon*>(&other)->is_sneaking_)
    {
        return false;
    }

    // Monster is blind?
    if (!prop_handler_->allow_see())
    {
        return false;
    }

    const LosResult los = fov::check_cell(pos,
                                          other.pos,
                                          hard_blocked_los);

    // LOS blocked hard (e.g. a wall or smoke)?
    if (los.is_blocked_hard)
    {
        return false;
    }

    const bool can_see_invis = has_prop(PropId::see_invis);

    // Actor is invisible, and monster cannot see invisible?
    if (other.has_prop(PropId::invis) &&
        !can_see_invis)
    {
        return false;
    }

    bool has_infravis =
        has_prop(PropId::infravis);

    const bool is_other_infra_visible =
        other.data().is_infra_visible;

    const bool can_see_actor_with_infravis =
        has_infravis && is_other_infra_visible;

    const bool can_see_other_in_drk =
        can_see_invis || can_see_actor_with_infravis;

    // Blocked by darkness, and not seeing actor with infravision?
    if (los.is_blocked_by_drk &&
        !can_see_other_in_drk)
    {
        return false;
    }

    // OK, all checks passed, actor can bee seen!
    return true;
}

void Mon::on_std_turn()
{
    on_std_turn_hook();
}

void Mon::on_hit(int& dmg,
                 const DmgType dmg_type,
                 const DmgMethod method,
                 const AllowWound allow_wound)
{
    (void)dmg;
    (void)dmg_type;
    (void)method;
    (void)allow_wound;

    aware_counter_ = std::max(data_->nr_turns_aware,
                              aware_counter_);
}

void Mon::move(Dir dir)
{
#ifndef NDEBUG
    if (dir == Dir::END)
    {
        TRACE << "Illegal direction parameter" << std::endl;
        ASSERT(false);
    }

    if (!map::is_pos_inside_map(pos, false))
    {
        TRACE << "Monster outside map" << std::endl;
        ASSERT(false);
    }
#endif // NDEBUG

    prop_handler().affect_move_dir(pos, dir);

    // Trap affects leaving?
    if (dir != Dir::center)
    {
        auto* f = map::cells[pos.x][pos.y].rigid;

        if (f->id() == FeatureId::trap)
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

    const P tgt_p(pos + dir_utils::offset(dir));

    if (dir != Dir::center && map::is_pos_inside_map(tgt_p, false))
    {
        pos = tgt_p;

        // Bump features in target cell (i.e. to trigger traps)
        std::vector<Mob*> mobs;
        game_time::mobs_at_pos(pos, mobs);

        for (auto* m : mobs)
        {
            m->bump(*this);
        }

        map::cells[pos.x][pos.y].rigid->bump(*this);
    }

    game_time::tick();
}

Clr Mon::clr() const
{
    if (state_ != ActorState::alive)
    {
        return data_->color;
    }

    Clr tmp_clr;

    if (prop_handler_->affect_actor_clr(tmp_clr))
    {
        return tmp_clr;
    }

    // If injured, draw as a shade of red
    const int current_hp = hp();
    const int current_hp_max = hp_max(true);

    if (current_hp < current_hp_max)
    {
        const int hp_pct =
            constr_in_range(0,
                            (current_hp * 100) / current_hp_max,
                            100);

        tmp_clr.r = std::max(192, (255 * hp_pct) / 100);
        tmp_clr.g = (64 * hp_pct) / 100;
        tmp_clr.b = (64 * hp_pct) / 100;

        return tmp_clr;
    }

    return data_->color;
}

int Mon::spell_skill(const SpellId id) const
{
    (void)id;

    return data_->spell_skill;
}

void Mon::hear_sound(const Snd& snd)
{
    if (is_alive() &&
        snd.is_alerting_mon())
    {
        become_aware_player(false);
    }
}

void Mon::speak_phrase(const AlertsMon alerts_others)
{
    const bool is_seen_by_player = map::player->can_see_actor(*this);

    const std::string msg = is_seen_by_player ?
                            aggro_phrase_mon_seen() :
                            aggro_phrase_mon_hidden();

    const SfxId sfx = is_seen_by_player ?
                       aggro_sfx_mon_seen() :
                       aggro_sfx_mon_hidden();

    Snd snd(msg,
            sfx,
            IgnoreMsgIfOriginSeen::no,
            pos,
            this,
            SndVol::low,
            alerts_others);

    snd_emit::run(snd);
}

void Mon::become_aware_player(const bool is_from_seeing,
                              const int factor)
{
    if (!is_alive() || is_actor_my_leader(map::player))
    {
        return;
    }

    const int nr_turns_aware = data_->nr_turns_aware * factor;

    const int awareness_count_before = aware_counter_;

    aware_counter_= std::max(nr_turns_aware,
                             awareness_count_before);

    if (awareness_count_before <= 0)
    {
        if (is_from_seeing &&
            map::player->can_see_actor(*this))
        {
            msg_log::add(name_the() + " sees me!");
        }

        if (rnd::coin_toss())
        {
            speak_phrase(AlertsMon::yes);
        }
    }
}

void Mon::set_player_aware_of_me(int duration_factor)
{
    is_sneaking_ = false;

    int nr_turns = 2 * duration_factor;

    if (player_bon::bg() == Bg::rogue)
    {
        nr_turns *= 8;
    }

    player_aware_of_me_counter_ =
        std::max(player_aware_of_me_counter_, nr_turns);
}

bool Mon::try_attack(Actor& defender)
{
    if (state_ != ActorState::alive ||
        ((aware_counter_ <= 0) && (leader_ != map::player)))
    {
        return false;
    }

    AiAvailAttacksData my_avail_attacks;

    avail_attacks(defender,
                  my_avail_attacks);

    const AiAttData att = choose_att(my_avail_attacks);

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

        // Check if friend is in the way (with a small chance to ignore this)
        bool is_blocked_by_friend = false;

        if (rnd::fraction(4, 5))
        {
            std::vector<P> line;

            line_calc::calc_new_line(pos,
                                     defender.pos,
                                     true,
                                     9999,
                                     false,
                                     line);

            for (P& line_pos : line)
            {
                if (line_pos != pos && line_pos != defender.pos)
                {
                    Actor* const actor_here = map::actor_at_pos(line_pos);

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

        const int nr_turns_no_ranged = data_->ranged_cooldown_turns;

        PropDisabledRanged* ranged_cooldown_prop =
            new PropDisabledRanged(PropTurns::specific, nr_turns_no_ranged);

        prop_handler_->try_add(ranged_cooldown_prop);

        const bool did_attack = attack::ranged(this,
                                               pos,
                                               defender.pos,
                                               *att.wpn);

        return did_attack;
    }

    return false;
}

void Mon::avail_attacks(Actor& defender, AiAvailAttacksData& dst)
{
    if (prop_handler_->allow_attack(Verbosity::silent))
    {
        dst.is_melee = is_pos_adj(pos, defender.pos, false);

        Wpn* wpn = nullptr;
        const size_t nr_intrinsics = inv_->intrinsics_size();

        if (dst.is_melee)
        {
            if (prop_handler_->allow_attack_melee(Verbosity::silent))
            {
                // Melee weapon in wielded slot?
                wpn = static_cast<Wpn*>(inv_->item_in_slot(SlotId::wpn));

                if (wpn)
                {
                    if (wpn->data().melee.is_melee_wpn)
                    {
                        dst.weapons.push_back(wpn);
                    }
                }

                // Intrinsic melee attacks?
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
        else // Ranged attack
        {
            if (prop_handler_->allow_attack_ranged(Verbosity::silent))
            {
                // Ranged weapon in wielded slot?
                wpn = static_cast<Wpn*>(inv_->item_in_slot(SlotId::wpn));

                if (wpn)
                {
                    if (wpn->data().ranged.is_ranged_wpn)
                    {
                        dst.weapons.push_back(wpn);

                        // Check if reload time instead
                        if ((wpn->nr_ammo_loaded_ == 0) &&
                            !wpn->data().ranged.has_infinite_ammo)
                        {
                            if (inv_->has_ammo_for_firearm_in_inventory())
                            {
                                dst.is_reload_needed = true;
                            }
                        }
                    }
                }

                // Intrinsic ranged attacks?
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

AiAttData Mon::choose_att(const AiAvailAttacksData& mon_avail_attacks)
{
    AiAttData att(nullptr, mon_avail_attacks.is_melee);

    if (mon_avail_attacks.weapons.empty())
    {
        return att;
    }

    const size_t idx = rnd::range(0, mon_avail_attacks.weapons.size() - 1);

    att.wpn = mon_avail_attacks.weapons[idx];

    return att;
}

bool Mon::is_leader_of(const Actor* const actor) const
{
    if (!actor || actor->is_player())
        return false;

    // Actor is a monster
    return static_cast<const Mon*>(actor)->leader_ == this;
}

bool Mon::is_actor_my_leader(const Actor* const actor) const
{
    return leader_ == actor;
}

int Mon::nr_mon_in_group()
{
    const Actor* const group_leader = leader_ ? leader_ : this;

    int ret = 1; // Starting at one to include leader

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

    const God* const god = gods::current_god();

    if (god && rnd::coin_toss())
    {
        const std::string name = god->name();
        const std::string descr = god->descr();
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
    // If we are on a low dlvl, let the vast majority of cultists carry pistols
    const bool is_low_dlvl = map::dlvl < 4;

    const int pistol = is_low_dlvl ? 20 : 6;
    const int pump_shotgun = pistol + 3;
    const int sawn_shotgun = pump_shotgun + 3;
    const int mg = sawn_shotgun + 1;
    const int tot = mg;
    const int rnd = map::dlvl == 0 ? pistol : rnd::range(1, tot);

    if (rnd <= pistol)
    {
        Item* item = item_factory::mk(ItemId::pistol);
        Wpn* wpn = static_cast<Wpn*>(item);
        const int ammo_cap = wpn->data().ranged.max_ammo;

        wpn->nr_ammo_loaded_ = rnd::range(ammo_cap / 4, ammo_cap);

        inv_->put_in_slot(SlotId::wpn, item);

        if (rnd::one_in(6))
        {
            inv_->put_in_backpack(item_factory::mk(ItemId::pistol_mag));
        }
    }
    else if (rnd <= pump_shotgun)
    {
        Item* item = item_factory::mk(ItemId::pump_shotgun);
        Wpn* wpn = static_cast<Wpn*>(item);
        const int ammo_cap = wpn->data().ranged.max_ammo;

        wpn->nr_ammo_loaded_ = rnd::range(ammo_cap / 4, ammo_cap);

        inv_->put_in_slot(SlotId::wpn, item);

        if (rnd::one_in(4))
        {
            item = item_factory::mk(ItemId::shotgun_shell);
            item->nr_items_ = rnd::range(1, 8);
            inv_->put_in_backpack(item);
        }
    }
    else if (rnd <= sawn_shotgun)
    {
        inv_->put_in_slot(SlotId::wpn, item_factory::mk(ItemId::sawed_off));

        if (rnd::one_in(4))
        {
            Item* item = item_factory::mk(ItemId::shotgun_shell);

            item->nr_items_ = rnd::range(1, 8);

            inv_->put_in_backpack(item);
        }
    }
    else // Machine gun
    {
        // Number of machine gun bullets loaded needs to be a multiple of the
        // number of projectiles fired in each burst
        Item* item = item_factory::mk(ItemId::machine_gun);

        Wpn* const wpn = static_cast<Wpn*>(item);

        const int cap_scaled = wpn->data().ranged.max_ammo / nr_mg_projectiles;

        const int min_scaled = cap_scaled / 4;

        wpn->nr_ammo_loaded_ =
            rnd::range(min_scaled, cap_scaled) * nr_mg_projectiles;

        inv_->put_in_slot(SlotId::wpn, item);
    }

    if (rnd::one_in(8))
    {
        inv_->put_in_backpack(
            item_factory::mk_random_scroll_or_potion(true, true));
    }

    if (rnd::one_in(12))
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());

        if (rnd::coin_toss())
        {
            spells_known_.push_back(spell_handling::random_spell_for_mon());
        }
    }
}

void BogTcher::mk_start_items()
{
    Item* item = item_factory::mk(ItemId::spike_gun);

    Wpn* wpn = static_cast<Wpn*>(item);

    const int ammo_cap = wpn->data().ranged.max_ammo;

    wpn->nr_ammo_loaded_ = rnd::range(ammo_cap / 4, ammo_cap);

    inv_->put_in_slot(SlotId::wpn, item);

    if (rnd::one_in(4))
    {
        item = item_factory::mk(ItemId::iron_spike);
        item->nr_items_ = rnd::range(4, 12);
        inv_->put_in_backpack(item);
    }
}

void CultistPriest::mk_start_items()
{
    spells_known_.push_back(new SpellHealSelf);
    spells_known_.push_back(new SpellDarkbolt);
    spells_known_.push_back(new SpellEnfeebleMon);
    spells_known_.push_back(new SpellKnockBack);

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellTeleport);
    }

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellSummonMon);
    }

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellPest);
    }

    Item* item = item_factory::mk(ItemId::dagger);

    item->melee_dmg_plus_ = 2;

    inv_->put_in_slot(SlotId::wpn, item);

    // Make some treasures to drop
    for (int i = rnd::range(1, 2); i > 0; --i)
    {
        inv_->put_in_backpack(
            item_factory::mk_random_scroll_or_potion(true, true));
    }
}

void CultistWizard::mk_start_items()
{
    spells_known_.push_back(new SpellHealSelf);
    spells_known_.push_back(new SpellDarkbolt);
    spells_known_.push_back(new SpellEnfeebleMon);
    spells_known_.push_back(new SpellKnockBack);

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellTeleport);
    }

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellSummonMon);
    }

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellPest);
    }

    // Make some treasures to drop
    for (int i = rnd::range(1, 2); i > 0; --i)
    {
        inv_->put_in_backpack(
            item_factory::mk_random_scroll_or_potion(true, true));
    }
}

void CultistGrandWizard::mk_start_items()
{
    spells_known_.push_back(new SpellHealSelf);
    spells_known_.push_back(new SpellDarkbolt);
    spells_known_.push_back(new SpellEnfeebleMon);
    spells_known_.push_back(new SpellKnockBack);
    spells_known_.push_back(new SpellAzaWrath);
    spells_known_.push_back(new SpellBurn);

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellTeleport);
    }

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellSummonMon);
    }

    if (rnd::coin_toss())
    {
        spells_known_.push_back(new SpellPest);
    }

    // Make some treasures to drop
    for (int i = rnd::range(1, 2); i > 0; --i)
    {
        inv_->put_in_backpack(
            item_factory::mk_random_scroll_or_potion(true, true));
    }
}

void FireHound::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::fire_hound_breath));
    inv_->put_in_intrinsics(item_factory::mk(ItemId::fire_hound_bite));
}

void Zuul::place_hook()
{
    if (actor_data::data[(size_t)ActorId::zuul].nr_left_allowed_to_spawn > 0)
    {
        // NOTE: Do not call die() here - that would have side effects. Instead,
        //      simply set the dead state to destroyed.
        state_ = ActorState::destroyed;

        Actor* actor = actor_factory::mk(ActorId::cultist_priest, pos);
        auto& priest_prop_handler = actor->prop_handler();

        auto* poss_by_zuul_prop = new PropPossByZuul(PropTurns::indefinite);

        priest_prop_handler.try_add(poss_by_zuul_prop,
                                    PropSrc::intr,
                                    true,
                                    Verbosity::silent);

        actor->restore_hp(999, false, Verbosity::silent);
    }
}

void Zuul::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::zuul_bite));
}

DidAction Vortex::on_act()
{
    if (!is_alive())
    {
        return DidAction::no;
    }

    if (pull_cooldown > 0)
    {
        --pull_cooldown;
    }

    if (aware_counter_ <= 0 || pull_cooldown > 0)
    {
        return DidAction::no;
    }

    const P& player_pos = map::player->pos;

    if (!is_pos_adj(pos, player_pos, true) &&
        rnd::coin_toss())
    {
        TRACE << "Vortex attempting to pull player" << std::endl;

        const P delta = player_pos - pos;

        P knock_back_from_pos = player_pos;

        if (delta.x >  1)
        {
            ++knock_back_from_pos.x;
        }

        if (delta.x < -1)
        {
            --knock_back_from_pos.x;
        }

        if (delta.y >  1)
        {
            ++knock_back_from_pos.y;
        }

        if (delta.y < -1)
        {
            --knock_back_from_pos.y;
        }

        if (knock_back_from_pos != player_pos)
        {
            TRACE << "Pos found to knockback player from: "
                  << knock_back_from_pos.x << ", "
                  << knock_back_from_pos.y << std::endl;

            TRACE << "Player pos: "
                  << player_pos.x << ", " << player_pos.y << std::endl;

            bool blocked_los[map_w][map_h];

             const R fov_rect = fov::get_fov_rect(pos);

             map_parsers::BlocksLos()
                 .run(blocked_los,
                      MapParseMode::overwrite,
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

                // TODO: There should be a sfx here

                knock_back::try_knock_back(*map::player,
                                           knock_back_from_pos,
                                           false,
                                           false);

                pull_cooldown = 2;

                game_time::tick();

                return DidAction::yes;
            }
        }
    }

    return DidAction::no;
}

void DustVortex::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::dust_vortex_engulf));
}

void FireVortex::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::fire_vortex_engulf));
}

DidAction Ghost::on_act()
{
    if (is_alive() &&
        (aware_counter_ > 0) &&
        is_pos_adj(pos, map::player->pos, false) &&
        rnd::one_in(4))
    {
        set_player_aware_of_me();

        const bool player_sees_me =
            map::player->can_see_actor(*this);

        const std::string name =
            player_sees_me ?
            name_the() : "It";

        msg_log::add(name + " reaches for me...");

        map::player->prop_handler().try_add(
            new PropSlowed(PropTurns::std));

        game_time::tick();

        return DidAction::yes;
    }

    return DidAction::no;
}

void Ghost::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ghost_claw));
}

void Phantasm::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::phantasm_sickle));
}

void Wraith::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::wraith_claw));
    spells_known_.push_back(spell_handling::random_spell_for_mon());
    spells_known_.push_back(spell_handling::random_spell_for_mon());
}

void MiGo::mk_start_items()
{
    Item* item = item_factory::mk(ItemId::mi_go_gun);
    Wpn* wpn = static_cast<Wpn*>(item);
    const int ammo_cap = wpn->data().ranged.max_ammo;

    wpn->nr_ammo_loaded_ = rnd::range(ammo_cap / 4, ammo_cap);

    inv_->put_in_slot(SlotId::wpn, item);

    if (id() == ActorId::mi_go)
    {
        inv_->put_in_intrinsics(item_factory::mk(ItemId::mi_go_sting));
    }
    else if (id() == ActorId::mi_go_commander)
    {
        inv_->put_in_intrinsics(item_factory::mk(ItemId::mi_go_commander_sting));

        if (rnd::one_in(3))
        {
            inv_->put_in_slot(SlotId::body, item_factory::mk(ItemId::armor_mi_go));
        }
    }

    if (rnd::one_in(9))
    {
        inv_->put_in_backpack(item_factory::mk(ItemId::mi_go_gun_ammo));
    }

    spells_known_.push_back(new SpellTeleport);
    spells_known_.push_back(new SpellMiGoHypno);
    spells_known_.push_back(new SpellHealSelf);

    if (rnd::coin_toss())
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

void SentryDrone::mk_start_items()
{
    spells_known_.push_back(new SpellTeleport);
    spells_known_.push_back(new SpellHealSelf);
    spells_known_.push_back(new SpellDarkbolt);
    spells_known_.push_back(new SpellBurn);
}

void FlyingPolyp::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::polyp_tentacle));
}

void GreaterPolyp::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::greater_polyp_tentacle));
}

void Rat::mk_start_items()
{
    Item* item = nullptr;

    if (rnd::percent() < 15)
    {
        item = item_factory::mk(ItemId::rat_bite_diseased);
    }
    else
    {
        item = item_factory::mk(ItemId::rat_bite);
    }

    inv_->put_in_intrinsics(item);
}

void RatThing::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::rat_thing_bite));
}

void BrownJenkin::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::brown_jenkin_bite));

    spells_known_.push_back(new SpellTeleport);
}

void Shadow::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::shadow_claw));
}

void InvisStalker::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::invis_stalker_claw));
}

void Ghoul::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ghoul_claw));
}

void Ghoul::place_hook()
{
    // If player is Ghoul, then Ghouls are allied to player
    if (player_bon::bg() == Bg::ghoul)
    {
        leader_ = map::player;
    }
}

DidAction Ghoul::on_act()
{
    if (!is_alive())
    {
        return DidAction::no;
    }

    if (rnd::coin_toss())
    {
        const auto did_action = try_eat_corpse();

        if (did_action == DidAction::yes)
        {
            game_time::tick();

            return did_action;
        }
    }

    return DidAction::no;
}

void Mummy::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::mummy_maul));

    spells_known_.push_back(spell_handling::mk_spell_from_id(SpellId::disease));

    const int nr_spells = 3;

    for (int i = 0; i < nr_spells; ++i)
    {
        spells_known_.push_back(spell_handling::random_spell_for_mon());
    }
}

DidAction Mummy::on_act()
{

    return DidAction::no;
}

void MummyCrocHead::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::croc_head_mummy_spear));
}

void MummyUnique::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::mummy_maul));

    spells_known_.push_back(spell_handling::mk_spell_from_id(SpellId::disease));

    spells_known_.push_back(spell_handling::random_spell_for_mon());
    spells_known_.push_back(spell_handling::random_spell_for_mon());
    spells_known_.push_back(spell_handling::random_spell_for_mon());
}

DidAction Khephren::on_act()
{
    if (is_alive() &&
        (aware_counter_ > 0) &&
        !has_summoned_locusts)
    {
        bool blocked[map_w][map_h];

         const R fov_rect = fov::get_fov_rect(pos);

         map_parsers::BlocksLos()
             .run(blocked,
                  MapParseMode::overwrite,
                  fov_rect);

        if (can_see_actor(*(map::player), blocked))
        {
            map_parsers::BlocksMoveCmn(ParseActors::yes)
                .run(blocked);

            const int spawn_after_x = map_w / 2;

            for (int y = 0; y < map_h; ++y)
            {
                for (int x = 0; x <= spawn_after_x; ++x)
                {
                    blocked[x][y] = true;
                }
            }

            std::vector<P> free_cells;

            to_vec(blocked, false, free_cells);

            sort(begin(free_cells), end(free_cells), IsCloserToPos(pos));

            const size_t nr_of_spawns = 15;

            if (free_cells.size() >= nr_of_spawns + 1)
            {
                msg_log::add("Khephren calls a plague of Locusts!");

                map::player->incr_shock(ShockLvl::terrifying,
                                        ShockSrc::misc);

                for (size_t i = 0; i < nr_of_spawns; ++i)
                {
                    Actor* const actor =
                        actor_factory::mk(ActorId::locust, free_cells[0]);

                    Mon* const mon = static_cast<Mon*>(actor);
                    mon->aware_counter_ = 999;
                    mon->leader_ = leader_ ? leader_ : this;

                    free_cells.erase(begin(free_cells));
                }

                has_summoned_locusts = true;

                game_time::tick();

                return DidAction::yes;
            }
        }
    }

    return DidAction::no;
}

void DeepOne::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::deep_one_javelin_att));
    inv_->put_in_intrinsics(item_factory::mk(ItemId::deep_one_spear_att));
}

void Ape::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ape_maul));
}

DidAction Ape::on_act()
{
    if (frenzy_cooldown_ > 0)
    {
        --frenzy_cooldown_;
    }

    if ((frenzy_cooldown_ <= 0) &&
        tgt_ &&
        (hp() <= (hp_max(true) / 2)))
    {
        frenzy_cooldown_ = 30;

        const int nr_frenzy_turns = rnd::range(4, 6);

        prop_handler_->try_add(
            new PropFrenzied(PropTurns::specific, nr_frenzy_turns));
    }

    return DidAction::no;
}

void Raven::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::raven_peck));
}

void GiantBat::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::giant_bat_bite));
}

void Byakhee::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::byakhee_claw));
}

void GiantMantis::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::giant_mantis_claw));
}

void Chthonian::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::chthonian_bite));
}

void DeathFiend::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::death_fiend_claw));

    spells_known_.push_back(new SpellEnfeebleMon);
}

void HuntingHorror::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::hunting_horror_bite));
}

DidAction KeziahMason::on_act()
{
    if (is_alive() &&
        (aware_counter_ > 0) &&
        !has_summoned_jenkin)
    {
        bool blocked_los[map_w][map_h];

         const R fov_rect = fov::get_fov_rect(pos);

         map_parsers::BlocksLos()
             .run(blocked_los,
                  MapParseMode::overwrite,
                  fov_rect);

        if (can_see_actor(*(map::player), blocked_los))
        {
            map_parsers::BlocksMoveCmn(ParseActors::yes)
                .run(blocked_los);

            std::vector<P> line;
            line_calc::calc_new_line(pos,
                                     map::player->pos,
                                     true,
                                     9999,
                                     false,
                                     line);

            const int line_size = line.size();

            for (int i = 0; i < line_size; ++i)
            {
                const P c = line[i];

                if (!blocked_los[c.x][c.y])
                {
                    // TODO: Use the generalized summoning functionality
                    set_player_aware_of_me();

                    msg_log::add("Keziah summons Brown Jenkin!");

                    Actor* const actor =
                        actor_factory::mk(ActorId::brown_jenkin, c);

                    Mon* mon = static_cast<Mon*>(actor);

                    has_summoned_jenkin = true;
                    mon->aware_counter_ = 999;

                    mon->leader_ =
                        leader_ ?
                        leader_ : this;

                    game_time::tick();
                    return DidAction::yes;
                }
            }
        }
    }

    return DidAction::no;
}

void KeziahMason::mk_start_items()
{
    spells_known_.push_back(new SpellTeleport);
    spells_known_.push_back(new SpellHealSelf);
    spells_known_.push_back(new SpellSummonMon);
    spells_known_.push_back(new SpellPest);
    spells_known_.push_back(new SpellDarkbolt);
    spells_known_.push_back(new SpellEnfeebleMon);


    // Make some treasures to drop
    for (int i = rnd::range(2, 3); i > 0; --i)
    {
        inv_->put_in_backpack(
            item_factory::mk_random_scroll_or_potion(true, true));
    }
}

void LengElder::on_std_turn_hook()
{
    if (is_alive())
    {
        aware_counter_ += 100;

        if (has_given_item_to_player_)
        {
            bool blocked_los[map_w][map_h];

             const R fov_rect = fov::get_fov_rect(pos);

             map_parsers::BlocksLos()
                 .run(blocked_los,
                      MapParseMode::overwrite,
                      fov_rect);

            if (can_see_actor(*map::player, blocked_los))
            {
                if (nr_turns_to_hostile_ <= 0)
                {
                    msg_log::add("I am ripped to pieces!!!", clr_msg_bad);
                    map::player->hit(999, DmgType::pure);
                }
                else
                {
                    --nr_turns_to_hostile_;
                }
            }
        }
        else // Has not given item to player
        {
            const bool is_player_see_me = map::player->can_see_actor(*this);

            const bool is_player_adj = is_pos_adj(pos, map::player->pos, false);

            if (is_player_see_me && is_player_adj)
            {
                msg_log::add("I perceive a cloaked figure standing before me...",
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);

                msg_log::add("It is the Elder Hierophant of the Leng monastery, ");

                msg_log::add("the High Priest Not to Be Described.",
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);


                // auto& inv = map::player->inv();
                // TODO: Which item to give?
                // inv.put_in_backpack(item_factory::mk(ItemId::hideous_mask));

                has_given_item_to_player_ = true;
                nr_turns_to_hostile_ = rnd::range(9, 11);
            }
        }
    }
}

void LengElder::mk_start_items()
{

}

void Ooze::on_std_turn_hook()
{
    if (is_alive() && !prop_handler_->has_prop(PropId::burning))
    {
        restore_hp(1, false, Verbosity::silent);
    }
}

void OozeBlack::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ooze_black_spew_pus));
}

void OozeClear::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ooze_clear_spew_pus));
}

void OozePutrid::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ooze_putrid_spew_pus));
}

void OozePoison::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::ooze_poison_spew_pus));
}

void ColorOoSpace::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::color_oo_space_touch));
}

DidAction ColorOoSpace::on_act()
{
    Rigid* r = map::cells[pos.x][pos.y].rigid;

    r->corrupt_color();

    return DidAction::no;
}

Clr ColorOoSpace::clr() const
{
    Clr clr = clr_magenta_lgt;

    clr.r = rnd::range(40, 255);
    clr.g = rnd::range(40, 255);
    clr.b = rnd::range(40, 255);

    return clr;
}

void ColorOoSpace::on_std_turn_hook()
{
    if (is_alive())
    {
        if (!prop_handler_->has_prop(PropId::burning))
        {
            restore_hp(1, false, Verbosity::silent);
        }

        if (map::player->can_see_actor(*this))
        {
            const int nr_turns_confused = rnd::range(8, 12);

            map::player->prop_handler().try_add(
                new PropConfused(PropTurns::specific, nr_turns_confused));
        }
    }
}

DidAction Spider::on_act()
{
    return DidAction::no;
}

void GreenSpider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::green_spider_bite));
}

void WhiteSpider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::white_spider_bite));
}

void RedSpider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::red_spider_bite));
}

void ShadowSpider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::shadow_spider_bite));
}

void LengSpider::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::leng_spider_bite));
}

void PitViper::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::pit_viper_bite));
}

void SpittingCobra::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::spitting_cobra_bite));
    inv_->put_in_intrinsics(item_factory::mk(ItemId::spitting_cobra_spit));
}

void BlackMamba::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::black_mamba_bite));
}

void Wolf::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::wolf_bite));
}

void WormMass::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::worm_mass_bite));
}

void WormMass::on_death()
{
    // Split into other worms on death. Splitting is only allowed if this is an
    // "original" worm, i.e. not split from another, and if the worm is not
    // destroyed "too hard" (e.g. by a near explosion or a sledge hammer), and
    // if the worm is not burning.

    if (allow_split_ &&
        (hp_ > -10) &&
        !prop_handler_->has_prop(PropId::burning) &&
        !map::cells[pos.x][pos.y].rigid->is_bottomless() &&
        game_time::actors.size() < max_nr_actors_on_map)
    {
        std::vector<ActorId> worm_ids(2, id());

        std::vector<Mon*> summoned;

        // NOTE: If dying worm has a leader, that actor is set as leader for the
        //       spawned worms
        actor_factory::summon(pos,
                              worm_ids,
                              MakeMonAware::yes,
                              leader_,
                              &summoned,
                              Verbosity::silent);

        ASSERT(summoned.size() == 2);

        // If no leader has been set yet, assign the first worm as leader of the
        // second
        if (!leader_ && summoned.size() >= 2)
        {
            summoned[1]->leader_ = summoned[0];
        }

        for (Mon* const mon : summoned)
        {
            // Do not allow summoned worms to split again
            static_cast<WormMass*>(mon)->allow_split_ = false;

            // Do not allow summoned worms to attack immediately
            mon->prop_handler().try_add(
                new PropDisabledAttack(PropTurns::specific, 1));
        }
    }
}

void MindWorms::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::mind_worms_bite));
}

DidAction GiantLocust::on_act()
{
    if (is_alive() &&
        (aware_counter_ > 0) &&
        !prop_handler_->has_prop(PropId::burning) &&
        (game_time::actors.size() < max_nr_actors_on_map) &&
        rnd::one_in(spawn_new_one_in_n))
    {
        bool blocked[map_w][map_h];

        map_parsers::BlocksActor(*this, ParseActors::yes)
            .run(blocked,
                 MapParseMode::overwrite,
                 R(pos - 1, pos + 1));

        for (const P& d : dir_utils::dir_list)
        {
            const P p_adj(pos + d);

            if (!blocked[p_adj.x][p_adj.y])
            {
                Actor* const actor = actor_factory::mk(data_->id, p_adj);

                GiantLocust* const locust = static_cast<GiantLocust*>(actor);

                spawn_new_one_in_n += 4;

                locust->spawn_new_one_in_n = spawn_new_one_in_n;
                locust->aware_counter_ = aware_counter_;
                locust->leader_ = leader_ ? leader_ : this;

                game_time::tick();

                return DidAction::yes;
            }
        }
    }

    return DidAction::no;
}

void GiantLocust::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::giant_locust_bite));
}

DidAction LordOfShadows::on_act()
{
    return DidAction::no;
}

void LordOfShadows::mk_start_items()
{

}

DidAction LordOfSpiders::on_act()
{
    if (is_alive() &&
        (aware_counter_ > 0) &&
        rnd::coin_toss())
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

                        auto* const mimic = static_cast<Rigid*>(d.mk_obj(p));

                        Trap* const f = new Trap(p, mimic, TrapId::web);

                        map::put(f);

                        f->reveal(Verbosity::silent);
                    }
                }
            }
        }
    }

    return DidAction::no;
}

void LordOfSpiders::mk_start_items()
{

}

DidAction LordOfSpirits::on_act()
{
    return DidAction::no;
}

void LordOfSpirits::mk_start_items()
{

}

DidAction LordOfPestilence::on_act()
{
    return DidAction::no;
}

void LordOfPestilence::mk_start_items()
{

}

DidAction Zombie::on_act()
{
    return try_resurrect();
}

DidAction Zombie::try_resurrect()
{
    if (!is_corpse() || has_resurrected)
    {
        return DidAction::no;
    }

    const int min_nr_turns_until_rise = 2;
    const int rise_one_in_n = 4;

    if (dead_turn_counter < min_nr_turns_until_rise)
    {
        ++dead_turn_counter;
    }
    else if (rnd::one_in(rise_one_in_n))
    {
        Actor* actor = map::actor_at_pos(pos);

        if (!actor)
        {
            state_ = ActorState::alive;
            hp_ = (hp_max(true) * 3) / 4;

            has_resurrected = true;

            --data_->nr_kills;

            if (map::cells[pos.x][pos.y].is_seen_by_player)
            {
                msg_log::add(corpse_name_the() + " rises again!!",
                             clr_text,
                             true);

                map::player->incr_shock(ShockLvl::frightening,
                                        ShockSrc::see_mon);
            }

            aware_counter_ += data_->nr_turns_aware * 2;

            game_time::tick();

            return DidAction::yes;
        }
    }

    return DidAction::no;
}

void Zombie::on_death()
{
    // If resurrected once and has corpse, blow up the corpse
    if (has_resurrected && is_corpse())
    {
        state_ = ActorState::destroyed;
        map::mk_blood(pos);
        map::mk_gore(pos);
    }

    // If corpse is destroyed, occasionally spawn Zombie parts. Spawning is
    // only allowed if the corpse is not destroyed "too hard" (e.g. by a near
    // explosion or a sledge hammer). This also serves to reward heavy weapons,
    // since they will more often prevent spawning nasty stuff.

    const int summon_one_in_n = 7;

    if ((state_ == ActorState::destroyed) &&
        (hp_ > -8) &&
        !map::cells[pos.x][pos.y].rigid->is_bottomless() &&
        rnd::one_in(summon_one_in_n))
    {
        ActorId id_to_spawn = ActorId::END;

        // With a small chance, spawn a Floating Skull, otherwise spawn Hands or
        // Intestines
        const int roll = rnd::one_in(50) ? 3 : rnd::range(1, 2);

        const std::string my_name = name_the();

        std::string spawn_msg = "";

        if (roll == 1)
        {
            id_to_spawn = ActorId::crawling_hand;

            spawn_msg =
                "The hand of " +
                my_name +
                " comes off and starts crawling around!";
        }
        else if (roll == 2)
        {
            id_to_spawn = ActorId::crawling_intestines;

            spawn_msg =
                "The intestines of " +
                my_name +
                " starts crawling around!";
        }
        else
        {
            id_to_spawn = ActorId::floating_skull;

            spawn_msg =
                "The head of " +
                my_name +
                " starts floating around!";
        }

        if (map::cells[pos.x][pos.y].is_seen_by_player)
        {
            ASSERT(!spawn_msg.empty());

            msg_log::add(spawn_msg);

            map::player->incr_shock(ShockLvl::frightening,
                                    ShockSrc::see_mon);
        }

        ASSERT(id_to_spawn != ActorId::END);

        actor_factory::summon(pos, {id_to_spawn},
                              MakeMonAware::yes,
                              nullptr,
                              nullptr,
                              Verbosity::silent);
    }
}

void ZombieClaw::mk_start_items()
{
    Item* item = nullptr;

    if (rnd::percent() < 20)
    {
        item = item_factory::mk(ItemId::zombie_claw_diseased);
    }
    else // Not diseased
    {
        item = item_factory::mk(ItemId::zombie_claw);
    }

    inv_->put_in_intrinsics(item);
}

void ZombieAxe::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::zombie_axe));
}

void BloatedZombie::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::bloated_zombie_punch));
    inv_->put_in_intrinsics(item_factory::mk(ItemId::bloated_zombie_spit));
}

DidAction MajorClaphamLee::on_act()
{
    if (try_resurrect() == DidAction::yes)
    {
        return DidAction::yes;
    }

    if (is_alive() &&
        (aware_counter_ > 0) &&
        !has_summoned_tomb_legions)
    {
        bool blocked_los[map_w][map_h];

         const R fov_rect = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
            .run(blocked_los,
                 MapParseMode::overwrite,
                 fov_rect);

        if (can_see_actor(*(map::player), blocked_los))
        {
            set_player_aware_of_me();

            msg_log::add("Major Clapham Lee calls forth his Tomb-Legions!");

            std::vector<ActorId> mon_ids = {ActorId::dean_halsey};

            const int nr_of_extra_spawns = 4;

            for (int i = 0; i < nr_of_extra_spawns; ++i)
            {
                const int zombie_type = rnd::range(1, 3);

                ActorId mon_id = ActorId::zombie;

                switch (zombie_type)
                {
                case 1:
                    mon_id = ActorId::zombie;
                    break;

                case 2:
                    mon_id = ActorId::zombie_axe;
                    break;

                case 3:
                    mon_id = ActorId::bloated_zombie;
                    break;
                }

                mon_ids.push_back(mon_id);
            }

            actor_factory::summon(pos, mon_ids,
                                  MakeMonAware::yes,
                                  this);

            has_summoned_tomb_legions = true;

            map::player->incr_shock(ShockLvl::terrifying,
                                    ShockSrc::misc);

            game_time::tick();

            return DidAction::yes;
        }
    }

    return DidAction::no;
}

void CrawlingIntestines::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::crawling_intestines_strangle));
}

void CrawlingHand::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::crawling_hand_strangle));
}

void Thing::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::thing_strangle));
    spells_known_.push_back(new SpellTeleport);
}

void FloatingSkull::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::floating_skull_bite));
}

DidAction FloatingSkull::on_act()
{
    if (is_alive() &&
        (aware_counter_ > 0) &&
        rnd::one_in(8))
    {
        bool blocked_los[map_w][map_h];

         const R fov_rect = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
            .run(blocked_los,
                 MapParseMode::overwrite,
                 fov_rect);

        if (can_see_actor(*map::player, blocked_los))
        {
            const std::string name = name_the();

            const bool player_see_me = map::player->can_see_actor(*this);

            std::string snd_msg = player_see_me ? name : "Someone";

            snd_msg += " spews forth a litany of curses.";

            Snd snd(snd_msg,
                    SfxId::END,
                    IgnoreMsgIfOriginSeen::no,
                    pos,
                    this,
                    SndVol::high,
                    AlertsMon::no);

            snd_emit::run(snd);

            Prop* const prop = new PropCursed(PropTurns::std);

            map::player->prop_handler().try_add(prop, PropSrc::intr);

            return DidAction::yes;
        }
    }

    return DidAction::no;
}

DidAction Mold::on_act()
{
    if (is_alive() &&
        (game_time::actors.size() < max_nr_actors_on_map) &&
        rnd::one_in(spawn_new_one_in_n))
    {
        bool blocked[map_w][map_h];

        map_parsers::BlocksActor(*this, ParseActors::yes)
            .run(blocked,
                 MapParseMode::overwrite,
                 R(pos - 1, pos + 1));

        for (const P& d : dir_utils::dir_list)
        {
            const P adj_pos(pos + d);

            if (!blocked[adj_pos.x][adj_pos.y])
            {
                Actor* const actor = actor_factory::mk(data_->id, adj_pos);
                Mold* const mold = static_cast<Mold*>(actor);
                mold->aware_counter_ = aware_counter_;
                mold->leader_ = leader_ ? leader_ : this;
                game_time::tick();
                return DidAction::yes;
            }
        }
    }

    return DidAction::no;
}

void Mold::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::mold_spores));
}

void GasSpore::on_death()
{
    TRACE_FUNC_BEGIN;

    explosion::run(pos, ExplType::expl);

    TRACE_FUNC_END;
}

TheHighPriest::TheHighPriest() :
    Mon                 (),
    has_greeted_player_ (false) {}

void TheHighPriest::mk_start_items()
{
    inv_->put_in_intrinsics(item_factory::mk(ItemId::the_high_priest_claw));

    spells_known_.push_back(new SpellSummonMon());
    spells_known_.push_back(new SpellBurn());
    spells_known_.push_back(new SpellEnfeebleMon());
    spells_known_.push_back(new SpellMiGoHypno());
    spells_known_.push_back(new SpellPest());
    spells_known_.push_back(new SpellTeleport());
    spells_known_.push_back(new SpellAzaWrath());
}

void TheHighPriest::on_death()
{
    msg_log::add("The ground rumbles...",
                 clr_white,
                 false,
                 MorePromptOnMsg::yes);

    const P stair_pos(map_w - 2, 11);

    map::put(new Stairs(stair_pos));
    map::put(new RubbleLow(stair_pos - P(1, 0)));

    const int nr_snakes = rnd::range(4, 5);

    std::vector<ActorId> snake_ids(nr_snakes, ActorId::pit_viper);

    actor_factory::summon(pos, snake_ids);
}

void TheHighPriest::on_std_turn_hook()
{
    const int regen_every_n_turn = 3;

    if (is_alive() && (game_time::turn_nr() % regen_every_n_turn == 0))
    {
        restore_hp(1, false, Verbosity::silent);
    }
}

DidAction TheHighPriest::on_act()
{
    if (!is_alive())
    {
        return DidAction::no;
    }

    if (!has_greeted_player_)
    {
        msg_log::add("A booming voice echoes through the halls.",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        audio::play(SfxId::boss_voice1);

        has_greeted_player_ = true;
        aware_counter_ += data_->nr_turns_aware;
    }

    if (rnd::coin_toss())
    {
        map::cells[pos.x][pos.y].rigid->mk_bloody();
        map::cells[pos.x][pos.y].rigid->try_put_gore();
    }

    return DidAction::no;
}

AnimatedWpn::AnimatedWpn() :
    Mon                     (),
    nr_turns_until_drop_    (rnd::range(75, 125)) {}

std::string AnimatedWpn::name_the() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    ASSERT(item);

    const std::string name = item->name(ItemRefType::plain,
                                        ItemRefInf::yes,
                                        ItemRefAttInf::none);

    return "The floating " + name;
}

std::string AnimatedWpn::name_a() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    ASSERT(item);

    const std::string name = item->name(ItemRefType::plain,
                                        ItemRefInf::yes,
                                        ItemRefAttInf::none);

    return "A floating " + name;
}

char AnimatedWpn::glyph() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    ASSERT(item);

    return item->glyph();
}

Clr AnimatedWpn::clr() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    ASSERT(item);

    return item->clr();
}

TileId AnimatedWpn::tile() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    ASSERT(item);

    return item->tile();
}

std::string AnimatedWpn::descr() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    ASSERT(item);

    std::string str = item->name(ItemRefType::a,
                                  ItemRefInf::yes,
                                  ItemRefAttInf::none);

    text_format::first_to_upper(str);

    str += ", floating through the air as if wielded by some invisible hand.";

    return str;
}

std::string AnimatedWpn::death_msg() const
{
    Item* item = inv_->item_in_slot(SlotId::wpn);

    if (!item)
    {
        ASSERT(false);

        // Release build robustness
        return "";
    }

    const std::string name = item->name(ItemRefType::plain,
                                        ItemRefInf::yes,
                                        ItemRefAttInf::none);

    return "The " + name + " suddenly becomes lifeless and drops down.";
}

void AnimatedWpn::on_std_turn_hook()
{
    if (!is_alive())
    {
        return;
    }

    if (nr_turns_until_drop_ <= 0)
    {
        die(true, false, true);
    }
    else // Not yet time to die
    {
        --nr_turns_until_drop_;
    }
}
