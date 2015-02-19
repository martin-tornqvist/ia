#include "feature_rigid.h"

#include <string>

#include "init.h"
#include "log.h"
#include "render.h"
#include "map.h"
#include "utils.h"
#include "popup.h"
#include "map_travel.h"
#include "save_handling.h"
#include "item_factory.h"
#include "map_parsing.h"
#include "feature_mob.h"
#include "drop.h"
#include "explosion.h"
#include "actor_factory.h"
#include "actor_mon.h"
#include "query.h"
#include "pickup.h"

using namespace std;

//--------------------------------------------------------------------- RIGID
Rigid::Rigid(Pos pos) :
    Feature     (pos),
    gore_tile_   (Tile_id::empty),
    gore_glyph_  (0),
    is_bloody_   (false),
    burn_state_  (Burn_state::not_burned) {}

void Rigid::on_new_turn()
{
    if (burn_state_ == Burn_state::burning)
    {
        clear_gore();

        auto scorch_actor = [](Actor & actor)
        {
            if (&actor == Map::player)
            {
                Log::add_msg("I am scorched by flames.", clr_msg_bad);
            }
            else
            {
                if (Map::player->can_see_actor(actor, nullptr))
                {
                    Log::add_msg(actor.get_name_the() + " is scorched by flames.",
                                clr_msg_good);
                }
            }
            actor.hit(1, Dmg_type::fire);
        };

        //TODO: Hit dead actors

        //Hit actor standing on feature
        auto* actor = Utils::get_actor_at_pos(pos_);
        if (actor)
        {
            //Occasionally try to set actor on fire, otherwise just do small fire damage
            if (Rnd::one_in(4))
            {
                auto& prop_handler = actor->get_prop_handler();
                prop_handler.try_apply_prop(new Prop_burning(Prop_turns::std));
            }
            else
            {
                scorch_actor(*actor);
            }
        }

        //Finished burning?
        int finish_burning_one_in_n = 1;
        int hit_adjacent_one_in_n   = 1;

        switch (get_matl())
        {
        case Matl::fluid:
        case Matl::empty:
            finish_burning_one_in_n = 1;
            hit_adjacent_one_in_n   = 1;
            break;

        case Matl::stone:
            finish_burning_one_in_n = 12;
            hit_adjacent_one_in_n   = 12;
            break;

        case Matl::metal:
            finish_burning_one_in_n = 12;
            hit_adjacent_one_in_n   = 8;
            break;

        case Matl::plant:
            finish_burning_one_in_n = 30;
            hit_adjacent_one_in_n   = 12;
            break;

        case Matl::wood:
            finish_burning_one_in_n = 60;
            hit_adjacent_one_in_n   = 16;
            break;

        case Matl::cloth:
            finish_burning_one_in_n = 20;
            hit_adjacent_one_in_n   = 8;
            break;
        }

        if (Rnd::one_in(finish_burning_one_in_n))
        {
            burn_state_ = Burn_state::has_burned;
            if (on_finished_burning() == Was_destroyed::yes)
            {
                return;
            }
        }

        //Hit adjacent features and actors?
        if (Rnd::one_in(hit_adjacent_one_in_n))
        {
            const Pos p(Dir_utils::get_rnd_adj_pos(pos_, false));
            if (Utils::is_pos_inside_map(p))
            {
                Map::cells[p.x][p.y].rigid->hit(Dmg_type::fire, Dmg_method::elemental);

                actor = Utils::get_actor_at_pos(p);
                if (actor) {scorch_actor(*actor);}
            }
        }

        //Create smoke?
        if (Rnd::one_in(20))
        {
            const Pos p(Dir_utils::get_rnd_adj_pos(pos_, true));
            if (Utils::is_pos_inside_map(p))
            {
                if (!Cell_check::Blocks_move_cmn(false).check(Map::cells[p.x][p.y]))
                {
                    Game_time::add_mob(new Smoke(p, 10));
                }
            }
        }
    }

    //Run specialized new turn actions
    on_new_turn_();
}

void Rigid::try_start_burning(const bool IS_MSG_ALLOWED)
{
    clear_gore();

    if (burn_state_ == Burn_state::not_burned)
    {
        if (Map::is_pos_seen_by_player(pos_) && IS_MSG_ALLOWED)
        {
            string str = get_name(Article::the) + " catches fire.";
            str[0] = toupper(str[0]);
            Log::add_msg(str);
        }
        burn_state_ = Burn_state::burning;
    }
}

Was_destroyed Rigid::on_finished_burning()
{
    return Was_destroyed::no;
}

void Rigid::disarm()
{
    Log::add_msg(msg_disarm_no_trap);
    Render::draw_map_and_interface();
}

Did_open Rigid::open(Actor* const actor_opening)
{
    (void)actor_opening;
    return Did_open::no;
}

Did_trigger_trap Rigid::trigger_trap(Actor* const actor)
{
    (void)actor;
    return Did_trigger_trap::no;
}

void Rigid::hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* actor)
{
    bool is_feature_hit = true;

    if (actor == Map::player && dmg_method == Dmg_method::kick)
    {
        const bool IS_BLIND    = !Map::player->get_prop_handler().allow_see();
        const bool IS_BLOCKING = !can_move_cmn() && get_id() != Feature_id::stairs;

        if (IS_BLOCKING)
        {
            const string name = IS_BLIND ? "something" : get_name(Article::a);
            Log::add_msg("I kick " + name + "!");

            if (Rnd::one_in(4))
            {
                Log::add_msg("I sprain myself.", clr_msg_bad);
                actor->hit(Rnd::range(1, 3), Dmg_type::pure);
            }

            if (Rnd::one_in(4))
            {
                Log::add_msg("I am off-balance.");

                actor->get_prop_handler().try_apply_prop(
                    new Prop_paralyzed(Prop_turns::specific, 2));
            }

        }
        else //Not blocking
        {
            is_feature_hit = false;
            Log::add_msg("I kick the air!");
            Audio::play(Sfx_id::miss_medium);
        }
    }

    if (is_feature_hit)
    {
        on_hit(dmg_type, dmg_method, actor);
    }

    if (actor)
    {
        //TODO: This should probably be done elsewhere.
        Game_time::tick();
    }
}

int Rigid::get_shock_when_adj() const
{
    return get_data().shock_when_adjacent;
}

void Rigid::try_put_gore()
{
    if (get_data().can_have_gore)
    {
        const int ROLL_GLYPH = Rnd::dice(1, 4);
        switch (ROLL_GLYPH)
        {
        case 1: gore_glyph_ = ',';  break;
        case 2: gore_glyph_ = '`';  break;
        case 3: gore_glyph_ = 39;   break;
        case 4: gore_glyph_ = ';';  break;
        }
    }

    const int ROLL_TILE = Rnd::dice(1, 8);

    switch (ROLL_TILE)
    {
    case 1: gore_tile_ = Tile_id::gore1;  break;
    case 2: gore_tile_ = Tile_id::gore2;  break;
    case 3: gore_tile_ = Tile_id::gore3;  break;
    case 4: gore_tile_ = Tile_id::gore4;  break;
    case 5: gore_tile_ = Tile_id::gore5;  break;
    case 6: gore_tile_ = Tile_id::gore6;  break;
    case 7: gore_tile_ = Tile_id::gore7;  break;
    case 8: gore_tile_ = Tile_id::gore8;  break;
    }
}

Clr Rigid::get_clr() const
{
    if (burn_state_ == Burn_state::burning)
    {
        return clr_orange;
    }
    else
    {
        if (is_bloody_)
        {
            return clr_red_lgt;
        }
        else
        {
            return burn_state_ == Burn_state::not_burned ? get_clr_() : clr_gray_drk;
        }
    }
}

Clr Rigid::get_clr_bg() const
{
    switch (burn_state_)
    {
    case Burn_state::not_burned:  return get_clr_bg_();
    case Burn_state::burning:    return Clr {Uint8(Rnd::range(32, 255)), 0, 0, 0};
    case Burn_state::has_burned:  return get_clr_bg_();
    }
    assert(false && "Failed to set color");
    return clr_yellow;
}

void Rigid::clear_gore()
{
    gore_tile_   = Tile_id::empty;
    gore_glyph_  = ' ';
    is_bloody_   = false;
}

//--------------------------------------------------------------------- FLOOR
Floor::Floor(Pos pos) :
    Rigid   (pos),
    type_   (Floor_type::cmn) {}

void Floor::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;
        if (Rnd::one_in(3))
        {
            try_start_burning(false);
        }
    }
}

Tile_id Floor::get_tile() const
{
    return get_burn_state() == Burn_state::has_burned ?
           Tile_id::scorched_ground :
           get_data().tile;
}

