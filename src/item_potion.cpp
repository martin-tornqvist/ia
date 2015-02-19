#include "Item_potion.h"

#include <climits>

#include "Init.h"
#include "Properties.h"
#include "Actor_player.h"
#include "Log.h"
#include "Map.h"
#include "Actor_mon.h"
#include "Player_spells_handling.h"
#include "Item_scroll.h"
#include "Game_time.h"
#include "Audio.h"
#include "Render.h"
#include "Inventory.h"
#include "Map_travel.h"
#include "Map_parsing.h"
#include "Utils.h"
#include "Feature_rigid.h"
#include "Item_factory.h"

using namespace std;

Consume_item Potion::activate(Actor* const actor)
{
    assert(actor);

    if (actor->get_prop_handler().allow_eat(true))
    {
        quaff(*actor);
        return Consume_item::yes;

    }
    return Consume_item::no;
}

void Potion::identify(const bool IS_SILENT_IDENTIFY)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (!IS_SILENT_IDENTIFY)
        {
            const string name = get_name(Item_ref_type::a, Item_ref_inf::none);
            Log::add_msg("It was " + name + ".");
            Map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
        }
    }
}

vector<string> Potion::get_descr() const
{
    if (data_->is_identified)
    {
        return get_descr_identified();
    }
    else
    {
        return data_->base_descr;
    }
}

void Potion::collide(const Pos& pos, Actor* const actor)
{
    if (!Map::cells[pos.x][pos.y].rigid->is_bottomless() || actor)
    {

        const bool PLAYER_SEE_CELL = Map::cells[pos.x][pos.y].is_seen_by_player;

        if (PLAYER_SEE_CELL)
        {
            //TODO: Use standard animation
            Render::draw_glyph('*', Panel::map, pos, data_->clr);

            if (actor)
            {
                if (actor->is_alive())
                {
                    Log::add_msg("The potion shatters on " + actor->get_name_the() + ".");
                }
            }
            else
            {
                Feature* const f = Map::cells[pos.x][pos.y].rigid;
                Log::add_msg("The potion shatters on " + f->get_name(Article::the) + ".");
            }
        }
        //If the blow from the bottle didn't kill the actor, apply what's inside
        if (actor)
        {
            if (actor->is_alive())
            {
                collide_(pos, actor);
                if (actor->is_alive() && !data_->is_identified && PLAYER_SEE_CELL)
                {
                    Log::add_msg("It had no apparent effect...");
                }
            }
        }
    }
}

void Potion::quaff(Actor& actor)
{
    if (actor.is_player())
    {
        data_->is_tried = true;

        Audio::play(Sfx_id::potion_quaff);

        if (data_->is_identified)
        {
            const string name = get_name(Item_ref_type::a, Item_ref_inf::none);
            Log::add_msg("I drink " + name + "...");
        }
        else //Unidentified
        {
            const string name = get_name(Item_ref_type::plain, Item_ref_inf::none);
            Log::add_msg("I drink an unknown " + name + "...");
        }
        Map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
    }

    quaff_(actor);

    if (Map::player->is_alive())
    {
        Game_time::tick();
    }
}

string Potion::get_name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

