#include "actor_player.hpp"

#include <string>

#include "init.hpp"
#include "render.hpp"
#include "audio.hpp"
#include "feature_trap.hpp"
#include "create_character.hpp"
#include "msg_log.hpp"
#include "character_lines.hpp"
#include "popup.hpp"
#include "dungeon_master.hpp"
#include "map.hpp"
#include "explosion.hpp"
#include "actor_mon.hpp"
#include "feature_door.hpp"
#include "feature_mob.hpp"
#include "query.hpp"
#include "attack.hpp"
#include "fov.hpp"
#include "item_factory.hpp"
#include "actor_factory.hpp"
#include "player_bon.hpp"
#include "inventory.hpp"
#include "inventory_handling.hpp"
#include "player_spells_handling.hpp"
#include "bot.hpp"
#include "input.hpp"
#include "map_parsing.hpp"
#include "properties.hpp"
#include "item_device.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "text_format.hpp"
#include "save_handling.hpp"
#include "insanity.hpp"
#include "reload.hpp"

Player::Player() :
    Actor(),
    active_medical_bag          (nullptr),
    active_explosive            (nullptr),
    tgt_                        (nullptr),
    wait_turns_left             (-1),
    ins_                        (0),
    shock_                      (0.0),
    shock_tmp_                  (0.0),
    perm_shock_taken_cur_turn_  (0.0),
    nr_steps_until_free_action_ (-1),
    nr_turns_until_ins_         (-1),
    nr_quick_move_steps_left_   (-1),
    quick_move_dir_             (Dir::END),
    nr_turns_until_rspell_      (-1),
    unarmed_wpn_                (nullptr) {}

Player::~Player()
{
    delete active_explosive;
    delete unarmed_wpn_;
}

void Player::mk_start_items()
{
    data_->ability_vals.reset();

    bool has_pistol         = true;
    bool has_medbag         = true;
    bool has_lantern        = true;
    bool has_leather_jacket = true;

    int nr_cartridges   = 2;
    int nr_dynamite     = 2;
    int nr_molotov      = 2;
    int nr_thr_knives   = 6;

    //------------------------------------------------------- BACKGROUND SPECIFIC SETUP
    const Bg bg = player_bon::bg();

    switch (bg)
    {
    case Bg::occultist:
    {
        //Occultist starts with one less cartridge, and no explosives or
        //throwing knives.
        --nr_cartridges;
        nr_dynamite     = 0;
        nr_molotov      = 0;
        nr_thr_knives   = 0;

        //Occultist starts with a Darkbolt scroll, and one other random scroll.
        //(Both are identified.)
        Item* scroll = item_factory::mk(ItemId::scroll_darkbolt);
        static_cast<Scroll*>(scroll)->identify(Verbosity::silent);
        inv_->put_in_backpack(scroll);

        while (true)
        {
            scroll = item_factory::mk_random_scroll_or_potion(true, false);

            SpellId        spell_id     = scroll->data().spell_cast_from_scroll;
            Spell* const    spell       = spell_handling::mk_spell_from_id(spell_id);
            const bool      is_avail    = spell->is_avail_for_player();
            const bool      spi_cost_ok = spell->spi_cost(true).max <=
                                          player_bon::spi_occultist_can_cast_at_lvl(4);
            delete spell;

            if (is_avail && spi_cost_ok && spell_id != SpellId::darkbolt)
            {
                static_cast<Scroll*>(scroll)->identify(Verbosity::silent);
                inv_->put_in_backpack(scroll);
                break;
            }
        }

        //Occultist starts with a few potions (identified).
        const int nr_potions = 2;

        for (int i = 0; i < nr_potions; ++i)
        {
            Item* const potion = item_factory::mk_random_scroll_or_potion(false, true);
            static_cast<Potion*>(potion)->identify(Verbosity::silent);
            inv_->put_in_backpack(potion);
        }
    }
    break;

    case Bg::rogue:
    {
        //Rogue starts with extra throwing knives
        nr_thr_knives += 6;

        //Rogue starts with a +1 dagger
        auto* const dagger = item_factory::mk(ItemId::dagger);

        static_cast<Wpn*>(dagger)->melee_dmg_plus_ = 1;

        inv_->put_in_slot(SlotId::wpn, dagger);

        //Rogue starts with some iron spikes (useful tool)
        inv_->put_in_backpack(item_factory::mk(ItemId::iron_spike, 8));
    }
    break;

    case Bg::war_vet:
    {
        //War Veteran starts with an extra cartridge, some smoke grenades and a
        //gas mask
        ++nr_cartridges;
        inv_->put_in_backpack(item_factory::mk(ItemId::smoke_grenade, 4));
        inv_->put_in_backpack(item_factory::mk(ItemId::gas_mask));
    }
    break;

    case Bg::ghoul:
    {
        //Ghoul starts with no items
        has_pistol          = false;
        has_medbag          = false;
        has_lantern         = false;
        has_leather_jacket  = false;

        nr_cartridges   = 0;
        nr_dynamite     = 0;
        nr_molotov      = 0;
        nr_thr_knives   = 0;
    }
    break;

    case Bg::END:
        break;
    }

    //------------------------------------------------------- GENERAL SETUP
    //Randomize a melee weapon if Occultist or War Veteran
    if (bg == Bg::occultist || bg == Bg::war_vet)
    {
        const int weapon_choice = rnd::range(1, 5);
        auto      weapon_id     = ItemId::dagger;

        switch (weapon_choice)
        {
        case 1:
            weapon_id = ItemId::dagger;
            break;

        case 2:
            weapon_id = ItemId::hatchet;
            break;

        case 3:
            weapon_id = ItemId::hammer;
            break;

        case 4:
            weapon_id = ItemId::machete;
            break;

        case 5:
            weapon_id = ItemId::axe;
            break;
        }

        inv_->put_in_slot(SlotId::wpn, item_factory::mk(weapon_id));
    }

    //Unarmed attack
    if (player_bon::bg() == Bg::ghoul)
    {
        unarmed_wpn_ = static_cast<Wpn*>(item_factory::mk(ItemId::player_ghoul_claw));
    }
    else //Not ghoul
    {
        unarmed_wpn_ = static_cast<Wpn*>(item_factory::mk(ItemId::player_punch));
    }

    if (has_pistol)
    {
        inv_->put_in_slot(SlotId::wpn_alt, item_factory::mk(ItemId::pistol));
    }

    for (int i = 0; i < nr_cartridges; ++i)
    {
        inv_->put_in_backpack(item_factory::mk(ItemId::pistol_mag));
    }

    if (nr_dynamite > 0)
    {
        inv_->put_in_backpack(item_factory::mk(ItemId::dynamite,  nr_dynamite));
    }

    if (nr_molotov > 0)
    {
        inv_->put_in_backpack(item_factory::mk(ItemId::molotov,   nr_molotov));
    }

    if (nr_thr_knives > 0)
    {
        inv_->put_in_slot(SlotId::thrown, item_factory::mk(ItemId::thr_knife, nr_thr_knives));
    }

    if (has_medbag)
    {
        inv_->put_in_backpack(item_factory::mk(ItemId::medical_bag));
    }

    if (has_lantern)
    {
        inv_->put_in_backpack(item_factory::mk(ItemId::lantern));
    }

    if (has_leather_jacket)
    {
        inv_->put_in_slot(SlotId::body, item_factory::mk(ItemId::armor_leather_jacket));
    }
}