string Floor::get_name(const Article article) const
{
    string ret = article == Article::a ? "" : "the ";

    if (get_burn_state() == Burn_state::burning)
    {
        ret += "flames";
    }
    else
    {
        if (get_burn_state() == Burn_state::has_burned) {ret += "scorched ";}

        switch (type_)
        {
        case Floor_type::cmn:        ret += "stone floor";   break;
        case Floor_type::cave:       ret += "cavern floor";  break;
        case Floor_type::stone_path:  ret += "stone path";    break;
        }
    }

    return ret;
}

Clr Floor::get_clr_() const
{
    return clr_gray;
}

//--------------------------------------------------------------------- WALL
Wall::Wall(Pos pos) :
    Rigid     (pos),
    type_     (Wall_type::cmn),
    is_mossy_  (false) {}

void Wall::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)actor;

    auto destr_adj_doors = [&]()
    {
        for (const Pos& d : Dir_utils::cardinal_list)
        {
            const Pos p(pos_ + d);
            if (Utils::is_pos_inside_map(p))
            {
                if (Map::cells[p.x][p.y].rigid->get_id() == Feature_id::door)
                {
                    Map::put(new Rubble_low(p));
                }
            }
        }
    };

    auto mk_low_rubble_and_rocks = [&]()
    {
        const Pos p(pos_);
        Map::put(new Rubble_low(p)); //NOTE: "this" is now deleted!
        if (Rnd::coin_toss()) {Item_factory::mk_item_on_floor(Item_id::rock, p);}
    };

    if (dmg_type == Dmg_type::physical)
    {
        if (dmg_method == Dmg_method::forced)
        {
            destr_adj_doors();
            mk_low_rubble_and_rocks();
        }

        if (dmg_method == Dmg_method::explosion)
        {
            destr_adj_doors();

            if (Rnd::coin_toss())
            {
                mk_low_rubble_and_rocks();
            }
            else
            {
                Map::put(new Rubble_high(pos_));
            }
        }

        if (dmg_method == Dmg_method::blunt_heavy)
        {
            if (Rnd::fraction(1, 4))
            {
                destr_adj_doors();

                if (Rnd::coin_toss())
                {
                    mk_low_rubble_and_rocks();
                }
                else
                {
                    Map::put(new Rubble_high(pos_));
                }
            }
        }
    }
}

bool Wall::is_tile_any_wall_front(const Tile_id tile)
{
    return
        tile == Tile_id::wall_front      ||
        tile == Tile_id::wall_front_alt1  ||
        tile == Tile_id::wall_front_alt2  ||
        tile == Tile_id::cave_wall_front  ||
        tile == Tile_id::egypt_wall_front;
}

bool Wall::is_tile_any_wall_top(const Tile_id tile)
{
    return
        tile == Tile_id::wall_top      ||
        tile == Tile_id::cave_wall_top  ||
        tile == Tile_id::egypt_wall_top ||
        tile == Tile_id::rubble_high;
}

string Wall::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    if (is_mossy_) {ret += "moss-grown ";}

    switch (type_)
    {
    case Wall_type::cmn:
    case Wall_type::cmn_alt:
    case Wall_type::leng_monestary:
    case Wall_type::egypt:
        ret += "stone wall";
        break;

    case Wall_type::cave:
        ret += "cavern wall";
        break;

    case Wall_type::cliff:
        ret += "cliff";
        break;
    }
    return ret;
}

Clr Wall::get_clr_() const
{
    if (is_mossy_) {return clr_green_drk;}
    switch (type_)
    {
    case Wall_type::cliff:
        return clr_gray_drk;

    case Wall_type::egypt:
    case Wall_type::cave:
        return clr_brown_gray;

    case Wall_type::cmn:
    case Wall_type::cmn_alt:
        return clr_gray;

    case Wall_type::leng_monestary:
        return clr_red;
    }
    assert(false && "Failed to set color");
    return clr_yellow;
}

char Wall::get_glyph() const
{
    return Config::is_ascii_wall_full_square() ? 10 : '#';
}

Tile_id Wall::get_front_wall_tile() const
{
    if (Config::is_tiles_wall_full_square())
    {
        switch (type_)
        {
        case Wall_type::cmn:
        case Wall_type::cmn_alt:
            return Tile_id::wall_top;

        case Wall_type::cliff:
        case Wall_type::cave:
            return Tile_id::cave_wall_top;

        case Wall_type::leng_monestary:
        case Wall_type::egypt:
            return Tile_id::egypt_wall_top;
        }
    }
    else
    {
        switch (type_)
        {
        case Wall_type::cmn:
            return Tile_id::wall_front;

        case Wall_type::cmn_alt:
            return Tile_id::wall_front_alt1;

        case Wall_type::cliff:
        case Wall_type::cave:
            return Tile_id::cave_wall_front;

        case Wall_type::leng_monestary:
        case Wall_type::egypt:
            return Tile_id::egypt_wall_front;
        }
    }
    assert(false && "Failed to set front wall tile");
    return Tile_id::empty;
}

Tile_id Wall::get_top_wall_tile() const
{
    switch (type_)
    {
    case Wall_type::cmn:
    case Wall_type::cmn_alt:
        return Tile_id::wall_top;

    case Wall_type::cliff:
    case Wall_type::cave:
        return Tile_id::cave_wall_top;

    case Wall_type::leng_monestary:
    case Wall_type::egypt:
        return Tile_id::egypt_wall_top;
    }
    assert(false && "Failed to set top wall tile");
    return Tile_id::empty;
}

void Wall::set_rnd_cmn_wall()
{
    const int RND = Rnd::range(1, 6);
    switch (RND)
    {
    case 1:   type_ = Wall_type::cmn_alt; break;
    default:  type_ = Wall_type::cmn;    break;
    }
}

void Wall::set_random_is_moss_grown()
{
    is_mossy_ = Rnd::one_in(40);
}

//--------------------------------------------------------------------- HIGH RUBBLE
Rubble_high::Rubble_high(Pos pos) :
    Rigid(pos) {}

void Rubble_high::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                       Actor* const actor)
{
    (void)actor;

    auto mk_low_rubble_and_rocks = [&]()
    {
        const Pos p(pos_);
        Map::put(new Rubble_low(p)); //NOTE: "this" is now deleted!
        if (Rnd::coin_toss()) {Item_factory::mk_item_on_floor(Item_id::rock, p);}
    };

    if (dmg_type == Dmg_type::physical)
    {

        if (dmg_method == Dmg_method::forced)
        {
            mk_low_rubble_and_rocks();
        }

        if (dmg_method == Dmg_method::explosion)
        {
            mk_low_rubble_and_rocks();
        }

        if (dmg_method == Dmg_method::blunt_heavy)
        {
            if (Rnd::fraction(2, 4)) {mk_low_rubble_and_rocks();}
        }
    }
}

string Rubble_high::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "big pile of debris";
}

Clr Rubble_high::get_clr_() const
{
    return clr_gray;
}

//--------------------------------------------------------------------- LOW RUBBLE
Rubble_low::Rubble_low(Pos pos) : Rigid(pos) {}

void Rubble_low::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                      Actor* const actor)
{
    (void)actor;

    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        try_start_burning(false);
    }
}

string Rubble_low::get_name(const Article article) const
{
    string ret = "";
    if (article == Article::the)               {ret += "the ";}
    if (get_burn_state() == Burn_state::burning)  {ret += "burning ";}
    return ret + "rubble";
}

Clr Rubble_low::get_clr_() const
{
    return clr_gray;
}

//--------------------------------------------------------------------- BONES
Bones::Bones(Pos pos) : Rigid(pos) {}

void Bones::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                  Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

string Bones::get_name(const Article article) const
{
    string ret = "";
    if (article == Article::the) {ret += "the ";}
    return ret + "bones";
}

Clr Bones::get_clr_() const
{
    return clr_red;
}

//--------------------------------------------------------------------- GRAVE
Grave_stone::Grave_stone(Pos pos) :
    Rigid(pos) {}

void Grave_stone::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                       Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Grave_stone::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player()) {Log::add_msg(inscr_);}
}

string Grave_stone::get_name(const Article article) const
{
    const string ret = article == Article::a ? "a " : "the ";
    return ret + "gravestone; " + inscr_;
}

Clr Grave_stone::get_clr_() const
{
    return clr_white;
}

//--------------------------------------------------------------------- CHURCH BENCH
Church_bench::Church_bench(Pos pos) : Rigid(pos) {}

void Church_bench::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                        Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Church_bench::get_name(const Article article) const
{
    const string ret = article == Article::a ? "a " : "the ";
    return ret + "church bench";
}

Clr Church_bench::get_clr_() const
{
    return clr_brown;
}

//--------------------------------------------------------------------- STATUE
Statue::Statue(Pos pos) :
    Rigid   (pos),
    type_   (Rnd::one_in(8) ? Statue_type::ghoul : Statue_type::cmn) {}

int Statue::get_shock_when_adj() const
{
    return type_ == Statue_type::ghoul ? 15 : 0;
}

