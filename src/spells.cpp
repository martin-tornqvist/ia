#include "Spells.h"

#include <algorithm>
#include <vector>
#include <assert.h>

#include "Init.h"
#include "Render.h"
#include "Actor_mon.h"
#include "Actor_player.h"
#include "Log.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Map.h"
#include "Actor_factory.h"
#include "Feature_trap.h"
#include "Player_spells_handling.h"
#include "Item_scroll.h"
#include "Inventory.h"
#include "Map_parsing.h"
#include "Line_calc.h"
#include "Sdl_wrapper.h"
#include "Player_bon.h"
#include "Utils.h"
#include "Dungeon_master.h"

using namespace std;

namespace
{

const int SUMMON_HOSTILE_ONE_IN_N = 7;

} //namespace

namespace Spell_handling
{

Spell* get_random_spell_for_mon()
{
    vector<Spell_id> bucket;
    for (int i = 0; i < int(Spell_id::END); ++i)
    {
        Spell* const spell = mk_spell_from_id(Spell_id(i));
        if (spell->is_avail_for_all_mon()) {bucket.push_back(Spell_id(i));}
        delete spell;
    }
    const int ELEMENT = Rnd::range(0, bucket.size() - 1);
    return mk_spell_from_id(bucket[ELEMENT]);
}

Spell* mk_spell_from_id(const Spell_id spell_id)
{
    switch (spell_id)
    {
    case Spell_id::slow_mon:              return new Spell_slow_mon;
    case Spell_id::terrify_mon:           return new Spell_terrify_mon;
    case Spell_id::paralyze_mon:          return new Spell_paralyze_mon;
    case Spell_id::disease:              return new Spell_disease;
    case Spell_id::darkbolt:             return new Spell_darkbolt;
    case Spell_id::aza_wrath:             return new Spell_aza_wrath;
    case Spell_id::summon:               return new Spell_summon_mon;
    case Spell_id::heal_self:             return new Spell_heal_self;
    case Spell_id::knock_back:            return new Spell_knock_back;
    case Spell_id::teleport:             return new Spell_teleport;
    case Spell_id::mayhem:               return new Spell_mayhem;
    case Spell_id::pest:                 return new Spell_pest;
    case Spell_id::det_items:             return new Spell_det_items;
    case Spell_id::det_traps:             return new Spell_det_traps;
    case Spell_id::det_mon:               return new Spell_det_mon;
    case Spell_id::opening:              return new Spell_opening;
    case Spell_id::sacr_life:             return new Spell_sacr_life;
    case Spell_id::sacr_spi:              return new Spell_sacr_spi;
    case Spell_id::cloud_minds:           return new Spell_cloud_minds;
    case Spell_id::bless:                return new Spell_bless;
    case Spell_id::mi_go_hypno:            return new Spell_mi_go_hypno;
    case Spell_id::burn:                 return new Spell_burn;
    case Spell_id::elem_res:              return new Spell_elem_res;
    case Spell_id::pharaoh_staff:         return new Spell_pharaoh_staff;
    case Spell_id::light:                return new Spell_light;
    case Spell_id::END: {}
        break;
    }
    assert(false && "No spell found for ID");
    return nullptr;
}

} //Spell_handling

Range Spell::get_spi_cost(const bool IS_BASE_COST_ONLY, Actor* const caster) const
{
    int cost_max = get_max_spi_cost_();

    if (caster == Map::player && !IS_BASE_COST_ONLY)
    {
        const int X0 = max(0, caster->pos.x - 1);
        const int Y0 = max(0, caster->pos.y - 1);
        const int X1 = min(MAP_W - 1, caster->pos.x + 1);
        const int Y1 = min(MAP_H - 1, caster->pos.y + 1);

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                if (Map::cells[x][y].rigid->get_id() == Feature_id::altar)
                {
                    cost_max -= 1;
                }
            }
        }

        bool IS_WARLOCK     = Player_bon::traits[int(Trait::warlock)];
        bool IS_BLOOD_SORC  = Player_bon::traits[int(Trait::blood_sorcerer)];
        bool IS_SEER        = Player_bon::traits[int(Trait::seer)];
        bool IS_SUMMONER    = Player_bon::traits[int(Trait::summoner)];

        if (IS_BLOOD_SORC) {cost_max--;}

        switch (get_id())
        {
        case Spell_id::darkbolt:       if (IS_WARLOCK)  --cost_max;     break;
        case Spell_id::aza_wrath:       if (IS_WARLOCK)  --cost_max;     break;
        case Spell_id::mayhem:         if (IS_WARLOCK)  --cost_max;     break;
        case Spell_id::det_mon:         if (IS_SEER)     --cost_max;     break;
        case Spell_id::det_items:       if (IS_SEER)     cost_max -= 3;  break;
        case Spell_id::det_traps:       if (IS_SEER)     cost_max -= 3;  break;
        case Spell_id::summon:      if (IS_SUMMONER) --cost_max;     break;
        case Spell_id::pest:           if (IS_SUMMONER) --cost_max;     break;
        case Spell_id::pharaoh_staff:   if (IS_SUMMONER) --cost_max;     break;
        default: {} break;
        }

        Prop_handler& prop_hlr = caster->get_prop_handler();

        bool props[size_t(Prop_id::END)];
        prop_hlr.get_prop_ids(props);

        if (!prop_hlr.allow_see()) {--cost_max;}
        if (props[int(Prop_id::blessed)])  {--cost_max;}
        if (props[int(Prop_id::cursed)])   {cost_max += 3;}
    }

    cost_max             = max(1, cost_max);
    const int COST_MIN  = max(1, cost_max / 2);

    return Range(COST_MIN, cost_max);
}

