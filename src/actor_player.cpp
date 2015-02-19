#include "actor_player.hpp"

#include "init.hpp"
#include "render.hpp"
#include "audio.hpp"
#include "feature_trap.hpp"
#include "create_character.hpp"
#include "msg_log.hpp"
#include "character_lines.hpp"
#include "popup.hpp"
#include "postmortem.hpp"
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

using namespace std;

const int SHOCK_FROM_OBSESSION = 30;

Player::Player() :
    Actor                   (),
    active_medical_bag        (nullptr),
    wait_turns_left           (-1),
    active_explosive         (nullptr),
    tgt_                    (nullptr),
    ins_                    (0),
    shock_                  (0.0),
    shock_tmp_               (0.0),
    perm_shock_taken_cur_turn_  (0.0),
    nr_moves_until_free_action_ (-1),
    nr_turns_until_ins_        (-1),
    nr_quick_move_steps_left_   (-1),
    quick_move_dir_           (Dir::END),
    CARRY_WEIGHT_BASE_      (450)
{
    for (int i = 0; i < int(Phobia::END); ++i)     {phobias[i]     = false;}
    for (int i = 0; i < int(Obsession::END); ++i)  {obsessions[i]  = false;}
}

Player::~Player()
{
    if (active_explosive) {delete active_explosive;}
}

void Player::mk_start_items()
{
    data_->ability_vals.reset();

    for (int i = 0; i < int(Phobia::END); ++i)     {phobias[i]     = false;}
    for (int i = 0; i < int(Obsession::END); ++i)  {obsessions[i]  = false;}

    int nr_cartridges  = 2;
    int nr_dynamite    = 2;
    int nr_molotov     = 2;
    int nr_thr_knives   = 6;

    //------------------------------------------------------- BACKGROUND SPECIFIC SETUP
    const auto bg = player_bon::get_bg();

    if (bg == Bg::occultist)
    {
        //Occultist starts with zero explosives and throwing knives.
        //(They are not so thematically fitting for this background.)
        nr_dynamite    = 0;
        nr_molotov     = 0;
        nr_thr_knives   = 0;

        //Occultist starts with a scroll of Darkbolt, and one other random scroll.
        //(Both are identified.)
        Item* scroll = item_factory::mk(Item_id::scroll_darkbolt);
        static_cast<Scroll*>(scroll)->identify(true);
        inv_->put_in_general(scroll);
        while (true)
        {
            scroll = item_factory::mk_random_scroll_or_potion(true, false);

            Spell_id       id          = scroll->get_data().spell_cast_from_scroll;
            Spell* const  spell       = spell_handling::mk_spell_from_id(id);
            const bool    IS_AVAIL    = spell->is_avail_for_player();
            const bool    SPI_COST_OK = spell->get_spi_cost(true).upper <=
                                        player_bon::get_spi_occultist_can_cast_at_lvl(4);
            delete spell;

            if (IS_AVAIL && SPI_COST_OK && id != Spell_id::darkbolt)
            {
                static_cast<Scroll*>(scroll)->identify(true);
                inv_->put_in_general(scroll);
                break;
            }
        }

        //Occultist starts with a few potions (identified).
        const int NR_POTIONS = 2;
        for (int i = 0; i < NR_POTIONS; ++i)
        {
            Item* const potion = item_factory::mk_random_scroll_or_potion(false, true);
            static_cast<Potion*>(potion)->identify(true);
            inv_->put_in_general(potion);
        }
    }

    if (bg == Bg::rogue)
    {
        //Rogue starts with extra throwing knives
        nr_thr_knives += 6;

        //Rogue starts with a +1 dagger
        auto* const dagger = item_factory::mk(Item_id::dagger);
        static_cast<Wpn*>(dagger)->melee_dmg_plus_ = 1;
        inv_->slots_[int(Slot_id::wielded)].item = dagger;

        //Rogue starts with some iron spikes (useful tool)
        inv_->put_in_general(item_factory::mk(Item_id::iron_spike, 8));
    }

    if (bg == Bg::war_vet)
    {
        //War Veteran starts with some smoke grenades and a gas mask
        inv_->put_in_general(item_factory::mk(Item_id::smoke_grenade, 4));
        inv_->put_in_general(item_factory::mk(Item_id::gas_mask));
    }

    //------------------------------------------------------- GENERAL SETUP
    //Randomize a melee weapon if not already wielding one.
    if (!inv_->slots_[int(Slot_id::wielded)].item)
    {
        const int WEAPON_CHOICE = rnd::range(1, 5);
        auto      weapon_id      = Item_id::dagger;

        switch (WEAPON_CHOICE)
        {
        case 1:   weapon_id = Item_id::dagger;   break;
        case 2:   weapon_id = Item_id::hatchet;  break;
        case 3:   weapon_id = Item_id::hammer;   break;
        case 4:   weapon_id = Item_id::machete;  break;
        case 5:   weapon_id = Item_id::axe;      break;
        }
        inv_->put_in_slot(Slot_id::wielded, item_factory::mk(weapon_id));
    }

    inv_->put_in_slot(Slot_id::wielded_alt, item_factory::mk(Item_id::pistol));

    for (int i = 0; i < nr_cartridges; ++i)
    {
        inv_->put_in_general(item_factory::mk(Item_id::pistol_clip));
    }

    if (nr_dynamite > 0)
    {
        inv_->put_in_general(item_factory::mk(Item_id::dynamite,  nr_dynamite));
    }
    if (nr_molotov > 0)
    {
        inv_->put_in_general(item_factory::mk(Item_id::molotov,   nr_molotov));
    }

    if (nr_thr_knives > 0)
    {
        inv_->put_in_slot(Slot_id::thrown, item_factory::mk(Item_id::thr_knife, nr_thr_knives));
    }

    inv_->put_in_slot(Slot_id::body, item_factory::mk(Item_id::armor_leather_jacket));

    inv_->put_in_general(item_factory::mk(Item_id::electric_lantern));
    inv_->put_in_general(item_factory::mk(Item_id::medical_bag));
}