void Statue::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::physical && dmg_method == Dmg_method::kick)
    {
        assert(actor);

        bool props[size_t(Prop_id::END)];
        actor->get_prop_handler().get_prop_ids(props);

        if (props[int(Prop_id::weakened)])
        {
            Log::add_msg("It wiggles a bit.");
            return;
        }

        const Alerts_mon alerts_mon = actor == Map::player ?
                                    Alerts_mon::yes :
                                    Alerts_mon::no;

        if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            Log::add_msg("It topples over.");
        }

        Snd snd("I hear a crash.", Sfx_id::END, Ignore_msg_if_origin_seen::yes,
                pos_, actor, Snd_vol::low, alerts_mon);

        Snd_emit::emit_snd(snd);

        const Pos dst_pos = pos_ + (pos_ - actor->pos);

        Map::put(new Rubble_low(pos_)); //NOTE: "this" is now deleted!

        Map::player->update_fov();
        Render::draw_map_and_interface();


        Actor* const actor_behind = Utils::get_actor_at_pos(dst_pos);

        if (actor_behind && actor_behind->is_alive())
        {
            bool props_actor_behind[size_t(Prop_id::END)];
            actor_behind->get_prop_handler().get_prop_ids(props_actor_behind);
            if (!props_actor_behind[int(Prop_id::ethereal)])
            {
                if (actor_behind == Map::player)
                {
                    Log::add_msg("It falls on me!");
                }
                else if (Map::player->can_see_actor(*actor_behind, nullptr))
                {
                    Log::add_msg("It falls on " + actor_behind->get_name_a() + ".");
                }
                actor_behind->hit(Rnd::dice(3, 5), Dmg_type::physical);
            }
        }

        const auto rigid_id = Map::cells[dst_pos.x][dst_pos.y].rigid->get_id();

        //NOTE: This is kinda hacky, but the rubble is mostly just for decoration anyway,
        //so it doesn't really matter.
        if (
            rigid_id == Feature_id::floor ||
            rigid_id == Feature_id::grass ||
            rigid_id == Feature_id::carpet)
        {
            Map::put(new Rubble_low(dst_pos));
        }
    }
}

string Statue::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    switch (type_)
    {
    case Statue_type::cmn:   ret += "statue"; break;
    case Statue_type::ghoul: ret += "statue of a ghoulish creature"; break;
    }

    return ret;
}

Tile_id Statue::get_tile() const
{
    return type_ == Statue_type::cmn ? Tile_id::witch_or_warlock : Tile_id::ghoul;
}

Clr Statue::get_clr_() const
{
    return clr_white;
}

//--------------------------------------------------------------------- PILLAR
Pillar::Pillar(Pos pos) :
    Rigid(pos) {}

void Pillar::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Pillar::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "pillar";
}

Clr Pillar::get_clr_() const
{
    return clr_white;
}

//--------------------------------------------------------------------- MONOLITH
Monolith::Monolith(Pos pos) :
    Rigid(pos) {}

void Monolith::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Monolith::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "monolith";
}

Clr Monolith::get_clr_() const
{
    return clr_white;
}

//--------------------------------------------------------------------- STALAGMITE
Stalagmite::Stalagmite(Pos pos) :
    Rigid(pos) {}

void Stalagmite::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                       Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Stalagmite::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "stalagmite";
}

Clr Stalagmite::get_clr_() const
{
    return clr_brown_gray;
}

//--------------------------------------------------------------------- STAIRS
Stairs::Stairs(Pos pos) :
    Rigid(pos) {}

void Stairs::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Stairs::on_new_turn_()
{
    assert(!Map::cells[pos_.x][pos_.y].item);
}

void Stairs::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        const vector<string> choices {"Descend", "Save and quit", "Cancel"};
        const string title  = "A staircase leading downwards";
        const int CHOICE    =  Popup::show_menu_msg("", true, choices, title);

        switch (CHOICE)
        {
        case 0:
            Map::player->pos = pos_;
            Log::clear_log();
            Log::add_msg("I descend the stairs.");
            Render::draw_map_and_interface();
            Map_travel::go_to_nxt();
            break;

        case 1:
            Map::player->pos = pos_;
            Save_handling::save();
            Init::quit_to_main_menu = true;
            break;

        default:
            Log::clear_log();
            Render::draw_map_and_interface();
            break;
        }
    }
}

string Stairs::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "downward staircase";
}

Clr Stairs::get_clr_() const
{
    return clr_yellow;
}

//--------------------------------------------------------------------- BRIDGE
Tile_id Bridge::get_tile() const
{
    return dir_ == hor ? Tile_id::hangbridge_hor : Tile_id::hangbridge_ver;
}

void Bridge::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

char Bridge::get_glyph() const
{
    return dir_ == hor ? '|' : '=';
}

string Bridge::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "bridge";
}

Clr Bridge::get_clr_() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- SHALLOW LIQUID
Liquid_shallow::Liquid_shallow(Pos pos) :
    Rigid   (pos),
    type_   (Liquid_type::water) {}

void Liquid_shallow::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                          Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Liquid_shallow::bump(Actor& actor_bumping)
{
    bool props[size_t(Prop_id::END)];
    actor_bumping.get_prop_handler().get_prop_ids(props);

    if (!props[int(Prop_id::ethereal)] && !props[int(Prop_id::flying)])
    {
        actor_bumping.get_prop_handler().try_apply_prop(new Prop_waiting(Prop_turns::std));

        if (actor_bumping.is_player())
        {
            Log::add_msg("*glop*");
        }
    }
}

string Liquid_shallow::get_name(const Article article) const
{
    string ret = "";
    if (article == Article::the) {ret += "the ";}

    ret += "a pool of ";

    switch (type_)
    {
    case Liquid_type::water:   ret += "water"; break;
    case Liquid_type::acid:    ret += "acid";  break;
    case Liquid_type::blood:   ret += "blood"; break;
    case Liquid_type::lava:    ret += "lava";  break;
    case Liquid_type::mud:     ret += "mud";   break;
    }

    return ret;
}

Clr Liquid_shallow::get_clr_() const
{
    switch (type_)
    {
    case Liquid_type::water:   return clr_blue_lgt;  break;
    case Liquid_type::acid:    return clr_green_lgt; break;
    case Liquid_type::blood:   return clr_red_lgt;   break;
    case Liquid_type::lava:    return clr_orange;   break;
    case Liquid_type::mud:     return clr_brown;    break;
    }
    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- DEEP LIQUID
Liquid_deep::Liquid_deep(Pos pos) :
    Rigid   (pos),
    type_   (Liquid_type::water) {}

void Liquid_deep::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                       Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Liquid_deep::bump(Actor& actor_bumping)
{
    (void)actor_bumping;
}

string Liquid_deep::get_name(const Article article) const
{
    string ret = "";
    if (article == Article::the) {ret += "the ";}

    ret += "deep ";

    switch (type_)
    {
    case Liquid_type::water:   ret += "water"; break;
    case Liquid_type::acid:    ret += "acid";  break;
    case Liquid_type::blood:   ret += "blood"; break;
    case Liquid_type::lava:    ret += "lava";  break;
    case Liquid_type::mud:     ret += "mud";   break;
    }

    return ret;
}

Clr Liquid_deep::get_clr_() const
{
    switch (type_)
    {
    case Liquid_type::water:   return clr_blue;     break;
    case Liquid_type::acid:    return clr_green;    break;
    case Liquid_type::blood:   return clr_red;      break;
    case Liquid_type::lava:    return clr_orange;   break;
    case Liquid_type::mud:     return clr_brown_drk; break;
    }
    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- CHASM
Chasm::Chasm(Pos pos) :
    Rigid(pos) {}

void Chasm::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}


string Chasm::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "chasm";
}

Clr Chasm::get_clr_() const
{
    return clr_black;
}

//--------------------------------------------------------------------- LEVER
Lever::Lever(Pos pos) :
    Rigid             (pos),
    is_position_left_   (true),
    door_linked_to_     (nullptr)  {}

void Lever::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Lever::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "lever";
}

Clr Lever::get_clr_() const
{
    return is_position_left_ ? clr_gray : clr_white;
}

Tile_id Lever::get_tile() const
{
    return is_position_left_ ? Tile_id::lever_left : Tile_id::lever_right;
}

void Lever::pull()
{
    TRACE_FUNC_BEGIN;
    is_position_left_ = !is_position_left_;

    //TODO: Implement something like open_by_lever in the Door class instead of setting
    //"is_open_" etc directly.

//  if(!door_linked_to_->is_broken_) {
//    TRACE << "Door linked to is not broken" << endl;
//    if(!door_linked_to_->is_open_) {door_linked_to_->reveal(true);}
//    door_linked_to_->is_open_  = !door_linked_to_->is_open_;
//    door_linked_to_->is_stuck_ = false;
//  }
    Map::player->update_fov();
    Render::draw_map_and_interface();
    TRACE_FUNC_END;
}

//--------------------------------------------------------------------- ALTAR
Altar::Altar(Pos pos) :
    Rigid(pos) {}