Spell_effect_noticed Spell::cast(Actor* const caster, const bool IS_INTRINSIC) const
{
    TRACE_FUNC_BEGIN;
    if (caster->get_prop_handler().allow_cast_spell(true))
    {
        if (caster->is_player())
        {
            TRACE << "Player casting spell" << endl;
            const Shock_src shock_src = IS_INTRINSIC ?
                                      Shock_src::cast_intr_spell :
                                      Shock_src::use_strange_item;
            const int SHOCK_VALUE = IS_INTRINSIC ? get_shock_lvl_intr_cast() : 10;
            Map::player->incr_shock(SHOCK_VALUE, shock_src);
            if (Map::player->is_alive())
            {
                Audio::play(Sfx_id::spell_generic);
            }
        }
        else //Caster is not player
        {
            TRACE << "Monster casting spell" << endl;
            Mon* const mon = static_cast<Mon*>(caster);
            if (Map::cells[mon->pos.x][mon->pos.y].is_seen_by_player)
            {
                const string spell_str = mon->get_data().spell_cast_msg;
                if (!spell_str.empty())
                {
                    Log::add_msg(spell_str);
                }
            }
            mon->spell_cool_down_cur_ = mon->get_data().spell_cooldown_turns;
        }

        if (IS_INTRINSIC)
        {
            const Range cost = get_spi_cost(false, caster);
            caster->hit_spi(Rnd::range(cost), false);
        }

        Spell_effect_noticed is_noticed = Spell_effect_noticed::no;

        if (caster->is_alive())
        {
            is_noticed = cast_(caster);
        }

        Game_time::tick();
        TRACE_FUNC_END;
        return is_noticed;
    }
    TRACE_FUNC_END;
    return Spell_effect_noticed::no;
}

//------------------------------------------------------------ DARKBOLT
Spell_effect_noticed Spell_darkbolt::cast_(Actor* const caster) const
{
    Actor* tgt = nullptr;

    vector<Actor*> seen_actors;
    caster->get_seen_foes(seen_actors);

    if (seen_actors.empty())
    {
        return Spell_effect_noticed::no;
    }

    tgt = Utils::get_random_closest_actor(caster->pos, seen_actors);

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    tgt->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_msg, clr_white, false, true);
        return cast_(tgt);
    }

    vector<Pos> line;

    Line_calc::calc_new_line(caster->pos, tgt->pos, true, 999, false, line);

    Render::draw_map_and_interface();

    const size_t LINE_SIZE = line.size();

    for (size_t i = 1; i < LINE_SIZE; ++i)
    {
        const Pos& p = line[i];
        if (Config::is_tiles_mode())
        {
            Render::draw_tile(Tile_id::blast1, Panel::map, p, clr_magenta);
        }
        else
        {
            Render::draw_glyph('*', Panel::map, p, clr_magenta);
        }
        Render::update_screen();
        Sdl_wrapper::sleep(Config::get_delay_projectile_draw());
    }

    Render::draw_blast_at_cells(vector<Pos> {tgt->pos}, clr_magenta);

    bool    is_warlock_charged  = false;
    Clr     msg_clr            = clr_msg_good;
    string  tgt_str            = "I am";

    if (tgt->is_player())
    {
        msg_clr = clr_msg_bad;
    }
    else //Target is monster
    {
        tgt_str = tgt->get_name_the() + " is";

        if (Map::player->is_leader_of(tgt)) {msg_clr = clr_white;}
    }

    if (caster->is_player())
    {
        bool props[size_t(Prop_id::END)];
        caster->get_prop_handler().get_prop_ids(props);
        is_warlock_charged = props[int(Prop_id::warlock_charged)];
    }

    if (Map::player->can_see_actor(*tgt, nullptr))
    {
        Log::add_msg(tgt_str + " struck by a blast!", msg_clr);
    }

    tgt->get_prop_handler().try_apply_prop(new Prop_paralyzed(Prop_turns::specific, 2));

    Range dmg_range(4, 10);
    const int DMG = is_warlock_charged ? dmg_range.upper : Rnd::range(dmg_range);

    tgt->hit(DMG, Dmg_type::physical);

    Snd_emit::emit_snd({"", Sfx_id::END, Ignore_msg_if_origin_seen::yes, tgt->pos, nullptr,
                      Snd_vol::low, Alerts_mon::yes
                     });

    return Spell_effect_noticed::yes;
}