void Player::store_to_save_lines(vector<string>& lines) const
{
    lines.push_back(to_str(prop_handler_->applied_props_.size()));
    for (Prop* prop : prop_handler_->applied_props_)
    {
        lines.push_back(to_str(int(prop->get_id())));
        lines.push_back(to_str(prop->turns_left_));
        prop->store_to_save_lines(lines);
    }

    lines.push_back(to_str(ins_));
    lines.push_back(to_str(int(shock_)));
    lines.push_back(to_str(hp_));
    lines.push_back(to_str(hp_max_));
    lines.push_back(to_str(spi_));
    lines.push_back(to_str(spi_max_));
    lines.push_back(to_str(pos.x));
    lines.push_back(to_str(pos.y));

    for (int i = 0; i < int(Ability_id::END); ++i)
    {
        lines.push_back(to_str(data_->ability_vals.get_raw_val(Ability_id(i))));
    }

    for (int i = 0; i < int(Phobia::END); ++i)
    {
        lines.push_back(phobias[i] == 0 ? "0" : "1");
    }
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        lines.push_back(obsessions[i] == 0 ? "0" : "1");
    }
}

void Player::setup_from_save_lines(vector<string>& lines)
{
    const int NR_PROPS = to_int(lines.front());
    lines.erase(begin(lines));
    for (int i = 0; i < NR_PROPS; ++i)
    {
        const auto id = Prop_id(to_int(lines.front()));
        lines.erase(begin(lines));
        const int NR_TURNS = to_int(lines.front());
        lines.erase(begin(lines));
        auto* const prop = prop_handler_->mk_prop(id, Prop_turns::specific, NR_TURNS);
        prop_handler_->try_apply_prop(prop, true, true, true, true);
        prop->setup_from_save_lines(lines);
    }

    ins_ = to_int(lines.front());
    lines.erase(begin(lines));
    shock_ = double(to_int(lines.front()));
    lines.erase(begin(lines));
    hp_ = to_int(lines.front());
    lines.erase(begin(lines));
    hp_max_ = to_int(lines.front());
    lines.erase(begin(lines));
    spi_ = to_int(lines.front());
    lines.erase(begin(lines));
    spi_max_ = to_int(lines.front());
    lines.erase(begin(lines));
    pos.x = to_int(lines.front());
    lines.erase(begin(lines));
    pos.y = to_int(lines.front());
    lines.erase(begin(lines));

    for (int i = 0; i < int(Ability_id::END); ++i)
    {
        data_->ability_vals.set_val(Ability_id(i), to_int(lines.front()));
        lines.erase(begin(lines));
    }

    for (int i = 0; i < int(Phobia::END); ++i)
    {
        phobias[i] = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));
    }
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        obsessions[i] = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));
    }
}

void Player::on_hit(int& dmg)
{
    (void)dmg;

    if (!obsessions[int(Obsession::masochism)]) {incr_shock(1, Shock_src::misc);}

    render::draw_map_and_interface();
}

int Player::get_enc_percent() const
{
    const int TOTAL_W = inv_->get_total_item_weight();
    const int MAX_W   = get_carry_weight_lmt();
    return int((double(TOTAL_W) / double(MAX_W)) * 100.0);
}