void Altar::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Altar::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "altar";
}

Clr Altar::get_clr_() const
{
    return clr_white;
}

//--------------------------------------------------------------------- CARPET
Carpet::Carpet(Pos pos) :
    Rigid(pos) {}

void Carpet::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}

Was_destroyed Carpet::on_finished_burning()
{
    Floor* const floor = new Floor(pos_);
    floor->set_has_burned();
    Map::put(floor);
    return Was_destroyed::yes;
}

string Carpet::get_name(const Article article) const
{
    string ret = article == Article::a ? "" : "the ";
    return ret + "carpet";
}

Clr Carpet::get_clr_() const
{
    return clr_red;
}

//--------------------------------------------------------------------- GRASS
Grass::Grass(Pos pos) :
    Rigid   (pos),
    type_   (Grass_type::cmn)
{
    if (Rnd::one_in(5)) {type_ = Grass_type::withered;}
}

void Grass::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}


Tile_id Grass::get_tile() const
{
    return get_burn_state() == Burn_state::has_burned ?
           Tile_id::scorched_ground :
           get_data().tile;
}

string Grass::get_name(const Article article) const
{
    string ret = "";
    if (article == Article::the) {ret += "the ";}

    switch (get_burn_state())
    {
    case Burn_state::not_burned:
        switch (type_)
        {
        case Grass_type::cmn:
            return ret + "grass";

        case Grass_type::withered:
            return ret + "withered grass";
        }
        break;

    case Burn_state::burning:
        return ret + "burning grass";

    case Burn_state::has_burned:
        return ret + "scorched ground";
    }

    assert("Failed to set name" && false);
    return "";
}

Clr Grass::get_clr_() const
{
    switch (type_)
    {
    case Grass_type::cmn:      return clr_green;    break;
    case Grass_type::withered: return clr_brown_drk; break;
    }
    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- BUSH
Bush::Bush(Pos pos) :
    Rigid   (pos),
    type_   (Grass_type::cmn)
{
    if (Rnd::one_in(5)) {type_ = Grass_type::withered;}
}

void Bush::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,  Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}

Was_destroyed Bush::on_finished_burning()
{
    Grass* const grass = new Grass(pos_);
    grass->set_has_burned();
    Map::put(grass);
    return Was_destroyed::yes;
}

string Bush::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    switch (get_burn_state())
    {
    case Burn_state::not_burned:
        switch (type_)
        {
        case Grass_type::cmn:
            return ret + "shrub";

        case Grass_type::withered:
            return ret + "withered shrub";
        }
        break;

    case Burn_state::burning:
        return ret + "burning shrub";

    case Burn_state::has_burned: {/*Should not happen*/}
        break;
    }

    assert("Failed to set name" && false);
    return "";
}

Clr Bush::get_clr_() const
{
    switch (type_)
    {
    case Grass_type::cmn:      return clr_green;    break;
    case Grass_type::withered: return clr_brown_drk; break;
    }
    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- TREE
Tree::Tree(Pos pos) :
    Rigid(pos) {}

void Tree::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;
        if (Rnd::one_in(3))
        {
            try_start_burning(false);
        }
    }
}

Was_destroyed Tree::on_finished_burning()
{
    if (Utils::is_pos_inside_map(pos_, false))
    {
        Grass* const grass = new Grass(pos_);
        grass->set_has_burned();
        Map::put(grass);
        return Was_destroyed::yes;
    }

    return Was_destroyed::no;
}

string Tree::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    switch (get_burn_state())
    {
    case Burn_state::not_burned:  {}                  break;
    case Burn_state::burning:    ret += "burning ";  break;
    case Burn_state::has_burned:  ret += "scorched "; break;
    }

    return ret + "tree";
}

Clr Tree::get_clr_() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- BRAZIER
string Brazier::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "brazier";
}

void Brazier::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                    Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

Clr Brazier::get_clr_() const
{
    return clr_yellow;
}

//--------------------------------------------------------------------- ITEM CONTAINER
Item_container::Item_container()
{
    for (auto* item : items_) {delete item;}

    items_.clear();
}

Item_container::~Item_container()
{
    for (auto* item : items_) {delete item;}
}

void Item_container::init(const Feature_id feature_id, const int NR_ITEMS_TO_ATTEMPT)
{
    for (auto* item : items_) {delete item;}

    items_.clear();

    if (NR_ITEMS_TO_ATTEMPT > 0)
    {
        while (items_.empty())
        {
            vector<Item_id> item_bucket;

            for (size_t i = 0; i < size_t(Item_id::END); ++i)
            {
                Item_data_t* const d = Item_data::data[i];

                for (auto container_spawn_rule : d->container_spawn_rules)
                {
                    if (
                        container_spawn_rule.feature_id == feature_id   &&
                        d->allow_spawn                               &&
                        Rnd::percent() < container_spawn_rule.pct_chance_to_incl)
                    {
                        item_bucket.push_back(Item_id(i));
                        break;
                    }
                }
            }

            for (int i = 0; i < NR_ITEMS_TO_ATTEMPT; ++i)
            {
                if (item_bucket.empty())
                {
                    break;
                }

                const int     IDX = Rnd::range(0, item_bucket.size() - 1);
                const Item_id  id  = item_bucket[IDX];

                //Check if this item is no longer allowed to spawn (e.g. a unique item)
                if (!Item_data::data[int(id)]->allow_spawn)
                {
                    item_bucket.erase(begin(item_bucket) + IDX);
                }
                else
                {
                    Item* item = Item_factory::mk(item_bucket[IDX]);
                    Item_factory::set_item_randomized_properties(item);
                    items_.push_back(item);
                }
            }
        }
    }
}

void Item_container::open(const Pos& feature_pos, Actor* const actor_opening)
{
    if (actor_opening)
    {
        for (auto* item : items_)
        {
            Log::clear_log();

            const string name = item->get_name(Item_ref_type::plural, Item_ref_inf::yes,
                                              Item_ref_att_inf::wpn_context);

            Log::add_msg("Pick up " + name + "? [y/n]");

            const Item_data_t&  data              = item->get_data();

            Wpn*              wpn               = data.ranged.is_ranged_wpn ?
                                                  static_cast<Wpn*>(item) : nullptr;

            const bool        IS_UNLOADABLE_WPN = wpn                    &&
                                                  wpn->nr_ammo_loaded > 0  &&
                                                  !data.ranged.has_infinite_ammo;

            if (IS_UNLOADABLE_WPN)
            {
                Log::add_msg("Unload? [G]");
            }

            Render::draw_map_and_interface();

            const Yes_no_answer answer = Query::yes_or_no(IS_UNLOADABLE_WPN ? 'G' : -1);

            if (answer == Yes_no_answer::yes)
            {
                Audio::play(Sfx_id::pickup);

                Inventory& inv = Map::player->get_inv();

                Item* const thrown_item = inv.slots_[size_t(Slot_id::thrown)].item;

                if (thrown_item && thrown_item->get_id() == item->get_id())
                {
                    thrown_item->nr_items_ += item->nr_items_;
                    delete item;
                }
                else //Item does not stack with current thrown weapon
                {
                    inv.put_in_general(item);
                }
            }
            else if (answer == Yes_no_answer::no)
            {
                Item_drop::drop_item_on_map(feature_pos, *item);
            }
            else //Special key (unload in this case)
            {
                assert(IS_UNLOADABLE_WPN);
                assert(wpn);
                assert(wpn->nr_ammo_loaded > 0);
                assert(!data.ranged.has_infinite_ammo);

                Audio::play(Sfx_id::pickup);

                Ammo* const spawned_ammo = Item_pickup::unload_ranged_wpn(*wpn);

                Map::player->get_inv().put_in_general(spawned_ammo);

                Item_drop::drop_item_on_map(feature_pos, *wpn);
            }
        }
        Log::clear_log();
        Log::add_msg("There are no more items.");
        Render::draw_map_and_interface();
    }
    else //Not opened by an actor (probably opened by a spell of opening)
    {
        for (auto* item : items_)
        {
            Item_drop::drop_item_on_map(feature_pos, *item);
        }
    }

    items_.clear();
}

void Item_container::destroy_single_fragile()
{
    //TODO: Generalize this function (perhaps something like "is_fragile" item data value)

    for (size_t i = 0; i < items_.size(); ++i)
    {
        Item* const item = items_[i];
        const Item_data_t& d = item->get_data();
        if (d.type == Item_type::potion || d.id == Item_id::molotov)
        {
            delete item;
            items_.erase(items_.begin() + i);
            Log::add_msg("I hear a muffled shatter.");
            break;
        }
    }
}