void Player::save() const
{
    prop_handler_->save();

    save_handling::put_int(ins_);
    save_handling::put_int(int(shock_));
    save_handling::put_int(hp_);
    save_handling::put_int(hp_max_);
    save_handling::put_int(spi_);
    save_handling::put_int(spi_max_);
    save_handling::put_int(pos.x);
    save_handling::put_int(pos.y);
    save_handling::put_int(nr_steps_until_free_action_);
    save_handling::put_int(nr_turns_until_rspell_);

    ASSERT(unarmed_wpn_);

    save_handling::put_int((int)unarmed_wpn_->id());

    for (int i = 0; i < (int)AbilityId::END; ++i)
    {
        const int v = data_->ability_vals.raw_val(AbilityId(i));

        save_handling::put_int(v);
    }
}

void Player::load()
{
    prop_handler_->load();

    ins_                        = save_handling::get_int();
    shock_                      = double(save_handling::get_int());
    hp_                         = save_handling::get_int();
    hp_max_                     = save_handling::get_int();
    spi_                        = save_handling::get_int();
    spi_max_                    = save_handling::get_int();
    pos.x                       = save_handling::get_int();
    pos.y                       = save_handling::get_int();
    nr_steps_until_free_action_ = save_handling::get_int();
    nr_turns_until_rspell_      = save_handling::get_int();

    ItemId unarmed_wpn_id = ItemId(save_handling::get_int());

    ASSERT(unarmed_wpn_id < ItemId::END);

    delete unarmed_wpn_;
    unarmed_wpn_ = nullptr;

    Item* const unarmed_item = item_factory::mk(unarmed_wpn_id);

    ASSERT(unarmed_item);

    unarmed_wpn_ = static_cast<Wpn*>(unarmed_item);

    for (int i = 0; i < (int)AbilityId::END; ++i)
    {
        const int v = save_handling::get_int();

        data_->ability_vals.set_val(AbilityId(i), v);
    }
}

bool Player::can_see_actor(const Actor& other) const
{
    if (this == &other)
    {
        return true;
    }

    if (init::is_cheat_vision_enabled)
    {
        return true;
    }

    const Cell& cell = map::cells[other.pos.x][other.pos.y];

    //Dead actors are seen if the cell is seen
    if (!other.is_alive() && cell.is_seen_by_player)
    {
        return true;
    }

    //Player is blind?
    if (!prop_handler_->allow_see())
    {
        return false;
    }

    const Mon* const mon = static_cast<const Mon*>(&other);

    //LOS blocked hard (e.g. a wall)?
    if (cell.player_los.is_blocked_hard)
    {
        return false;
    }

    const bool can_see_invis = has_prop(PropId::see_invis);

    //Monster is invisible, and player cannot see invisible?
    if (other.has_prop(PropId::invis) && !can_see_invis)
    {
        return false;
    }

    //Blocked by darkness, and not seeing monster with infravision?
    bool        HAS_INFRAVIS                = prop_handler_->has_prop(PropId::infravis);
    const bool  is_other_infra_visible      = other.data().is_infra_visible;

    const bool  can_see_other_with_infravis = HAS_INFRAVIS && is_other_infra_visible;

    const bool  can_see_other_in_drk        = can_see_invis || can_see_other_with_infravis;

    if (cell.player_los.is_blocked_by_drk && !can_see_other_in_drk)
    {
        return false;
    }

    //Monster is sneaking, and we cannot see it with infravision or magic seeing?
    if (
        mon->player_aware_of_me_counter_ <= 0   &&
        mon->is_sneaking_                       &&
        !can_see_other_with_infravis            &&
        !can_see_invis)
    {
        return false;
    }

    //OK, all checks passed, actor can bee seen!
    return true;
}