int Player::get_carry_weight_lmt() const
{
    const bool IS_TOUGH         = player_bon::traits[int(Trait::tough)];
    const bool IS_RUGGED        = player_bon::traits[int(Trait::rugged)];
    const bool IS_UNBREAKABLE   = player_bon::traits[int(Trait::unbreakable)];
    const bool IS_STRONG_BACKED = player_bon::traits[int(Trait::strong_backed)];

    bool props[size_t(Prop_id::END)];
    prop_handler_->get_prop_ids(props);
    const bool IS_WEAKENED = props[int(Prop_id::weakened)];

    const int CARRY_WEIGHT_MOD = (IS_TOUGH         * 10) +
                                 (IS_RUGGED        * 10) +
                                 (IS_UNBREAKABLE   * 10) +
                                 (IS_STRONG_BACKED * 30) -
                                 (IS_WEAKENED      * 15);

    return (CARRY_WEIGHT_BASE_ * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::get_shock_resistance(const Shock_src shock_src) const
{
    int res = 0;
    if (player_bon::traits[int(Trait::fearless)])    {res += 5;}
    if (player_bon::traits[int(Trait::cool_headed)])  {res += 20;}
    if (player_bon::traits[int(Trait::courageous)])  {res += 20;}

    switch (shock_src)
    {
    case Shock_src::use_strange_item:
        if (player_bon::get_bg() == Bg::occultist) {res += 50;}
        break;

    case Shock_src::cast_intr_spell:
    case Shock_src::see_mon:
    case Shock_src::time:
    case Shock_src::misc:
    case Shock_src::END: {}
        break;
    }

    return get_constr_in_range(0, res, 100);
}

double Player::get_shock_taken_after_mods(const int BASE_SHOCK,
        const Shock_src shock_src) const
{
    if (BASE_SHOCK == 0) {return 0.0;}

    const double SHOCK_RES_DB   = double(get_shock_resistance(shock_src));
    const double BASE_SHOCK_DB  = double(BASE_SHOCK);
    return (BASE_SHOCK_DB * (100.0 - SHOCK_RES_DB)) / 100.0;
}

void Player::incr_shock(const int SHOCK, Shock_src shock_src)
{
    const double SHOCK_AFTER_MODS = get_shock_taken_after_mods(SHOCK, shock_src);

    shock_                  += SHOCK_AFTER_MODS;
    perm_shock_taken_cur_turn_  += SHOCK_AFTER_MODS;

    constr_in_range(0.0, shock_, 100.0);
}

void Player::incr_shock(const Shock_lvl shock_value, Shock_src shock_src)
{
    incr_shock(int(shock_value), shock_src);
}

void Player::restore_shock(const int amount_restored, const bool IS_TEMP_SHOCK_RESTORED)
{
    shock_ = max(0.0, shock_ - amount_restored);

    if (IS_TEMP_SHOCK_RESTORED)
    {
        shock_tmp_ = 0.0;
    }

    update_tmp_shock();
}

void Player::incr_insanity()
{
    TRACE << "Increasing insanity" << endl;

    if (!config::is_bot_playing())
    {
        const int INS_INCR = 6;
        ins_ += INS_INCR;
    }

    restore_shock(70, false);

    update_clr();
    render::draw_map_and_interface();

    if (get_insanity() >= 100)
    {
        const string msg = "My mind can no longer withstand what it has grasped. "
                           "I am hopelessly lost.";
        popup::show_msg(msg, true, "Completely insane!", Sfx_id::insanity_rise);
        die(true, false, false);
        return;
    }

    //This point reached means sanity is below 100%
    string msg = "Insanity draws nearer... ";

    //When long term insanity increases, something happens (mostly bad)
    //(Reroll until something actually happens)
    while (true)
    {
        const int RND_INS_EVENT = rnd::range(1, 9);

        switch (RND_INS_EVENT)
        {
        case 1:
        {
            if (rnd::coin_toss())
            {
                msg += "I let out a terrified shriek.";
            }
            else
            {
                msg += "I scream in terror.";
            }
            popup::show_msg(msg, true, "Screaming!", Sfx_id::insanity_rise);

            Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes, pos, this,
                    Snd_vol::high, Alerts_mon::yes);

            snd_emit::emit_snd(snd);
            return;
        } break;

        case 2:
        {
            msg += "I find myself babbling incoherently.";
            popup::show_msg(msg, true, "Babbling!", Sfx_id::insanity_rise);
            const string player_name = get_name_the();
            for (int i = rnd::range(3, 5); i > 0; --i)
            {
                msg_log::add(player_name + ": " + Cultist::get_cultist_phrase());
            }
            Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes, pos, this,
                    Snd_vol::low, Alerts_mon::yes);
            snd_emit::emit_snd(snd);
            return;
        } break;

        case 3:
        {
            msg += "I struggle to not fall into a stupor.";
            popup::show_msg(msg, true, "Fainting!", Sfx_id::insanity_rise);
            prop_handler_->try_apply_prop(new Prop_fainted(Prop_turns::std));
            return;
        } break;

        case 4:
        {
            msg += "I laugh nervously.";
            popup::show_msg(msg, true, "HAHAHA!", Sfx_id::insanity_rise);
            Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes, pos, this,
                    Snd_vol::low, Alerts_mon::yes);
            snd_emit::emit_snd(snd);
            return;
        } break;

        case 5:
        {
            bool props[size_t(Prop_id::END)];
            prop_handler_->get_prop_ids(props);

            if (ins_ >= 10 && !props[int(Prop_id::rFear)])
            {
                if (rnd::coin_toss())
                {
                    //Monster phobias

                    const int RND_MON = rnd::range(1, 4);

                    switch (RND_MON)
                    {
                    case 1:
                        if (!phobias[int(Phobia::rat)])
                        {
                            msg +=
                                "I am afflicted by Murophobia. Rats suddenly seem "
                                "terrifying.";
                            popup::show_msg(msg, true, "Murophobia!", Sfx_id::insanity_rise);
                            phobias[int(Phobia::rat)] = true;
                            return;
                        }
                        break;

                    case 2:
                        if (!phobias[int(Phobia::spider)])
                        {
                            msg +=
                                "I am afflicted by Arachnophobia. Spiders suddenly seem "
                                "terrifying.";
                            popup::show_msg(msg, true, "Arachnophobia!",
                                            Sfx_id::insanity_rise);
                            phobias[int(Phobia::spider)] = true;
                            return;
                        }
                        break;

                    case 3:
                        if (!phobias[int(Phobia::dog)])
                        {
                            msg +=
                                "I am afflicted by Cynophobia. Canines suddenly seem "
                                "terrifying.";
                            popup::show_msg(msg, true, "Cynophobia!", Sfx_id::insanity_rise);
                            phobias[int(Phobia::dog)] = true;
                            return;
                        }
                        break;

                    case 4:
                        if (!phobias[int(Phobia::undead)])
                        {
                            msg +=
                                "I am afflicted by Necrophobia. The undead suddenly "
                                "seem far more terrifying.";
                            popup::show_msg(msg, true, "Necrophobia!", Sfx_id::insanity_rise);
                            phobias[int(Phobia::undead)] = true;
                            return;
                        }
                        break;
                    }
                }
                else //Cointoss (not a monster phobia)
                {
                    //Environment phobias

                    const int RND_ENV_PHOBIA = rnd::range(1, 3);

                    switch (RND_ENV_PHOBIA)
                    {
                    case 1:
                        //Fear of cramped or open places
                        if (is_standing_in_open_space() && !phobias[int(Phobia::open_place)])
                        {
                            msg +=
                                "I am afflicted by Agoraphobia. Open places suddenly "
                                "seem terrifying.";
                            popup::show_msg(msg, true, "Agoraphobia!", Sfx_id::insanity_rise);
                            phobias[int(Phobia::open_place)] = true;
                            return;
                        }
                        else if (is_standing_in_cramped_space())
                        {
                            if (!phobias[int(Phobia::cramped_place)])
                            {
                                msg +=
                                    "I am afflicted by Claustrophobia. Confined places "
                                    "suddenly seem terrifying.";
                                popup::show_msg(msg, true, "Claustrophobia!",
                                                Sfx_id::insanity_rise);
                                phobias[int(Phobia::cramped_place)] = true;
                                return;
                            }
                        }
                        break;

                    case 2:
                        //Fear of deep places
                        if (!phobias[int(Phobia::deep_places)])
                        {
                            msg +=
                                "I am afflicted by Bathophobia. It suddenly seems "
                                "terrifying to delve deeper.";
                            popup::show_msg(msg, true, "Bathophobia!");
                            phobias[int(Phobia::deep_places)] = true;
                            return;
                        }
                        break;

                    case 3:
                        //Fear of the dark
                        if (!phobias[int(Phobia::dark)])
                        {
                            msg +=
                                "I am afflicted by Nyctophobia. Darkness suddenly seem "
                                "far more terrifying.";
                            popup::show_msg(msg, true, "Nyctophobia!");
                            phobias[int(Phobia::dark)] = true;
                            return;
                        }
                        break;
                    }
                }
            }
        } break;

        case 6:
        {
            if (ins_ > 20)
            {
                int obsessions_active = 0;
                for (int i = 0; i < int(Obsession::END); ++i)
                {
                    if (obsessions[i]) {obsessions_active++;}
                }
                if (obsessions_active == 0)
                {
                    const Obsession obsession =
                        (Obsession)(rnd::range(0, int(Obsession::END) - 1));
                    switch (obsession)
                    {
                    case Obsession::masochism:
                    {
                        msg +=
                            "To my alarm, I find myself encouraged by the sensation of "
                            "pain. Physical suffering does not frighten me at all. "
                            "However, my depraved mind can never find complete peace "
                            "(no shock from taking damage, but permanent +"
                            + to_str(SHOCK_FROM_OBSESSION) + "% shock).";
                        popup::show_msg(msg, true, "Masochistic obsession!",
                                        Sfx_id::insanity_rise);
                        obsessions[int(Obsession::masochism)] = true;
                        return;
                    } break;

                    case Obsession::sadism:
                    {
                        msg +=
                            "To my alarm, I find myself encouraged by the pain I cause "
                            "in others. For every life I take, I find a little relief. "
                            "However, my depraved mind can no longer find complete "
                            "peace (permanent +" + to_str(SHOCK_FROM_OBSESSION) + "% "
                            "shock).";
                        popup::show_msg(msg, true, "Sadistic obsession!",
                                        Sfx_id::insanity_rise);
                        obsessions[int(Obsession::sadism)] = true;
                        return;
                    } break;
                    default: {} break;
                    }
                }
            }
        } break;

        case 7:
        {
            if (ins_ > 8)
            {
                msg += "The shadows are closing in on me!";
                popup::show_msg(msg, true, "Haunted by shadows!", Sfx_id::insanity_rise);
                const int NR_SHADOWS_LOWER = 2;
                const int NR_SHADOWS_UPPER =
                    get_constr_in_range(2, (map::dlvl + 1) / 2, 6);
                const int NR =
                    rnd::range(NR_SHADOWS_LOWER, NR_SHADOWS_UPPER);
                actor_factory::summon(pos, vector<Actor_id>(NR, Actor_id::shadow), true);
                return;
            }
        } break;

        case 8:
        {
            msg +=
                "I find myself in a peculiar trance. I struggle to recall where I am, "
                "and what is happening.";
            popup::show_msg(msg, true, "Confusion!", Sfx_id::insanity_rise);
            prop_handler_->try_apply_prop(new Prop_confused(Prop_turns::std));
            return;
        } break;

        case 9:
        {
            msg +=
                "There is a strange itch, as if something is crawling on the back of "
                "my neck.";
            popup::show_msg(msg, true, "Strange sensation", Sfx_id::insanity_rise);
            return;
        } break;
        }
    }
}