bool Spell_darkbolt::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::one_in(3);
}

//------------------------------------------------------------ AZATHOTHS WRATH
Spell_effect_noticed Spell_aza_wrath::cast_(Actor* const caster) const
{
    Range dmg_range(4, 8);
    bool  is_warlock_charged = false;

    vector<Actor*>  tgts;
    caster->get_seen_foes(tgts);

    if (tgts.empty())
    {
        return Spell_effect_noticed::no;
    }

    //This point reached means targets are available
    if (caster->is_player())
    {
        bool props[size_t(Prop_id::END)];
        caster->get_prop_handler().get_prop_ids(props);

        is_warlock_charged = props[int(Prop_id::warlock_charged)];
    }

    Render::draw_blast_at_seen_actors(tgts, clr_red_lgt);

    for (Actor* const tgt : tgts)
    {
        //Spell reflection?
        bool tgt_props[size_t(Prop_id::END)];

        tgt->get_prop_handler().get_prop_ids(tgt_props);

        if (tgt_props[size_t(Prop_id::spell_reflect)])
        {
            Log::add_msg(spell_reflect_msg, clr_white, false, true);
            cast_(tgt);
            continue;
        }

        string  tgt_str  = "I am";
        Clr     msg_clr  = clr_msg_good;
        if (tgt->is_player())
        {
            msg_clr = clr_msg_bad;
        }
        else //Target is monster
        {
            tgt_str = tgt->get_name_the();
            if (Map::player->is_leader_of(tgt)) {msg_clr = clr_white;}
        }

        if (Map::player->can_see_actor(*tgt, nullptr))
        {
            Log::add_msg(tgt_str + " is struck by a roaring blast!", msg_clr);
        }

        tgt->get_prop_handler().try_apply_prop(new Prop_paralyzed(Prop_turns::specific, 2));

        const int DMG = is_warlock_charged ? dmg_range.upper : Rnd::range(dmg_range);

        tgt->hit(DMG, Dmg_type::physical);

        Snd_emit::emit_snd({"", Sfx_id::END, Ignore_msg_if_origin_seen::yes, tgt->pos, nullptr,
                          Snd_vol::high, Alerts_mon::yes
                         });
    }

    return Spell_effect_noticed::yes;
}

bool Spell_aza_wrath::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::one_in(3);
}

//------------------------------------------------------------ MAYHEM
Spell_effect_noticed Spell_mayhem::cast_(Actor* const caster) const
{
    const bool IS_PLAYER = caster->is_player();

    if ( Map::player->can_see_actor(*caster, nullptr))
    {
        string caster_name = IS_PLAYER ? "me" : caster->get_name_the();
        Log::add_msg("Destruction rages around " + caster_name + "!");
    }

    const Pos& caster_pos = caster->pos;

    const int NR_SWEEPS = 5;
    const int RADI      = FOV_STD_RADI_INT;

    const int X0 = max(1, caster_pos.x - RADI);
    const int Y0 = max(1, caster_pos.y - RADI);
    const int X1 = min(MAP_W - 1, caster_pos.x + RADI) - 1;
    const int Y1 = min(MAP_H - 1, caster_pos.y + RADI) - 1;

    for (int i = 0; i < NR_SWEEPS; ++i)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            for (int x = X0; x <= X1; ++x)
            {
                bool is_adj_to_walkable_cell = false;
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        const Rigid* const f = Map::cells[x + dx][y + dy].rigid;
                        if (f->can_move_cmn()) {is_adj_to_walkable_cell = true;}
                    }
                }
                if (is_adj_to_walkable_cell && Rnd::one_in(8))
                {
                    Map::cells[x][y].rigid->hit(Dmg_type::physical, Dmg_method::explosion);
                }
            }
        }
    }

    for (int y = Y0; y <= Y1; ++y)
    {
        for (int x = X0; x <= X1; ++x)
        {
            auto* const f = Map::cells[x][y].rigid;

            if (f->can_have_blood() && Rnd::one_in(10))
            {
                f->mk_bloody();
            }

            if (Rnd::one_in(4))
            {
                f->hit(Dmg_type::fire, Dmg_method::elemental, nullptr);
            }
        }
    }

    vector<Actor*> seen_foes;
    caster->get_seen_foes(seen_foes);

    for (auto* tgt : seen_foes)
    {
        //Spell reflection?
        bool tgt_props[size_t(Prop_id::END)];

        tgt->get_prop_handler().get_prop_ids(tgt_props);

        if (tgt_props[size_t(Prop_id::spell_reflect)])
        {
            Log::add_msg(spell_reflect_msg, clr_white, false, true);
            cast_(tgt);
            continue;
        }

        tgt->get_prop_handler().try_apply_prop(new Prop_burning(Prop_turns::std));
    }

    Snd_emit::emit_snd({"", Sfx_id::END, Ignore_msg_if_origin_seen::yes, caster_pos, nullptr,
                      Snd_vol::high, Alerts_mon::yes
                     });

    return Spell_effect_noticed::yes;
}