void Player::on_hit(int& dmg,
                    const DmgType dmg_type,
                    const DmgMethod method,
                    const AllowWound allow_wound)
{
    (void)method;

    if (!insanity::has_sympt(InsSymptId::masoch))
    {
        incr_shock(1, ShockSrc::misc);
    }

    const bool is_enough_dmg_for_wound  = dmg >= min_dmg_to_wound;
    const bool is_physical              = dmg_type == DmgType::physical;

    //Ghoul trait Indomitable Fury makes player immune to Wounds while Frenzied
    const bool is_ghoul_resist_wound = player_bon::traits[(size_t)Trait::indomitable_fury] &&
                                       prop_handler_->has_prop(PropId::frenzied);

    if (
        allow_wound == AllowWound::yes &&
        is_enough_dmg_for_wound         &&
        is_physical                     &&
        !is_ghoul_resist_wound          &&
        !config::is_bot_playing())
    {
        Prop* const prop = new PropWound(PropTurns::indefinite);

        auto nr_wounds = [&]()
        {
            if (prop_handler_->has_prop(PropId::wound))
            {
                const Prop* const prop = prop_handler_->prop(PropId::wound);

                const PropWound* const wound = static_cast<const PropWound*>(prop);

                return wound->nr_wounds();
            }

            return 0;
        };

        const int nr_wounds_before = nr_wounds();

        prop_handler_->try_add(prop);

        const int nr_wounds_after = nr_wounds();

        if (nr_wounds_after > nr_wounds_before)
        {
            if (insanity::has_sympt(InsSymptId::masoch))
            {
                dungeon_master::add_history_event("Experienced a very pleasant wound.");

                msg_log::add("Hehehe...");

                const double shock_restored = 10.0;

                shock_ = std::max(0.0, shock_ - shock_restored);
            }
            else //Not masochistic
            {
                dungeon_master::add_history_event("Sustained a severe wound.");
            }
        }
    }

    render::draw_map_state();
}

int Player::enc_percent() const
{
    const int total_w = inv_->total_item_weight();
    const int max_w   = carry_weight_lmt();

    return (int)(((double)total_w / (double)max_w) * 100.0);
}

int Player::carry_weight_lmt() const
{
    const bool is_tough         = player_bon::traits[(size_t)Trait::tough];
    const bool is_rugged        = player_bon::traits[(size_t)Trait::rugged];
    const bool is_unbreakable   = player_bon::traits[(size_t)Trait::unbreakable];
    const bool is_strong_backed = player_bon::traits[(size_t)Trait::strong_backed];

    const bool is_weakened      = has_prop(PropId::weakened);

    const int carry_weight_mod = (is_tough         * 10) +
                                 (is_rugged        * 10) +
                                 (is_unbreakable   * 10) +
                                 (is_strong_backed * 30) -
                                 (is_weakened      * 15);

    return (player_carry_weight_base * (carry_weight_mod + 100)) / 100;
}

int Player::shock_resistance(const ShockSrc shock_src) const
{
    int res = 0;

    if (player_bon::traits[(size_t)Trait::fearless])
    {
        res += 5;
    }

    if (player_bon::traits[(size_t)Trait::cool_headed])
    {
        res += 20;
    }

    if (player_bon::traits[(size_t)Trait::courageous])
    {
        res += 20;
    }

    switch (shock_src)
    {
    case ShockSrc::use_strange_item:
        if (player_bon::bg() == Bg::occultist)
        {
            res += 50;
        }
        break;

    case ShockSrc::see_mon:
        if (player_bon::bg() == Bg::ghoul)
        {
            res += 50;
        }
        break;

    case ShockSrc::cast_intr_spell:
    case ShockSrc::time:
    case ShockSrc::misc:
    case ShockSrc::END:
        break;
    }

    return constr_in_range(0, res, 100);
}

double Player::shock_taken_after_mods(const int base_shock,
                                      const ShockSrc shock_src) const
{
    if (base_shock == 0)
    {
        return 0.0;
    }

    const double shock_res_db   = double(shock_resistance(shock_src));
    const double base_shock_db  = double(base_shock);
    return (base_shock_db * (100.0 - shock_res_db)) / 100.0;
}

void Player::incr_shock(const int shock, ShockSrc shock_src)
{
    const double shock_after_mods = shock_taken_after_mods(shock, shock_src);

    shock_                      += shock_after_mods;
    perm_shock_taken_cur_turn_  += shock_after_mods;

    set_constr_in_range(0.0, shock_, 100.0);
}

void Player::incr_shock(const ShockLvl shock_value, ShockSrc shock_src)
{
    incr_shock((int)shock_value, shock_src);
}

void Player::restore_shock(const int amount_restored, const bool is_temp_shock_restored)
{
    shock_ = std::max(0.0, shock_ - amount_restored);

    if (is_temp_shock_restored)
    {
        shock_tmp_ = 0.0;
    }

    update_tmp_shock();
}

void Player::incr_insanity()
{
    TRACE << "Increasing insanity" << std::endl;

    if (!config::is_bot_playing())
    {
        const int ins_incr = 5;

        ins_ += ins_incr;
    }

    if (ins() >= 100)
    {
        const std::string msg = "My mind can no longer withstand what it has grasped. "
                                "I am hopelessly lost.";
        popup::show_msg(msg, true, "Insane!", SfxId::insanity_rise);
        die(true, false, false);
        return;
    }

    //This point reached means sanity is below 100%
    insanity::run_sympt();

    restore_shock(70, false);

    update_clr();
    render::draw_map_state();
}

bool Player::is_standing_in_open_place() const
{
     const R r(pos - 1, pos + 1);

    bool blocked[map_w][map_h];

    //NOTE: Checking if adjacent cells blocks projectiles is probably the best
    //way to determine if this is an open place. If we check for things that
    //block common movement, stuff like chasms would count as blocking.
    map_parse::run(cell_check::BlocksProjectiles(),
                   blocked,
                   MapParseMode::overwrite,
                   r);

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            if (blocked[x][y])
            {
                return false;
            }
        }
    }

    return true;
}

