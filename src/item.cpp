#include "init.hpp"

#include "item.hpp"

#include <assert.h>

#include "map.hpp"
#include "game_time.hpp"
#include "utils.hpp"
#include "map_parsing.hpp"
#include "properties.hpp"
#include "log.hpp"
#include "explosion.hpp"
#include "render.hpp"
#include "input.hpp"
#include "query.hpp"
#include "item_factory.hpp"
#include "feature_mob.hpp"
#include "feature_rigid.hpp"
#include "item_data.hpp"

using namespace std;

//--------------------------------------------------------- ITEM
Item::Item(Item_data_t* item_data) :
    nr_items_      (1),
    melee_dmg_plus_ (0),
    data_         (item_data) {}

Item::~Item()
{
    for (auto prop   : carrier_props_)   {delete prop;}
    for (auto spell  : carrier_spells_)  {delete spell;}
}

Item_id            Item::get_id()     const {return data_->id;}
const Item_data_t&  Item::get_data()   const {return *data_;}
Clr               Item::get_clr()    const {return data_->clr;}
char              Item::get_glyph()  const {return data_->glyph;}
Tile_id            Item::get_tile()   const {return data_->tile;}

vector<string> Item::get_descr() const
{
    return data_->base_descr;
}

int Item::get_weight() const
{
    return int(data_->weight) * nr_items_;
}

string Item::get_weight_str() const
{
    const int WEIGHT = get_weight();
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
    Log::add_msg("I cannot apply that.");
    return Consume_item::no;
}

string Item::get_name(const Item_ref_type      ref_type,
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
        case Main_att_mode::melee:  att_inf_used = Item_ref_att_inf::melee;  break;
        case Main_att_mode::ranged: att_inf_used = Item_ref_att_inf::ranged; break;
        case Main_att_mode::thrown: att_inf_used = Item_ref_att_inf::thrown; break;
        case Main_att_mode::none:   att_inf_used = Item_ref_att_inf::none;   break;
        }
    }

    const auto ability_vals = Map::player->get_data().ability_vals;

    if (att_inf_used == Item_ref_att_inf::melee)
    {
        const string    rolls_str    = to_str(data_->melee.dmg.first);
        const string    sides_str    = to_str(data_->melee.dmg.second);
        const int       PLUS        = melee_dmg_plus_;
        const string    plus_str     = PLUS == 0 ? "" :
                                      PLUS  > 0 ?
                                      ("+" + to_str(PLUS)) :
                                      ("-" + to_str(PLUS));
        const int       ITEM_SKILL  = data_->melee.hit_chance_mod;
        const int       MELEE_SKILL = ability_vals.get_val(Ability_id::melee, true,
                                      *(Map::player));
        const int       SKILL_TOT   = max(0, min(100, ITEM_SKILL + MELEE_SKILL));
        const string    skill_str    = to_str(SKILL_TOT) + "%";

        att_str = " " + rolls_str + "d" + sides_str + plus_str + " " + skill_str;
    }

    const int RANGED_SKILL = ability_vals.get_val(Ability_id::ranged, true, *(Map::player));

    if (att_inf_used == Item_ref_att_inf::ranged)
    {
        string dmg_str = data_->ranged.dmg_info_override;

        if (dmg_str.empty())
        {
            const int       MULTIPL     = data_->ranged.is_machine_gun ?
                                          NR_MG_PROJECTILES : 1;
            const string    rolls_str    = to_str(data_->ranged.dmg.rolls * MULTIPL);
            const string    sides_str    = to_str(data_->ranged.dmg.sides);
            const int       PLUS        = data_->ranged.dmg.plus * MULTIPL;
            const string    plus_str     = PLUS ==  0 ? "" :
                                          PLUS  > 0  ?
                                          ("+" + to_str(PLUS)) :
                                          ("-" + to_str(PLUS));
            dmg_str                      = rolls_str + "d" + sides_str + plus_str;
        }
        const int       ITEM_SKILL  = data_->ranged.hit_chance_mod;
        const int       SKILL_TOT   = max(0, min(100, ITEM_SKILL + RANGED_SKILL));
        const string    skill_str    = to_str(SKILL_TOT) + "%";

        att_str = " " + dmg_str + " " + skill_str;
    }

    if (att_inf_used == Item_ref_att_inf::thrown)
    {
        const string    rolls_str    = to_str(data_->ranged.throw_dmg.rolls);
        const string    sides_str    = to_str(data_->ranged.throw_dmg.sides);
        const int       PLUS        = data_->ranged.throw_dmg.plus;
        const string    plus_str     = PLUS ==  0 ? "" :
                                      PLUS  > 0 ? "+" :
                                      ("-" + to_str(PLUS));
        const int       ITEM_SKILL  = data_->ranged.throw_hit_chance_mod;
        const int       SKILL_TOT   = max(0, min(100, ITEM_SKILL + RANGED_SKILL));
        const string    skill_str    = to_str(SKILL_TOT) + "%";

        att_str = " " + rolls_str + "d" + sides_str + plus_str + " " + skill_str;
    }

    string inf_str = "";

    if (inf == Item_ref_inf::yes)
    {
        inf_str = get_name_inf();
        if (!inf_str.empty()) {inf_str.insert(0, " ");}
    }

    const auto& names_used = data_->is_identified ? data_->base_name : data_->base_name_un_id;

    return nr_str + names_used.names[int(ref_type_used)] + att_str + inf_str;
}