bool Spell_mayhem::allow_mon_cast_now(Mon& mon) const
{
    return mon.aware_counter_ > 0  &&
           Rnd::coin_toss()        &&
           (mon.tgt_ || Rnd::one_in(20));
}

//------------------------------------------------------------ PESTILENCE
Spell_effect_noticed Spell_pest::cast_(Actor* const caster) const
{
    const int RND = Rnd::range(1, 3);
    const Actor_id monster_id = RND == 1 ? Actor_id::green_spider :
                              RND == 2 ? Actor_id::red_spider   :
                              Actor_id::rat;

    const size_t NR_MON = Rnd::range(7, 10);

    Actor*  leader                  = nullptr;
    bool    did_player_summon_hostile  = false;

    if (caster->is_player())
    {
        const int N             = SUMMON_HOSTILE_ONE_IN_N *
                                  (Player_bon::traits[int(Trait::summoner)] ? 2 : 1);
        did_player_summon_hostile  = Rnd::one_in(N);
        leader                  = did_player_summon_hostile ? nullptr : caster;
    }
    else //Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;
        leader                    = caster_leader ? caster_leader : caster;
    }

    vector<Mon*> mon_summoned;

    Actor_factory::summon(caster->pos, {NR_MON, monster_id}, true, leader, &mon_summoned);

    bool is_any_seen_by_player = false;

    const int NR_TURNS_SUMMONED = Rnd::range(40, 70);

    for (Mon* const mon : mon_summoned)
    {
        mon->nr_turns_until_unsummoned_ = NR_TURNS_SUMMONED;
        if (Map::player->can_see_actor(*mon, nullptr))
        {
            is_any_seen_by_player = true;
        }
    }

    if (is_any_seen_by_player)
    {
        string caster_str = "me";

        if (!caster->is_player())
        {
            caster_str = caster->get_name_the();
        }

        Log::add_msg("Disgusting critters appear around " + caster_str + "!");

        if (did_player_summon_hostile)
        {
            Log::add_msg("They are hostile!", clr_msg_note, true, true);
        }

        return Spell_effect_noticed::yes;
    }

    return Spell_effect_noticed::no;
}

bool Spell_pest::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_         &&
           Rnd::coin_toss()  &&
           (mon.tgt_ || Rnd::one_in(20));
}

//------------------------------------------------------------ PHARAOH STAFF
Spell_effect_noticed Spell_pharaoh_staff::cast_(Actor* const caster) const
{
    //First check for a friendly mummy and heal it (as per the spell description)
    for (Actor* const actor : Game_time::actors_)
    {
        const auto id = actor->get_data().id;

        const bool IS_ACTOR_ID_OK = id == Actor_id::mummy ||
                                    id == Actor_id::croc_head_mummy;

        if (IS_ACTOR_ID_OK && caster->is_leader_of(actor))
        {
            actor->restore_hp(999);
            return Spell_effect_noticed::yes;
        }
    }

    //This point reached means no mummy controlled, summon a new one
    Actor*  leader                  = nullptr;
    bool    did_player_summon_hostile  = false;

    if (caster->is_player())
    {
        const int N             = SUMMON_HOSTILE_ONE_IN_N *
                                  (Player_bon::traits[int(Trait::summoner)] ? 2 : 1);
        did_player_summon_hostile  = Rnd::one_in(N);
        leader                  = did_player_summon_hostile ? nullptr : caster;
    }
    else //Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;
        leader                    = caster_leader ? caster_leader : caster;
    }

    vector<Mon*> summoned_mon;

    const auto id = Rnd::coin_toss() ? Actor_id::mummy : Actor_id::croc_head_mummy;

    Actor_factory::summon(caster->pos, {id}, false, leader, &summoned_mon);

    const Mon* const mon = summoned_mon[0];

    if (Map::player->can_see_actor(*mon, nullptr))
    {
        Log::add_msg(mon->get_name_a() + " appears!");

        if (did_player_summon_hostile)
        {
            Log::add_msg("It is hostile!", clr_msg_note, true, true);
        }

        return Spell_effect_noticed::yes;
    }

    return Spell_effect_noticed::no;
}