//--------------------------------------------------------------------- TOMB
Tomb::Tomb(const Pos& pos) :
    Rigid               (pos),
    is_open_             (false),
    is_trait_known_       (false),
    push_lid_one_in_n_      (Rnd::range(6, 14)),
    appearance_         (Tomb_appearance::common),
    is_random_appearance_ (false),
    trait_              (Tomb_trait::END)
{
    //Contained items
    const int NR_ITEMS_MIN  = Rnd::one_in(3) ? 0 : 1;
    const int NR_ITEMS_MAX  = NR_ITEMS_MIN +
                              (Player_bon::traits[int(Trait::treasure_hunter)] ? 1 : 0);

    item_container_.init(Feature_id::tomb, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

    //Appearance
    if (Rnd::one_in(12))
    {
        //Do not base appearance on items (random appearance)
        const int NR_APP    = int(Tomb_appearance::END);
        appearance_         = Tomb_appearance(Rnd::range(0, NR_APP - 1));

        is_random_appearance_ = true;
    }
    else //Base appearance on value of contained items
    {
        //Appearance is based on items inside
        for (Item* item : item_container_.items_)
        {
            const Item_value item_value = item->get_data().value;

            if (item_value == Item_value::major_treasure)
            {
                appearance_ = Tomb_appearance::marvelous;
                break;
            }
            else if (item_value == Item_value::minor_treasure)
            {
                appearance_ = Tomb_appearance::ornate;
            }
        }
    }

    if (!item_container_.items_.empty())
    {
        if (appearance_ == Tomb_appearance::marvelous && !is_random_appearance_)
        {
            trait_ = Tomb_trait::undead;
        }
        else //Randomize trait
        {
            const int RND = Rnd::percent();

            if (RND < 15)
            {
                trait_ = Tomb_trait::cursed;
            }
            else if (RND < 45)
            {
                trait_ = Tomb_trait::stench;
            }
            else if (RND < 75)
            {
                trait_ = Tomb_trait::undead;
            }
        }
    }
}

void Tomb::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

string Tomb::get_name(const Article article) const
{
    const bool    IS_EMPTY  = is_open_ && item_container_.items_.empty();
    const string  empty_str  = IS_EMPTY                ? "empty " : "";
    const string  open_str   = (is_open_ && !IS_EMPTY)  ? "open "  : "";

    string a = "";

    if (article == Article::a)
    {
        a = (is_open_ || appearance_ == Tomb_appearance::ornate) ?  "an " : "a ";
    }
    else
    {
        a = "the ";
    }

    string appear_str = "";

    if (!IS_EMPTY)
    {
        switch (appearance_)
        {
        case Tomb_appearance::common:
        case Tomb_appearance::END: {}
            break;

        case Tomb_appearance::ornate:
            appear_str = "ornate ";
            break;

        case Tomb_appearance::marvelous:
            appear_str = "marvelous ";
            break;
        }
    }

    return a + empty_str + open_str + appear_str + "tomb";
}

Tile_id Tomb::get_tile() const
{
    return is_open_ ? Tile_id::tomb_open : Tile_id::tomb_closed;
}

Clr Tomb::get_clr_() const
{
    switch (appearance_)
    {
    case Tomb_appearance::common:
        return clr_gray;

    case Tomb_appearance::ornate:
        return clr_white;

    case Tomb_appearance::marvelous:
        return clr_yellow;

    case Tomb_appearance::END: {}
        break;
    }
    assert("Failed to set Tomb color" && false);
    return clr_black;
}

void Tomb::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (item_container_.items_.empty() && is_open_)
        {
            Log::add_msg("The tomb is empty.");
        }
        else if (!Map::player->get_prop_handler().allow_see())
        {
            Log::add_msg("There is a stone box here.");
            Render::draw_map_and_interface();
        }
        else //Player can see
        {
            if (is_open_)
            {
                player_loot();
            }
            else //Not open
            {
                Log::add_msg("I attempt to push the lid.");

                bool props[size_t(Prop_id::END)];
                Map::player->get_prop_handler().get_prop_ids(props);

                if (props[int(Prop_id::weakened)])
                {
                    try_sprain_player();
                    Log::add_msg("It seems futile.", clr_msg_note, false, true);
                }
                else //Not weakened
                {
                    const int BON =
                        Player_bon::traits[int(Trait::unbreakable)]  ? 12  :
                        Player_bon::traits[int(Trait::rugged)]       ? 8   :
                        Player_bon::traits[int(Trait::tough)]        ? 4   : 0;

                    TRACE << "Base chance to push lid is: 1 in "
                          << push_lid_one_in_n_ << endl;

                    TRACE << "Bonus to roll: "
                          << BON << endl;

                    const int ROLL_TOT = Rnd::range(1, push_lid_one_in_n_) + BON;

                    TRACE << "Roll + bonus = " << ROLL_TOT << endl;

                    bool is_success = false;

                    if (ROLL_TOT < push_lid_one_in_n_ - 9)
                    {
                        Log::add_msg("It does not yield at all.");
                    }
                    else if (ROLL_TOT < push_lid_one_in_n_ - 1)
                    {
                        Log::add_msg("It resists.");
                    }
                    else if (ROLL_TOT == push_lid_one_in_n_ - 1)
                    {
                        Log::add_msg("It moves a little!");
                        push_lid_one_in_n_--;
                    }
                    else
                    {
                        is_success = true;
                    }

                    if (is_success)
                    {
                        open(Map::player);
                    }
                    else
                    {
                        try_sprain_player();
                    }
                }
            }

            Game_time::tick();
        }
    }
}

void Tomb::try_sprain_player()
{
    const int SPRAIN_ONE_IN_N =
        Player_bon::traits[int(Trait::unbreakable)]  ? 10 :
        Player_bon::traits[int(Trait::rugged)]       ? 8  :
        Player_bon::traits[int(Trait::tough)]        ? 6  : 4;

    if (Rnd::one_in(SPRAIN_ONE_IN_N))
    {
        Log::add_msg("I sprain myself.", clr_msg_bad);
        Map::player->hit(Rnd::range(1, 3), Dmg_type::pure);
    }
}

void Tomb::player_loot()
{
    Log::add_msg("I peer inside the tomb.");

    if (item_container_.items_.empty())
    {
        Log::add_msg("There is nothing of value inside.");
    }
    else
    {
        Log::add_msg("There are some items inside.", clr_white, false, true);

        item_container_.open(pos_, Map::player);
    }
}

Did_open Tomb::open(Actor* const actor_opening)
{
    if (is_open_)
    {
        return Did_open::no;
    }
    else //Was not already open
    {
        is_open_ = true;

        Snd_emit::emit_snd({"I hear heavy stone sliding.", Sfx_id::tomb_open,
                          Ignore_msg_if_origin_seen::yes, pos_, nullptr, Snd_vol::high,
                          Alerts_mon::yes
                         });

        if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            Render::draw_map_and_interface();
            Log::add_msg("The lid comes off.");
        }

        trigger_trap(actor_opening);

        Render::draw_map_and_interface();
        return Did_open::yes;
    }
}

Did_trigger_trap Tomb::trigger_trap(Actor* const actor)
{
    (void)actor;

    Did_trigger_trap did_trigger_trap = Did_trigger_trap::no;

    vector<Actor_id> actor_bucket;

    switch (trait_)
    {
    case Tomb_trait::undead:

        //Tomb contains major treasure?
        if (appearance_ == Tomb_appearance::marvelous && !is_random_appearance_)
        {
            actor_bucket = {Actor_id::wraith};
        }
        else //Not containing major treasure
        {
            actor_bucket =
            {
                Actor_id::ghost,
                Actor_id::phantasm,
                Actor_id::mummy,
                Actor_id::zombie,
                Actor_id::zombie_axe,
            };

            if (Rnd::one_in(10))
            {
                actor_bucket.push_back(Actor_id::croc_head_mummy);
            }
        }

        Log::add_msg("Something rises from the tomb!", clr_white, false, true);
        did_trigger_trap = Did_trigger_trap::yes;
        break;

    case Tomb_trait::cursed:
        Map::player->get_prop_handler().try_apply_prop(new Prop_cursed(Prop_turns::std));
        did_trigger_trap = Did_trigger_trap::yes;
        break;

    case Tomb_trait::stench:
        if (Rnd::coin_toss())
        {
            Log::add_msg("Fumes burst out from the tomb!", clr_white, false, true);
            Prop* prop    = nullptr;
            Clr fume_clr   = clr_magenta;
            const int RND = Rnd::percent();
            if (Map::dlvl >= MIN_DLVL_HARDER_TRAPS && RND < 20)
            {
                prop    = new Prop_poisoned(Prop_turns::std);
                fume_clr = clr_green_lgt;
            }
            else if (RND < 40)
            {
                prop    = new Prop_diseased(Prop_turns::specific, 50);
                fume_clr = clr_green;
            }
            else
            {
                prop = new Prop_paralyzed(Prop_turns::std);
                prop->turns_left_ *= 2;
            }
            Explosion::run_explosion_at(pos_, Expl_type::apply_prop, Expl_src::misc, 0,
                                      Sfx_id::END, prop, &fume_clr);
        }
        else //Not fumes
        {
            for (int i = 0; i < int(Actor_id::END); ++i)
            {
                const Actor_data_t& d = Actor_data::data[i];
                if (
                    d.intr_props[int(Prop_id::ooze)] &&
                    d.is_auto_spawn_allowed  &&
                    !d.is_unique)
                {
                    actor_bucket.push_back(Actor_id(i));
                }
            }
            Log::add_msg("Something creeps up from the tomb!", clr_white, false, true);
        }
        did_trigger_trap = Did_trigger_trap::yes;
        break;

    case Tomb_trait::END: {} break;
    }

    if (!actor_bucket.empty())
    {
        const size_t  IDX             = Rnd::range(0, actor_bucket.size() - 1);
        const Actor_id actor_id_to_spawn  = actor_bucket[IDX];
        Actor* const  mon             = Actor_factory::mk(actor_id_to_spawn, pos_);
        static_cast<Mon*>(mon)->become_aware(false);
    }

    trait_        = Tomb_trait::END;
    is_trait_known_ = true;

    return did_trigger_trap;
}