bool Player::is_standing_in_cramped_place() const
{
     const R r(pos - 1, pos + 1);

    bool blocked[map_w][map_h];

    //NOTE: Checking if adjacent cells blocks projectiles is probably the best
    //way to determine if this is an open place. If we check for things that
    //block common movement, stuff like chasms would count as blocking.
    map_parse::run(cell_check::BlocksProjectiles(),
                   blocked,
                   MapParseMode::overwrite,
                   r);

    int block_count = 0;

    const int min_nr_blocked_for_cramped = 6;

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            if (blocked[x][y])
            {
                ++block_count;

                if (block_count >= min_nr_blocked_for_cramped)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void Player::update_clr()
{
    if (!is_alive())
    {
        clr_ = clr_red;
        return;
    }

    if (prop_handler_->affect_actor_clr(clr_))
    {
        return;
    }

    if (active_explosive)
    {
        clr_ = clr_yellow;
        return;
    }

    const int cur_shock = shock_ + shock_tmp_;

    if (cur_shock >= 75)
    {
        clr_ = clr_magenta;
        return;
    }

    clr_ = data_->color;
}

void Player::set_quick_move(const Dir dir)
{
    nr_quick_move_steps_left_   = 10;
    quick_move_dir_             = dir;
}

void Player::act()
{
#ifndef NDEBUG
    //Sanity check: Disease and infection should never be active at the same time
    ASSERT(!prop_handler_->has_prop(PropId::diseased) ||
           !prop_handler_->has_prop(PropId::infected));
#endif // NDEBUG

    render::draw_map_state();

    if (!is_alive())
    {
        return;
    }

    render::draw_map_state();

    map::cpy_render_array_to_visual_memory();

    if (tgt_ && tgt_->state() != ActorState::alive)
    {
        tgt_ = nullptr;
    }

    //NOTE: We cannot just check for "seen_foes()" here, since the result is
    //also used for setting player awareness below
    std::vector<Actor*> my_seen_actors;
    seen_actors(my_seen_actors);

    //Check if we should go back to inventory screen
    const auto inv_scr_on_new_turn = inv_handling::scr_to_open_on_new_turn;

    bool is_seeing_foe = false;

    for (Actor* seen_actor : my_seen_actors)
    {
        if (!is_leader_of(seen_actor))
        {
            is_seeing_foe = true;
            break;
        }
    }

    if (inv_scr_on_new_turn != InvScr::none)
    {
        if (is_seeing_foe)
        {
            inv_handling::scr_to_open_on_new_turn           = InvScr::none;
            inv_handling::browser_idx_to_set_on_new_turn    = 0;
        }
        else //No seen enemies
        {
            switch (inv_scr_on_new_turn)
            {
            case InvScr::inv:
                inv_handling::run_inv_screen();
                break;

            case InvScr::apply:
                inv_handling::run_apply_screen();
                break;

            case InvScr::equip:
                inv_handling::run_equip_screen(*inv_handling::equip_slot_to_open_on_new_turn);
                break;

            case InvScr::none:
                break;
            }

            return;
        }
    }

    for (Actor* const actor : my_seen_actors)
    {
        static_cast<Mon*>(actor)->set_player_aware_of_me();
    }

    if (active_medical_bag)
    {
        active_medical_bag->continue_action();
        return;
    }

    if (wait_turns_left > 0)
    {
        --wait_turns_left;

        //game_time::tick();

        move(Dir::center);

        return;
    }

    //Quick move
    if (nr_quick_move_steps_left_ > 0)
    {
        //NOTE: There is no need to check for items here, since the message
        //from stepping on an item will interrupt player actions.

        const P tgt(pos + dir_utils::offset(quick_move_dir_));

        const Cell&         tgt_cell   = map::cells[tgt.x][tgt.y];
        const Rigid* const  tgt_rigid  = tgt_cell.rigid;

        const bool is_tgt_known_trap  = tgt_rigid->id() == FeatureId::trap &&
                                        !static_cast<const Trap*>(tgt_rigid)->is_hidden();

        const bool should_abort = !tgt_rigid->can_move_cmn()                        ||
                                  is_tgt_known_trap                                 ||
                                  tgt_rigid->burn_state() == BurnState::burning    ||
                                  (tgt_cell.is_dark && !tgt_cell.is_lit);

        if (should_abort)
        {
            nr_quick_move_steps_left_   = -1;
            quick_move_dir_             = Dir::END;
        }
        else //Keep going!
        {
            --nr_quick_move_steps_left_;
            move(quick_move_dir_);
            return;
        }
    }

    //If this point is reached - read input from player
    if (config::is_bot_playing())
    {
        bot::act();
    }
    else //Not bot playing
    {
        input::clear_events();
        input::map_mode_input();
    }
}

void Player::on_actor_turn()
{
    reset_perm_shock_taken_cur_turn();

    map::player->update_fov();

    std::vector<Actor*> my_seen_foes;
    seen_foes(my_seen_foes);

    insanity::on_new_player_turn(my_seen_foes);

    //Run new turn events on all items
    auto& inv = map::player->inv();

    for (Item* const item : inv.backpack_)
    {
        item->on_actor_turn_in_inv(InvType::backpack);
    }

    for (InvSlot& slot : inv.slots_)
    {
        if (slot.item)
        {
            slot.item->on_actor_turn_in_inv(InvType::slots);
        }
    }
}

void Player::update_tmp_shock()
{
    shock_tmp_ = 0.0;

    //Shock from obsession?
    if (
        insanity::has_sympt(InsSymptId::sadism) ||
        insanity::has_sympt(InsSymptId::masoch))
    {
        shock_tmp_ += double(shock_from_obsession);
    }

    if (prop_handler_->allow_see())
    {
        //Temporary shock from darkness
        Cell& cell = map::cells[pos.x][pos.y];

        if (cell.is_dark && !cell.is_lit)
        {
            shock_tmp_ += shock_taken_after_mods(20, ShockSrc::misc);
        }

        //Temporary shock from seen features
        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P p(pos + d);

            const int base_shock = map::cells[p.x][p.y].rigid->shock_when_adj();

            shock_tmp_ += shock_taken_after_mods(base_shock, ShockSrc::misc);
        }
    }
    else //Is blind
    {
        shock_tmp_ += shock_taken_after_mods(30, ShockSrc::misc);
    }

    shock_tmp_ = std::min(99.0, shock_tmp_);
}

int Player::ins() const
{
    int out = ins_;

    //INS from items
    for (auto& slot : inv_->slots_)
    {
        if (slot.item)
        {
            const ItemDataT& d = slot.item->data();

            //NOTE: Having an item equiped also counts as carrying it
            if (d.is_ins_raied_while_carried || d.is_ins_raied_while_equiped)
            {
                out += ins_from_disturbing_items;
            }
        }
    }

    for (const Item* const item : inv_->backpack_)
    {
        if (item->data().is_ins_raied_while_carried)
        {
            out += ins_from_disturbing_items;
        }
    }

    return std::min(100, out);
}

void Player::on_std_turn()
{
#ifndef NDEBUG
    //Sanity check: Disease and infection should not be active at the same time
    ASSERT(!prop_handler_->has_prop(PropId::diseased) ||
           !prop_handler_->has_prop(PropId::infected));
#endif // NDEBUG

    if (!is_alive())
    {
        return;
    }

    update_tmp_shock();

    //Spell resistance
    const int spi_trait_lvl = player_bon::traits[(size_t)Trait::mighty_spirit] ? 3 :
                              player_bon::traits[(size_t)Trait::strong_spirit] ? 2 :
                              player_bon::traits[(size_t)Trait::stout_spirit]  ? 1 : 0;

    if (spi_trait_lvl > 0 && !prop_handler_->has_prop(PropId::rSpell))
    {
        if (nr_turns_until_rspell_ <= 0)
        {
            //Cooldown has finished, OR countdown has not yet been initialized

            if (nr_turns_until_rspell_ == 0)
            {
                //Cooldown has finished
                prop_handler_->try_add(new PropRSpell(PropTurns::indefinite));

                msg_log::more_prompt();
            }

            const int nr_turns_base = 125 + rnd::range(0, 25);

            const int nr_turns_bon  = (spi_trait_lvl - 1) * 50;

            nr_turns_until_rspell_  = std::max(10, nr_turns_base - nr_turns_bon);
        }

        if (!prop_handler_->has_prop(PropId::rSpell) && nr_turns_until_rspell_ > 0)
        {
            //Spell resistance is in cooldown state, decrement number of
            //remaining turns
            --nr_turns_until_rspell_;
        }
    }

    if (active_explosive)
    {
        active_explosive->on_std_turn_player_hold_ignited();
    }

    std::vector<Actor*> my_seen_foes;
    seen_foes(my_seen_foes);

    double shock_from_mon_cur_player_turn = 0.0;

    for (Actor* actor : my_seen_foes)
    {
        dungeon_master::on_mon_seen(*actor);

        Mon* mon = static_cast<Mon*>(actor);

        mon->set_player_aware_of_me();

        const ActorDataT& mon_data = mon->data();

        if (mon_data.mon_shock_lvl != MonShockLvl::none)
        {
            switch (mon_data.mon_shock_lvl)
            {
            case MonShockLvl::unsettling:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.05,  1.0);
                break;

            case MonShockLvl::frightening:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.1,   1.0);
                break;

            case MonShockLvl::terrifying:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.5,   2.0);
                break;

            case MonShockLvl::mind_shattering:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.75,  3.0);
                break;

            default:
                break;
            }

            if (shock_from_mon_cur_player_turn < 2.5)
            {
                incr_shock((int)floor(mon->shock_caused_cur_), ShockSrc::see_mon);

                shock_from_mon_cur_player_turn += mon->shock_caused_cur_;
            }
        }
    }

    //Some shock is taken every Nth turn
    if (prop_handler_->allow_act())
    {
        int incr_shock_every_n_turns = 12;

        if (player_bon::bg() == Bg::rogue)
        {
            incr_shock_every_n_turns *= 2;
        }

        const int turn = game_time::turn();

        if ((turn % incr_shock_every_n_turns == 0) && turn > 1)
        {
            //Occasionally cause a sudden shock spike, to make it less predictable
            if (rnd::one_in(850))
            {
                std::string msg = "";

                if (rnd::coin_toss())
                {
                    msg = "I have a bad feeling about this...";
                }
                else
                {
                    msg = "A chill runs down my spine...";
                }

                msg_log::add(msg, clr_msg_note, false, MorePromptOnMsg::yes);

                incr_shock(ShockLvl::heavy, ShockSrc::misc);

                render::draw_map_state();
            }
            else //No randomized shock spike
            {
                if (map::dlvl != 0)
                {
                    incr_shock(1, ShockSrc::time);
                }
            }
        }
    }

    //Take sanity hit from high shock?
    if (shock_tot() >= 100)
    {
        nr_turns_until_ins_ = nr_turns_until_ins_ < 0 ? 3 : nr_turns_until_ins_ - 1;

        if (nr_turns_until_ins_ > 0)
        {
            render::draw_map_state(UpdateScreen::yes);

            msg_log::add("I feel my sanity slipping...",
                         clr_msg_note,
                         true,
                         MorePromptOnMsg::yes);
        }
        else //Time to go crazy!
        {
            nr_turns_until_ins_ = -1;
            incr_insanity();

            if (is_alive())
            {
                game_time::tick();
            }

            return;
        }
    }
    else //Total shock is less than 100%
    {
        nr_turns_until_ins_ = -1;
    }

    //Check for monsters coming into view, and try to spot hidden monsters.
    for (Actor* actor : game_time::actors)
    {
        if (
            !actor->is_player()                 &&
            !map::player->is_leader_of(actor)   &&
            actor->is_alive())
        {
            Mon& mon                = *static_cast<Mon*>(actor);
            const bool is_mon_seen  = can_see_actor(*actor);

            if (is_mon_seen)
            {
                mon.is_sneaking_ = false;

                if (!mon.is_msg_mon_in_view_printed_)
                {
                    if (
                        active_medical_bag    ||
                        wait_turns_left > 0   ||
                        nr_quick_move_steps_left_ > 0)
                    {
                        msg_log::add(actor->name_a() + " comes into my view.",
                                     clr_white,
                                     true);
                    }

                    mon.is_msg_mon_in_view_printed_ = true;
                }
            }
            else //Monster is not seen
            {
                mon.is_msg_mon_in_view_printed_ = false;

                //Is the monster sneaking? Try to spot it
                //NOTE: Infravision is irrelevant here, since the monster would
                //have been completely seen already.
                if (
                    map::cells[mon.pos.x][mon.pos.y].is_seen_by_player &&
                    mon.is_sneaking_)
                {
                    const bool did_spot_sneaking = is_spotting_sneaking_actor(mon);

                    if (did_spot_sneaking)
                    {
                        mon.is_sneaking_ = false;

                        mon.set_player_aware_of_me();

                        render::draw_map_state();

                        const std::string mon_name = mon.name_a();

                        msg_log::add("I spot " + mon_name + "!",
                                     clr_msg_note,
                                     true,
                                     MorePromptOnMsg::yes);
                    }
                }
            }
        }
    }

    //Regenerate Hit Points
    if (
        !has_prop(PropId::poisoned) &&
        player_bon::bg() != Bg::ghoul)
    {
        int nr_turns_per_hp = 20;

        //Wounds affect hp regen?
        int nr_wounds = 0;

        if (prop_handler_->has_prop(PropId::wound))
        {
            Prop* const prop = prop_handler_->prop(PropId::wound);

            nr_wounds = static_cast<PropWound*>(prop)->nr_wounds();
        }

        const bool is_survivalist = player_bon::traits[(size_t)Trait::survivalist];

        const int wound_effect_div = is_survivalist ? 2 : 1;

        nr_turns_per_hp += ((nr_wounds * 4) / wound_effect_div);

        //Items affect hp regen?
        for (const auto& slot : inv_->slots_)
        {
            if (slot.item)
            {
                nr_turns_per_hp += slot.item->hp_regen_change(InvType::slots);
            }
        }

        for (const Item* const item : inv_->backpack_)
        {
            nr_turns_per_hp += item->hp_regen_change(InvType::backpack);
        }

        //Rapid Recoverer trait affects hp regen?
        const bool is_rapid_recover = player_bon::traits[(size_t)Trait::rapid_recoverer];

        if (is_rapid_recover)
        {
            nr_turns_per_hp /= 2;
        }

        nr_turns_per_hp = std::max(2, nr_turns_per_hp);

        const int turn       = game_time::turn();
        const int current_hp = hp();
        const int max_hp     = hp_max(true);

        if (
            (current_hp < max_hp)           &&
            ((turn % nr_turns_per_hp) == 0) &&
            turn > 1)
        {
            ++hp_;
        }
    }

    //Try to spot hidden traps and doors
    if (!has_prop(PropId::confused) && prop_handler_->allow_see())
    {
        const int r = player_bon::traits[(size_t)Trait::perceptive] ? 3 :
                      player_bon::traits[(size_t)Trait::observant]  ? 2 : 1;

        int x0 = std::max(0, pos.x - r);
        int y0 = std::max(0, pos.y - r);
        int x1 = std::min(map_w - 1, pos.x + r);
        int y1 = std::min(map_h - 1, pos.y + r);

        for (int y = y0; y <= y1; ++y)
        {
            for (int x = x0; x <= x1; ++x)
            {
                if (map::cells[x][y].is_seen_by_player)
                {
                    auto* f = map::cells[x][y].rigid;

                    if (f->id() == FeatureId::trap)
                    {
                        static_cast<Trap*>(f)->player_try_spot_hidden();
                    }

                    if (f->id() == FeatureId::door)
                    {
                        static_cast<Door*>(f)->player_try_spot_hidden();
                    }
                }
            }
        }
    }
}