bool Spell_pharaoh_staff::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::one_in(4);
}

//------------------------------------------------------------ DETECT ITEMS
Spell_effect_noticed Spell_det_items::cast_(Actor* const caster) const
{
    (void)caster;

    const int RADI    = FOV_STD_RADI_INT + 3;
    const int ORIG_X  = Map::player->pos.x;
    const int ORIG_Y  = Map::player->pos.y;
    const int X0      = max(0, ORIG_X - RADI);
    const int Y0      = max(0, ORIG_Y - RADI);
    const int X1      = min(MAP_W - 1, ORIG_X + RADI);
    const int Y1      = min(MAP_H - 1, ORIG_Y + RADI);

    vector<Pos> items_revealed_cells;

    for (int y = Y0; y < Y1; ++y)
    {
        for (int x = X0; x <= X1; ++x)
        {
            Item* item = Map::cells[x][y].item;
            if (item)
            {
                Map::cells[x][y].is_seen_by_player = true;
                Map::cells[x][y].is_explored     = true;
                items_revealed_cells.push_back(Pos(x, y));
            }
        }
    }
    if (!items_revealed_cells.empty())
    {
        Render::draw_map_and_interface();
        Map::player->update_fov();
        Render::draw_blast_at_cells(items_revealed_cells, clr_white);
        Render::draw_map_and_interface();

        if (items_revealed_cells.size() == 1)
        {
            Log::add_msg("An item is revealed to me.");
        }
        if (items_revealed_cells.size() > 1)
        {
            Log::add_msg("Some items are revealed to me.");
        }
        return Spell_effect_noticed::yes;
    }
    return Spell_effect_noticed::no;
}

//------------------------------------------------------------ DETECT TRAPS
Spell_effect_noticed Spell_det_traps::cast_(Actor* const caster) const
{
    (void)caster;

    vector<Pos> traps_revealed_cells;

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (Map::cells[x][y].is_seen_by_player)
            {
                auto* const f = Map::cells[x][y].rigid;
                if (f->get_id() == Feature_id::trap)
                {
                    auto* const trap = static_cast<Trap*>(f);
                    trap->reveal(false);
                    traps_revealed_cells.push_back(Pos(x, y));
                }
            }
        }
    }

    if (!traps_revealed_cells.empty())
    {
        Render::draw_map_and_interface();
        Map::player->update_fov();
        Render::draw_blast_at_cells(traps_revealed_cells, clr_white);
        Render::draw_map_and_interface();
        if (traps_revealed_cells.size() == 1)
        {
            Log::add_msg("A hidden trap is revealed to me.");
        }
        if (traps_revealed_cells.size() > 1)
        {
            Log::add_msg("Some hidden traps are revealed to me.");
        }
        return Spell_effect_noticed::yes;
    }
    return Spell_effect_noticed::no;
}

//------------------------------------------------------------ DETECT MONSTERS
Spell_effect_noticed Spell_det_mon::cast_(Actor* const caster) const
{
    (void)caster;

    bool                is_seer      = Player_bon::traits[int(Trait::seer)];
    const int           MULTIPLIER  = 6 * (is_seer ? 3 : 1);
    Spell_effect_noticed  is_noticed   = Spell_effect_noticed::no;

    for (Actor* actor : Game_time::actors_)
    {
        if (!actor->is_player())
        {
            static_cast<Mon*>(actor)->player_become_aware_of_me(MULTIPLIER);
            is_noticed = Spell_effect_noticed::yes;
        }
    }

    if (is_noticed == Spell_effect_noticed::yes)
    {
        Log::add_msg("I detect creatures.");
    }

    return is_noticed;
}

//------------------------------------------------------------ OPENING
Spell_effect_noticed Spell_opening::cast_(Actor* const caster) const
{
    (void)caster;

    bool is_any_opened = false;

    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            const auto& cell = Map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                Did_open did_open = cell.rigid->open(nullptr);

                if (did_open == Did_open::yes)
                {
                    is_any_opened = true;
                }
            }
        }
    }

    if (!is_any_opened)
    {
        return Spell_effect_noticed::no;
    }

    Map::player->update_fov();
    Render::draw_map_and_interface();
    return Spell_effect_noticed::yes;
}

//------------------------------------------------------------ SACRIFICE LIFE
Spell_effect_noticed Spell_sacr_life::cast_(Actor* const caster) const
{
    (void)caster;

    //Convert every 2 HP to 1 SPI

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    Map::player->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_self_msg, clr_white, false, true);
        return Spell_effect_noticed::no;
    }

    const int PLAYER_HP_CUR = Map::player->get_hp();

    if (PLAYER_HP_CUR > 2)
    {
        const int HP_DRAINED = ((PLAYER_HP_CUR - 1) / 2) * 2;
        Map::player->hit(HP_DRAINED, Dmg_type::pure);
        Map::player->restore_spi(HP_DRAINED, true, true);
        return Spell_effect_noticed::yes;
    }
    return Spell_effect_noticed::no;
}