bool Player::is_standing_in_open_space() const
{
    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);
    for (int x = pos.x - 1; x <= pos.x + 1; ++x)
    {
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            if (blocked[x][y]) {return false;}
        }
    }
    return true;
}

bool Player::is_standing_in_cramped_space() const
{
    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);
    int block_count = 0;
    for (int x = pos.x - 1; x <= pos.x + 1; ++x)
    {
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            if (blocked[x][y])
            {
                block_count++;
                if (block_count >= 6) {return true;}
            }
        }
    }

    return false;
}

void Player::test_phobias()
{
    if (!map::player->get_prop_handler().allow_act())
    {
        return;
    }

    if (rnd::one_in(10))
    {
        //Monster phobia?
        vector<Actor*> seen_foes;
        get_seen_foes(seen_foes);

        for (Actor* const actor : seen_foes)
        {
            const actor_data_t& mon_data = actor->get_data();
            if (mon_data.is_canine && phobias[int(Phobia::dog)])
            {
                msg_log::add("I am plagued by my canine phobia!");
                prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
                return;
            }
            if (mon_data.is_rat && phobias[int(Phobia::rat)])
            {
                msg_log::add("I am plagued by my rat phobia!");
                prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
                return;
            }
            if (mon_data.is_undead && phobias[int(Phobia::undead)])
            {
                msg_log::add("I am plagued by my phobia of the dead!");
                prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
                return;
            }
            if (mon_data.is_spider && phobias[int(Phobia::spider)])
            {
                msg_log::add("I am plagued by my spider phobia!");
                prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
                return;
            }
        }

        //Environment phobia?
        if (phobias[int(Phobia::open_place)] && is_standing_in_open_space())
        {
            msg_log::add("I am plagued by my phobia of open places!");
            prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
            return;
        }

        if (phobias[int(Phobia::cramped_place)] && is_standing_in_cramped_space())
        {
            msg_log::add("I am plagued by my phobia of closed places!");
            prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
            return;
        }

        for (const Pos& d : dir_utils::dir_list)
        {
            const Pos p(pos + d);

            if (
                phobias[int(Phobia::deep_places)] &&
                map::cells[p.x][p.y].rigid->is_bottomless())
            {
                msg_log::add("I am plagued by my phobia of deep places!");
                prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
                break;
            }

            if (
                phobias[int(Phobia::dark)]  &&
                map::cells[p.x][p.y].is_dark &&
                !map::cells[p.x][p.y].is_lit)
            {
                msg_log::add("I am plagued by my fear of the dark!");
                prop_handler_->try_apply_prop(new Prop_terrified(Prop_turns::std));
                break;
            }
        }
    }
}