void Potion_vitality::quaff_(Actor& actor)
{
    actor.get_prop_handler().end_applied_props_by_magic_healing();

    //HP is always restored at least up to maximum HP, but can go beyond
    const int HP          = actor.get_hp();
    const int HP_MAX      = actor.get_hp_max(true);
    const int HP_RESTORED = max(20, HP_MAX - HP);

    actor.restore_hp(HP_RESTORED, true, true);

    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_vitality::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_spirit::quaff_(Actor& actor)
{

    //SPI is always restored at least up to maximum SPI, but can go beyond
    const int SPI           = actor.get_spi();
    const int SPI_MAX       = actor.get_spi_max();
    const int SPI_RESTORED  = max(10, SPI_MAX - SPI);

    actor.restore_spi(SPI_RESTORED, true, true);

    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_spirit::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_blindness::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_blind(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_blindness::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_paral::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_paralyzed(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_paral::collide_(const Pos& pos, Actor* const actor)
{

    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_disease::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_diseased(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_conf::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_confused(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_conf::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_frenzy::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_frenzied(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_frenzy::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_fortitude::quaff_(Actor& actor)
{
    Prop_handler& prop_handler = actor.get_prop_handler();

    Prop_rFear*      const r_fear   = new Prop_rFear(Prop_turns::std);
    Prop_rConfusion* const r_conf   = new Prop_rConfusion(
        Prop_turns::specific, r_fear->turns_left_);
    Prop_rSleep*     const r_sleep  = new Prop_rSleep(
        Prop_turns::specific, r_fear->turns_left_);

    prop_handler.try_apply_prop(r_fear);
    prop_handler.try_apply_prop(r_conf);
    prop_handler.try_apply_prop(r_sleep);

    if (actor.is_player())
    {
        bool is_phobias_cured = false;
        for (int i = 0; i < int(Phobia::END); ++i)
        {
            if (Map::player->phobias[i])
            {
                Map::player->phobias[i] = false;
                is_phobias_cured = true;
            }
        }

        if (is_phobias_cured)
        {
            Log::add_msg("All my phobias are cured!");
        }

        bool is_obsessions_cured = false;
        for (int i = 0; i < int(Obsession::END); ++i)
        {
            if (Map::player->obsessions[i])
            {
                Map::player->obsessions[i] = false;
                is_obsessions_cured = true;
            }
        }
        if (is_obsessions_cured)
        {
            Log::add_msg("All my obsessions are cured!");
        }

        Map::player->restore_shock(999, false);
        Log::add_msg("I feel more at ease.");
    }

    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_fortitude::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_poison::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_poisoned(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_poison::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_rFire::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_rFire(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_rFire::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_antidote::quaff_(Actor& actor)
{
    const bool WAS_POISONED = actor.get_prop_handler().end_applied_prop(Prop_id::poisoned);

    if (WAS_POISONED && Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_antidote::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_rElec::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_rElec(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_rElec::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_rAcid::quaff_(Actor& actor)
{
    actor.get_prop_handler().try_apply_prop(new Prop_rAcid(Prop_turns::std));
    if (Map::player->can_see_actor(actor, nullptr))
    {
        identify(false);
    }
}

void Potion_rAcid::collide_(const Pos& pos, Actor* const actor)
{
    (void)pos;
    if (actor) {quaff_(*actor);}
}

void Potion_insight::quaff_(Actor& actor)
{
    (void)actor;

    auto& inv = Map::player->get_inv();

    vector<Item*> identify_bucket;

    auto& slots = inv.slots_;
    for (Inv_slot& slot : slots)
    {
        Item* const item = slot.item;
        if (item)
        {
            const Item_data_t& d = item->get_data();
            if (!d.is_identified) {identify_bucket.push_back(item);}
        }
    }
    vector<Item*>& general = inv.general_;
    for (Item* item : general)
    {
        if (item->get_id() != Item_id::potion_insight)
        {
            const Item_data_t& d = item->get_data();
            if (!d.is_identified) {identify_bucket.push_back(item);}
        }
    }

    const size_t NR_ELEMENTS = identify_bucket.size();
    if (NR_ELEMENTS > 0)
    {
        const int     IDX             = Rnd::range(0, NR_ELEMENTS - 1);
        Item* const   item            = identify_bucket[IDX];
        const string  item_name_before  = item->get_name(Item_ref_type::a, Item_ref_inf::none);

        Log::add_msg("I gain intuitions about " + item_name_before + "...", clr_white, false,
                    true);

        item->identify(true);

        const string item_name_after = item->get_name(Item_ref_type::a, Item_ref_inf::none);

        Render::draw_map_and_interface(true);

        if (item_name_before != item_name_after)
        {
            Log::add_msg("It is identified as " + item_name_after + "!");
        }
        else //Item name is same as before
        {
            //Typically, items that change names when identified have a "nonsense" name
            //first (e.g. "A Green Potion"), that change into something more descriptive
            //(e.g. "A Potion of Fire Resistance"). In those cases, the message can be
            //something like "Aha, [old name] is a [new name]!". But when the name is the
            //same, the message needs to be different, (e.g. "An Iron Ring"). Then we
            //print a message like "Aha, I understand it now".

            //TODO: This is hacky and fragile, but it works for now.

            Log::add_msg("All its properties are known to me.");
        }
    }

    identify(false);
}

void Potion_clairv::quaff_(Actor& actor)
{
    if (actor.is_player())
    {
        Log::add_msg("I see far and wide!");

        vector<Pos> anim_cells;
        anim_cells.clear();

        bool blocked[MAP_W][MAP_H];
        Map_parse::run(Cell_check::Blocks_los(), blocked);
        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                Cell& cell = Map::cells[x][y];
                if (!blocked[x][y] && !cell.is_dark)
                {
                    cell.is_explored = true;
                    cell.is_seen_by_player = true;
                    anim_cells.push_back(Pos(x, y));
                }
            }
        }

        Render::draw_map_and_interface(false);
//    Map::update_visual_memory();
        Map::player->update_fov();

        Render::draw_blast_at_cells(anim_cells, clr_white);
    }
    identify(false);
}

void Potion_descent::quaff_(Actor& actor)
{
    (void)actor;
    if (Map::dlvl < DLVL_LAST - 1)
    {
        Render::draw_map_and_interface();
        Log::add_msg("I sink downwards!", clr_white, false, true);
        Map_travel::go_to_nxt();
    }
    else
    {
        Log::add_msg("I feel a faint sinking sensation.");
    }

    identify(false);
}

namespace Potion_handling
{

namespace
{

vector<Potion_look> potion_looks_;

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    //Init possible potion colors and fake names
    potion_looks_.clear();
    potion_looks_.push_back(Potion_look {"Golden",   "a Golden",   clr_yellow});
    potion_looks_.push_back(Potion_look {"Yellow",   "a Yellow",   clr_yellow});
    potion_looks_.push_back(Potion_look {"Dark",     "a Dark",     clr_gray});
    potion_looks_.push_back(Potion_look {"Black",    "a Black",    clr_gray});
    potion_looks_.push_back(Potion_look {"Oily",     "an Oily",    clr_gray});
    potion_looks_.push_back(Potion_look {"Smoky",    "a Smoky",    clr_white});
    potion_looks_.push_back(Potion_look {"Slimy",    "a Slimy",    clr_green});
    potion_looks_.push_back(Potion_look {"Green",    "a Green",    clr_green_lgt});
    potion_looks_.push_back(Potion_look {"Fiery",    "a Fiery",    clr_red_lgt});
    potion_looks_.push_back(Potion_look {"Murky",    "a Murky",    clr_brown_drk});
    potion_looks_.push_back(Potion_look {"Muddy",    "a Muddy",    clr_brown});
    potion_looks_.push_back(Potion_look {"Violet",   "a Violet",   clr_violet});
    potion_looks_.push_back(Potion_look {"Orange",   "an Orange",  clr_orange});
    potion_looks_.push_back(Potion_look {"Watery",   "a Watery",   clr_blue_lgt});
    potion_looks_.push_back(Potion_look {"Metallic", "a Metallic", clr_gray});
    potion_looks_.push_back(Potion_look {"Clear",    "a Clear",    clr_white_high});
    potion_looks_.push_back(Potion_look {"Misty",    "a Misty",    clr_white_high});
    potion_looks_.push_back(Potion_look {"Bloody",   "a Bloody",   clr_red});
    potion_looks_.push_back(Potion_look {"Magenta",  "a Magenta",  clr_magenta});
    potion_looks_.push_back(Potion_look {"Clotted",  "a Clotted",  clr_green});
    potion_looks_.push_back(Potion_look {"Moldy",    "a Moldy",    clr_brown});
    potion_looks_.push_back(Potion_look {"Frothy",   "a Frothy",   clr_white});

    TRACE << "Init potion names" << endl;
    for (auto* const d : Item_data::data)
    {
        if (d->type == Item_type::potion)
        {
            //Color and false name
            const int ELEMENT = Rnd::range(0, potion_looks_.size() - 1);

            Potion_look& look = potion_looks_[ELEMENT];

            d->base_name_un_id.names[int(Item_ref_type::plain)]   = look.name_plain + " potion";
            d->base_name_un_id.names[int(Item_ref_type::plural)]  = look.name_plain + " potions";
            d->base_name_un_id.names[int(Item_ref_type::a)]       = look.name_a     + " potion";
            d->clr = look.clr;

            potion_looks_.erase(potion_looks_.begin() + ELEMENT);

            //True name
            const Potion* const potion =
                static_cast<const Potion*>(Item_factory::mk(d->id, 1));

            const string REAL_TYPE_NAME = potion->get_real_name();

            delete potion;

            const string REAL_NAME        = "Potion of "    + REAL_TYPE_NAME;
            const string REAL_NAME_PLURAL = "Potions of "   + REAL_TYPE_NAME;
            const string REAL_NAME_A      = "a potion of "  + REAL_TYPE_NAME;

            d->base_name.names[int(Item_ref_type::plain)]  = REAL_NAME;
            d->base_name.names[int(Item_ref_type::plural)] = REAL_NAME_PLURAL;
            d->base_name.names[int(Item_ref_type::a)]      = REAL_NAME_A;
        }
    }

    TRACE_FUNC_END;
}

void store_to_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t* const d = Item_data::data[i];
        if (d->type == Item_type::potion)
        {
            lines.push_back(d->base_name_un_id.names[int(Item_ref_type::plain)]);
            lines.push_back(d->base_name_un_id.names[int(Item_ref_type::plural)]);
            lines.push_back(d->base_name_un_id.names[int(Item_ref_type::a)]);
            lines.push_back(to_str(d->clr.r));
            lines.push_back(to_str(d->clr.g));
            lines.push_back(to_str(d->clr.b));
        }
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t* const d = Item_data::data[i];
        if (d->type == Item_type::potion)
        {
            d->base_name_un_id.names[int(Item_ref_type::plain)]  = lines.front();
            lines.erase(begin(lines));
            d->base_name_un_id.names[int(Item_ref_type::plural)] = lines.front();
            lines.erase(begin(lines));
            d->base_name_un_id.names[int(Item_ref_type::a)]      = lines.front();
            lines.erase(begin(lines));
            d->clr.r = to_int(lines.front());
            lines.erase(begin(lines));
            d->clr.g = to_int(lines.front());
            lines.erase(begin(lines));
            d->clr.b = to_int(lines.front());
            lines.erase(begin(lines));
        }
    }
}

} //Potion_handling