bool Item::is_in_effective_range_lmt(const Pos& p0, const Pos& p1) const
{
    return Utils::king_dist(p0, p1) <= data_->ranged.effective_range;
}

//--------------------------------------------------------- ARMOR
Armor::Armor(Item_data_t* const item_data) :
    Item  (item_data),
    dur_  (Rnd::range(80, 100)) {}

void Armor::store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(dur_));
}

void Armor::setup_from_save_lines(vector<string>& lines)
{
    dur_ = to_int(lines.front());
    lines.erase(begin(lines));
}

void Armor::on_equip(const bool IS_SILENT)
{
    on_equip_(IS_SILENT);
}

Unequip_allowed Armor::on_unequip()
{
    Render::draw_map_and_interface();

    const Unequip_allowed unequip_allowed = on_unequip_();

    if (unequip_allowed == Unequip_allowed::yes)
    {
        const string name = get_name(Item_ref_type::plain, Item_ref_inf::none);
        Log::add_msg("I take off my " + name + ".", clr_white, false, true);
    }

    Game_time::tick();

    return unequip_allowed;
}

string Armor::get_armor_data_line(const bool WITH_BRACKETS) const
{
    const int       AP      = get_armor_points();
    const string    ap_str   = to_str(max(0, AP));

    return WITH_BRACKETS ? ("[" + ap_str + "]") : ap_str;
}

int Armor::take_dur_hit_and_get_reduced_dmg(const int DMG_BEFORE)
{
    TRACE_FUNC_BEGIN;

    //AP, Armor points:
    //  - Damage soaked up instead of hitting the player
    //DFF, Damage (to) Durability Factor:
    //  - A factor of how much damage the armor durability takes per attack damage point

    const int     AP_BEFORE       = get_armor_points();
    const double  DDF_BASE        = data_->armor.dmg_to_durability_factor;
    //TODO: Add check for if wearer is player!
    const double  DDF_WAR_VET_MOD = Player_bon::get_bg() == Bg::war_vet ? 0.5 : 1.0;
    const double  DDF_K           = 1.5;
    const double  DMG_BEFORE_DB   = double(DMG_BEFORE);

    dur_ -= int(DMG_BEFORE_DB * DDF_BASE * DDF_WAR_VET_MOD * DDF_K);

    dur_                          = max(0, dur_);
    const int AP_AFTER            = get_armor_points();

    if (AP_AFTER < AP_BEFORE && AP_AFTER != 0)
    {
        const string armor_name = get_name(Item_ref_type::plain);
        Log::add_msg("My " + armor_name + " is damaged!", clr_msg_note);
    }

    TRACE << "Damage before: " + to_str(DMG_BEFORE) << endl;

    const int DMG_AFTER = max(1, DMG_BEFORE - AP_BEFORE);

    TRACE << "Damage after: " + to_str(DMG_AFTER) << endl;

    TRACE_FUNC_END;
    return DMG_AFTER;
}

