#include "init.hpp"

#include "item.hpp"

#include <cassert>

#include "map.hpp"
#include "game_time.hpp"
#include "utils.hpp"
#include "map_parsing.hpp"
#include "properties.hpp"
#include "msg_log.hpp"
#include "explosion.hpp"
#include "render.hpp"
#include "input.hpp"
#include "query.hpp"
#include "item_factory.hpp"
#include "feature_mob.hpp"
#include "feature_rigid.hpp"
#include "item_data.hpp"

using namespace std;

//---------------------------------------------------------- ITEM
Item::Item(Item_data_t* item_data) :
    nr_items_       (1),
    melee_dmg_plus_ (0),
    data_           (item_data),
    carrier_props_  (),
    carrier_spells_ () {}

Item& Item::operator=(const Item& other)
{
    nr_items_       = other.nr_items_;
    melee_dmg_plus_ = other.melee_dmg_plus_;
    data_           = other.data_;
    carrier_props_  = other.carrier_props_;
    carrier_spells_ = other.carrier_spells_;

    return *this;
}

Item::~Item()
{
    for (auto prop   : carrier_props_)
    {
        delete prop;
    }

    for (auto spell  : carrier_spells_)
    {
        delete spell;
    }
}

Item_id Item::id() const
{
    return data_->id;
}

const Item_data_t& Item::data() const
{
    return *data_;
}

Clr Item::clr() const
{
    return data_->clr;
}

char Item::glyph() const
{
    return data_->glyph;
}

Tile_id Item::tile() const
{
    return data_->tile;
}

vector<string> Item::descr() const
{
    return data_->base_descr;
}

int Item::weight() const
{
    return int(data_->weight) * nr_items_;
}

string Item::weight_str() const
{
    const int WEIGHT = weight();

    if (WEIGHT <= (int(Item_weight::extra_light) + int(Item_weight::light)) / 2)
    {
        return "very light";
    }

    if (WEIGHT <= (int(Item_weight::light) + int(Item_weight::medium)) / 2)
    {
        return "light";
    }

    if (WEIGHT <= (int(Item_weight::medium) + int(Item_weight::heavy)) / 2)
    {
        return "a bit heavy";
    }

    return "heavy";
}

Consume_item Item::activate(Actor* const actor)
{
    (void)actor;
    msg_log::add("I cannot apply that.");
    return Consume_item::no;
}