//--------------------------------------------------------------------- CHEST
Chest::Chest(const Pos& pos) :
    Rigid               (pos),
    is_open_             (false),
    is_locked_           (false),
    is_trapped_          (false),
    is_trap_status_known_  (false),
    matl_               (Chest_matl(Rnd::range(0, int(Chest_matl::END) - 1))),
    TRAP_DET_LVL        (Rnd::range(0, 2))
{
    const bool  IS_TREASURE_HUNTER  = Player_bon::traits[int(Trait::treasure_hunter)];
    const int   NR_ITEMS_MIN        = Rnd::one_in(10)      ? 0 : 1;
    const int   NR_ITEMS_MAX        = IS_TREASURE_HUNTER  ? 3 : 2;

    item_container_.init(Feature_id::chest, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

    if (!item_container_.items_.empty())
    {
        is_locked_   = Rnd::fraction(4, 10);
        is_trapped_  = Rnd::fraction(6, 10);
    }
}

void Chest::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (item_container_.items_.empty() && is_open_)
        {
            Log::add_msg("The chest is empty.");
            Render::draw_map_and_interface();
        }
        else if (!Map::player->get_prop_handler().allow_see())
        {
            Log::add_msg("There is a chest here.");
            Render::draw_map_and_interface();
        }
        else //Player can see
        {
            if (is_locked_)
            {
                Log::add_msg("The chest is locked.");
                Render::draw_map_and_interface();
            }
            else //Not locked
            {
                if (is_open_)
                {
                    player_loot();
                }
                else
                {
                    open(Map::player);
                }
                Game_time::tick();
            }
        }
    }
}

void Chest::try_find_trap()
{
    assert(is_trapped_);
    assert(!is_open_);

    const bool CAN_DET_TRAP =
        TRAP_DET_LVL == 0                           ||
        Player_bon::traits[int(Trait::perceptive)]   ||
        (TRAP_DET_LVL == 1 && Player_bon::traits[int(Trait::observant)]);

    if (CAN_DET_TRAP)
    {
        is_trap_status_known_ = true;
        Log::add_msg("There appears to be a hidden trap.");
        Render::draw_map_and_interface();
    }
}

void Chest::try_sprain_player()
{
    const int SPRAIN_ONE_IN_N =
        Player_bon::traits[int(Trait::unbreakable)]  ? 10 :
        Player_bon::traits[int(Trait::rugged)]       ? 8  :
        Player_bon::traits[int(Trait::tough)]        ? 6  : 4;

    if (Rnd::one_in(SPRAIN_ONE_IN_N))
    {
        Log::add_msg("I sprain myself.", clr_msg_bad);
        Map::player->hit(Rnd::range(1, 3), Dmg_type::pure);
    }
}

void Chest::player_loot()
{
    Log::add_msg("I search the chest.");

    if (item_container_.items_.empty())
    {
        Log::add_msg("There is nothing of value inside.");
    }
    else //Not empty
    {
        Log::add_msg("There are some items inside.", clr_white, false, true);
        item_container_.open(pos_, Map::player);
    }
}

Did_open Chest::open(Actor* const actor_opening)
{
    is_locked_           = false;
    is_trap_status_known_  = true;

    if (is_open_)
    {
        return Did_open::no;
    }
    else //Chest was not already open
    {
        is_open_ = true;

        if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            Log::add_msg("The chest opens.");
        }

        Render::draw_map_and_interface();

        trigger_trap(actor_opening);

        Render::draw_map_and_interface();

        return Did_open::yes;
    }
}

void Chest::hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)actor;

    switch (dmg_type)
    {
    case Dmg_type::physical:
    {
        switch (dmg_method)
        {
        case Dmg_method::kick:
        {
            if (!Map::player->get_prop_handler().allow_see())
            {
                //If player is blind, call the parent hit function instead
                //(generic kicking)
                Rigid::hit(dmg_type, dmg_method, Map::player);
            }
            else if (!is_locked_)
            {
                Log::add_msg("It is not locked.");
            }
            else if (is_open_)
            {
                Log::add_msg("It is already open.");
            }
            else
            {
                Log::add_msg("I kick the lid.");

                bool props[size_t(Prop_id::END)];
                Map::player->get_prop_handler().get_prop_ids(props);

                if (props[int(Prop_id::weakened)] || matl_ == Chest_matl::iron)
                {
                    try_sprain_player();
                    Log::add_msg("It seems futile.", clr_msg_note, false, true);
                }
                else
                {
                    if (
                        !props[int(Prop_id::blessed)] &&
                        (props[int(Prop_id::cursed)] || Rnd::one_in(3)))
                    {
                        item_container_.destroy_single_fragile();
                    }

                    const int OPEN_ONE_IN_N =
                        Player_bon::traits[int(Trait::unbreakable)]  ? 1 :
                        Player_bon::traits[int(Trait::rugged)]       ? 2 :
                        Player_bon::traits[int(Trait::tough)]        ? 3 : 4;

                    if (Rnd::one_in(OPEN_ONE_IN_N))
                    {
                        Log::add_msg("The lock breaks!", clr_white, false, true);
                        is_locked_ = false;
                    }
                    else
                    {
                        Log::add_msg("The lock resists.");
                        try_sprain_player();
                    }
                }
                Game_time::tick();
            }
        }
        break; //Kick

        default: {}
            break;

        } //dmg_method

    } //Physical damage

    default: {}
        break;

    } //dmg_type

    //TODO: Force lock with weapon
//      Inventory& inv    = Map::player->get_inv();
//      Item* const item  = inv.get_item_in_slot(Slot_id::wielded);
//
//      if(!item) {
//        Log::add_msg(
//          "I attempt to punch the lock open, nearly breaking my hand.",
//          clr_msg_bad);
//        Map::player->hit(1, Dmg_type::pure, false);
//      } else {
//        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);
//
//        if(Rnd::percent() < CHANCE_TO_DMG_WPN) {
//          const string wpn_name = Item_data::get_item_ref(
//                                   *item, Item_ref_type::plain, true);
//
//          Wpn* const wpn = static_cast<Wpn*>(item);
//
//          if(wpn->melee_dmg_plus == 0) {
//            Log::add_msg("My " + wpn_name + " breaks!");
//            delete wpn;
//            inv.get_slot(Slot_id::wielded)->item = nullptr;
//          } else {
//            Log::add_msg("My " + wpn_name + " is damaged!");
//            wpn->melee_dmg_plus--;
//          }
//          return;
//        }
//
//        if(IS_WEAK) {
//          Log::add_msg("It seems futile.");
//        } else {
//          const int CHANCE_TO_OPEN = 40;
//          if(Rnd::percent() < CHANCE_TO_OPEN) {
//            Log::add_msg("I force the lock open!");
//            open();
//          } else {
//            Log::add_msg("The lock resists.");
//          }
//        }
//      }
}

void Chest::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Chest::disarm()
{
    //NOTE: Only an actual attempt at disarming a trap counts as a turn. Just discovering
    //the trap does not end the turn.

    if (is_locked_)
    {
        Log::add_msg("The chest is locked.");
        Render::draw_map_and_interface();
        return;
    }

    if (is_trapped_)
    {
        //First search for a trap if not already known
        if (!is_trap_status_known_)
        {
            try_find_trap();

            //If trap was unknown, give the player a chance to abort
            if (is_trap_status_known_)
            {
                Log::add_msg("Attempt to disarm it? [y/n]");
                Render::draw_map_and_interface();
                const auto answer = Query::yes_or_no();
                if (answer == Yes_no_answer::no)
                {
                    Log::clear_log();
                    Log::add_msg("I leave the chest for now.");
                    Render::draw_map_and_interface();
                    return;
                }
            }
        }

        //Try disarming trap
        if (is_trap_status_known_)
        {
            Log::add_msg("I attempt to disarm the chest...", clr_white, false, true);
            Log::clear_log();

            const Fraction disarm_chance(3, 5);

            if (Rnd::fraction(disarm_chance))
            {
                Log::add_msg("I successfully disarm it!");
                Render::draw_map_and_interface();
                is_trapped_ = false;
            }
            else //Failed to disarm
            {
                trigger_trap(Map::player); //NOTE: This will disable the trap
            }
            Game_time::tick();
            return;
        }
    }

    //This point reached means the chest is not locked, but there were no attempt at
    //disarming a trap (either the trap is not known, or the chest is not trapped)
    if (!is_trap_status_known_)
    {
        //The chest could be trapped
        Log::add_msg("The chest does not appear to be trapped.");
    }
    else if (!is_trapped_)
    {
        //Player knows for sure that the chest is *not* trapped
        Log::add_msg("The chest is not trapped.");
    }

    Render::draw_map_and_interface();
}