int Armor::get_armor_points() const
{
    const int AP_MAX = data_->armor.armor_points;

    if (dur_ > 60) {return AP_MAX;}
    if (dur_ > 40) {return max(0, AP_MAX - 1);}
    if (dur_ > 25) {return max(0, AP_MAX - 2);}
    if (dur_ > 15) {return max(0, AP_MAX - 3);}

    return 0;
}

void Armor_asb_suit::on_equip_(const bool IS_SILENT)
{
    (void)IS_SILENT;

    carrier_props_.push_back(new Prop_rFire   (Prop_turns::indefinite));
    carrier_props_.push_back(new Prop_rAcid   (Prop_turns::indefinite));
    carrier_props_.push_back(new Prop_rElec   (Prop_turns::indefinite));
    carrier_props_.push_back(new Prop_rBreath (Prop_turns::indefinite));
}

Unequip_allowed Armor_asb_suit::on_unequip_()
{
    for (Prop* prop : carrier_props_) {delete prop;}
    carrier_props_.clear();

    return Unequip_allowed::yes;
}

void Armor_heavy_coat::on_equip_(const bool IS_SILENT)
{
    (void)IS_SILENT;

    carrier_props_.push_back(new Prop_rCold(Prop_turns::indefinite));
}

Unequip_allowed Armor_heavy_coat::on_unequip_()
{
    for (Prop* prop : carrier_props_) {delete prop;}
    carrier_props_.clear();

    return Unequip_allowed::yes;
}

void Armor_mi_go::on_std_turn_in_inv(const Inv_type inv_type)
{
    (void)inv_type;

    if (dur_ < 100)
    {
        const int AP_BEFORE = get_armor_points();

        dur_ = 100;

        const int AP_AFTER  = get_armor_points();

        if (AP_AFTER > AP_BEFORE)
        {
            const string name = get_name(Item_ref_type::plain, Item_ref_inf::none);
            Log::add_msg("My " + name + " reconstructs itself.", clr_msg_note, false, true);
        }
    }
}

void Armor_mi_go::on_equip_(const bool IS_SILENT)
{
    if (!IS_SILENT)
    {
        Render::draw_map_and_interface();
        Log::add_msg("The armor joins with my skin!", clr_white, false, true);
        Map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
    }
}

Unequip_allowed Armor_mi_go::on_unequip_()
{
    Render::draw_map_and_interface();
    Log::add_msg("I attempt to tear off the armor, it rips my skin!", clr_msg_bad, false,
                true);

    Map::player->hit(Rnd::range(1, 3), Dmg_type::pure);

    if (Rnd::coin_toss())
    {
        //NOTE: There is no need to print a message here, a message is always printed when
        //taking off armor.
        return Unequip_allowed::yes;
    }
    else
    {
        Log::add_msg("I fail to tear it off.", clr_white, false, true);
        return Unequip_allowed::no;
    }
}

//--------------------------------------------------------- WEAPON
void Wpn::store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(melee_dmg_plus_));
    lines.push_back(to_str(nr_ammo_loaded));
}

void Wpn::setup_from_save_lines(vector<string>& lines)
{
    melee_dmg_plus_ = to_int(lines.front());
    lines.erase(begin(lines));
    nr_ammo_loaded = to_int(lines.front());
    lines.erase(begin(lines));
}

Wpn::Wpn(Item_data_t* const item_data, Item_data_t* const ammo_data, int ammo_cap,
         bool is_using_clip) :
    Item                (item_data),
    AMMO_CAP            (ammo_cap),
    IS_USING_CLIP       (is_using_clip),
    nr_ammo_loaded        (AMMO_CAP),
    ammo_data_           (ammo_data) {}

Clr Wpn::get_clr() const
{
    if (data_->ranged.is_ranged_wpn && !data_->ranged.has_infinite_ammo)
    {
        if (nr_ammo_loaded == 0)
        {
            Clr ret = data_->clr;
            ret.r /= 2; ret.g /= 2; ret.b /= 2;
            return ret;
        }
    }
    return data_->clr;
}