//------------------------------------------------------------ SACRIFICE SPIRIT
Spell_effect_noticed Spell_sacr_spi::cast_(Actor* const caster) const
{
    (void)caster;

    //Convert all SPI to HP

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    Map::player->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_self_msg, clr_white, false, true);
        return Spell_effect_noticed::no;
    }

    const int PLAYER_SPI_CUR = Map::player->get_spi();

    if (PLAYER_SPI_CUR > 0)
    {
        const int HP_DRAINED = PLAYER_SPI_CUR - 1;
        Map::player->hit_spi(HP_DRAINED, true);
        Map::player->restore_hp(HP_DRAINED, true, true);
        return Spell_effect_noticed::yes;
    }
    return Spell_effect_noticed::no;
}

//------------------------------------------------------------ ROGUE HIDE
Spell_effect_noticed Spell_cloud_minds::cast_(Actor* const caster) const
{

    (void)caster;
    Log::add_msg("I vanish from the minds of my enemies.");

    for (Actor* actor : Game_time::actors_)
    {
        if (!actor->is_player())
        {
            Mon* const mon = static_cast<Mon*>(actor);
            mon->aware_counter_ = 0;
        }
    }
    return Spell_effect_noticed::yes;
}

//------------------------------------------------------------ BLESS
Spell_effect_noticed Spell_bless::cast_(Actor* const caster) const
{
    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    caster->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        if (caster->is_player())
        {
            Log::add_msg(spell_reflect_self_msg, clr_white, false, true);
        }
        return Spell_effect_noticed::no;
    }

    caster->get_prop_handler().try_apply_prop(new Prop_blessed(Prop_turns::std));
    return Spell_effect_noticed::yes;
}

//------------------------------------------------------------ LIGHT
Spell_effect_noticed Spell_light::cast_(Actor* const caster) const
{
    caster->get_prop_handler().try_apply_prop(new Prop_radiant(Prop_turns::std));
    return Spell_effect_noticed::yes;
}

//------------------------------------------------------------ TELEPORT
Spell_effect_noticed Spell_teleport::cast_(Actor* const caster) const
{
    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    caster->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        if (caster->is_player())
        {
            Log::add_msg(spell_reflect_self_msg, clr_white, false, true);
        }
        return Spell_effect_noticed::no;
    }

    caster->teleport();
    return Spell_effect_noticed::yes;
}

bool Spell_teleport::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_                                 &&
           mon.get_hp() <= (mon.get_hp_max(true) / 2)  &&
           Rnd::coin_toss();
}

//------------------------------------------------------------ ELEMENTAL RES
Spell_effect_noticed Spell_elem_res::cast_(Actor* const caster) const
{
    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    caster->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        if (caster->is_player())
        {
            Log::add_msg(spell_reflect_self_msg, clr_white, false, true);
        }
        return Spell_effect_noticed::no;
    }

    const int DURATION = 20;
    Prop_handler& prop_hlr = caster->get_prop_handler();
    prop_hlr.try_apply_prop(new Prop_rFire(Prop_turns::specific, DURATION));
    prop_hlr.try_apply_prop(new Prop_rElec(Prop_turns::specific, DURATION));
    prop_hlr.try_apply_prop(new Prop_rCold(Prop_turns::specific, DURATION));
    return Spell_effect_noticed::yes;
}

bool Spell_elem_res::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::one_in(3);
}

//------------------------------------------------------------ KNOCKBACK
Spell_effect_noticed Spell_knock_back::cast_(Actor* const caster) const
{
    assert(!caster->is_player());

    Clr     msg_clr      = clr_msg_good;
    string  tgt_str      = "me";
    Actor*  caster_used  = caster;
    Actor*  tgt         = static_cast<Mon*>(caster_used)->tgt_;
    assert(tgt);

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    tgt->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_msg, clr_white, false, true);
        swap(caster_used, tgt);
    }

    if (tgt->is_player())
    {
        msg_clr = clr_msg_bad;
    }
    else //Target is monster
    {
        tgt_str = tgt->get_name_the();
        if (Map::player->is_leader_of(tgt)) {msg_clr = clr_white;}
    }

    if (Map::player->can_see_actor(*tgt, nullptr))
    {
        Log::add_msg("A force pushes " + tgt_str + "!", msg_clr);
    }

    Knock_back::try_knock_back(*tgt, caster->pos, false);

    return Spell_effect_noticed::no;
}

bool Spell_knock_back::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_;
}