Did_trigger_trap Chest::trigger_trap(Actor* const actor)
{
    //Chest is not trapped? (Either already triggered, or chest were created wihout trap)
    if (!is_trapped_)
    {
        is_trap_status_known_ = true;
        return Did_trigger_trap::no;
    }

    const bool IS_SEEN = Map::cells[pos_.x][pos_.y].is_seen_by_player;

    if (IS_SEEN)
    {
        Log::add_msg("A hidden trap on the chest triggers...", clr_white, false, true);
    }

    is_trap_status_known_  = true;
    is_trapped_          = false;

    //Nothing happens?
    bool player_props[size_t(Prop_id::END)];
    Map::player->get_prop_handler().get_prop_ids(player_props);

    const int TRAP_NO_ACTION_ONE_IN_N = player_props[int(Prop_id::blessed)] ? 2 :
                                        player_props[int(Prop_id::cursed)]  ? 20 : 4;

    if (Rnd::one_in(TRAP_NO_ACTION_ONE_IN_N))
    {
        if (IS_SEEN)
        {
            Log::clear_log();
            Log::add_msg("...but nothing happens.");
            return Did_trigger_trap::no;
        }
    }

    //Fire explosion?
    const int FIRE_EXPLOSION_ONE_IN_N = 5;
    if (Map::dlvl >= MIN_DLVL_HARDER_TRAPS && Rnd::one_in(FIRE_EXPLOSION_ONE_IN_N))
    {
        if (IS_SEEN)
        {
            Log::clear_log();
            Log::add_msg("Flames burst out from the chest!", clr_white, false, true);
        }

        Explosion::run_explosion_at(pos_, Expl_type::apply_prop, Expl_src::misc, 0,
                                  Sfx_id::explosion, new Prop_burning(Prop_turns::std));

        return Did_trigger_trap::yes;
    }

    if (actor == Map::player && Rnd::coin_toss())
    {
        //Needle
        Log::add_msg("A needle pierces my skin!", clr_msg_bad, true);
        actor->hit(Rnd::range(1, 3), Dmg_type::physical);

        if (Map::dlvl < MIN_DLVL_HARDER_TRAPS)
        {
            //Weak poison
            actor->get_prop_handler().try_apply_prop(new Prop_poisoned(Prop_turns::specific,
                                                 POISON_DMG_N_TURN * 3));
        }
        else //We're at the deep end of the pool now, apply strong poison
        {
            actor->get_prop_handler().try_apply_prop(new Prop_poisoned(Prop_turns::std));
        }
    }
    else //Is monster, or cointoss is false
    {
        //Fumes
        if (IS_SEEN)
        {
            Log::clear_log();
            Log::add_msg("Fumes burst out from the chest!", clr_white, false, true);
        }

        Prop*     prop    = nullptr;
        Clr       fume_clr = clr_magenta;
        const int RND     = Rnd::percent();

        if (Map::dlvl >= MIN_DLVL_HARDER_TRAPS && RND < 20)
        {
            prop    = new Prop_poisoned(Prop_turns::std);
            fume_clr = clr_green_lgt;
        }
        else if (RND < 40)
        {
            prop    = new Prop_diseased(Prop_turns::specific, 50);
            fume_clr = clr_green;
        }
        else
        {
            prop = new Prop_paralyzed(Prop_turns::std);
            prop->turns_left_ *= 2;
        }
        Explosion::run_explosion_at(pos_, Expl_type::apply_prop, Expl_src::misc, 0,
                                  Sfx_id::END, prop, &fume_clr);
    }
    return Did_trigger_trap::yes;
}

string Chest::get_name(const Article article) const
{
    const bool    IS_EMPTY        = item_container_.items_.empty() && is_open_;
    const bool    IS_KNOWN_TRAP   = is_trapped_ && is_trap_status_known_;
    const string  locked_str       = is_locked_               ? "locked "   : "";
    const string  empty_str        = IS_EMPTY                ? "empty "    : "";
    const string  trap_str         = IS_KNOWN_TRAP           ? "trapped "  : "";
    const string  open_str         = (is_open_ && !IS_EMPTY)  ? "open "     : "";

    string a = "";

    if (article == Article::a)
    {
        a = (
                is_locked_ ||
                (!is_open_ && (matl_ == Chest_matl::wood || IS_KNOWN_TRAP))
            ) ?
            "a " : "an ";
    }
    else
    {
        a = "the ";
    }

    const string matl_str = is_open_ ? "" : matl_ == Chest_matl::wood ? "wooden " : "iron ";

    return a + locked_str + empty_str + open_str + trap_str + matl_str + "chest";
}

Tile_id Chest::get_tile() const
{
    return is_open_ ? Tile_id::chest_open : Tile_id::chest_closed;
}

Clr Chest::get_clr_() const
{
    return matl_ == Chest_matl::wood ? clr_brown_drk : clr_gray;
}

//--------------------------------------------------------------------- FOUNTAIN
Fountain::Fountain(const Pos& pos) :
    Rigid             (pos),
    fountain_effects_  (vector<Fountain_effect>()),
    fountain_matl_     (Fountain_matl::stone),
    nr_drinks_left_     (Rnd::range(3, 4))
{
    if (Rnd::one_in(16))
    {
        fountain_matl_ = Fountain_matl::gold;
    }

    switch (fountain_matl_)
    {
    case Fountain_matl::stone:
        if (Rnd::fraction(5, 6))
        {
            fountain_effects_.push_back(Fountain_effect::refreshing);
        }
        else
        {
            const int   NR_TYPES  = int(Fountain_effect::END);
            const auto  effect    = Fountain_effect(Rnd::range(0, NR_TYPES - 1));
            fountain_effects_.push_back(effect);
        }
        break;

    case Fountain_matl::gold:
        vector<Fountain_effect> effect_bucket
        {
            Fountain_effect::refreshing,
            Fountain_effect::spirit,
            Fountain_effect::vitality,
            Fountain_effect::r_fire,
            Fountain_effect::r_cold,
            Fountain_effect::r_elec,
            Fountain_effect::r_fear,
            Fountain_effect::r_conf
        };

        std::random_shuffle(begin(effect_bucket), end(effect_bucket));

        const int NR_EFFECTS = 3;

        for (int i = 0; i < NR_EFFECTS; ++i)
        {
            fountain_effects_.push_back(effect_bucket[i]);
        }
        break;
    }
}

void Fountain::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                     Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

Clr Fountain::get_clr_() const
{
    if (nr_drinks_left_ <= 0)
    {
        return clr_gray;
    }
    else
    {
        switch (fountain_matl_)
        {
        case Fountain_matl::stone: return clr_blue_lgt;
        case Fountain_matl::gold:  return clr_yellow;
        }
    }
    assert("Failed to get fountain color" && false);
    return clr_black;
}

string Fountain::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    switch (fountain_matl_)
    {
    case Fountain_matl::stone: {}                break;
    case Fountain_matl::gold:  ret += "golden "; break;
    }
    return ret + "fountain";
}