void Player::on_log_msg_printed()
{
    //NOTE: There cannot be any calls to msg_log::add() in this function, as
    //that would cause infinite recursion!

    //All messages abort waiting
    wait_turns_left = -1;

    //All messages abort quick move
    nr_quick_move_steps_left_   = -1;
    quick_move_dir_             = Dir::END;
}

void Player::interrupt_actions()
{
    //Abort browsing inventory
    inv_handling::scr_to_open_on_new_turn           = InvScr::none;
    inv_handling::browser_idx_to_set_on_new_turn    = 0;

    //Abort healing
    if (active_medical_bag)
    {
        active_medical_bag->interrupted();
        active_medical_bag = nullptr;
    }

    //Abort waiting
    wait_turns_left = -1;

    //Abort quick move
    nr_quick_move_steps_left_   = -1;
    quick_move_dir_             = Dir::END;
}

void Player::hear_sound(const Snd& snd,
                        const bool is_origin_seen_by_player,
                        const Dir dir_to_origin,
                        const int percent_audible_distance)
{
    (void)is_origin_seen_by_player;

    const SfxId        sfx         = snd.sfx();
    const std::string&  msg         = snd.msg();
    const bool          has_snd_msg = !msg.empty() && msg != " ";

    if (has_snd_msg)
    {
        msg_log::add(msg,
                     clr_text,
                     false,
                     snd.should_add_more_prompt_on_msg());
    }

    //Play audio after message to ensure sync between audio and animation.
    audio::play(sfx, dir_to_origin, percent_audible_distance);

    if (has_snd_msg)
    {
        Actor* const actor_who_made_snd = snd.actor_who_made_sound();

        if (actor_who_made_snd && actor_who_made_snd != this)
        {
            static_cast<Mon*>(actor_who_made_snd)->set_player_aware_of_me();
        }
    }
}