string Item::name(const Item_ref_type      ref_type,
                  const Item_ref_inf       inf,
                  const Item_ref_att_inf    att_inf) const
{
    Item_ref_type ref_type_used = ref_type;

    if (ref_type == Item_ref_type::plural && (!data_->is_stackable || nr_items_ == 1))
    {
        ref_type_used = Item_ref_type::a;
    }

    string nr_str = "";

    if (ref_type_used == Item_ref_type::plural)
    {
        nr_str = to_str(nr_items_) + " ";
    }

    string att_str = "";

    Item_ref_att_inf att_inf_used = att_inf;

    if (att_inf == Item_ref_att_inf::wpn_context)
    {
        switch (data_->main_att_mode)
        {
        case Main_att_mode::melee:
            att_inf_used = Item_ref_att_inf::melee;
            break;

        case Main_att_mode::ranged:
            att_inf_used = Item_ref_att_inf::ranged;
            break;

        case Main_att_mode::thrown:
            att_inf_used = Item_ref_att_inf::thrown;
            break;

        case Main_att_mode::none:
            att_inf_used = Item_ref_att_inf::none;
            break;
        }
    }

    if (att_inf_used == Item_ref_att_inf::melee)
    {
        const string    rolls_str       = to_str(data_->melee.dmg.first);
        const string    sides_str       = to_str(data_->melee.dmg.second);
        const int       PLUS            = melee_dmg_plus_;
        const string    plus_str        = PLUS == 0 ? "" :
                                          PLUS  > 0 ?
                                          ("+" + to_str(PLUS)) :
                                          ("-" + to_str(PLUS));
        const int       ITEM_SKILL      = data_->melee.hit_chance_mod;
        const int       MELEE_SKILL     = map::player->ability(Ability_id::melee, true);
        const int       SKILL_TOT       = max(0, min(100, ITEM_SKILL + MELEE_SKILL));
        const string    skill_str       = to_str(SKILL_TOT) + "%";

        att_str = " " + rolls_str + "d" + sides_str + plus_str + " " + skill_str;
    }

    const int RANGED_SKILL = map::player->ability(Ability_id::ranged, true);

    if (att_inf_used == Item_ref_att_inf::ranged)
    {
        string dmg_str = data_->ranged.dmg_info_override;

        if (dmg_str.empty())
        {
            const int       MULTIPL     = data_->ranged.is_machine_gun ?
                                          NR_MG_PROJECTILES : 1;
            const string    rolls_str   = to_str(data_->ranged.dmg.rolls * MULTIPL);
            const string    sides_str   = to_str(data_->ranged.dmg.sides);
            const int       PLUS        = data_->ranged.dmg.plus * MULTIPL;
            const string    plus_str    = PLUS ==  0 ? "" :
                                          PLUS  > 0  ?
                                          ("+" + to_str(PLUS)) :
                                          ("-" + to_str(PLUS));
            dmg_str                     = rolls_str + "d" + sides_str + plus_str;
        }

        const int       ITEM_SKILL      = data_->ranged.hit_chance_mod;
        const int       SKILL_TOT       = max(0, min(100, ITEM_SKILL + RANGED_SKILL));
        const string    skill_str       = to_str(SKILL_TOT) + "%";

        att_str = " " + dmg_str + " " + skill_str;
    }

    if (att_inf_used == Item_ref_att_inf::thrown)
    {
        const string    rolls_str       = to_str(data_->ranged.throw_dmg.rolls);
        const string    sides_str       = to_str(data_->ranged.throw_dmg.sides);
        const int       PLUS            = data_->ranged.throw_dmg.plus;
        const string    plus_str        = PLUS ==  0 ? "" :
                                          PLUS  > 0 ? "+" :
                                          ("-" + to_str(PLUS));
        const int       ITEM_SKILL      = data_->ranged.throw_hit_chance_mod;
        const int       SKILL_TOT       = max(0, min(100, ITEM_SKILL + RANGED_SKILL));
        const string    skill_str       = to_str(SKILL_TOT) + "%";

        att_str = " " + rolls_str + "d" + sides_str + plus_str + " " + skill_str;
    }

    string inf_str = "";

    if (inf == Item_ref_inf::yes)
    {
        inf_str = name_inf();

        if (!inf_str.empty()) {inf_str.insert(0, " ");}
    }

    const auto& names_used = data_->is_identified ? data_->base_name : data_->base_name_un_id;

    return nr_str + names_used.names[int(ref_type_used)] + att_str + inf_str;
}

bool Item::is_in_effective_range_lmt(const Pos& p0, const Pos& p1) const
{
    return utils::king_dist(p0, p1) <= data_->ranged.effective_range;
}

void Item::add_carrier_prop(Prop* const prop, Actor& actor, const Verbosity verbosity)
{
    assert(prop);

    actor.prop_handler().add_prop_from_equiped_item(this, prop);
}

void Item::clear_carrier_props(Actor& actor)
{
    actor.prop_handler().remove_props_for_item(this);
}

void Item::add_carrier_spell(Spell* const spell)
{
    assert(spell);

    carrier_spells_.push_back(spell);
}

void Item::clear_carrier_spells()
{
    for (Spell* const spell : carrier_spells_)
    {
        delete spell;
    }

    carrier_spells_.clear();
}

//---------------------------------------------------------- ARMOR
Armor::Armor(Item_data_t* const item_data) :
    Item    (item_data),
    dur_    (rnd::range(80, 100)) {}

void Armor::store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(dur_));
}

void Armor::setup_from_save_lines(vector<string>& lines)
{
    dur_ = to_int(lines.front());
    lines.erase(begin(lines));
}

Unequip_allowed Armor::on_unequip(Actor& actor)
{
    render::draw_map_and_interface();

    const Unequip_allowed unequip_allowed = on_unequip_hook(actor);

    if (unequip_allowed == Unequip_allowed::yes)
    {
        const string armor_name = name(Item_ref_type::plain, Item_ref_inf::none);

        msg_log::add("I take off my " + armor_name + ".", clr_white, false,
                     More_prompt_on_msg::yes);
    }

    game_time::tick();

    return unequip_allowed;
}