void Fountain::bump(Actor& actor_bumping)
{
    if (!actor_bumping.is_player())
    {
        return;
    }

    if (nr_drinks_left_ <= 0)
    {
        Log::add_msg("The fountain is dried out.");
    }
    else
    {
        Prop_handler& prop_hlr = Map::player->get_prop_handler();

        if (!prop_hlr.allow_see())
        {
            Log::clear_log();
            Log::add_msg("There is a fountain here. Drink from it? [y/n]");
            Render::draw_map_and_interface();

            const auto answer = Query::yes_or_no();

            if (answer == Yes_no_answer::no)
            {
                Log::clear_log();
                Log::add_msg("I leave the fountain for now.");
                Render::draw_map_and_interface();
                return;
            }
        }

        if (!prop_hlr.allow_eat(true))
        {
            return;
        }

        Log::clear_log();
        Log::add_msg("I drink from the fountain...");

        Audio::play(Sfx_id::fountain_drink);

        for (auto effect : fountain_effects_)
        {
            switch (effect)
            {
            case Fountain_effect::refreshing:
            {
                Log::add_msg("It's very refreshing.");
                Map::player->restore_hp(1, false);
                Map::player->restore_spi(1, false);
                Map::player->restore_shock(5, false);
            } break;

            case Fountain_effect::curse:
            {
                prop_hlr.try_apply_prop(new Prop_cursed(Prop_turns::std));
            } break;

            case Fountain_effect::spirit:
            {
                Map::player->restore_spi(2, true, true);
            } break;

            case Fountain_effect::vitality:
            {
                Map::player->restore_hp(2, true, true);
            } break;

            case Fountain_effect::disease:
            {
                prop_hlr.try_apply_prop(new Prop_diseased(Prop_turns::specific, 50));
            } break;

            case Fountain_effect::poison:
            {
                prop_hlr.try_apply_prop(new Prop_poisoned(Prop_turns::std));
            } break;

            case Fountain_effect::frenzy:
            {
                prop_hlr.try_apply_prop(new Prop_frenzied(Prop_turns::std));
            } break;

            case Fountain_effect::paralyze:
            {
                prop_hlr.try_apply_prop(new Prop_paralyzed(Prop_turns::std));
            } break;

            case Fountain_effect::blind:
            {
                prop_hlr.try_apply_prop(new Prop_blind(Prop_turns::std));
            } break;

            case Fountain_effect::faint:
            {
                prop_hlr.try_apply_prop(new Prop_fainted(Prop_turns::specific, 10));
            } break;

            case Fountain_effect::r_fire:
            {
                Prop* const prop = new Prop_rFire(Prop_turns::std);
                prop->turns_left_ *= 2;
                prop_hlr.try_apply_prop(prop);
            } break;

            case Fountain_effect::r_cold:
            {
                Prop* const prop = new Prop_rCold(Prop_turns::std);
                prop->turns_left_ *= 2;
                prop_hlr.try_apply_prop(prop);
            } break;

            case Fountain_effect::r_elec:
            {
                Prop* const prop = new Prop_rElec(Prop_turns::std);
                prop->turns_left_ *= 2;
                prop_hlr.try_apply_prop(prop);
            } break;

            case Fountain_effect::r_conf:
            {
                Prop* const prop = new Prop_rConfusion(Prop_turns::std);
                prop->turns_left_ *= 2;
                prop_hlr.try_apply_prop(prop);
            } break;

            case Fountain_effect::r_fear:
            {
                Prop* const prop = new Prop_rFear(Prop_turns::std);
                prop->turns_left_ *= 2;
                prop_hlr.try_apply_prop(prop);
            } break;

            case Fountain_effect::END: {}
                break;
            }
        }

        --nr_drinks_left_;

        if (nr_drinks_left_ <= 0)
        {
            Log::add_msg("The fountain dries out.");
        }
        Render::draw_map_and_interface();
        Game_time::tick();
    }
}

//--------------------------------------------------------------------- CABINET
Cabinet::Cabinet(const Pos& pos) :
    Rigid   (pos),
    is_open_ (false)
{
    const int IS_EMPTY_N_IN_10  = 5;
    const int NR_ITEMS_MIN      = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
    const int NR_ITEMS_MAX      = Player_bon::traits[int(Trait::treasure_hunter)] ?
                                  2 : 1;

    item_container_.init(Feature_id::cabinet, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
}

void Cabinet::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                    Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Cabinet::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (item_container_.items_.empty() && is_open_)
        {
            Log::add_msg("The cabinet is empty.");
        }
        else if (!Map::player->get_prop_handler().allow_see())
        {
            Log::add_msg("There is a cabinet here.");
            Render::draw_map_and_interface();
        }
        else //Can see
        {
            if (is_open_)
            {
                player_loot();
            }
            else
            {
                open(Map::player);
            }
        }
    }
}

void Cabinet::player_loot()
{
    Log::add_msg("I search the cabinet.");

    if (item_container_.items_.empty())
    {
        Log::add_msg("There is nothing of value inside.");
    }
    else
    {
        Log::add_msg("There are some items inside.", clr_white, false, true);

        item_container_.open(pos_, Map::player);
    }
}

Did_open Cabinet::open(Actor* const actor_opening)
{
    (void)actor_opening;

    if (is_open_)
    {
        return Did_open::no;
    }
    else //Was not already open
    {
        is_open_ = true;

        if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            Render::draw_map_and_interface();
            Log::add_msg("The cabinet opens.");
        }

        Render::draw_map_and_interface(true);

        return Did_open::yes;
    }
}

string Cabinet::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    if (get_burn_state() == Burn_state::burning)
    {
        ret += "burning ";
    }

    return ret + "cabinet";
}

Tile_id Cabinet::get_tile() const
{
    return is_open_ ? Tile_id::cabinet_open : Tile_id::cabinet_closed;
}

Clr Cabinet::get_clr_() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- COCOON
Cocoon::Cocoon(const Pos& pos) :
    Rigid      (pos),
    is_trapped_ (Rnd::fraction(6, 10)),
    is_open_    (false)
{
    if (is_trapped_)
    {
        item_container_.init(Feature_id::cocoon, 0);
    }
    else
    {
        const bool  IS_TREASURE_HUNTER =
            Player_bon::traits[int(Trait::treasure_hunter)];

        const int   IS_EMPTY_N_IN_10    = 6;
        const int   NR_ITEMS_MIN        = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
        const int   NR_ITEMS_MAX        = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);
        item_container_.init(Feature_id::cocoon, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
    }
}

void Cocoon::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Cocoon::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (item_container_.items_.empty() && is_open_)
        {
            Log::add_msg("The cocoon is empty.");
        }
        else if (!Map::player->get_prop_handler().allow_see())
        {
            Log::add_msg("There is a cocoon here.");
            Render::draw_map_and_interface();
        }
        else //Player can see
        {
            if (Map::player->phobias[int(Phobia::spider)])
            {
                Map::player->get_prop_handler().try_apply_prop(
                    new Prop_terrified(Prop_turns::std));
            }

            if (is_open_)
            {
                player_loot();
            }
            else
            {
                open(Map::player);
            }
        }
    }
}

Did_trigger_trap Cocoon::trigger_trap(Actor* const actor)
{
    (void)actor;

    if (is_trapped_)
    {
        const int RND = Rnd::percent();

        if (RND < 15)
        {
            //A dead body
            Log::add_msg("There is a half-dissolved human body inside!");
            Map::player->incr_shock(Shock_lvl::heavy, Shock_src::misc);
            is_trapped_ = false;
            return Did_trigger_trap::yes;
        }
        else if (RND < 50)
        {
            //Spiders
            TRACE << "Attempting to spawn spiders" << endl;
            vector<Actor_id> spawn_bucket;
            for (int i = 0; i < int(Actor_id::END); ++i)
            {
                const Actor_data_t& d = Actor_data::data[i];

                if (
                    d.is_spider                      &&
                    d.actor_size == Actor_size::floor &&
                    d.is_auto_spawn_allowed            &&
                    !d.is_unique)
                {
                    spawn_bucket.push_back(d.id);
                }
            }

            const int NR_CANDIDATES = spawn_bucket.size();
            if (NR_CANDIDATES > 0)
            {
                TRACE << "Spawn candidates found, attempting to place" << endl;
                Log::add_msg("There are spiders inside!");
                const int NR_SPIDERS          = Rnd::range(2, 5);
                const int IDX                 = Rnd::range(0, NR_CANDIDATES - 1);
                const Actor_id actor_id_to_summon = spawn_bucket[IDX];
                Actor_factory::summon(
                    pos_, vector<Actor_id>(NR_SPIDERS, actor_id_to_summon), true);
                is_trapped_ = false;
                return Did_trigger_trap::yes;
            }
        }
    }
    return Did_trigger_trap::no;
}

void Cocoon::player_loot()
{
    Log::add_msg("I search the Cocoon.");

    if (item_container_.items_.empty())
    {
        Log::add_msg("It is empty.");
    }
    else
    {
        Log::add_msg("There are some items inside.", clr_white, false, true);

        item_container_.open(pos_, Map::player);
    }
}

Did_open Cocoon::open(Actor* const actor_opening)
{
    if (is_open_)
    {
        return Did_open::no;
    }
    else //Was not already open
    {
        is_open_ = true;

        Render::draw_map_and_interface(true);

        if (Map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            Log::add_msg("The cocoon opens.");
        }

        trigger_trap(actor_opening);

        return Did_open::yes;
    }
}

string Cocoon::get_name(const Article article) const
{
    string ret = article == Article::a ? "a " : "the ";

    if (get_burn_state() == Burn_state::burning)
    {
        ret += "burning ";
    }

    return ret + "cocoon";
}

Tile_id Cocoon::get_tile() const
{
    return is_open_ ? Tile_id::cocoon_open : Tile_id::cocoon_closed;
}

Clr Cocoon::get_clr_() const
{
    return clr_white;
}