//------------------------------------------------------------ PROP ON OTHERS
Spell_effect_noticed Spell_prop_on_mon::cast_(Actor* const caster) const
{
    const Prop_id prop_id = get_prop_id();

    vector<Actor*> tgts;
    caster->get_seen_foes(tgts);

    if (tgts.empty())
    {
        return Spell_effect_noticed::no;
    }

    Render::draw_blast_at_seen_actors(tgts, clr_magenta);

    for (Actor* const tgt : tgts)
    {
        Prop_handler& prop_handler = tgt->get_prop_handler();

        //Spell reflection?
        bool tgt_props[size_t(Prop_id::END)];

        prop_handler.get_prop_ids(tgt_props);

        if (tgt_props[size_t(Prop_id::spell_reflect)])
        {
            Log::add_msg(spell_reflect_msg, clr_white, false, true);
            cast_(tgt);
            continue;
        }

        Prop* const prop = prop_handler.mk_prop(prop_id, Prop_turns::std);
        prop_handler.try_apply_prop(prop);
    }

    return Spell_effect_noticed::yes;
}

bool Spell_prop_on_mon::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_;
}

bool Spell_paralyze_mon::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::one_in(3);
}

//------------------------------------------------------------ DISEASE
Spell_effect_noticed Spell_disease::cast_(Actor* const caster) const
{
    assert(!caster->is_player());

    Actor* caster_used = caster;
    Actor* tgt        = static_cast<Mon*>(caster_used)->tgt_;

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    tgt->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_msg, clr_white, false, true);
        swap(caster_used, tgt);
    }

    string actor_name = "me";

    if (!tgt->is_player())
    {
        actor_name = tgt->get_name_the();
    }

    if (Map::player->can_see_actor(*tgt, nullptr))
    {
        Log::add_msg("A horrible disease is starting to afflict " + actor_name + "!");
    }

    tgt->get_prop_handler().try_apply_prop(new Prop_diseased(Prop_turns::specific, 50));
    return Spell_effect_noticed::no;
}

bool Spell_disease::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::coin_toss();
}

//------------------------------------------------------------ SUMMON MONSTER
Spell_effect_noticed Spell_summon_mon::cast_(Actor* const caster) const
{
    //Try to summon a creature inside the player's FOV (inside the standard range), in a
    //free visible cell. If no such cell is available, instead summon near the caster.

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(true), blocked);

    vector<Pos> free_cells_seen_by_player;
    const int RADI = FOV_STD_RADI_INT;
    const Pos player_pos(Map::player->pos);
    const int X0 = max(0, player_pos.x - RADI);
    const int Y0 = max(0, player_pos.y - RADI);
    const int X1 = min(MAP_W, player_pos.x + RADI) - 1;
    const int Y1 = min(MAP_H, player_pos.y + RADI) - 1;
    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            if (!blocked[x][y] && Map::cells[x][y].is_seen_by_player)
            {
                free_cells_seen_by_player.push_back(Pos(x, y));
            }
        }
    }

    Pos summon_pos(-1, -1);

    if (free_cells_seen_by_player.empty())
    {
        //No free cells seen by player, instead summon near the caster.
        vector<Pos> free_cells_vector;
        Utils::mk_vector_from_bool_map(false, blocked, free_cells_vector);
        if (!free_cells_vector.empty())
        {
            sort(free_cells_vector.begin(), free_cells_vector.end(),
                 Is_closer_to_pos(caster->pos));
            summon_pos = free_cells_vector[0];
        }
    }
    else //There are free cells seen by the player available
    {
        const size_t IDX  = Rnd::range(0, free_cells_seen_by_player.size() - 1);
        summon_pos         = free_cells_seen_by_player[IDX];
    }

    vector<Actor_id> summon_bucket;
    for (int i = 0; i < int(Actor_id::END); ++i)
    {
        const Actor_data_t& data = Actor_data::data[i];
        if (data.can_be_summoned)
        {
            //Method for finding eligible monsters depends on if player or monster is
            //casting.
            int dlvl_max = -1;

            if (caster->is_player())
            {
                //Compare player CVL with monster's allowed spawning DLVL.
                const int PLAYER_CLVL     = Dungeon_master::get_cLvl();
                const int PLAYER_CLVL_PCT = (PLAYER_CLVL * 100) / PLAYER_MAX_CLVL;

                dlvl_max                   = (PLAYER_CLVL_PCT * DLVL_LAST) / 100;
            }
            else //Caster is monster
            {
                //Compare caster and summoned monster's allowed spawning DLVL.
                dlvl_max = caster->get_data().spawn_min_dLVL;
            }

            if (data.spawn_min_dLVL <= dlvl_max)
            {
                summon_bucket.push_back(Actor_id(i));
            }
        }
    }

    if (summon_bucket.empty())
    {
        TRACE << "No elligible monsters found for spawning" << endl;
        assert(false);
        return Spell_effect_noticed::no;
    }

    const int       IDX                     = Rnd::range(0, summon_bucket.size() - 1);
    const Actor_id   mon_id                   = summon_bucket[IDX];
    Actor*          leader                  = nullptr;
    bool            did_player_summon_hostile  = false;

    if (caster->is_player())
    {
        const int N             = SUMMON_HOSTILE_ONE_IN_N *
                                  (Player_bon::traits[int(Trait::summoner)] ? 2 : 1);
        did_player_summon_hostile  = Rnd::one_in(N);
        leader                  = did_player_summon_hostile ? nullptr : caster;
    }
    else //Caster is monster
    {
        Actor* const caster_leader = static_cast<Mon*>(caster)->leader_;
        leader                    = caster_leader ? caster_leader : caster;
    }

    vector<Mon*> mon_summoned;

    Actor_factory::summon(summon_pos, {mon_id}, true, leader, &mon_summoned);

    Mon* const mon                = mon_summoned[0];
    mon->nr_turns_until_unsummoned_  = Rnd::range(40, 70);;

    if (Map::player->can_see_actor(*mon, nullptr))
    {
        Log::add_msg(mon->get_name_a() + " appears!");

        if (did_player_summon_hostile)
        {
            Log::add_msg("It is hostile!", clr_msg_note, true, true);
        }

        return Spell_effect_noticed::yes;
    }

    return Spell_effect_noticed::no;
}