string Armor::armor_data_line(const bool WITH_BRACKETS) const
{
    const int       AP      = armor_points();
    const string    ap_str  = to_str(max(0, AP));

    return WITH_BRACKETS ? ("[" + ap_str + "]") : ap_str;
}

int Armor::take_dur_hit_and_get_reduced_dmg(const int DMG_BEFORE)
{
    TRACE_FUNC_BEGIN;

    //AP, Armor points:
    //  - Damage soaked up instead of hitting the player
    //DFF, Damage (to) Durability Factor:
    //  - A factor of how much damage the armor durability takes per attack damage point

    const int     AP_BEFORE       = armor_points();
    const double  DDF_BASE        = data_->armor.dmg_to_durability_factor;

    //TODO: Add check for if wearer is player!

    const double  DDF_WAR_VET_MOD = player_bon::bg() == Bg::war_vet ? 0.5 : 1.0;
    const double  DDF_K           = 1.5;
    const double  DMG_BEFORE_DB   = double(DMG_BEFORE);

    dur_ -= int(DMG_BEFORE_DB * DDF_BASE * DDF_WAR_VET_MOD * DDF_K);

    dur_                          = max(0, dur_);
    const int AP_AFTER            = armor_points();

    if (AP_AFTER < AP_BEFORE && AP_AFTER != 0)
    {
        const string armor_name = name(Item_ref_type::plain);
        msg_log::add("My " + armor_name + " is damaged!", clr_msg_note);
    }

    TRACE << "Damage before: " + to_str(DMG_BEFORE) << endl;

    const int DMG_AFTER = max(1, DMG_BEFORE - AP_BEFORE);

    TRACE << "Damage after: " + to_str(DMG_AFTER) << endl;

    TRACE_FUNC_END;
    return DMG_AFTER;
}

int Armor::armor_points() const
{
    const int AP_MAX = data_->armor.armor_points;

    if (dur_ > 60) {return AP_MAX;}

    if (dur_ > 40) {return max(0, AP_MAX - 1);}

    if (dur_ > 25) {return max(0, AP_MAX - 2);}

    if (dur_ > 15) {return max(0, AP_MAX - 3);}

    return 0;
}

void Armor_asb_suit::on_equip(Actor& actor, const Verbosity verbosity)
{
    add_carrier_prop(new Prop_rFire(Prop_turns::indefinite),    actor, verbosity);
    add_carrier_prop(new Prop_rAcid(Prop_turns::indefinite),    actor, verbosity);
    add_carrier_prop(new Prop_rElec(Prop_turns::indefinite),    actor, verbosity);
    add_carrier_prop(new Prop_rBreath(Prop_turns::indefinite),  actor, verbosity);
}

Unequip_allowed Armor_asb_suit::on_unequip_hook(Actor& actor)
{
    clear_carrier_props(actor);

    return Unequip_allowed::yes;
}

void Armor_heavy_coat::on_equip(Actor& actor, const Verbosity verbosity)
{
    add_carrier_prop(new Prop_rCold(Prop_turns::indefinite), actor, verbosity);
}

Unequip_allowed Armor_heavy_coat::on_unequip_hook(Actor& actor)
{
    clear_carrier_props(actor);

    return Unequip_allowed::yes;
}

void Armor_mi_go::on_std_turn_in_inv(const Inv_type inv_type)
{
    (void)inv_type;

    if (dur_ < 100)
    {
        const int AP_BEFORE = armor_points();

        dur_ = 100;

        const int AP_AFTER  = armor_points();

        if (AP_AFTER > AP_BEFORE)
        {
            const string armor_name = name(Item_ref_type::plain, Item_ref_inf::none);
            msg_log::add("My " + armor_name + " reconstructs itself.", clr_msg_note, false,
                         More_prompt_on_msg::yes);
        }
    }
}