void Player::update_clr()
{
    if (!is_alive())
    {
        clr_ = clr_red;
        return;
    }

    if (prop_handler_->change_actor_clr(clr_))
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
    nr_quick_move_steps_left_ = 10;
    quick_move_dir_         = dir;
}

void Player::on_actor_turn()
{
    render::draw_map_and_interface();

    reset_perm_shock_taken_cur_turn();

    if (!is_alive())
    {
        return;
    }

    if (active_medical_bag)
    {
        active_medical_bag->continue_action();
        return;
    }

    if (wait_turns_left > 0)
    {
        wait_turns_left--;
        game_time::tick();
        return;
    }

    if (tgt_ && tgt_->get_state() != Actor_state::alive)
    {
        tgt_ = nullptr;
    }

    //If player dropped item, check if should go back to inventory screen
    const auto inv_screen_after_drop = inv_handling::screen_to_open_after_drop;

    if (inv_screen_after_drop != Inv_scr_id::END)
    {
        vector<Actor*> seen_foes;
        get_seen_foes(seen_foes);

        if (seen_foes.empty())
        {
            switch (inv_screen_after_drop)
            {
            case Inv_scr_id::inv:
                inv_handling::run_inv_screen();
                break;

            case Inv_scr_id::equip:
                inv_handling::run_equip_screen(*inv_handling::equip_slot_to_open_after_drop);
                break;

            case Inv_scr_id::END: {} break;
            }
            return;
        }
        else //There are seen monsters
        {
            inv_handling::screen_to_open_after_drop    = Inv_scr_id::END;
            inv_handling::browser_idx_to_set_after_drop = 0;
        }
    }

    //Quick move
    if (nr_quick_move_steps_left_ > 0)
    {
        //NOTE: There is no need to check for items here, since the message from stepping
        //on an item will interrupt player actions.

        const Pos dest_pos(pos + dir_utils::get_offset(quick_move_dir_));

        const Cell&         tgt_cell   = map::cells[dest_pos.x][dest_pos.y];
        const Rigid* const  tgt_rigid  = tgt_cell.rigid;

        const bool IS_TGT_KNOWN_TRAP  = tgt_rigid->get_id() == Feature_id::trap &&
                                        !static_cast<const Trap*>(tgt_rigid)->is_hidden();

        const bool SHOULD_ABORT = !tgt_rigid->can_move_cmn()                         ||
                                  IS_TGT_KNOWN_TRAP                               ||
                                  tgt_rigid->get_burn_state() == Burn_state::burning  ||
                                  (tgt_cell.is_dark && !tgt_cell.is_lit);

        if (SHOULD_ABORT)
        {
            nr_quick_move_steps_left_ = -1;
            quick_move_dir_         = Dir::END;
        }
        else
        {
            --nr_quick_move_steps_left_;
            move_dir(quick_move_dir_);
            return;
        }
    }

    //If this point is reached - read input from player
    if (config::is_bot_playing())
    {
        bot::act();
    }
    else
    {
        input::clear_events();
        input::map_mode_input();
    }
}

void Player::update_tmp_shock()
{
    shock_tmp_ = 0.0;

    //Shock from obsession?
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        if (obsessions[i])
        {
            shock_tmp_ += double(SHOCK_FROM_OBSESSION);
            break;
        }
    }

    //Temporary shock from darkness
    Cell& cell = map::cells[pos.x][pos.y];

    if (cell.is_dark && !cell.is_lit)
    {
        shock_tmp_ += get_shock_taken_after_mods(20, Shock_src::misc);
    }

    //Temporary shock from features
    for (const Pos& d : dir_utils::dir_list)
    {
        const Pos p(pos + d);

        const int BASE_SHOCK = map::cells[p.x][p.y].rigid->get_shock_when_adj();

        shock_tmp_ += get_shock_taken_after_mods(BASE_SHOCK, Shock_src::misc);
    }

    //Temporary shock from items
    for (auto& slot : inv_->slots_)
    {
        if (slot.item)
        {
            const int BASE_SHOCK = slot.item->get_data().shock_while_equipped;

            shock_tmp_ += get_shock_taken_after_mods(BASE_SHOCK, Shock_src::use_strange_item);
        }
    }

    for (const Item* const item : inv_->general_)
    {
        const int BASE_SHOCK = item->get_data().shock_while_in_backpack;

        shock_tmp_ += get_shock_taken_after_mods(BASE_SHOCK, Shock_src::use_strange_item);
    }

    shock_tmp_ = min(99.0, shock_tmp_);
}