void Wpn::set_random_melee_plus()
{
    melee_dmg_plus_ = 0;

    int chance = 45;
    while (Rnd::percent() < chance && melee_dmg_plus_ < 3)
    {
        melee_dmg_plus_++;
        chance -= 5;
    }
}

string Wpn::get_name_inf() const
{
    if (data_->ranged.is_ranged_wpn && !data_->ranged.has_infinite_ammo)
    {
        return to_str(nr_ammo_loaded) + "/" + to_str(AMMO_CAP);
    }
    return "";
}

//--------------------------------------------------------- STAFF OF THE PHARAOHS
Pharaoh_staff::Pharaoh_staff(Item_data_t* const item_data) : Wpn(item_data, nullptr)
{
    item_data->allow_spawn = false;

    carrier_spells_.push_back(new Spell_pharaoh_staff);
}

//--------------------------------------------------------- MACHINE GUN
Machine_gun::Machine_gun(Item_data_t* const item_data, Item_data_t* const ammo_data) :
    Wpn(item_data, ammo_data, ammo_data->ranged.max_nr_ammo_in_clip, true) {}

//--------------------------------------------------------- MI-GO ELECTRIC GUN
Mi_go_gun::Mi_go_gun(Item_data_t* const item_data, Item_data_t* const ammo_data) :
    Wpn(item_data, ammo_data, ammo_data->ranged.max_nr_ammo_in_clip, true) {}

//--------------------------------------------------------- SPIKE GUN
Spike_gun::Spike_gun(Item_data_t* const item_data, Item_data_t* const ammo_data) :
    Wpn(item_data, ammo_data, 12, false) {}

//--------------------------------------------------------- INCINERATOR
Incinerator::Incinerator(Item_data_t* const item_data, Item_data_t* const ammo_data) :
    Wpn(item_data, ammo_data, ammo_data->ranged.max_nr_ammo_in_clip, true) {}

void Incinerator::on_projectile_blocked(
    const Pos& pos, Actor* actor_hit)
{
    (void)actor_hit;
    Explosion::run_explosion_at(pos, Expl_type::expl);
}

//--------------------------------------------------------- AMMO CLIP
Ammo_clip::Ammo_clip(Item_data_t* const item_data) : Ammo(item_data)
{
    set_full_ammo();
}

void Ammo_clip::set_full_ammo()
{
    ammo_ = data_->ranged.max_nr_ammo_in_clip;
}

//--------------------------------------------------------- MEDICAL BAG
const int NR_TRN_BEFORE_HEAL  = 10;
const int NR_TRN_PER_HP       = 2;