void Player::move(Dir dir)
{
    if (!is_alive())
    {
        return;
    }

    //Store the original intended direction, for use later in this function
    const Dir intended_dir = dir;

    prop_handler_->affect_move_dir(pos, dir);

    //Trap affects leaving?
    if (dir != Dir::center)
    {
        Feature* f = map::cells[pos.x][pos.y].rigid;

        if (f->id() == FeatureId::trap)
        {
            TRACE << "Standing on trap, check if affects move" << std::endl;
            dir = static_cast<Trap*>(f)->actor_try_leave(*this, dir);
        }
    }

    PassTime pass_time = PassTime::yes;

    const P tgt(pos + dir_utils::offset(dir));

    //Attacking, bumping stuff, staggering from encumbrance, etc?
    if (dir != Dir::center)
    {
        //Check if map features are blocking (used later)
        Cell& cell = map::cells[tgt.x][tgt.y];
        bool is_features_allow_move = cell.rigid->can_move(*this);

        std::vector<Mob*> mobs;
        game_time::mobs_at_pos(tgt, mobs);

        if (is_features_allow_move)
        {
            for (auto* m : mobs)
            {
                if (!m->can_move(*this))
                {
                    is_features_allow_move = false;
                    break;
                }
            }
        }

        Mon* const mon = static_cast<Mon*>(map::actor_at_pos(tgt));

        //Hostile monster here?
        if (mon && !is_leader_of(mon))
        {
            const bool can_see_mon      = map::player->can_see_actor(*mon);
            const bool is_aware_of_mon  = mon->player_aware_of_me_counter_ > 0;

            if (is_aware_of_mon)
            {
                if (prop_handler_->allow_attack_melee(Verbosity::verbose))
                {
                    Item* const wpn_item = inv_->item_in_slot(SlotId::wpn);

                    if (wpn_item)
                    {
                        Wpn* const wpn = static_cast<Wpn*>(wpn_item);

                        //If this is also a ranged weapon, ask if player really
                        //intended to use it as melee weapon
                        if (
                            wpn->data().ranged.is_ranged_wpn &&
                            config::is_ranged_wpn_meleee_prompt())
                        {
                            const std::string wpn_name = wpn->name(ItemRefType::a);

                            const std::string mon_name = can_see_mon ? mon->name_the() : "it";

                            msg_log::add("Attack " + mon_name + " with " + wpn_name + "? [y/n]",
                                         clr_white_high);

                            render::draw_map_state();

                            if (query::yes_or_no() == YesNoAnswer::no)
                            {
                                msg_log::clear();
                                render::draw_map_state();

                                return;
                            }
                        }

                        attack::melee(this, pos, *mon, *wpn);
                        tgt_ = mon;

                        return;
                    }
                    else //No melee weapon wielded
                    {
                        hand_att(*mon);
                    }
                }

                return;
            }
            else //There is a monster here that player is unaware of
            {
                //If player is unaware of the monster, it should never be seen
                ASSERT(!can_see_mon);

                if (is_features_allow_move)
                {
                    //Cell is not blocked, reveal monster here and return
                    mon->set_player_aware_of_me();

                    render::draw_map_state();

                    const ActorDataT& d = mon->data();

                    const std::string mon_ref = d.is_ghost      ? "some foul entity" :
                                                (d.is_humanoid  ? "someone" : "a creature");

                    msg_log::add("There is " + mon_ref + " here!",
                                 clr_msg_note,
                                 false,
                                 MorePromptOnMsg::yes);

                    return;
                }

                //NOTE: The target is blocked by map features. Do NOT reveal
                //the monster - just act like the monster isn't there, and
                //let the code below handle the situation.
            }
        }

        if (is_features_allow_move)
        {
            //Encumbrance
            const int enc = enc_percent();

            Prop* const wound_prop = prop_handler_->prop(PropId::wound);

            int nr_wounds = 0;

            if (wound_prop)
            {
                nr_wounds = static_cast<PropWound*>(wound_prop)->nr_wounds();
            }

            const int min_nr_wounds_for_stagger = 3;

            if (enc >= enc_immobile_lvl)
            {
                msg_log::add("I am too encumbered to move!");
                render::draw_map_state();
                return;
            }
            else if (enc >= 100 || nr_wounds >= min_nr_wounds_for_stagger)
            {
                msg_log::add("I stagger.", clr_msg_note);
                prop_handler_->try_add(new PropWaiting(PropTurns::std));
            }

            //Displace allied monster
            if (mon && is_leader_of(mon))
            {
                if (mon->player_aware_of_me_counter_ > 0)
                {
                    std::string mon_name = can_see_actor(*mon) ? mon->name_a() : "it";

                    msg_log::add("I displace " + mon_name + ".");
                }

                mon->pos = pos;
            }

            pos = tgt;

            const int free_step_every_n_turn =
                player_bon::traits[(size_t)Trait::mobile]       ? 3 :
                player_bon::traits[(size_t)Trait::lithe]        ? 4 :
                player_bon::traits[(size_t)Trait::dexterous]    ? 5 : 0;

            if (free_step_every_n_turn > 0)
            {
                if (nr_steps_until_free_action_ == -1)
                {
                    //Steps until free action has not been initialized before
                    //(e.g. player recently picked dexterous)
                    nr_steps_until_free_action_ = free_step_every_n_turn - 2;
                }
                else if (nr_steps_until_free_action_ == 0)
                {
                    //Time for a free move!
                    nr_steps_until_free_action_ = free_step_every_n_turn - 1;

                    pass_time = PassTime::no;
                }
                else
                {
                    //Not yet free move
                    --nr_steps_until_free_action_;
                }
            }

            //Print message if walking on item
            Item* const item = map::cells[pos.x][pos.y].item;

            if (item)
            {
                const bool can_see = prop_handler_->allow_see();

                msg_log::add(can_see ?
                             "I see here:" :
                             "I try to feel what is lying here...",
                             clr_white,
                             true);

                std::string item_name = item->name(ItemRefType::plural,
                                                   ItemRefInf::yes,
                                                   ItemRefAttInf::wpn_context);

                text_format::first_to_upper(item_name);

                msg_log::add(item_name + ".");
            }
        }

        //NOTE: bump() prints block messages.
        for (auto* mob : mobs)
        {
            mob->bump(*this);
        }

        map::cells[tgt.x][tgt.y].rigid->bump(*this);
    }

    //If position is at the destination now, it means that the player either:
    // * did an actual move to another cell, or
    // * that player waited in the current cell on purpose, or
    // * that the player was stuck (e.g. in a spider web)
    //In either case, the game time is ticked here (since no melee attack or
    //other "time advancing" action has occurred)
    if (pos == tgt)
    {
        //Here we rearrange the pistol magazines, if:
        // * Player intended to wait in place (i.e. the parameter
        //   direction is "center"), and,
        // * Player is not stuck in a trap, and
        // * No monsters are seen
        if (intended_dir == Dir::center)
        {
            bool is_stuck = false;

            const Rigid* const rigid = map::cells[pos.x][pos.y].rigid;

            if (rigid->id() == FeatureId::trap)
            {
                is_stuck = static_cast<const Trap*>(rigid)->is_holding_actor();
            }

            if (!is_stuck)
            {
                std::vector<Actor*> my_seen_foes;
                seen_foes(my_seen_foes);

                if (my_seen_foes.empty())
                {
                    reload::player_arrange_pistol_mags();
                }
            }
        }

        game_time::tick(pass_time);
    }
}

