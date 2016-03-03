#include "actor_player.hpp"

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
#include "utils.hpp"
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
        //Occultist starts with zero explosives and throwing knives.
        //(They are not so thematically fitting for this background.)
        nr_dynamite     = 0;
        nr_molotov      = 0;
        nr_thr_knives   = 0;

        //Occultist starts with a scroll of Darkbolt, and one other random scroll.
        //(Both are identified.)
        Item* scroll = item_factory::mk(Item_id::scroll_darkbolt);
        static_cast<Scroll*>(scroll)->identify(Verbosity::silent);
        inv_->put_in_backpack(scroll);

        while (true)
        {
            scroll = item_factory::mk_random_scroll_or_potion(true, false);

            Spell_id        spell_id    = scroll->data().spell_cast_from_scroll;
            Spell* const    spell       = spell_handling::mk_spell_from_id(spell_id);
            const bool      IS_AVAIL    = spell->is_avail_for_player();
            const bool      SPI_COST_OK = spell->spi_cost(true).max <=
                                          player_bon::spi_occultist_can_cast_at_lvl(4);
            delete spell;

            if (IS_AVAIL && SPI_COST_OK && spell_id != Spell_id::darkbolt)
            {
                static_cast<Scroll*>(scroll)->identify(Verbosity::silent);
                inv_->put_in_backpack(scroll);
                break;
            }
        }

        //Occultist starts with a few potions (identified).
        const int NR_POTIONS = 2;

        for (int i = 0; i < NR_POTIONS; ++i)
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
        auto* const dagger = item_factory::mk(Item_id::dagger);

        static_cast<Wpn*>(dagger)->melee_dmg_plus_ = 1;

        inv_->put_in_slot(Slot_id::wpn, dagger);

        //Rogue starts with some iron spikes (useful tool)
        inv_->put_in_backpack(item_factory::mk(Item_id::iron_spike, 8));
    }
    break;

    case Bg::war_vet:
    {
        //War Veteran starts with some smoke grenades and a gas mask
        inv_->put_in_backpack(item_factory::mk(Item_id::smoke_grenade, 4));
        inv_->put_in_backpack(item_factory::mk(Item_id::gas_mask));
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
        const int WEAPON_CHOICE = rnd::range(1, 5);
        auto      weapon_id     = Item_id::dagger;

        switch (WEAPON_CHOICE)
        {
        case 1:
            weapon_id = Item_id::dagger;
            break;

        case 2:
            weapon_id = Item_id::hatchet;
            break;

        case 3:
            weapon_id = Item_id::hammer;
            break;

        case 4:
            weapon_id = Item_id::machete;
            break;

        case 5:
            weapon_id = Item_id::axe;
            break;
        }

        inv_->put_in_slot(Slot_id::wpn, item_factory::mk(weapon_id));
    }

    //Unarmed attack
    if (player_bon::bg() == Bg::ghoul)
    {
        unarmed_wpn_ = static_cast<Wpn*>(item_factory::mk(Item_id::player_ghoul_claw));
    }
    else //Not ghoul
    {
        unarmed_wpn_ = static_cast<Wpn*>(item_factory::mk(Item_id::player_punch));
    }

    if (has_pistol)
    {
        inv_->put_in_slot(Slot_id::wpn_alt, item_factory::mk(Item_id::pistol));
    }

    for (int i = 0; i < nr_cartridges; ++i)
    {
        inv_->put_in_backpack(item_factory::mk(Item_id::pistol_mag));
    }

    if (nr_dynamite > 0)
    {
        inv_->put_in_backpack(item_factory::mk(Item_id::dynamite,  nr_dynamite));
    }

    if (nr_molotov > 0)
    {
        inv_->put_in_backpack(item_factory::mk(Item_id::molotov,   nr_molotov));
    }

    if (nr_thr_knives > 0)
    {
        inv_->put_in_slot(Slot_id::thrown, item_factory::mk(Item_id::thr_knife, nr_thr_knives));
    }

    if (has_medbag)
    {
        inv_->put_in_backpack(item_factory::mk(Item_id::medical_bag));
    }

    if (has_lantern)
    {
        inv_->put_in_backpack(item_factory::mk(Item_id::lantern));
    }

    if (has_leather_jacket)
    {
        inv_->put_in_slot(Slot_id::body, item_factory::mk(Item_id::armor_leather_jacket));
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

    IA_ASSERT(unarmed_wpn_);

    save_handling::put_int(int(unarmed_wpn_->id()));

    for (int i = 0; i < int(Ability_id::END); ++i)
    {
        const int V = data_->ability_vals.raw_val(Ability_id(i));

        save_handling::put_int(V);
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

    Item_id unarmed_wpn_id = Item_id(save_handling::get_int());

    IA_ASSERT(unarmed_wpn_id < Item_id::END);

    delete unarmed_wpn_;
    unarmed_wpn_ = nullptr;

    Item* const unarmed_item = item_factory::mk(unarmed_wpn_id);

    IA_ASSERT(unarmed_item);

    unarmed_wpn_ = static_cast<Wpn*>(unarmed_item);

    for (int i = 0; i < int(Ability_id::END); ++i)
    {
        const int V = save_handling::get_int();

        data_->ability_vals.set_val(Ability_id(i), V);
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

    const bool CAN_SEE_INVIS = has_prop(Prop_id::see_invis);

    //Monster is invisible, and player cannot see invisible?
    if (other.has_prop(Prop_id::invis) && !CAN_SEE_INVIS)
    {
        return false;
    }

    //Blocked by darkness, and not seeing monster with infravision?
    bool        HAS_INFRAVIS                = prop_handler_->has_prop(Prop_id::infravis);
    const bool  IS_OTHER_INFRA_VISIBLE      = other.data().is_infra_visible;

    const bool  CAN_SEE_OTHER_WITH_INFRAVIS = HAS_INFRAVIS && IS_OTHER_INFRA_VISIBLE;

    const bool  CAN_SEE_OTHER_IN_DRK        = CAN_SEE_INVIS || CAN_SEE_OTHER_WITH_INFRAVIS;

    if (cell.player_los.is_blocked_by_drk && !CAN_SEE_OTHER_IN_DRK)
    {
        return false;
    }

    //Monster is sneaking, and we cannot see it with infravision or magic seeing?
    if (mon->is_sneaking_ && !CAN_SEE_OTHER_WITH_INFRAVIS && !CAN_SEE_INVIS)
    {
        return false;
    }

    //OK, all checks passed, actor can bee seen!
    return true;
}

void Player::on_hit(int& dmg,
                    const Dmg_type dmg_type,
                    const Dmg_method method,
                    const Allow_wound allow_wound)
{
    (void)method;

    if (!insanity::has_sympt(Ins_sympt_id::masoch))
    {
        incr_shock(1, Shock_src::misc);
    }

    const bool IS_ENOUGH_DMG_FOR_WOUND  = dmg >= MIN_DMG_TO_WOUND;
    const bool IS_PHYSICAL              = dmg_type == Dmg_type::physical;

    //Ghoul trait Indomitable Fury makes player immune to Wounds while Frenzied
    const bool IS_GHOUL_RESIST_WOUND = player_bon::traits[size_t(Trait::indomitable_fury)] &&
                                       prop_handler_->has_prop(Prop_id::frenzied);

    if (
        allow_wound == Allow_wound::yes &&
        IS_ENOUGH_DMG_FOR_WOUND         &&
        IS_PHYSICAL                     &&
        !IS_GHOUL_RESIST_WOUND          &&
        !config::is_bot_playing())
    {
        Prop* const prop = new Prop_wound(Prop_turns::indefinite);

        auto nr_wounds = [&]()
        {
            if (prop_handler_->has_prop(Prop_id::wound))
            {
                const Prop* const prop = prop_handler_->prop(Prop_id::wound);

                const Prop_wound* const wound = static_cast<const Prop_wound*>(prop);

                return wound->nr_wounds();
            }

            return 0;
        };

        const int NR_WOUNDS_BEFORE = nr_wounds();

        prop_handler_->try_add(prop);

        const int NR_WOUNDS_AFTER = nr_wounds();

        if (NR_WOUNDS_AFTER > NR_WOUNDS_BEFORE)
        {
            if (insanity::has_sympt(Ins_sympt_id::masoch))
            {
                dungeon_master::add_history_event("Experienced a very pleasant wound.");

                msg_log::add("Hehehe...");

                const double SHOCK_RESTORED = 10.0;

                shock_ = std::max(0.0, shock_ - SHOCK_RESTORED);
            }
            else //Not masochistic
            {
                dungeon_master::add_history_event("Sustained a severe wound.");
            }
        }
    }

    render::draw_map_and_interface();
}

int Player::enc_percent() const
{
    const int TOTAL_W = inv_->total_item_weight();
    const int MAX_W   = carry_weight_lmt();

    return int((double(TOTAL_W) / double(MAX_W)) * 100.0);
}

int Player::carry_weight_lmt() const
{
    const bool IS_TOUGH         = player_bon::traits[size_t(Trait::tough)];
    const bool IS_RUGGED        = player_bon::traits[size_t(Trait::rugged)];
    const bool IS_UNBREAKABLE   = player_bon::traits[size_t(Trait::unbreakable)];
    const bool IS_STRONG_BACKED = player_bon::traits[size_t(Trait::strong_backed)];

    const bool IS_WEAKENED      = has_prop(Prop_id::weakened);

    const int CARRY_WEIGHT_MOD = (IS_TOUGH         * 10) +
                                 (IS_RUGGED        * 10) +
                                 (IS_UNBREAKABLE   * 10) +
                                 (IS_STRONG_BACKED * 30) -
                                 (IS_WEAKENED      * 15);

    return (PLAYER_CARRY_WEIGHT_BASE * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::shock_resistance(const Shock_src shock_src) const
{
    int res = 0;

    if (player_bon::traits[size_t(Trait::fearless)])
    {
        res += 5;
    }

    if (player_bon::traits[size_t(Trait::cool_headed)])
    {
        res += 20;
    }

    if (player_bon::traits[size_t(Trait::courageous)])
    {
        res += 20;
    }

    switch (shock_src)
    {
    case Shock_src::use_strange_item:
        if (player_bon::bg() == Bg::occultist)
        {
            res += 50;
        }
        break;

    case Shock_src::see_mon:
        if (player_bon::bg() == Bg::ghoul)
        {
            res += 50;
        }
        break;

    case Shock_src::cast_intr_spell:
    case Shock_src::time:
    case Shock_src::misc:
    case Shock_src::END:
        break;
    }

    return utils::constr_in_range(0, res, 100);
}

double Player::shock_taken_after_mods(const int BASE_SHOCK,
                                      const Shock_src shock_src) const
{
    if (BASE_SHOCK == 0)
    {
        return 0.0;
    }

    const double SHOCK_RES_DB   = double(shock_resistance(shock_src));
    const double BASE_SHOCK_DB  = double(BASE_SHOCK);
    return (BASE_SHOCK_DB * (100.0 - SHOCK_RES_DB)) / 100.0;
}

void Player::incr_shock(const int SHOCK, Shock_src shock_src)
{
    const double SHOCK_AFTER_MODS = shock_taken_after_mods(SHOCK, shock_src);

    shock_                      += SHOCK_AFTER_MODS;
    perm_shock_taken_cur_turn_  += SHOCK_AFTER_MODS;

    utils::set_constr_in_range(0.0, shock_, 100.0);
}

void Player::incr_shock(const Shock_lvl shock_value, Shock_src shock_src)
{
    incr_shock(int(shock_value), shock_src);
}

void Player::restore_shock(const int amount_restored, const bool IS_TEMP_SHOCK_RESTORED)
{
    shock_ = std::max(0.0, shock_ - amount_restored);

    if (IS_TEMP_SHOCK_RESTORED)
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
        const int INS_INCR = 5;

        ins_ += INS_INCR;
    }

    restore_shock(70, false);

    update_clr();
    render::draw_map_and_interface();

    if (ins() >= 100)
    {
        const std::string msg = "My mind can no longer withstand what it has grasped. "
                                "I am hopelessly lost.";
        popup::show_msg(msg, true, "Insane!", Sfx_id::insanity_rise);
        die(true, false, false);
        return;
    }

    //This point reached means sanity is below 100%
    insanity::gain_sympt();
}

bool Player::is_standing_in_open_place() const
{
    const Rect r(pos - 1, pos + 1);

    bool blocked[MAP_W][MAP_H];

    //NOTE: Checking if adjacent cells blocks projectiles is probably the best
    //way to determine if this is an open place. If we check for things that
    //block common movement, stuff like chasms would count as blocking.
    map_parse::run(cell_check::Blocks_projectiles(),
                   blocked,
                   Map_parse_mode::overwrite,
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
    const Rect r(pos - 1, pos + 1);

    bool blocked[MAP_W][MAP_H];

    //NOTE: Checking if adjacent cells blocks projectiles is probably the best
    //way to determine if this is an open place. If we check for things that
    //block common movement, stuff like chasms would count as blocking.
    map_parse::run(cell_check::Blocks_projectiles(),
                   blocked,
                   Map_parse_mode::overwrite,
                   r);

    int block_count = 0;

    const int MIN_NR_BLOCKED_FOR_CRAMPED = 6;

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            if (blocked[x][y])
            {
                ++block_count;

                if (block_count >= MIN_NR_BLOCKED_FOR_CRAMPED)
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

    const int CUR_SHOCK = shock_ + shock_tmp_;

    if (CUR_SHOCK >= 75)
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
    IA_ASSERT(!prop_handler_->has_prop(Prop_id::diseased) ||
           !prop_handler_->has_prop(Prop_id::infected));
#endif // NDEBUG

    render::draw_map_and_interface();

    if (!is_alive())
    {
        return;
    }

    render::draw_map_and_interface();

    map::cpy_render_array_to_visual_memory();

    if (tgt_ && tgt_->state() != Actor_state::alive)
    {
        tgt_ = nullptr;
    }

    std::vector<Actor*> my_seen_foes;
    seen_foes(my_seen_foes);

    //Check if we should go back to inventory screen
    const auto inv_scr_on_new_turn = inv_handling::scr_to_open_on_new_turn;

    if (inv_scr_on_new_turn != Inv_scr::none)
    {
        if (my_seen_foes.empty())
        {
            switch (inv_scr_on_new_turn)
            {
            case Inv_scr::inv:
                inv_handling::run_inv_screen();
                break;

            case Inv_scr::apply:
                inv_handling::run_apply_screen();
                break;

            case Inv_scr::equip:
                inv_handling::run_equip_screen(*inv_handling::equip_slot_to_open_on_new_turn);
                break;

            case Inv_scr::none:
                break;
            }

            return;
        }
        else //There are seen monsters
        {
            inv_handling::scr_to_open_on_new_turn           = Inv_scr::none;
            inv_handling::browser_idx_to_set_on_new_turn    = 0;
        }
    }

    for (Actor* const actor : my_seen_foes)
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
        game_time::tick();
        return;
    }

    //Quick move
    if (nr_quick_move_steps_left_ > 0)
    {
        //NOTE: There is no need to check for items here, since the message from stepping
        //on an item will interrupt player actions.

        const P tgt(pos + dir_utils::offset(quick_move_dir_));

        const Cell&         tgt_cell   = map::cells[tgt.x][tgt.y];
        const Rigid* const  tgt_rigid  = tgt_cell.rigid;

        const bool IS_TGT_KNOWN_TRAP  = tgt_rigid->id() == Feature_id::trap &&
                                        !static_cast<const Trap*>(tgt_rigid)->is_hidden();

        const bool SHOULD_ABORT = !tgt_rigid->can_move_cmn()                        ||
                                  IS_TGT_KNOWN_TRAP                                 ||
                                  tgt_rigid->burn_state() == Burn_state::burning    ||
                                  (tgt_cell.is_dark && !tgt_cell.is_lit);

        if (SHOULD_ABORT)
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
        item->on_actor_turn_in_inv(Inv_type::backpack);
    }

    for (Inv_slot& slot : inv.slots_)
    {
        if (slot.item)
        {
            slot.item->on_actor_turn_in_inv(Inv_type::slots);
        }
    }
}

void Player::update_tmp_shock()
{
    shock_tmp_ = 0.0;

    //Shock from obsession?
    if (
        insanity::has_sympt(Ins_sympt_id::sadism) ||
        insanity::has_sympt(Ins_sympt_id::masoch))
    {
        shock_tmp_ += double(SHOCK_FROM_OBSESSION);
    }

    if (prop_handler_->allow_see())
    {
        //Temporary shock from darkness
        Cell& cell = map::cells[pos.x][pos.y];

        if (cell.is_dark && !cell.is_lit)
        {
            shock_tmp_ += shock_taken_after_mods(20, Shock_src::misc);
        }

        //Temporary shock from seen features
        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P p(pos + d);

            const int BASE_SHOCK = map::cells[p.x][p.y].rigid->shock_when_adj();

            shock_tmp_ += shock_taken_after_mods(BASE_SHOCK, Shock_src::misc);
        }
    }
    else //Is blind
    {
        shock_tmp_ += shock_taken_after_mods(30, Shock_src::misc);
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
            const Item_data_t& d = slot.item->data();

            //NOTE: Having an item equiped also counts as carrying it
            if (d.is_ins_raied_while_carried || d.is_ins_raied_while_equiped)
            {
                out += INS_FROM_DISTURBING_ITEMS;
            }
        }
    }

    for (const Item* const item : inv_->backpack_)
    {
        if (item->data().is_ins_raied_while_carried)
        {
            out += INS_FROM_DISTURBING_ITEMS;
        }
    }

    return std::min(100, out);
}

void Player::on_std_turn()
{
#ifndef NDEBUG
    //Sanity check: Disease and infection should never be active at the same time
    IA_ASSERT(!prop_handler_->has_prop(Prop_id::diseased) ||
           !prop_handler_->has_prop(Prop_id::infected));
#endif // NDEBUG

    if (!is_alive())
    {
        return;
    }

    update_tmp_shock();

    //Spell resistance
    const int SPI_TRAIT_LVL = player_bon::traits[size_t(Trait::mighty_spirit)] ? 3 :
                              player_bon::traits[size_t(Trait::strong_spirit)] ? 2 :
                              player_bon::traits[size_t(Trait::stout_spirit)]  ? 1 : 0;

    if (SPI_TRAIT_LVL > 0 && !prop_handler_->has_prop(Prop_id::rSpell))
    {
        if (nr_turns_until_rspell_ <= 0)
        {
            //Cooldown has finished, OR countdown has not yet been initialized

            if (nr_turns_until_rspell_ == 0)
            {
                //Cooldown has finished
                prop_handler_->try_add(new Prop_rSpell(Prop_turns::indefinite));

                msg_log::more_prompt();
            }

            const int NR_TURNS_BASE = 125 + rnd::range(0, 25);

            const int NR_TURNS_BON  = (SPI_TRAIT_LVL - 1) * 50;

            nr_turns_until_rspell_  = std::max(10, NR_TURNS_BASE - NR_TURNS_BON);
        }

        if (!prop_handler_->has_prop(Prop_id::rSpell) && nr_turns_until_rspell_ > 0)
        {
            //Spell resistance is in cooldown state, decrement number of remaining turns
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

        const Actor_data_t& mon_data = mon->data();

        if (mon_data.mon_shock_lvl != Mon_shock_lvl::none)
        {
            switch (mon_data.mon_shock_lvl)
            {
            case Mon_shock_lvl::unsettling:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.05,  1.0);
                break;

            case Mon_shock_lvl::frightening:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.1,   1.0);
                break;

            case Mon_shock_lvl::terrifying:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.5,   2.0);
                break;

            case Mon_shock_lvl::mind_shattering:
                mon->shock_caused_cur_ = std::min(mon->shock_caused_cur_ + 0.75,  3.0);
                break;

            default:
                break;
            }

            if (shock_from_mon_cur_player_turn < 2.5)
            {
                incr_shock(int(floor(mon->shock_caused_cur_)), Shock_src::see_mon);
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

        const int TURN = game_time::turn();

        if ((TURN % incr_shock_every_n_turns == 0) && TURN > 1)
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

                msg_log::add(msg, clr_msg_note, false, More_prompt_on_msg::yes);

                incr_shock(Shock_lvl::heavy, Shock_src::misc);

                render::draw_map_and_interface();
            }
            else //No randomized shock spike
            {
                if (map::dlvl != 0)
                {
                    incr_shock(1, Shock_src::time);
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
            render::draw_map_and_interface(true);

            msg_log::add("I feel my sanity slipping...",
                         clr_msg_note,
                         true,
                         More_prompt_on_msg::yes);
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
            const bool IS_MON_SEEN  = can_see_actor(*actor);

            if (IS_MON_SEEN)
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
                //NOTE: Infravision is irrelevant here, since the monster would have been
                //completely seen already.
                if (map::cells[mon.pos.x][mon.pos.y].is_seen_by_player && mon.is_sneaking_)
                {
                    const bool DID_SPOT_SNEAKING = is_spotting_sneaking_actor(mon);

                    if (DID_SPOT_SNEAKING)
                    {
                        mon.is_sneaking_ = false;

                        mon.set_player_aware_of_me();

                        render::draw_map_and_interface();

                        const std::string mon_name = mon.name_a();

                        msg_log::add("I spot " + mon_name + "!",
                                     clr_msg_note,
                                     true,
                                     More_prompt_on_msg::yes);
                    }
                }
            }
        }
    }

    //HP regen
    if (!has_prop(Prop_id::poisoned) && player_bon::bg() != Bg::ghoul)
    {
        int nr_turns_per_hp = 12;

        //Wounds affect HP regen?
        int nr_wounds = 0;

        if (prop_handler_->has_prop(Prop_id::wound))
        {
            Prop* const prop = prop_handler_->prop(Prop_id::wound);

            nr_wounds = static_cast<Prop_wound*>(prop)->nr_wounds();
        }

        const bool IS_SURVIVALIST = player_bon::traits[size_t(Trait::survivalist)];

        const int WOUND_EFFECT_DIV = IS_SURVIVALIST ? 2 : 1;

        nr_turns_per_hp += ((nr_wounds * 4) / WOUND_EFFECT_DIV);

        //Items affect HP regen?
        for (const auto& slot : inv_->slots_)
        {
            if (slot.item)
            {
                nr_turns_per_hp += slot.item->hp_regen_change(Inv_type::slots);
            }
        }

        for (const Item* const item : inv_->backpack_)
        {
            nr_turns_per_hp += item->hp_regen_change(Inv_type::backpack);
        }

        //Rapid Recoverer trait affects HP regen?
        const bool IS_RAPID_RECOVER = player_bon::traits[size_t(Trait::rapid_recoverer)];

        if (IS_RAPID_RECOVER)
        {
            nr_turns_per_hp /= 2;
        }

        nr_turns_per_hp = std::max(2, nr_turns_per_hp);

        const int TURN      = game_time::turn();
        const int HP        = hp();
        const int HP_MAX    = hp_max(true);

        if (
            (HP < HP_MAX)                   &&
            ((TURN % nr_turns_per_hp) == 0) &&
            TURN > 1)
        {
            ++hp_;
        }
    }

    //Try to spot hidden traps and doors
    if (!has_prop(Prop_id::confused) && prop_handler_->allow_see())
    {
        const int R = player_bon::traits[size_t(Trait::perceptive)] ? 3 :
                      (player_bon::traits[size_t(Trait::observant)] ? 2 : 1);

        int x0 = std::max(0, pos.x - R);
        int y0 = std::max(0, pos.y - R);
        int x1 = std::min(MAP_W - 1, pos.x + R);
        int y1 = std::min(MAP_H - 1, pos.y + R);

        for (int y = y0; y <= y1; ++y)
        {
            for (int x = x0; x <= x1; ++x)
            {
                if (map::cells[x][y].is_seen_by_player)
                {
                    auto* f = map::cells[x][y].rigid;

                    if (f->id() == Feature_id::trap)
                    {
                        static_cast<Trap*>(f)->player_try_spot_hidden();
                    }

                    if (f->id() == Feature_id::door)
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
    //NOTE: There cannot be any calls to msg_log::add() in this function, as that would
    //cause endless recursion.

    //Abort waiting
    wait_turns_left = -1;

    //Abort quick move
    nr_quick_move_steps_left_ = -1;
    quick_move_dir_ = Dir::END;
}

void Player::interrupt_actions()
{
    //Abort browsing inventory
    inv_handling::scr_to_open_on_new_turn           = Inv_scr::none;
    inv_handling::browser_idx_to_set_on_new_turn    = 0;

    //Abort waiting
    if (wait_turns_left > 0)
    {
        msg_log::add("I stop waiting.", clr_white);
        render::draw_map_and_interface();
    }

    wait_turns_left = -1;

    //Abort healing
    if (active_medical_bag)
    {
        active_medical_bag->interrupted();
        active_medical_bag = nullptr;
    }

    //Abort quick move
    nr_quick_move_steps_left_ = -1;
    quick_move_dir_         = Dir::END;
}

void Player::hear_sound(const Snd& snd,
                        const bool IS_ORIGIN_SEEN_BY_PLAYER,
                        const Dir dir_to_origin,
                        const int PERCENT_AUDIBLE_DISTANCE)
{
    (void)IS_ORIGIN_SEEN_BY_PLAYER;

    const Sfx_id        sfx         = snd.sfx();
    const std::string&  msg         = snd.msg();
    const bool          HAS_SND_MSG = !msg.empty() && msg != " ";

    if (HAS_SND_MSG)
    {
        msg_log::add(msg, clr_white, false, snd.should_add_more_prompt_on_msg());
    }

    //Play audio after message to ensure sync between audio and animation.
    audio::play(sfx, dir_to_origin, PERCENT_AUDIBLE_DISTANCE);

    if (HAS_SND_MSG)
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

    prop_handler_->affect_move_dir(pos, dir);

    //Trap affects leaving?
    if (dir != Dir::center)
    {
        Feature* f = map::cells[pos.x][pos.y].rigid;

        if (f->id() == Feature_id::trap)
        {
            TRACE << "Standing on trap, check if affects move" << std::endl;
            dir = static_cast<Trap*>(f)->actor_try_leave(*this, dir);
        }
    }

    Pass_time pass_time = Pass_time::yes;

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

        Mon* const mon = static_cast<Mon*>(utils::actor_at_pos(tgt));

        //Hostile monster here?
        if (mon && !is_leader_of(mon))
        {
            const bool CAN_SEE_MON      = map::player->can_see_actor(*mon);
            const bool IS_AWARE_OF_MON  = mon->player_aware_of_me_counter_ > 0;

            if (IS_AWARE_OF_MON)
            {
                if (prop_handler_->allow_attack_melee(Verbosity::verbose))
                {
                    Item* const wpn_item = inv_->item_in_slot(Slot_id::wpn);

                    if (wpn_item)
                    {
                        Wpn* const wpn = static_cast<Wpn*>(wpn_item);

                        //If this is also a ranged weapon, ask if player really intended to
                        //use it as melee weapon
                        if (
                            wpn->data().ranged.is_ranged_wpn &&
                            config::is_ranged_wpn_meleee_prompt())
                        {
                            const std::string wpn_name = wpn->name(Item_ref_type::a);

                            const std::string mon_name = CAN_SEE_MON ? mon->name_the() : "it";

                            msg_log::add("Attack " + mon_name + " with " + wpn_name + "? [y/n]",
                                         clr_white_high);

                            render::draw_map_and_interface();

                            if (query::yes_or_no() == Yes_no_answer::no)
                            {
                                msg_log::clear();
                                render::draw_map_and_interface();

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
                IA_ASSERT(!CAN_SEE_MON);

                if (is_features_allow_move)
                {
                    //Cell is not blocked, reveal that there is a monster here and return

                    mon->set_player_aware_of_me();

                    render::draw_map_and_interface();

                    const Actor_data_t& d = mon->data();

                    const std::string mon_ref = d.is_ghost      ? "some foul entity" :
                                                (d.is_humanoid  ? "someone" : "a creature");

                    msg_log::add("There is " + mon_ref + " here!",
                                 clr_msg_note,
                                 false,
                                 More_prompt_on_msg::yes);

                    return;
                }

                //NOTE: This point reached means the target is blocked by map features. Do NOT
                //reveal the monster - just act like the monster isn't there, and let the code
                //below handle the situation.
            }
        }

        if (is_features_allow_move)
        {
            //Encumbrance
            const int ENC = enc_percent();

            Prop* const wound_prop = prop_handler_->prop(Prop_id::wound);

            int nr_wounds = 0;

            if (wound_prop)
            {
                nr_wounds = static_cast<Prop_wound*>(wound_prop)->nr_wounds();
            }

            const int MIN_NR_WOUNDS_FOR_STAGGER = 3;

            if (ENC >= ENC_IMMOBILE_LVL)
            {
                msg_log::add("I am too encumbered to move!");
                render::draw_map_and_interface();
                return;
            }
            else if (ENC >= 100 || nr_wounds >= MIN_NR_WOUNDS_FOR_STAGGER)
            {
                msg_log::add("I stagger.", clr_msg_note);
                prop_handler_->try_add(new Prop_waiting(Prop_turns::std));
            }

            //Displace allied monster
            if (mon && is_leader_of(mon))
            {
                msg_log::add("I displace " + mon->name_a() + ".");
                mon->pos = pos;
            }

            pos = tgt;

            const int FREE_STEP_EVERY_N_TURN =
                player_bon::traits[size_t(Trait::mobile)]     ? 3 :
                player_bon::traits[size_t(Trait::lithe)]      ? 4 :
                player_bon::traits[size_t(Trait::dexterous)]  ? 5 : 0;

            if (FREE_STEP_EVERY_N_TURN > 0)
            {
                if (nr_steps_until_free_action_ == -1)
                {
                    //Steps until free action has not been initialized before (e.g. player recently
                    //picked dexterous)
                    nr_steps_until_free_action_ = FREE_STEP_EVERY_N_TURN - 2;
                }
                else if (nr_steps_until_free_action_ == 0)
                {
                    //Time for a free move!
                    nr_steps_until_free_action_ = FREE_STEP_EVERY_N_TURN - 1;

                    pass_time = Pass_time::no;
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
                const bool CAN_SEE = prop_handler_->allow_see();

                msg_log::add(CAN_SEE ?
                             "I see here:" :
                             "I try to feel what is lying here...",
                             clr_white,
                             true);

                std::string item_name = item->name(Item_ref_type::plural,
                                                   Item_ref_inf::yes,
                                                   Item_ref_att_inf::wpn_context);

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

    //If position equals the destination at this point, it means that player either:
    // * did an actual move to another cell, or
    // * that player waited in the current cell on purpose, or
    // * that the player was stuck (e.g. in a spider web)
    //In either case, the game time is ticked here (since no melee attack or other "time advancing"
    //action has occurred)
    if (pos == tgt)
    {
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
        tgt_->state() == Actor_state::alive         &&
        utils::is_pos_adj(pos, tgt_->pos, false)    &&
        can_see_actor(*tgt_))
    {
        move(dir_utils::dir(tgt_->pos - pos));
        return;
    }

    //If this line reached, there is no adjacent cur target.
    for (const P& d : dir_utils::dir_list)
    {
        Actor* const actor = utils::actor_at_pos(pos + d);

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

    const Actor_data_t& d = defender.data();

    //TODO: This is REALLY hacky, it should be done another way.
    if (
        d.actor_size == Actor_size::floor &&
        (d.is_spider                    ||
         d.is_rat                       ||
         d.is_snake                     ||
         d.id == Actor_id::worm_mass    ||
         d.id == Actor_id::mind_worms))
    {
        kick_wpn = static_cast<Wpn*>(item_factory::mk(Item_id::player_stomp));
    }
    else
    {
        kick_wpn = static_cast<Wpn*>(item_factory::mk(Item_id::player_kick));
    }

    attack::melee(this, pos, defender, *kick_wpn);
    delete kick_wpn;
}

Wpn& Player::unarmed_wpn()
{
    IA_ASSERT(unarmed_wpn_);

    return *unarmed_wpn_;
}

void Player::hand_att(Actor& defender)
{
    Wpn& wpn = unarmed_wpn();

    attack::melee(this, pos, defender, wpn);
}

void Player::add_light_hook(bool light_map[MAP_W][MAP_H]) const
{
    Lgt_size lgt_size = Lgt_size::none;

    if (active_explosive)
    {
        if (active_explosive->data().id == Item_id::flare)
        {
            lgt_size = Lgt_size::fov;
        }
    }

    if (lgt_size != Lgt_size::fov)
    {
        for (Item* const item : inv_->backpack_)
        {
            Lgt_size item_lgt_size = item->lgt_size();

            if (int(lgt_size) < int(item_lgt_size))
            {
                lgt_size = item_lgt_size;
            }
        }
    }

    switch (lgt_size)
    {
    case Lgt_size::fov:
    {
        bool hard_blocked[MAP_W][MAP_H];

        const Rect fov_lmt = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       hard_blocked,
                       Map_parse_mode::overwrite,
                       fov_lmt);

        Los_result fov[MAP_W][MAP_H];

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

    case Lgt_size::small:
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            for (int x = pos.x - 1; x <= pos.x + 1; ++x)
            {
                light_map[x][y] = true;
            }
        }
        break;

    case Lgt_size::none:
        break;
    }
}

void Player::update_fov()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Cell& cell = map::cells[x][y];

            cell.is_seen_by_player              = false;
            cell.player_los.is_blocked_hard     = true;
            cell.player_los.is_blocked_by_drk   = false;
        }
    }

    if (prop_handler_->allow_see())
    {
        bool hard_blocked[MAP_W][MAP_H];

        const Rect fov_lmt = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       hard_blocked,
                       Map_parse_mode::overwrite,
                       fov_lmt);

        Los_result fov[MAP_W][MAP_H];

        fov::run(pos, hard_blocked, fov);

        for (int x = fov_lmt.p0.x; x <= fov_lmt.p1.x; ++x)
        {
            for (int y = fov_lmt.p0.y; y <= fov_lmt.p1.y; ++y)
            {
                const Los_result&   los     = fov[x][y];
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
        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                map::cells[x][y].is_seen_by_player = true;
            }
        }
    }

    //Explore
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Cell& cell = map::cells[x][y];
            const bool IS_BLOCKING = cell_check::Blocks_move_cmn(false).check(cell);

            //Do not explore dark floor cells
            if (cell.is_seen_by_player && (!cell.is_dark || IS_BLOCKING))
            {
                cell.is_explored = true;
            }
        }
    }
}

void Player::fov_hack()
{
    bool blocked_los[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_los(), blocked_los);

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (blocked_los[x][y] && blocked[x][y])
            {
                const P p(x, y);

                for (const P& d : dir_utils::dir_list)
                {
                    const P p_adj(p + d);

                    if (utils::is_pos_inside_map(p_adj))
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