void Medical_bag::on_pickup_to_backpack(Inventory& inv)
{
    //Check for existing medical bag in inventory
    for (Item* const other : inv.general_)
    {
        if (other != this && other->get_id() == get_id())
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
    (void)actor;

    vector<Actor*> seen_foes;
    Map::player->get_seen_foes(seen_foes);
    if (!seen_foes.empty())
    {
        Log::add_msg("Not while an enemy is near.");
        cur_action_ = Med_bag_action::END;
        return Consume_item::no;
    }

    cur_action_ = choose_action();

    Log::clear_log();

    if (cur_action_ == Med_bag_action::END)
    {
        return Consume_item::no;
    }

    //Check if chosen action can be done
    bool props[size_t(Prop_id::END)];
    Map::player->get_prop_handler().get_prop_ids(props);
    switch (cur_action_)
    {
    case Med_bag_action::treat_wounds:
        if (Map::player->get_hp() >= Map::player->get_hp_max(true))
        {
            Log::add_msg("I have no wounds to treat.");
            cur_action_ = Med_bag_action::END;
            return Consume_item::no;
        }
        break;

    case Med_bag_action::sanitize_infection:
        if (!props[int(Prop_id::infected)])
        {
            Log::add_msg("I have no infection to sanitize.");
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
        is_enough_suppl = get_tot_suppl_for_sanitize() <= nr_supplies_;
        break;

    case Med_bag_action::treat_wounds: //Costs one supply per turn
    case Med_bag_action::END: {}
        break;
    }

    if (!is_enough_suppl)
    {
        Log::add_msg("I do not have enough supplies for that.");
        cur_action_ = Med_bag_action::END;
        return Consume_item::no;
    }

    //Action can be done
    Map::player->active_medical_bag = this;

    switch (cur_action_)
    {
    case Med_bag_action::treat_wounds:
        Log::add_msg("I start treating my wounds...");
        nr_turns_until_heal_wounds_ = NR_TRN_BEFORE_HEAL;
        break;

    case Med_bag_action::sanitize_infection:
        Log::add_msg("I start to sanitize an infection...");
        nr_turns_left_sanitize_ = get_tot_turns_for_sanitize();
        break;

    case Med_bag_action::END: {} break;
    }

    Game_time::tick();

    return Consume_item::no;
}

Med_bag_action Medical_bag::choose_action() const
{
    Log::clear_log();

    bool props[size_t(Prop_id::END)];
    Map::player->get_prop_handler().get_prop_ids(props);

    //Infections are treated first
    if (props[int(Prop_id::infected)])
    {
        return Med_bag_action::sanitize_infection;
    }

    return Med_bag_action::treat_wounds;


//  Log::add_msg("Use Medical Bag how? [h/enter] Treat wounds [s] Sanitize infection",
//              clr_white_high);

//  Render::draw_map_and_interface(true);
//
//  while (true)
//  {
//    const Key_data d = Query::letter(true);
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

        auto& player = *Map::player;

        const bool IS_HEALER = Player_bon::traits[int(Trait::healer)];

        if (nr_turns_until_heal_wounds_ > 0)
        {
            nr_turns_until_heal_wounds_ -= IS_HEALER ? 2 : 1;
        }
        else
        {
            //If player is healer, double the rate of HP healing.
            const int NR_TRN_PER_HP_W_BON =
                IS_HEALER ? (NR_TRN_PER_HP / 2) : NR_TRN_PER_HP;

            if (Game_time::get_turn() % NR_TRN_PER_HP_W_BON == 0)
            {
                player.restore_hp(1, false);
            }

            //The rate of supply use is consistent (this means that with the healer
            // trait, you spend half the time and supplies, as per the description).
            if (Game_time::get_turn() % NR_TRN_PER_HP == 0)
            {
                --nr_supplies_;
            }
        }

        if (nr_supplies_ <= 0)
        {
            Log::add_msg("No more medical supplies.");
            finish_cur_action();
            return;
        }

        if (player.get_hp() >= player.get_hp_max(true))
        {
            finish_cur_action();
            return;
        }

        Game_time::tick();

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
            Game_time::tick();
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
    Map::player->active_medical_bag = nullptr;

    switch (cur_action_)
    {
    case Med_bag_action::sanitize_infection:
    {
        Map::player->get_prop_handler().end_applied_prop(Prop_id::infected);
        nr_supplies_ -= get_tot_suppl_for_sanitize();
    } break;

    case Med_bag_action::treat_wounds:
    {
        Log::add_msg("I finish treating my wounds.");
    } break;

    case Med_bag_action::END: {} break;
    }

    cur_action_ = Med_bag_action::END;

    if (nr_supplies_ <= 0)
    {
        Map::player->get_inv().remove_item_in_backpack_with_ptr(this, true);
    }
}

void Medical_bag::interrupted()
{
    Log::add_msg("My healing is disrupted.", clr_white, false);

    nr_turns_until_heal_wounds_ = -1;
    nr_turns_left_sanitize_    = -1;

    Map::player->active_medical_bag = nullptr;
}

int Medical_bag::get_tot_turns_for_sanitize() const
{
    return Player_bon::traits[int(Trait::healer)] ? 10 : 20;
}

int Medical_bag::get_tot_suppl_for_sanitize() const
{
    return Player_bon::traits[int(Trait::healer)] ? 3 : 6;
}

//--------------------------------------------------------- HIDEOUS MASK
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
//        const Pos p(Map::player->pos);
//        for (auto* const actor : Game_time::actors_)
//        {
//            if (actor->is_alive() && Utils::is_pos_adj(p, actor->pos, false))
//            {
//                adj_actors.push_back(actor);
//            }
//        }
//        if (!adj_actors.empty())
//        {
//            bool blocked_los[MAP_W][MAP_H];
//            Map_parse::run(Cell_check::Blocks_los(), blocked_los);
//            for (auto* const actor : adj_actors)
//            {
//                if (Rnd::one_in(4) && actor->can_see_actor(*Map::player, blocked_los))
//                {
//                    actor->get_prop_handler().try_apply_prop(
//                        new Prop_terrified(Prop_turns::std));
//                }
//            }
//        }
//    }
//}

//--------------------------------------------------------- GAS MASK
void Gas_mask::on_equip(const bool IS_SILENT)
{
    (void)IS_SILENT;

    carrier_props_.push_back(new Prop_rBreath(Prop_turns::indefinite));
}

Unequip_allowed Gas_mask::on_unequip()
{
    for (Prop* prop : carrier_props_) {delete prop;}
    carrier_props_.clear();

    return Unequip_allowed::yes;
}

void Gas_mask::decr_turns_left(Inventory& carrier_inv)
{
    --nr_turns_left_;

    if (nr_turns_left_ <= 0)
    {
        Log::add_msg("My " + get_name(Item_ref_type::plain, Item_ref_inf::none) + " expires.",
                    clr_msg_note, true, true);
        carrier_inv.decr_item(this);
    }
}

//--------------------------------------------------------- EXPLOSIVE
Consume_item Explosive::activate(Actor* const actor)
{
    (void)actor;
    //Make a copy to use as the held ignited explosive.
    auto* cpy = static_cast<Explosive*>(Item_factory::mk(get_data().id, 1));

    cpy->fuse_turns_               = get_std_fuse_turns();
    Map::player->active_explosive  = cpy;
    Map::player->update_clr();
    cpy->on_player_ignite();
    return Consume_item::yes;
}

//--------------------------------------------------------- DYNAMITE
void Dynamite::on_player_ignite() const
{
    const bool IS_SWIFT   = Player_bon::traits[int(Trait::dem_expert)] && Rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    Log::add_msg("I " + swift_str + "light a dynamite stick.");
    Render::draw_map_and_interface();
    Game_time::tick(IS_SWIFT);
}

void Dynamite::on_std_turn_player_hold_ignited()
{
    fuse_turns_--;
    if (fuse_turns_ > 0)
    {
        string fuse_msg = "***F";
        for (int i = 0; i < fuse_turns_; ++i) {fuse_msg += "Z";}
        fuse_msg += "***";
        Log::add_msg(fuse_msg, clr_yellow);
    }
    else
    {
        Log::add_msg("The dynamite explodes in my hand!");
        Map::player->active_explosive = nullptr;
        Explosion::run_explosion_at(Map::player->pos, Expl_type::expl);
        Map::player->update_clr();
        fuse_turns_ = -1;
        delete this;
    }
}

void Dynamite::on_thrown_ignited_landing(const Pos& p)
{
    Game_time::add_mob(new Lit_dynamite(p, fuse_turns_));
}

void Dynamite::on_player_paralyzed()
{
    Log::add_msg("The lit Dynamite stick falls from my hand!");
    Map::player->active_explosive = nullptr;
    Map::player->update_clr();
    const Pos& p = Map::player->pos;
    auto* const f = Map::cells[p.x][p.y].rigid;
    if (!f->is_bottomless()) {Game_time::add_mob(new Lit_dynamite(p, fuse_turns_));}
    delete this;
}

//--------------------------------------------------------- MOLOTOV
void Molotov::on_player_ignite() const
{
    const bool IS_SWIFT   = Player_bon::traits[int(Trait::dem_expert)] &&
                            Rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    Log::add_msg("I " + swift_str + "light a Molotov Cocktail.");
    Render::draw_map_and_interface();
    Game_time::tick(IS_SWIFT);
}

void Molotov::on_std_turn_player_hold_ignited()
{
    fuse_turns_--;

    if (fuse_turns_ <= 0)
    {
        Log::add_msg("The Molotov Cocktail explodes in my hand!");
        Map::player->active_explosive = nullptr;
        Map::player->update_clr();
        Explosion::run_explosion_at(Map::player->pos, Expl_type::apply_prop, Expl_src::misc, 0,
                                  Sfx_id::explosion_molotov, new Prop_burning(Prop_turns::std));
        delete this;
    }
}

void Molotov::on_thrown_ignited_landing(const Pos& p)
{
    const int D = Player_bon::traits[int(Trait::dem_expert)] ? 1 : 0;
    Explosion::run_explosion_at(p, Expl_type::apply_prop, Expl_src::player_use_moltv_intended, D,
                              Sfx_id::explosion_molotov, new Prop_burning(Prop_turns::std));
}


void Molotov::on_player_paralyzed()
{
    Log::add_msg("The lit Molotov Cocktail falls from my hand!");
    Map::player->active_explosive = nullptr;
    Map::player->update_clr();
    Explosion::run_explosion_at(Map::player->pos, Expl_type::apply_prop, Expl_src::misc, 0,
                              Sfx_id::explosion_molotov, new Prop_burning(Prop_turns::std));
    delete this;
}

//--------------------------------------------------------- FLARE
void Flare::on_player_ignite() const
{
    const bool IS_SWIFT   = Player_bon::traits[int(Trait::dem_expert)] &&
                            Rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    Log::add_msg("I " + swift_str + "light a Flare.");
    Game_time::update_light_map();
    Map::player->update_fov();
    Render::draw_map_and_interface();
    Game_time::tick(IS_SWIFT);
}

void Flare::on_std_turn_player_hold_ignited()
{
    fuse_turns_--;
    if (fuse_turns_ <= 0)
    {
        Log::add_msg("The flare is extinguished.");
        Map::player->active_explosive = nullptr;
        Map::player->update_clr();
        delete this;
    }
}

void Flare::on_thrown_ignited_landing(const Pos& p)
{
    Game_time::add_mob(new Lit_flare(p, fuse_turns_));
    Game_time::update_light_map();
    Map::player->update_fov();
    Render::draw_map_and_interface();
}

void Flare::on_player_paralyzed()
{
    Log::add_msg("The lit Flare falls from my hand!");
    Map::player->active_explosive = nullptr;
    Map::player->update_clr();
    const Pos&  p = Map::player->pos;
    auto* const f = Map::cells[p.x][p.y].rigid;
    if (!f->is_bottomless()) {Game_time::add_mob(new Lit_flare(p, fuse_turns_));}
    Game_time::update_light_map();
    Map::player->update_fov();
    Render::draw_map_and_interface();
    delete this;
}

//--------------------------------------------------------- SMOKE GRENADE
void Smoke_grenade::on_player_ignite() const
{
    const bool IS_SWIFT   = Player_bon::traits[int(Trait::dem_expert)] &&
                            Rnd::coin_toss();
    const string swift_str = IS_SWIFT ? "swiftly " : "";

    Log::add_msg("I " + swift_str + "ignite a smoke grenade.");
    Render::draw_map_and_interface();
    Game_time::tick(IS_SWIFT);
}

void Smoke_grenade::on_std_turn_player_hold_ignited()
{
    if (fuse_turns_ < get_std_fuse_turns() && Rnd::coin_toss())
    {
        Explosion::run_smoke_explosion_at(Map::player->pos);
    }
    fuse_turns_--;
    if (fuse_turns_ <= 0)
    {
        Log::add_msg("The smoke grenade is extinguished.");
        Map::player->active_explosive = nullptr;
        Map::player->update_clr();
        delete this;
    }
}

void Smoke_grenade::on_thrown_ignited_landing(const Pos& p)
{
    Explosion::run_smoke_explosion_at(p);
    Map::player->update_fov();
    Render::draw_map_and_interface();
}

void Smoke_grenade::on_player_paralyzed()
{
    Log::add_msg("The ignited smoke grenade falls from my hand!");
    Map::player->active_explosive = nullptr;
    Map::player->update_clr();
    const Pos&  p = Map::player->pos;
    auto* const f = Map::cells[p.x][p.y].rigid;
    if (!f->is_bottomless()) {Explosion::run_smoke_explosion_at(Map::player->pos);}
    Map::player->update_fov();
    Render::draw_map_and_interface();
    delete this;
}

Clr Smoke_grenade::get_ignited_projectile_clr() const
{
    return get_data().clr;
}