bool Player::is_free_step_turn() const
{
    return nr_steps_until_free_action_ == 0;
}

void Player::auto_melee()
{
    if (
        tgt_                                        &&
        tgt_->state() == ActorState::alive         &&
        is_pos_adj(pos, tgt_->pos, false)    &&
        can_see_actor(*tgt_))
    {
        move(dir_utils::dir(tgt_->pos - pos));
        return;
    }

    //If this line reached, there is no adjacent cur target.
    for (const P& d : dir_utils::dir_list)
    {
        Actor* const actor = map::actor_at_pos(pos + d);

        if (actor && !is_leader_of(actor) && can_see_actor(*actor))
        {
            tgt_ = actor;
            move(dir_utils::dir(d));
            return;
        }
    }
}

void Player::kick_mon(Actor& defender)
{
    Wpn* kick_wpn = nullptr;

    const ActorDataT& d = defender.data();

    //TODO: This is REALLY hacky, it should be done another way.
    if (
        d.actor_size == ActorSize::floor &&
        (d.is_spider                    ||
         d.is_rat                       ||
         d.is_snake                     ||
         d.id == ActorId::worm_mass    ||
         d.id == ActorId::mind_worms))
    {
        kick_wpn = static_cast<Wpn*>(item_factory::mk(ItemId::player_stomp));
    }
    else
    {
        kick_wpn = static_cast<Wpn*>(item_factory::mk(ItemId::player_kick));
    }

    attack::melee(this, pos, defender, *kick_wpn);
    delete kick_wpn;
}