bool Spell_summon_mon::allow_mon_cast_now(Mon& mon) const
{
    return mon.aware_counter_ > 0  &&
           Rnd::coin_toss()        &&
           (mon.tgt_ || Rnd::one_in(20));
}

//------------------------------------------------------------ HEAL SELF
Spell_effect_noticed Spell_heal_self::cast_(Actor* const caster) const
{
    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    caster->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        if (caster->is_player())
        {
            Log::add_msg(spell_reflect_self_msg, clr_white, false, true);
        }
        return Spell_effect_noticed::no;
    }

    //The spell effect is noticed if any hit points were restored
    const bool IS_ANY_HP_HEALED = caster->restore_hp(999, true);

    return IS_ANY_HP_HEALED ? Spell_effect_noticed::yes : Spell_effect_noticed::no;
}

bool Spell_heal_self::allow_mon_cast_now(Mon& mon) const
{
    return mon.get_hp() < mon.get_hp_max(true);
}

//------------------------------------------------------------ MI-GO HYPNOSIS
Spell_effect_noticed Spell_mi_go_hypno::cast_(Actor* const caster) const
{
    assert(!caster->is_player());

    Actor* caster_used = caster;
    Actor* tgt        = static_cast<Mon*>(caster_used)->tgt_;

    assert(tgt);

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    tgt->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_msg, clr_white, false, true);
        swap(caster_used, tgt);
    }

    if (tgt->is_player())
    {
        Log::add_msg("There is a sharp droning in my head!");
    }

    if (Rnd::coin_toss())
    {
        Prop* const prop = new Prop_fainted(Prop_turns::specific, Rnd::range(2, 10));
        tgt->get_prop_handler().try_apply_prop(prop);
    }
    else
    {
        Log::add_msg("I feel dizzy.");
    }

    return Spell_effect_noticed::yes;
}

bool Spell_mi_go_hypno::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && mon.tgt_->is_player() && Rnd::one_in(4);
}

//------------------------------------------------------------ IMMOLATION
Spell_effect_noticed Spell_burn::cast_(Actor* const caster) const
{
    assert(!caster->is_player());

    Actor* caster_used = caster;
    Actor* tgt        = static_cast<Mon*>(caster_used)->tgt_;

    assert(tgt);

    //Spell reflection?
    bool tgt_props[size_t(Prop_id::END)];

    tgt->get_prop_handler().get_prop_ids(tgt_props);

    if (tgt_props[size_t(Prop_id::spell_reflect)])
    {
        Log::add_msg(spell_reflect_msg, clr_white, false, true);
        swap(caster_used, tgt);
    }

    string tgt_str = "me";

    if (!tgt->is_player())
    {
        tgt_str = tgt->get_name_the();
    }

    if (Map::player->can_see_actor(*tgt, nullptr))
    {
        Log::add_msg("Flames are rising around " + tgt_str + "!");
    }

    Prop* const prop = new Prop_burning(Prop_turns::specific, Rnd::range(3, 4));
    tgt->get_prop_handler().try_apply_prop(prop);

    return Spell_effect_noticed::yes;
}

bool Spell_burn::allow_mon_cast_now(Mon& mon) const
{
    return mon.tgt_ && Rnd::one_in(4);
}