void Armor_mi_go::on_equip(Actor& actor, const Verbosity verbosity)
{
    (void)actor;

    if (verbosity == Verbosity::verbose)
    {
        render::draw_map_and_interface();
        msg_log::add("The armor joins with my skin!", clr_white, false, More_prompt_on_msg::yes);
        map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
    }
}

Unequip_allowed Armor_mi_go::on_unequip_hook(Actor& actor)
{
    (void)actor;

    render::draw_map_and_interface();
    msg_log::add("I attempt to tear off the armor, it rips my skin!", clr_msg_bad, false,
                 More_prompt_on_msg::yes);

    map::player->hit(rnd::range(1, 3), Dmg_type::pure);

    if (rnd::coin_toss())
    {
        //NOTE: There is no need to print a message here, a message is always printed when
        //taking off armor.
        return Unequip_allowed::yes;
    }
    else
    {
        msg_log::add("I fail to tear it off.", clr_white, false, More_prompt_on_msg::yes);
        return Unequip_allowed::no;
    }
}

//---------------------------------------------------------- WEAPON
Wpn::Wpn(Item_data_t* const item_data) :
    Item                (item_data),
    nr_ammo_loaded_     (0),
    ammo_data_          (nullptr)
{
    const auto ammo_item_id = data_->ranged.ammo_item_id;

    if (ammo_item_id != Item_id::END)
    {
        ammo_data_      = &item_data::data[int(ammo_item_id)];
        nr_ammo_loaded_ = data_->ranged.max_ammo;
    }
}

void Wpn::store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(melee_dmg_plus_));
    lines.push_back(to_str(nr_ammo_loaded_));
}

void Wpn::setup_from_save_lines(vector<string>& lines)
{
    melee_dmg_plus_ = to_int(lines.front());
    lines.erase(begin(lines));
    nr_ammo_loaded_ = to_int(lines.front());
    lines.erase(begin(lines));
}

Clr Wpn::clr() const
{
    if (data_->ranged.is_ranged_wpn && !data_->ranged.has_infinite_ammo)
    {
        if (nr_ammo_loaded_ == 0)
        {
            Clr ret = data_->clr;
            ret.r /= 2;
            ret.g /= 2;
            ret.b /= 2;
            return ret;
        }
    }

    return data_->clr;
}

void Wpn::set_random_melee_plus()
{
    melee_dmg_plus_ = 0;

    int chance = 45;

    while (rnd::percent(chance) && melee_dmg_plus_ < 3)
    {
        melee_dmg_plus_++;
        chance -= 5;
    }
}

string Wpn::name_inf() const
{
    if (data_->ranged.is_ranged_wpn && !data_->ranged.has_infinite_ammo)
    {
        return to_str(nr_ammo_loaded_) + "/" + to_str(data_->ranged.max_ammo);
    }

    return "";
}

//---------------------------------------------------------- STAFF OF THE PHARAOHS
Pharaoh_staff::Pharaoh_staff(Item_data_t* const item_data) : Wpn(item_data)
{
    item_data->allow_spawn = false;

    add_carrier_spell(new Spell_pharaoh_staff);
}