Wpn& Player::unarmed_wpn()
{
    ASSERT(unarmed_wpn_);

    return *unarmed_wpn_;
}

void Player::hand_att(Actor& defender)
{
    Wpn& wpn = unarmed_wpn();

    attack::melee(this, pos, defender, wpn);
}

void Player::add_light_hook(bool light_map[map_w][map_h]) const
{
    LgtSize lgt_size = LgtSize::none;

    if (active_explosive)
    {
        if (active_explosive->data().id == ItemId::flare)
        {
            lgt_size = LgtSize::fov;
        }
    }

    if (lgt_size != LgtSize::fov)
    {
        for (Item* const item : inv_->backpack_)
        {
            LgtSize item_lgt_size = item->lgt_size();

            if ((int)lgt_size < (int)item_lgt_size)
            {
                lgt_size = item_lgt_size;
            }
        }
    }

    switch (lgt_size)
    {
    case LgtSize::fov:
    {
        bool hard_blocked[map_w][map_h];

         const R fov_lmt = fov::get_fov_rect(pos);

        map_parse::run(cell_check::BlocksLos(),
                       hard_blocked,
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
    break;

    case LgtSize::small:
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            for (int x = pos.x - 1; x <= pos.x + 1; ++x)
            {
                light_map[x][y] = true;
            }
        }
        break;

    case LgtSize::none:
        break;
    }
}

void Player::update_fov()
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Cell& cell = map::cells[x][y];

            cell.is_seen_by_player              = false;
            cell.player_los.is_blocked_hard     = true;
            cell.player_los.is_blocked_by_drk   = false;
        }
    }

    if (prop_handler_->allow_see())
    {
        bool hard_blocked[map_w][map_h];

         const R fov_lmt = fov::get_fov_rect(pos);

        map_parse::run(cell_check::BlocksLos(),
                       hard_blocked,
                       MapParseMode::overwrite,
                       fov_lmt);

        LosResult fov[map_w][map_h];

        fov::run(pos, hard_blocked, fov);

        for (int x = fov_lmt.p0.x; x <= fov_lmt.p1.x; ++x)
        {
            for (int y = fov_lmt.p0.y; y <= fov_lmt.p1.y; ++y)
            {
                const LosResult&   los     = fov[x][y];
                Cell&               cell    = map::cells[x][y];

                cell.is_seen_by_player      = !los.is_blocked_hard && !los.is_blocked_by_drk;
                cell.player_los             = los;
            }
        }

        map::cells[pos.x][pos.y].is_seen_by_player = true;

        fov_hack();
    }

    if (init::is_cheat_vision_enabled)
    {
        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                map::cells[x][y].is_seen_by_player = true;
            }
        }
    }

    //Explore
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Cell& cell = map::cells[x][y];
            const bool is_blocking = cell_check::BlocksMoveCmn(false).check(cell);

            //Do not explore dark floor cells
            if (cell.is_seen_by_player && (!cell.is_dark || is_blocking))
            {
                cell.is_explored = true;
            }
        }
    }
}

void Player::fov_hack()
{
    bool blocked_los[map_w][map_h];
    map_parse::run(cell_check::BlocksLos(), blocked_los);

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksMoveCmn(false), blocked);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (blocked_los[x][y] && blocked[x][y])
            {
                const P p(x, y);

                for (const P& d : dir_utils::dir_list)
                {
                    const P p_adj(p + d);

                    if (map::is_pos_inside_map(p_adj))
                    {
                        const Cell& adj_cell = map::cells[p_adj.x][p_adj.y];

                        if (
                            adj_cell.is_seen_by_player              &&
                            (!adj_cell.is_dark || adj_cell.is_lit)  &&
                            !blocked[p_adj.x][p_adj.y])
                        {
                            Cell& cell                      = map::cells[x][y];
                            cell.is_seen_by_player          = true;
                            cell.player_los.is_blocked_hard = false;

                            break;
                        }
                    }
                }
            }
        }
    }
}

bool Player::is_leader_of(const Actor* const actor) const
{
    if (!actor || actor == this)
    {
        return false;
    }

    //Actor is monster
    return static_cast<const Mon*>(actor)->leader_ == this;
}

bool Player::is_actor_my_leader(const Actor* const actor) const
{
    //Should never happen
    (void)actor;
    return false;
}