void Player::on_std_turn()
{
    update_tmp_shock();

    if (active_explosive)   {active_explosive->on_std_turn_player_hold_ignited();}

    if (!active_medical_bag) {test_phobias();}

    vector<Actor*> seen_foes;
    get_seen_foes(seen_foes);

    double shock_from_mon_cur_player_turn = 0.0;

    for (Actor* actor : seen_foes)
    {
        dungeon_master::on_mon_seen(*actor);

        Mon* mon = static_cast<Mon*>(actor);

        mon->player_become_aware_of_me();

        const actor_data_t& data = mon->get_data();
        if (data.mon_shock_lvl != Mon_shock_lvl::none)
        {
            switch (data.mon_shock_lvl)
            {
            case Mon_shock_lvl::unsettling:
                mon->shock_caused_cur_ = min(mon->shock_caused_cur_ + 0.05,  1.0);
                break;

            case Mon_shock_lvl::scary:
                mon->shock_caused_cur_ = min(mon->shock_caused_cur_ + 0.1,   1.0);
                break;

            case Mon_shock_lvl::terrifying:
                mon->shock_caused_cur_ = min(mon->shock_caused_cur_ + 0.5,   2.0);
                break;

            case Mon_shock_lvl::mind_shattering:
                mon->shock_caused_cur_ = min(mon->shock_caused_cur_ + 0.75,  3.0);
                break;

            default: {} break;
            }
            if (shock_from_mon_cur_player_turn < 2.5)
            {
                incr_shock(int(floor(mon->shock_caused_cur_)), Shock_src::see_mon);
                shock_from_mon_cur_player_turn += mon->shock_caused_cur_;
            }
        }
    }

    //Some shock is taken every Xth turn
    if (prop_handler_->allow_act())
    {
        int turn_nr_incr_shock = 12;

        if (player_bon::get_bg() == Bg::rogue)
        {
            turn_nr_incr_shock *= 2;
        }

        const int TURN = game_time::get_turn();

        if (TURN % turn_nr_incr_shock == 0 && TURN > 1)
        {
            if (rnd::one_in(850))
            {
                if (rnd::coin_toss())
                {
                    popup::show_msg("I have a bad feeling about this...", true);
                }
                else
                {
                    popup::show_msg("A chill runs down my spine...", true);
                }
                incr_shock(Shock_lvl::heavy, Shock_src::misc);
                render::draw_map_and_interface();
            }
            else
            {
                if (map::dlvl != 0)
                {
                    incr_shock(1, Shock_src::time);
                }
            }
        }
    }

    //Take sanity hit from high shock?
    if (get_shock_total() >= 100)
    {
        nr_turns_until_ins_ = nr_turns_until_ins_ < 0 ? 3 : nr_turns_until_ins_ - 1;
        if (nr_turns_until_ins_ > 0)
        {
            render::draw_map_and_interface(true);
            msg_log::add("I feel my sanity slipping...", clr_msg_note, true, true);
        }
        else
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
    for (Actor* actor : game_time::actors_)
    {
        if (!actor->is_player() && !map::player->is_leader_of(actor) && actor->is_alive())
        {
            Mon& mon                = *static_cast<Mon*>(actor);
            const bool IS_MON_SEEN  = can_see_actor(*actor, nullptr);
            if (IS_MON_SEEN)
            {
                if (!mon.is_msg_mon_in_view_printed_)
                {
                    if (
                        active_medical_bag    ||
                        wait_turns_left > 0   ||
                        nr_quick_move_steps_left_ > 0)
                    {
                        msg_log::add(actor->get_name_a() + " comes into my view.", clr_white,
                                     true);
                    }
                    mon.is_msg_mon_in_view_printed_ = true;
                }
            }
            else //Monster is not seen
            {
                mon.is_msg_mon_in_view_printed_ = false;

                //Is the monster sneaking? Try to spot it
                if (
                    map::cells[mon.pos.x][mon.pos.y].is_seen_by_player &&
                    mon.is_stealth_                                  &&
                    is_spotting_hidden_actor(mon))
                {
                    mon.is_stealth_ = false;
                    update_fov();
                    render::draw_map_and_interface();
                    const string mon_name = mon.get_name_a();
                    msg_log::add("I spot " + mon_name + "!", clr_msg_note, true, true);
                }
            }
        }
    }

    const int DECR_ABOVE_MAX_N_TURNS = 7;
    if (get_hp() > get_hp_max(true))
    {
        if (game_time::get_turn() % DECR_ABOVE_MAX_N_TURNS == 0) {hp_--;}
    }
    if (get_spi() > get_spi_max())
    {
        if (game_time::get_turn() % DECR_ABOVE_MAX_N_TURNS == 0) {spi_--;}
    }

    if (!active_medical_bag)
    {
        bool props[size_t(Prop_id::END)];
        prop_handler_->get_prop_ids(props);

        if (!props[int(Prop_id::poisoned)])
        {
            int nr_turns_per_hp = 40;

            if (player_bon::traits[int(Trait::rapid_recoverer)])  {nr_turns_per_hp -= 12;}
            if (player_bon::traits[int(Trait::survivalist)])     {nr_turns_per_hp -= 12;}

            //Items affect HP regen?
            for (const auto& slot : inv_->slots_)
            {
                if (slot.item)
                {
                    nr_turns_per_hp += slot.item->get_hp_regen_change(Inv_type::slots);
                }
            }

            for (const Item* const item : inv_->general_)
            {
                nr_turns_per_hp += item->get_hp_regen_change(Inv_type::general);
            }

            const int TURN = game_time::get_turn();

            if (get_hp() < get_hp_max(true) && (TURN % nr_turns_per_hp == 0)  && TURN > 1)
            {
                ++hp_;
            }
        }

        if (!props[int(Prop_id::confused)] && prop_handler_->allow_see())
        {
            const int R = player_bon::traits[int(Trait::perceptive)] ? 3 :
                          (player_bon::traits[int(Trait::observant)] ? 2 : 1);

            int x0 = max(0, pos.x - R);
            int y0 = max(0, pos.y - R);
            int x1 = min(MAP_W - 1, pos.x + R);
            int y1 = min(MAP_H - 1, pos.y + R);

            for (int y = y0; y <= y1; ++y)
            {
                for (int x = x0; x <= x1; ++x)
                {
                    if (map::cells[x][y].is_seen_by_player)
                    {
                        auto* f = map::cells[x][y].rigid;

                        if (f->get_id() == Feature_id::trap)
                        {
                            static_cast<Trap*>(f)->player_try_spot_hidden();
                        }
                        if (f->get_id() == Feature_id::door)
                        {
                            static_cast<Door*>(f)->player_try_spot_hidden();
                        }
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
    wait_turns_left         = -1;

    //Abort quick move
    nr_quick_move_steps_left_ = -1;
    quick_move_dir_         = Dir::END;
}

void Player::interrupt_actions()
{
    render::draw_map_and_interface();

    //Abort browsing inventory
    inv_handling::screen_to_open_after_drop    = Inv_scr_id::END;
    inv_handling::browser_idx_to_set_after_drop = 0;

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

void Player::hear_sound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                        const Dir dir_to_origin,
                        const int PERCENT_AUDIBLE_DISTANCE)
{
    const Sfx_id     sfx         = snd.get_sfx();
    const string&   msg         = snd.get_msg();
    const bool      HAS_SND_MSG = !msg.empty() && msg != " ";

    if (HAS_SND_MSG)
    {
        msg_log::add(msg, clr_white);
    }

    //Play audio after message to ensure sync between audio and animation
    //If origin is hidden, we only play the sound if there is a message
    if (HAS_SND_MSG || IS_ORIGIN_SEEN_BY_PLAYER)
    {
        audio::play(sfx, dir_to_origin, PERCENT_AUDIBLE_DISTANCE);
    }

    if (HAS_SND_MSG)
    {
        Actor* const actor_who_made_snd = snd.get_actor_who_made_sound();
        if (actor_who_made_snd && actor_who_made_snd != this)
        {
            static_cast<Mon*>(actor_who_made_snd)->player_become_aware_of_me();
        }
    }
}

void Player::move_dir(Dir dir)
{
    if (is_alive())
    {
        prop_handler_->change_move_dir(pos, dir);

        //Trap affects leaving?
        if (dir != Dir::center)
        {
            Feature* f = map::cells[pos.x][pos.y].rigid;
            if (f->get_id() == Feature_id::trap)
            {
                TRACE << "Standing on trap, check if affects move" << endl;
                dir = static_cast<Trap*>(f)->actor_try_leave(*this, dir);
            }
        }

        bool is_free_turn = false;;

        const Pos dest(pos + dir_utils::get_offset(dir));

        if (dir != Dir::center)
        {
            Mon* const mon_at_dest = static_cast<Mon*>(utils::get_actor_at_pos(dest));

            //Attack?
            if (mon_at_dest && !is_leader_of(mon_at_dest))
            {
                if (prop_handler_->allow_attack_melee(true))
                {
                    bool has_melee_wpn = false;
                    Item* const item = inv_->get_item_in_slot(Slot_id::wielded);
                    if (item)
                    {
                        Wpn* const wpn = static_cast<Wpn*>(item);

                        if (wpn->get_data().melee.is_melee_wpn)
                        {
                            if (
                                config::is_ranged_wpn_meleee_prompt()   &&
                                can_see_actor(*mon_at_dest, nullptr)  &&
                                wpn->get_data().ranged.is_ranged_wpn)
                            {
                                const string wpn_name = wpn->get_name(Item_ref_type::a);

                                msg_log::add("Attack " + mon_at_dest->get_name_the() +
                                             " with " + wpn_name + " ? [y/n]",
                                             clr_white_high);

                                render::draw_map_and_interface();

                                if (query::yes_or_no() == Yes_no_answer::no)
                                {
                                    msg_log::clear();
                                    render::draw_map_and_interface();
                                    return;
                                }
                            }
                            attack::melee(*this, *wpn, *mon_at_dest);
                            tgt_ = mon_at_dest;
                            return;
                        }
                    }
                    if (!has_melee_wpn)
                    {
                        punch_mon(*mon_at_dest);
                    }
                }
                return;
            }

            //This point reached means no actor in the destination cell.

            //Blocking mobile or rigid?
            bool props[size_t(Prop_id::END)];
            get_prop_handler().get_prop_ids(props);
            Cell& cell = map::cells[dest.x][dest.y];
            bool is_features_allow_move = cell.rigid->can_move(props);

            vector<Mob*> mobs;
            game_time::get_mobs_at_pos(dest, mobs);

            if (is_features_allow_move)
            {
                for (auto* m : mobs)
                {
                    if (!m->can_move(props))
                    {
                        is_features_allow_move = false;
                        break;
                    }
                }
            }

            if (is_features_allow_move)
            {
                //Encumbrance
                const int ENC = get_enc_percent();
                if (ENC >= ENC_IMMOBILE_LVL)
                {
                    msg_log::add("I am too encumbered to move!");
                    render::draw_map_and_interface();
                    return;
                }
                else if (ENC >= 100)
                {
                    msg_log::add("I stagger.", clr_msg_note);
                    prop_handler_->try_apply_prop(new Prop_waiting(Prop_turns::std));
                }

                //Displace allied monster
                if (mon_at_dest && is_leader_of(mon_at_dest))
                {
                    msg_log::add("I displace " + mon_at_dest->get_name_a() + ".");
                    mon_at_dest->pos = pos;
                }

                pos = dest;

                const int FREE_MOVE_EVERY_N_TURN =
                    player_bon::traits[int(Trait::mobile)]     ? 2 :
                    player_bon::traits[int(Trait::lithe)]      ? 4 :
                    player_bon::traits[int(Trait::dexterous)]  ? 5 : 0;

                if (FREE_MOVE_EVERY_N_TURN > 0)
                {
                    if (nr_moves_until_free_action_ == -1)
                    {
                        nr_moves_until_free_action_ = FREE_MOVE_EVERY_N_TURN - 2;
                    }
                    else if (nr_moves_until_free_action_ == 0)
                    {
                        nr_moves_until_free_action_ = FREE_MOVE_EVERY_N_TURN - 1;
                        is_free_turn = true;
                    }
                    else
                    {
                        nr_moves_until_free_action_--;
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
                                 clr_white, true);

                    string item_name = item->get_name(Item_ref_type::plural, Item_ref_inf::yes,
                                                      Item_ref_att_inf::wpn_context);

                    text_format::first_to_upper(item_name);

                    msg_log::add(item_name + ".");
                }
            }

            //NOTE: bump() prints block messages.
            for (auto* m : mobs) {m->bump(*this);}

            map::cells[dest.x][dest.y].rigid->bump(*this);
        }

        if (pos == dest)
        {
            game_time::tick(is_free_turn);
            return;
        }
    }
}

void Player::auto_melee()
{
    if (
        tgt_                                    &&
        tgt_->get_state() == Actor_state::alive   &&
        utils::is_pos_adj(pos, tgt_->pos, false)  &&
        can_see_actor(*tgt_, nullptr))
    {
        move_dir(dir_utils::get_dir(tgt_->pos - pos));
        return;
    }

    //If this line reached, there is no adjacent cur target.
    for (const Pos& d : dir_utils::dir_list)
    {
        Actor* const actor = utils::get_actor_at_pos(pos + d);
        if (actor && !is_leader_of(actor) && can_see_actor(*actor, nullptr))
        {
            tgt_ = actor;
            move_dir(dir_utils::get_dir(d));
            return;
        }
    }
}

void Player::kick_mon(Actor& actor_to_kick)
{
    Wpn* kick_wpn = nullptr;

    const actor_data_t& d = actor_to_kick.get_data();

    if (d.actor_size == Actor_size::floor && (d.is_spider || d.is_rat))
    {
        kick_wpn = static_cast<Wpn*>(item_factory::mk(Item_id::player_stomp));
    }
    else
    {
        kick_wpn = static_cast<Wpn*>(item_factory::mk(Item_id::player_kick));
    }
    attack::melee(*this, *kick_wpn, actor_to_kick);
    delete kick_wpn;
}

void Player::punch_mon(Actor& actor_to_punch)
{
    //Spawn a temporary punch weapon to attack with
    Wpn* punch_wpn = static_cast<Wpn*>(item_factory::mk(Item_id::player_punch));
    attack::melee(*this, *punch_wpn, actor_to_punch);
    delete punch_wpn;
}

void Player::add_light_(bool light_map[MAP_W][MAP_H]) const
{
    Lgt_size lgt_size = Lgt_size::none;

    if (active_explosive)
    {
        if (active_explosive->get_data().id == Item_id::flare)
        {
            lgt_size = Lgt_size::fov;
        }
    }

    if (lgt_size != Lgt_size::fov)
    {
        for (Item* const item : inv_->general_)
        {
            Lgt_size item_lgt_size = item->get_lgt_size();

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
        bool my_light[MAP_W][MAP_H];
        utils::reset_array(my_light, false);

        const int R = FOV_STD_RADI_INT;

        Pos p0(max(0,         pos.x - R), max(0,         pos.y - R));
        Pos p1(min(MAP_W - 1, pos.x + R), min(MAP_H - 1, pos.y + R));

        bool blocked_los[MAP_W][MAP_H];

        for (int y = p0.y; y <= p1.y; ++y)
        {
            for (int x = p0.x; x <= p1.x; ++x)
            {
                const auto* const f = map::cells[x][y].rigid;
                blocked_los[x][y]    = !f->is_los_passable();
            }
        }

        fov::run_fov_on_array(blocked_los, pos, my_light, false);
        for (int y = p0.y; y <= p1.y; ++y)
        {
            for (int x = p0.x; x <= p1.x; ++x)
            {
                if (my_light[x][y])
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

    case Lgt_size::none: {}
        break;
    }
}

void Player::update_fov()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            map::cells[x][y].is_seen_by_player = false;
        }
    }

    if (prop_handler_->allow_see())
    {
        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_los(), blocked);
        fov::run_player_fov(blocked, pos);
        map::cells[pos.x][pos.y].is_seen_by_player = true;
    }

    if (prop_handler_->allow_see()) {FOVhack();}

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

void Player::FOVhack()
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
                const Pos p(x, y);

                for (const Pos& d : dir_utils::dir_list)
                {
                    const Pos p_adj(p + d);

                    if (utils::is_pos_inside_map(p_adj))
                    {
                        const Cell& adj_cell = map::cells[p_adj.x][p_adj.y];
                        if (
                            adj_cell.is_seen_by_player              &&
                            (!adj_cell.is_dark || adj_cell.is_lit)  &&
                            !blocked[p_adj.x][p_adj.y])
                        {
                            map::cells[x][y].is_seen_by_player = true;
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