//---------------------------------------------------------- SAWED OFF SHOTGUN
Sawed_off::Sawed_off(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- PUMP SHOTGUN
Pump_shotgun::Pump_shotgun(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- FLARE GUN
Flare_gun::Flare_gun(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- PISTOL
Pistol::Pistol(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- MACHINE GUN
Machine_gun::Machine_gun(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- MI-GO ELECTRIC GUN
Mi_go_gun::Mi_go_gun(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- SPIKE GUN
Spike_gun::Spike_gun(Item_data_t* const item_data) :
    Wpn(item_data) {}

//---------------------------------------------------------- INCINERATOR
Incinerator::Incinerator(Item_data_t* const item_data) :
    Wpn(item_data) {}

void Incinerator::on_projectile_blocked(
    const Pos& pos, Actor* actor_hit)
{
    (void)actor_hit;
    explosion::run_explosion_at(pos, Expl_type::expl);
}

//---------------------------------------------------------- AMMO CLIP
Ammo_clip::Ammo_clip(Item_data_t* const item_data) : Ammo(item_data)
{
    set_full_ammo();
}

void Ammo_clip::set_full_ammo()
{
    ammo_ = data_->ranged.max_ammo;
}

//---------------------------------------------------------- MEDICAL BAG
void Medical_bag::on_pickup_to_backpack(Inventory& inv)
{
    //Check for existing medical bag in inventory
    for (Item* const other : inv.general_)
    {
        if (other != this && other->id() == id())
        {
            //Add my turns left to the other medical bag, then destroy self
            static_cast<Medical_bag*>(other)->nr_supplies_ += nr_supplies_;
            inv.remove_item_in_backpack_with_ptr(this, true);
            return;
        }
    }
}

Consume_item Medical_bag::activate(Actor* const actor)
{
    if (player_bon::bg() == Bg::ghoul)
    {
        msg_log::add("It is of no use to me.");
        cur_action_ = Med_bag_action::END;
        return Consume_item::no;
    }

    vector<Actor*> seen_foes;
    map::player->seen_foes(seen_foes);

    if (!seen_foes.empty())
    {
        msg_log::add("Not while an enemy is near.");
        cur_action_ = Med_bag_action::END;
        return Consume_item::no;
    }

    cur_action_ = choose_action();

    msg_log::clear();

    if (cur_action_ == Med_bag_action::END)
    {
        return Consume_item::no;
    }

    //Check if chosen action can be done
    switch (cur_action_)
    {
    case Med_bag_action::treat_wounds:
        if (map::player->hp() >= map::player->hp_max(true))
        {
            msg_log::add("I have no wounds to treat.");
            cur_action_ = Med_bag_action::END;
            return Consume_item::no;
        }

        break;

    case Med_bag_action::sanitize_infection:
        if (!actor->has_prop(Prop_id::infected))
        {
            msg_log::add("I have no infection to sanitize.");
            cur_action_ = Med_bag_action::END;
            return Consume_item::no;
        }

        break;

    case Med_bag_action::END: {}
        break;
    }

    bool is_enough_suppl = true;

    switch (cur_action_)
    {
    case Med_bag_action::sanitize_infection:
        is_enough_suppl = tot_suppl_for_sanitize() <= nr_supplies_;
        break;

    case Med_bag_action::treat_wounds: //Costs one supply per turn
    case Med_bag_action::END: {}
        break;
    }

    if (!is_enough_suppl)
    {
        msg_log::add("I do not have enough supplies for that.");
        cur_action_ = Med_bag_action::END;
        return Consume_item::no;
    }

    //Action can be done
    map::player->active_medical_bag = this;

    switch (cur_action_)
    {
    case Med_bag_action::treat_wounds:
        msg_log::add("I start treating my wounds...");
        nr_turns_until_heal_wounds_ = MEDICAL_BAG_NR_TRN_BEFORE_HEAL;
        break;

    case Med_bag_action::sanitize_infection:
        msg_log::add("I start to sanitize an infection...");
        nr_turns_left_sanitize_ = tot_turns_for_sanitize();
        break;

    case Med_bag_action::END: {} break;
    }

    game_time::tick();

    return Consume_item::no;
}

Med_bag_action Medical_bag::choose_action() const
{
    msg_log::clear();

    //Infections are treated first
    if (map::player->has_prop(Prop_id::infected))
    {
        return Med_bag_action::sanitize_infection;
    }

    return Med_bag_action::treat_wounds;


//  msg_log::add("Use Medical Bag how? [h/enter] Treat wounds [s] Sanitize infection",
//              clr_white_high);

//  render::draw_map_and_interface(true);
//
//  while (true)
//  {
//    const Key_data d = query::letter(true);
//    if (d.sdl_key == SDLK_ESCAPE || d.sdl_key == SDLK_SPACE)
//    {
//      return Med_bag_action::END;
//    }
//    else if (d.sdl_key == SDLK_RETURN || d.key == 'h')
//    {
//      return Med_bag_action::treat_wounds;
//    }
//    else if (d.key == 's')
//    {
//      return Med_bag_action::sanitize_infection;
//    }
//  }
//
//  return Med_bag_action(Med_bag_action::END);
}

void Medical_bag::continue_action()
{
    switch (cur_action_)
    {
    case Med_bag_action::treat_wounds:
    {

        auto& player = *map::player;

        const bool IS_HEALER = player_bon::traits[int(Trait::healer)];

        if (nr_turns_until_heal_wounds_ > 0)
        {
            nr_turns_until_heal_wounds_ -= IS_HEALER ? 2 : 1;
        }
        else
        {
            //If player is healer, double the rate of HP healing.
            const int NR_TRN_PER_HP_W_BON = IS_HEALER ? (MEDICAL_BAG_NR_TRN_PER_HP / 2) :
                                            MEDICAL_BAG_NR_TRN_PER_HP;

            if (game_time::turn() % NR_TRN_PER_HP_W_BON == 0)
            {
                player.restore_hp(1, false, Verbosity::silent);
            }

            //The rate of supply use is consistent (this means that with the healer
            // trait, you spend half the time and supplies, as per the description).
            if (game_time::turn() % MEDICAL_BAG_NR_TRN_PER_HP == 0)
            {
                --nr_supplies_;
            }
        }

        if (nr_supplies_ <= 0)
        {
            msg_log::add("No more medical supplies.");
            finish_cur_action();
            return;
        }

        if (player.hp() >= player.hp_max(true))
        {
            finish_cur_action();
            return;
        }

        game_time::tick();

    } break;

    case Med_bag_action::sanitize_infection:
    {
        --nr_turns_left_sanitize_;

        if (nr_turns_left_sanitize_ <= 0)
        {
            finish_cur_action();
        }
        else
        {
            game_time::tick();
        }
    } break;

    case Med_bag_action::END:
    {
        assert(false && "Illegal action");
    }
    break;
    }
}

void Medical_bag::finish_cur_action()
{
    map::player->active_medical_bag = nullptr;

    switch (cur_action_)
    {
    case Med_bag_action::sanitize_infection:
    {
        map::player->prop_handler().end_prop(Prop_id::infected);
        nr_supplies_ -= tot_suppl_for_sanitize();
    } break;

    case Med_bag_action::treat_wounds:
    {
        msg_log::add("I finish treating my wounds.");
    } break;

    case Med_bag_action::END: {} break;
    }

    cur_action_ = Med_bag_action::END;

    if (nr_supplies_ <= 0)
    {
        map::player->inv().remove_item_in_backpack_with_ptr(this, true);
    }
}

void Medical_bag::interrupted()
{
    msg_log::add("My healing is disrupted.", clr_white, false);

    nr_turns_until_heal_wounds_ = -1;
    nr_turns_left_sanitize_    = -1;

    map::player->active_medical_bag = nullptr;
}

int Medical_bag::tot_turns_for_sanitize() const
{
    return player_bon::traits[int(Trait::healer)] ? 10 : 20;
}

int Medical_bag::tot_suppl_for_sanitize() const
{
    return player_bon::traits[int(Trait::healer)] ? 3 : 6;
}

//---------------------------------------------------------- HIDEOUS MASK
//Hideous_mask::Hideous_mask(Item_data_t* item_data) : Headwear(item_data)
//{
//    item_data->allow_spawn = false;
//}
//
//void Hideous_mask::on_std_turn_in_inv(const Inv_type inv_type)
//{
//    if (inv_type == Inv_type::slots)
//    {
//        vector<Actor*> adj_actors;
//        const Pos p(map::player->pos);
//        for (auto* const actor : game_time::actors_)
//        {
//            if (actor->is_alive() && utils::is_pos_adj(p, actor->pos, false))
//            {
//                adj_actors.push_back(actor);
//            }
//        }
//        if (!adj_actors.empty())
//        {
//            bool blocked_los[MAP_W][MAP_H];
//            map_parse::run(cell_check::Blocks_los(), blocked_los);
//            for (auto* const actor : adj_actors)
//            {
//                if (rnd::one_in(4) && actor->can_see_actor(*map::player, blocked_los))
//                {
//                    actor->prop_handler().try_add_prop(
//                        new Prop_terrified(Prop_turns::std));
//                }
//            }
//        }
//    }
//}

//---------------------------------------------------------- GAS MASK
void Gas_mask::on_equip(Actor& actor, const Verbosity verbosity)
{
    add_carrier_prop(new Prop_rBreath(Prop_turns::indefinite), actor, verbosity);
}

Unequip_allowed Gas_mask::on_unequip(Actor& actor)
{
    clear_carrier_props(actor);

    return Unequip_allowed::yes;
}

void Gas_mask::decr_turns_left(Inventory& carrier_inv)
{
    --nr_turns_left_;

    if (nr_turns_left_ <= 0)
    {
        msg_log::add("My " + name(Item_ref_type::plain, Item_ref_inf::none) + " expires.",
                     clr_msg_note, true, More_prompt_on_msg::yes);
        carrier_inv.decr_item(this);
    }
}

//---------------------------------------------------------- EXPLOSIVE
Consume_item Explosive::activate(Actor* const actor)
{
    (void)actor;
    //Make a copy to use as the held ignited explosive.
    auto* cpy = static_cast<Explosive*>(item_factory::mk(data().id, 1));

    cpy->fuse_turns_               = std_fuse_turns();
    map::player->active_explosive  = cpy;
    map::player->update_clr();
    cpy->on_player_ignite();
    return Consume_item::yes;
}

//---------------------------------------------------------- DYNAMITE
void Dynamite::on_player_ignite() const
{
    const bool IS_SWIFT   = player_bon::traits[int(Trait::dem_expert)] && rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    msg_log::add("I " + swift_str + "light a dynamite stick.");
    render::draw_map_and_interface();
    game_time::tick(IS_SWIFT);
}

void Dynamite::on_std_turn_player_hold_ignited()
{
    fuse_turns_--;

    if (fuse_turns_ > 0)
    {
        string fuse_msg = "***F";

        for (int i = 0; i < fuse_turns_; ++i) {fuse_msg += "Z";}

        fuse_msg += "***";
        msg_log::add(fuse_msg, clr_yellow);
    }
    else
    {
        msg_log::add("The dynamite explodes in my hand!");
        map::player->active_explosive = nullptr;
        explosion::run_explosion_at(map::player->pos, Expl_type::expl);
        map::player->update_clr();
        fuse_turns_ = -1;
        delete this;
    }
}

void Dynamite::on_thrown_ignited_landing(const Pos& p)
{
    game_time::add_mob(new Lit_dynamite(p, fuse_turns_));
}

void Dynamite::on_player_paralyzed()
{
    msg_log::add("The lit Dynamite stick falls from my hand!");
    map::player->active_explosive = nullptr;
    map::player->update_clr();
    const Pos& p = map::player->pos;
    auto* const f = map::cells[p.x][p.y].rigid;

    if (!f->is_bottomless()) {game_time::add_mob(new Lit_dynamite(p, fuse_turns_));}

    delete this;
}

//---------------------------------------------------------- MOLOTOV
void Molotov::on_player_ignite() const
{
    const bool IS_SWIFT   = player_bon::traits[int(Trait::dem_expert)] &&
                            rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    msg_log::add("I " + swift_str + "light a Molotov Cocktail.");
    render::draw_map_and_interface();
    game_time::tick(IS_SWIFT);
}

void Molotov::on_std_turn_player_hold_ignited()
{
    fuse_turns_--;

    if (fuse_turns_ <= 0)
    {
        msg_log::add("The Molotov Cocktail explodes in my hand!");
        map::player->active_explosive = nullptr;
        map::player->update_clr();

        const Pos player_pos = map::player->pos;

        Snd snd("I hear an explosion!", Sfx_id::explosion_molotov, Ignore_msg_if_origin_seen::yes,
                player_pos, nullptr, Snd_vol::high, Alerts_mon::yes);

        snd_emit::emit_snd(snd);

        explosion::run_explosion_at(player_pos, Expl_type::apply_prop, Expl_src::misc,
                                    Emit_expl_snd::no, 0, new Prop_burning(Prop_turns::std));

        delete this;
    }
}

void Molotov::on_thrown_ignited_landing(const Pos& p)
{
    const int D = player_bon::traits[int(Trait::dem_expert)] ? 1 : 0;

    Snd snd("I hear an explosion!", Sfx_id::explosion_molotov, Ignore_msg_if_origin_seen::yes,
            p, nullptr, Snd_vol::high, Alerts_mon::yes);

    snd_emit::emit_snd(snd);

    explosion::run_explosion_at(p, Expl_type::apply_prop, Expl_src::player_use_moltv_intended,
                                Emit_expl_snd::no, D , new Prop_burning(Prop_turns::std));
}


void Molotov::on_player_paralyzed()
{
    msg_log::add("The lit Molotov Cocktail falls from my hand!");
    map::player->active_explosive = nullptr;
    map::player->update_clr();

    const Pos player_pos = map::player->pos;

    Snd snd("I hear an explosion!", Sfx_id::explosion_molotov, Ignore_msg_if_origin_seen::yes,
            player_pos, nullptr, Snd_vol::high, Alerts_mon::yes);

    snd_emit::emit_snd(snd);

    explosion::run_explosion_at(player_pos, Expl_type::apply_prop, Expl_src::misc,
                                Emit_expl_snd::no, 0, new Prop_burning(Prop_turns::std));

    delete this;
}

//---------------------------------------------------------- FLARE
void Flare::on_player_ignite() const
{
    const bool IS_SWIFT   = player_bon::traits[int(Trait::dem_expert)] &&
                            rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    msg_log::add("I " + swift_str + "light a Flare.");
    game_time::update_light_map();
    map::player->update_fov();
    render::draw_map_and_interface();
    game_time::tick(IS_SWIFT);
}

void Flare::on_std_turn_player_hold_ignited()
{
    fuse_turns_--;

    if (fuse_turns_ <= 0)
    {
        msg_log::add("The flare is extinguished.");
        map::player->active_explosive = nullptr;
        map::player->update_clr();
        delete this;
    }
}

void Flare::on_thrown_ignited_landing(const Pos& p)
{
    game_time::add_mob(new Lit_flare(p, fuse_turns_));
    game_time::update_light_map();
    map::player->update_fov();
    render::draw_map_and_interface();
}

void Flare::on_player_paralyzed()
{
    msg_log::add("The lit Flare falls from my hand!");
    map::player->active_explosive = nullptr;
    map::player->update_clr();
    const Pos&  p = map::player->pos;
    auto* const f = map::cells[p.x][p.y].rigid;

    if (!f->is_bottomless()) {game_time::add_mob(new Lit_flare(p, fuse_turns_));}

    game_time::update_light_map();
    map::player->update_fov();
    render::draw_map_and_interface();
    delete this;
}

//---------------------------------------------------------- SMOKE GRENADE
void Smoke_grenade::on_player_ignite() const
{
    const bool IS_SWIFT   = player_bon::traits[int(Trait::dem_expert)] &&
                            rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    msg_log::add("I " + swift_str + "ignite a smoke grenade.");
    render::draw_map_and_interface();
    game_time::tick(IS_SWIFT);
}

void Smoke_grenade::on_std_turn_player_hold_ignited()
{
    if (fuse_turns_ < std_fuse_turns() && rnd::coin_toss())
    {
        explosion::run_smoke_explosion_at(map::player->pos);
    }

    fuse_turns_--;

    if (fuse_turns_ <= 0)
    {
        msg_log::add("The smoke grenade is extinguished.");
        map::player->active_explosive = nullptr;
        map::player->update_clr();
        delete this;
    }
}

void Smoke_grenade::on_thrown_ignited_landing(const Pos& p)
{
    explosion::run_smoke_explosion_at(p);
    map::player->update_fov();
    render::draw_map_and_interface();
}

void Smoke_grenade::on_player_paralyzed()
{
    msg_log::add("The ignited smoke grenade falls from my hand!");
    map::player->active_explosive = nullptr;
    map::player->update_clr();
    const Pos&  p = map::player->pos;
    auto* const f = map::cells[p.x][p.y].rigid;

    if (!f->is_bottomless()) {explosion::run_smoke_explosion_at(map::player->pos);}

    map::player->update_fov();
    render::draw_map_and_interface();
    delete this;
}

Clr Smoke_grenade::ignited_projectile_clr() const
{
    return data().clr;
}

