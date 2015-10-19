#include "feature_rigid.hpp"

#include <string>

#include "init.hpp"
#include "msg_log.hpp"
#include "render.hpp"
#include "map.hpp"
#include "utils.hpp"
#include "popup.hpp"
#include "map_travel.hpp"
#include "save_handling.hpp"
#include "item_factory.hpp"
#include "map_parsing.hpp"
#include "feature_mob.hpp"
#include "drop.hpp"
#include "explosion.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "query.hpp"
#include "pickup.hpp"
#include "dungeon_master.hpp"
#include "sound.hpp"

//--------------------------------------------------------------------- RIGID
Rigid::Rigid(const P& feature_pos) :
    Feature(feature_pos),
    gore_tile_(Tile_id::empty),
    gore_glyph_(0),
    is_bloody_(false),
    burn_state_(Burn_state::not_burned) {}

void Rigid::on_new_turn()
{
    if (burn_state_ == Burn_state::burning)
    {
        clear_gore();

        auto scorch_actor = [](Actor & actor)
        {
            if (&actor == map::player)
            {
                msg_log::add("I am scorched by flames.", clr_msg_bad);
            }
            else
            {
                if (map::player->can_see_actor(actor))
                {
                    msg_log::add(actor.name_the() + " is scorched by flames.", clr_msg_good);
                }
            }

            actor.hit(1, Dmg_type::fire);
        };

        //TODO: Hit dead actors

        //Hit actor standing on feature
        auto* actor = utils::actor_at_pos(pos_);

        if (actor)
        {
            //Occasionally try to set actor on fire, otherwise just do small fire damage
            if (rnd::one_in(4))
            {
                auto& prop_handler = actor->prop_handler();
                prop_handler.try_add_prop(new Prop_burning(Prop_turns::std));
            }
            else
            {
                scorch_actor(*actor);
            }
        }

        //Finished burning?
        int finish_burning_one_in_n = 1;
        int hit_adjacent_one_in_n   = 1;

        switch (matl())
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

        if (rnd::one_in(finish_burning_one_in_n))
        {
            burn_state_ = Burn_state::has_burned;

            if (on_finished_burning() == Was_destroyed::yes)
            {
                return;
            }
        }

        //Hit adjacent features and actors?
        if (rnd::one_in(hit_adjacent_one_in_n))
        {
            const P p(dir_utils::rnd_adj_pos(pos_, false));

            if (utils::is_pos_inside_map(p))
            {
                map::cells[p.x][p.y].rigid->hit(Dmg_type::fire, Dmg_method::elemental);

                actor = utils::actor_at_pos(p);

                if (actor) {scorch_actor(*actor);}
            }
        }

        //Create smoke?
        if (rnd::one_in(20))
        {
            const P p(dir_utils::rnd_adj_pos(pos_, true));

            if (utils::is_pos_inside_map(p))
            {
                if (!cell_check::Blocks_move_cmn(false).check(map::cells[p.x][p.y]))
                {
                    game_time::add_mob(new Smoke(p, 10));
                }
            }
        }
    }

    //Run specialized new turn actions
    on_new_turn_hook();
}

void Rigid::try_start_burning(const bool IS_MSG_ALLOWED)
{
    clear_gore();

    if (burn_state_ == Burn_state::not_burned)
    {
        if (map::is_pos_seen_by_player(pos_) && IS_MSG_ALLOWED)
        {
            std::string str = name(Article::the) + " catches fire.";
            str[0] = toupper(str[0]);
            msg_log::add(str);
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
    msg_log::add(msg_disarm_no_trap);
    render::draw_map_and_interface();
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

    if (actor == map::player && dmg_method == Dmg_method::kick)
    {
        const bool IS_BLIND    = !map::player->prop_handler().allow_see();
        const bool IS_BLOCKING = !can_move_cmn() && id() != Feature_id::stairs;

        if (IS_BLOCKING)
        {
            const std::string rigid_name = IS_BLIND ? "something" : name(Article::a);
            msg_log::add("I kick " + rigid_name + "!");

            if (rnd::one_in(4))
            {
                msg_log::add("I sprain myself.", clr_msg_bad);
                actor->hit(rnd::range(1, 3), Dmg_type::pure);
            }

            if (rnd::one_in(4))
            {
                msg_log::add("I am off-balance.");

                Prop* prop = new Prop_paralyzed(Prop_turns::specific, rnd::range(1, 3));

                actor->prop_handler().try_add_prop(prop, Prop_src::intr, false, Verbosity::silent);
            }

        }
        else //Not blocking
        {
            is_feature_hit = false;
            msg_log::add("I kick the air!");
            audio::play(Sfx_id::miss_medium);
        }
    }

    if (is_feature_hit)
    {
        on_hit(dmg_type, dmg_method, actor);
    }

    if (actor)
    {
        //TODO: This should probably be done elsewhere.
        game_time::tick();
    }
}

int Rigid::shock_when_adj() const
{
    return data().shock_when_adjacent;
}

void Rigid::try_put_gore()
{
    if (data().can_have_gore)
    {
        const int ROLL_GLYPH = rnd::dice(1, 4);

        switch (ROLL_GLYPH)
        {
        case 1:
            gore_glyph_ = ',';
            break;

        case 2:
            gore_glyph_ = '`';
            break;

        case 3:
            gore_glyph_ = 39;
            break;

        case 4:
            gore_glyph_ = ';';
            break;
        }
    }

    const int ROLL_TILE = rnd::dice(1, 8);

    switch (ROLL_TILE)
    {
    case 1:
        gore_tile_ = Tile_id::gore1;
        break;

    case 2:
        gore_tile_ = Tile_id::gore2;
        break;

    case 3:
        gore_tile_ = Tile_id::gore3;
        break;

    case 4:
        gore_tile_ = Tile_id::gore4;
        break;

    case 5:
        gore_tile_ = Tile_id::gore5;
        break;

    case 6:
        gore_tile_ = Tile_id::gore6;
        break;

    case 7:
        gore_tile_ = Tile_id::gore7;
        break;

    case 8:
        gore_tile_ = Tile_id::gore8;
        break;
    }
}

Clr Rigid::clr() const
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
            return burn_state_ == Burn_state::not_burned ?
                   clr_default() : clr_gray_drk;
        }
    }
}

Clr Rigid::clr_bg() const
{
    switch (burn_state_)
    {
    case Burn_state::not_burned:
        return clr_bg_default();

    case Burn_state::burning:
        return Clr {Uint8(rnd::range(32, 255)), 0, 0, 0};

    case Burn_state::has_burned:
        return clr_bg_default();
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

void Rigid::add_light(bool light[MAP_W][MAP_H]) const
{
    if (burn_state_ == Burn_state::burning)
    {
        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P p(pos_ + d);

            if (utils::is_pos_inside_map(p, true))
            {
                light[p.x][p.y] = true;
            }
        }
    }

    add_light_hook(light);
}

//--------------------------------------------------------------------- FLOOR
Floor::Floor(const P& feature_pos) :
    Rigid   (feature_pos),
    type_   (Floor_type::cmn) {}

void Floor::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;

        if (rnd::one_in(3))
        {
            try_start_burning(false);
        }
    }
}

Tile_id Floor::tile() const
{
    return burn_state() == Burn_state::has_burned ?
           Tile_id::scorched_ground :
           data().tile;
}

std::string Floor::name(const Article article) const
{
    std::string ret = article == Article::a ? "" : "the ";

    if (burn_state() == Burn_state::burning)
    {
        ret += "flames";
    }
    else
    {
        if (burn_state() == Burn_state::has_burned) {ret += "scorched ";}

        switch (type_)
        {
        case Floor_type::cmn:
            ret += "stone floor";
            break;

        case Floor_type::cave:
            ret += "cavern floor";
            break;

        case Floor_type::stone_path:
            ret += "stone path";
            break;
        }
    }

    return ret;
}

Clr Floor::clr_default() const
{
    return clr_gray;
}

//--------------------------------------------------------------------- WALL
Wall::Wall(const P& feature_pos) :
    Rigid(feature_pos),
    type_(Wall_type::cmn),
    is_mossy_(false) {}

void Wall::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)actor;

    auto destr_adj_doors = [&]()
    {
        for (const P& d : dir_utils::cardinal_list)
        {
            const P p(pos_ + d);

            if (utils::is_pos_inside_map(p))
            {
                if (map::cells[p.x][p.y].rigid->id() == Feature_id::door)
                {
                    map::put(new Rubble_low(p));
                }
            }
        }
    };

    auto mk_low_rubble_and_rocks = [&]()
    {
        const P p(pos_);
        map::put(new Rubble_low(p)); //NOTE: "this" is now deleted!

        if (rnd::coin_toss()) {item_factory::mk_item_on_floor(Item_id::rock, p);}
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

            if (rnd::coin_toss())
            {
                mk_low_rubble_and_rocks();
            }
            else
            {
                map::put(new Rubble_high(pos_));
            }
        }

        if (dmg_method == Dmg_method::blunt_heavy)
        {
            if (rnd::fraction(1, 4))
            {
                destr_adj_doors();

                if (rnd::coin_toss())
                {
                    mk_low_rubble_and_rocks();
                }
                else
                {
                    map::put(new Rubble_high(pos_));
                }
            }
        }
    }
}

bool Wall::is_tile_any_wall_front(const Tile_id tile)
{
    return
        tile == Tile_id::wall_front         ||
        tile == Tile_id::wall_front_alt1    ||
        tile == Tile_id::wall_front_alt2    ||
        tile == Tile_id::cave_wall_front    ||
        tile == Tile_id::egypt_wall_front;
}

bool Wall::is_tile_any_wall_top(const Tile_id tile)
{
    return
        tile == Tile_id::wall_top       ||
        tile == Tile_id::cave_wall_top  ||
        tile == Tile_id::egypt_wall_top ||
        tile == Tile_id::rubble_high;
}

std::string Wall::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

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

Clr Wall::clr_default() const
{
    if (is_mossy_)
    {
        return clr_green_drk;
    }

    switch (type_)
    {
    case Wall_type::cliff:
        return clr_gray_drk;

    case Wall_type::egypt:
    case Wall_type::cave:
        return clr_brown_gray;

    case Wall_type::cmn:
    case Wall_type::cmn_alt:
        return map::wall_clr;

    case Wall_type::leng_monestary:
        return clr_red;
    }

    assert(false && "Failed to set color");
    return clr_yellow;
}

char Wall::glyph() const
{
    return config::is_text_mode_wall_full_square() ? 10 : '#';
}

Tile_id Wall::front_wall_tile() const
{
    if (config::is_tiles_wall_full_square())
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

Tile_id Wall::top_wall_tile() const
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
    const int RND = rnd::range(1, 6);

    switch (RND)
    {
    case 1:   type_ = Wall_type::cmn_alt; break;

    default:  type_ = Wall_type::cmn;    break;
    }
}

void Wall::set_random_is_moss_grown()
{
    is_mossy_ = rnd::one_in(40);
}

//--------------------------------------------------------------------- HIGH RUBBLE
Rubble_high::Rubble_high(const P& feature_pos) :
    Rigid(feature_pos) {}

void Rubble_high::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                         Actor* const actor)
{
    (void)actor;

    auto mk_low_rubble_and_rocks = [&]()
    {
        const P p(pos_);
        map::put(new Rubble_low(p)); //NOTE: "this" is now deleted!

        if (rnd::coin_toss()) {item_factory::mk_item_on_floor(Item_id::rock, p);}
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
            if (rnd::fraction(2, 4)) {mk_low_rubble_and_rocks();}
        }
    }
}

std::string Rubble_high::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "big pile of debris";
}

Clr Rubble_high::clr_default() const
{
    return map::wall_clr;
}

//--------------------------------------------------------------------- LOW RUBBLE
Rubble_low::Rubble_low(const P& feature_pos) :
    Rigid(feature_pos) {}

void Rubble_low::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                        Actor* const actor)
{
    (void)actor;

    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        try_start_burning(false);
    }
}

std::string Rubble_low::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the)
    {
        ret += "the ";
    }

    if (burn_state() == Burn_state::burning)
    {
        ret += "burning ";
    }

    return ret + "rubble";
}

Clr Rubble_low::clr_default() const
{
    return map::wall_clr;
}

//--------------------------------------------------------------------- BONES
Bones::Bones(const P& feature_pos) :
    Rigid(feature_pos) {}

void Bones::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                   Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Bones::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the)
    {
        ret += "the ";
    }

    return ret + "bones";
}

Clr Bones::clr_default() const
{
    return clr_red;
}

//--------------------------------------------------------------------- GRAVE
Grave_stone::Grave_stone(const P& feature_pos) :
    Rigid(feature_pos) {}

void Grave_stone::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                         Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void Grave_stone::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        msg_log::add(inscr_);
    }
}

std::string Grave_stone::name(const Article article) const
{
    const std::string ret = article == Article::a ?
                            "a " : "the ";

    return ret + "gravestone; " + inscr_;
}

Clr Grave_stone::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- CHURCH BENCH
Church_bench::Church_bench(const P& feature_pos) : Rigid(feature_pos) {}

void Church_bench::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                          Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Church_bench::name(const Article article) const
{
    const std::string ret = article == Article::a ?
                            "a " : "the ";

    return ret + "church bench";
}

Clr Church_bench::clr_default() const
{
    return clr_brown;
}

//--------------------------------------------------------------------- STATUE
Statue::Statue(const P& feature_pos) :
    Rigid   (feature_pos),
    type_   (rnd::one_in(8) ? Statue_type::ghoul : Statue_type::cmn) {}

int Statue::shock_when_adj() const
{
    return type_ == Statue_type::ghoul ? 15 : 0;
}

void Statue::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::physical && dmg_method == Dmg_method::kick)
    {
        assert(actor);

        if (actor->has_prop(Prop_id::weakened))
        {
            msg_log::add("It wiggles a bit.");
            return;
        }

        const Alerts_mon alerts_mon = actor == map::player ?
                                      Alerts_mon::yes :
                                      Alerts_mon::no;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("It topples over.");
        }

        Snd snd("I hear a crash.",
                Sfx_id::END,
                Ignore_msg_if_origin_seen::yes,
                pos_,
                actor,
                Snd_vol::low,
                alerts_mon);

        snd_emit::run(snd);

        const P dst_pos = pos_ + (pos_ - actor->pos);

        map::put(new Rubble_low(pos_)); //NOTE: "this" is now deleted!

        map::player->update_fov();

        render::draw_map_and_interface();

        Actor* const actor_behind = utils::actor_at_pos(dst_pos);

        if (actor_behind && actor_behind->is_alive())
        {
            if (!actor_behind->has_prop(Prop_id::ethereal))
            {
                if (actor_behind == map::player)
                {
                    msg_log::add("It falls on me!");
                }
                else if (map::player->can_see_actor(*actor_behind))
                {
                    msg_log::add("It falls on " + actor_behind->name_a() + ".");
                }

                actor_behind->hit(rnd::dice(3, 5), Dmg_type::physical);
            }
        }

        const auto rigid_id = map::cells[dst_pos.x][dst_pos.y].rigid->id();

        //NOTE: This is kinda hacky, but the rubble is mostly just for decoration anyway,
        //so it doesn't really matter.
        if (
            rigid_id == Feature_id::floor ||
            rigid_id == Feature_id::grass ||
            rigid_id == Feature_id::carpet)
        {
            map::put(new Rubble_low(dst_pos));
        }
    }
}

std::string Statue::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    switch (type_)
    {
    case Statue_type::cmn:
        ret += "statue";
        break;

    case Statue_type::ghoul:
        ret += "statue of a ghoulish creature";
        break;
    }

    return ret;
}

Tile_id Statue::tile() const
{
    return type_ == Statue_type::cmn ?
           Tile_id::witch_or_warlock : Tile_id::ghoul;
}

Clr Statue::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- PILLAR
Pillar::Pillar(const P& feature_pos) :
    Rigid(feature_pos) {}

void Pillar::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Pillar::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "pillar";
}

Clr Pillar::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- MONOLITH
Monolith::Monolith(const P& feature_pos) :
    Rigid(feature_pos) {}

void Monolith::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Monolith::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "monolith";
}

Clr Monolith::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- STALAGMITE
Stalagmite::Stalagmite(const P& feature_pos) :
    Rigid(feature_pos) {}

void Stalagmite::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Stalagmite::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "stalagmite";
}

Clr Stalagmite::clr_default() const
{
    return clr_brown_gray;
}

//--------------------------------------------------------------------- STAIRS
Stairs::Stairs(const P& feature_pos) :
    Rigid(feature_pos) {}

void Stairs::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

void Stairs::on_new_turn_hook()
{
    assert(!map::cells[pos_.x][pos_.y].item);
}

void Stairs::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        const std::vector<std::string> choices {"Descend", "Save and quit", "Cancel"};
        const std::string title  = "A staircase leading downwards";
        const int CHOICE    =  popup::show_menu_msg("", true, choices, title);

        switch (CHOICE)
        {
        case 0:
            map::player->pos = pos_;
            msg_log::clear();
            msg_log::add("I descend the stairs.");
            render::draw_map_and_interface();
            map_travel::go_to_nxt();
            break;

        case 1:
            map::player->pos = pos_;
            save_handling::save_game();
            init::quit_to_main_menu = true;
            break;

        default:
            msg_log::clear();
            render::draw_map_and_interface();
            break;
        }
    }
}

std::string Stairs::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "downward staircase";
}

Clr Stairs::clr_default() const
{
    return clr_yellow;
}

//--------------------------------------------------------------------- BRIDGE
Tile_id Bridge::tile() const
{
    return axis_ == Axis::hor ? Tile_id::hangbridge_hor : Tile_id::hangbridge_ver;
}

void Bridge::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

char Bridge::glyph() const
{
    return axis_ == Axis::hor ? '|' : '=';
}

std::string Bridge::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "bridge";
}

Clr Bridge::clr_default() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- SHALLOW LIQUID
Liquid_shallow::Liquid_shallow(const P& feature_pos) :
    Rigid   (feature_pos),
    type_   (Liquid_type::water) {}

void Liquid_shallow::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                            Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

void Liquid_shallow::bump(Actor& actor_bumping)
{
    if (!actor_bumping.has_prop(Prop_id::ethereal) && !actor_bumping.has_prop(Prop_id::flying))
    {
        actor_bumping.prop_handler().try_add_prop(new Prop_waiting(Prop_turns::std));

        if (actor_bumping.is_player())
        {
            Snd snd("*glop*",
                    Sfx_id::END,
                    Ignore_msg_if_origin_seen::no,
                    actor_bumping.pos,
                    &actor_bumping,
                    Snd_vol::low,
                    Alerts_mon::yes);

            snd_emit::run(snd);
        }
    }
}

std::string Liquid_shallow::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the)
    {
        ret += "the ";
    }

    ret += "a pool of ";

    switch (type_)
    {
    case Liquid_type::water:
        ret += "water";
        break;

    case Liquid_type::acid:
        ret += "acid";
        break;

    case Liquid_type::blood:
        ret += "blood";
        break;

    case Liquid_type::lava:
        ret += "lava";
        break;

    case Liquid_type::mud:
        ret += "mud";
        break;
    }

    return ret;
}

Clr Liquid_shallow::clr_default() const
{
    switch (type_)
    {
    case Liquid_type::water:
        return clr_blue_lgt;
        break;

    case Liquid_type::acid:
        return clr_green_lgt;
        break;

    case Liquid_type::blood:
        return clr_red_lgt;
        break;

    case Liquid_type::lava:
        return clr_orange;
        break;

    case Liquid_type::mud:
        return clr_brown;
        break;
    }

    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- DEEP LIQUID
Liquid_deep::Liquid_deep(const P& feature_pos) :
    Rigid(feature_pos),
    type_(Liquid_type::water) {}

void Liquid_deep::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                         Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

void Liquid_deep::bump(Actor& actor_bumping)
{
    (void)actor_bumping;
}

std::string Liquid_deep::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the) {ret += "the ";}

    ret += "deep ";

    switch (type_)
    {
    case Liquid_type::water:
        ret += "water";
        break;

    case Liquid_type::acid:
        ret += "acid";
        break;

    case Liquid_type::blood:
        ret += "blood";
        break;

    case Liquid_type::lava:
        ret += "lava";
        break;

    case Liquid_type::mud:
        ret += "mud";
        break;
    }

    return ret;
}

Clr Liquid_deep::clr_default() const
{
    switch (type_)
    {
    case Liquid_type::water:
        return clr_blue;
        break;

    case Liquid_type::acid:
        return clr_green;
        break;

    case Liquid_type::blood:
        return clr_red;
        break;

    case Liquid_type::lava:
        return clr_orange;
        break;

    case Liquid_type::mud:
        return clr_brown_drk;
        break;
    }

    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- CHASM
Chasm::Chasm(const P& feature_pos) :
    Rigid(feature_pos) {}

void Chasm::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}


std::string Chasm::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "chasm";
}

Clr Chasm::clr_default() const
{
    return clr_black;
}

//--------------------------------------------------------------------- LEVER
Lever::Lever(const P& feature_pos) :
    Rigid(feature_pos),
    is_position_left_(true),
    door_linked_to_(nullptr)  {}

void Lever::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

std::string Lever::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "lever";
}

Clr Lever::clr_default() const
{
    return is_position_left_ ? clr_gray : clr_white;
}

Tile_id Lever::tile() const
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
//    TRACE << "Door linked to is not broken" << std::endl;
//    if(!door_linked_to_->is_open_) {door_linked_to_->reveal(true);}
//    door_linked_to_->is_open_  = !door_linked_to_->is_open_;
//    door_linked_to_->is_stuck_ = false;
//  }
    map::player->update_fov();
    render::draw_map_and_interface();
    TRACE_FUNC_END;
}

//--------------------------------------------------------------------- ALTAR
Altar::Altar(const P& feature_pos) :
    Rigid(feature_pos) {}

void Altar::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

std::string Altar::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "altar";
}

Clr Altar::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- CARPET
Carpet::Carpet(const P& feature_pos) :
    Rigid(feature_pos) {}

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
    map::put(floor);
    return Was_destroyed::yes;
}

std::string Carpet::name(const Article article) const
{
    std::string ret = article == Article::a ? "" : "the ";
    return ret + "carpet";
}

Clr Carpet::clr_default() const
{
    return clr_red;
}

//--------------------------------------------------------------------- GRASS
Grass::Grass(const P& feature_pos) :
    Rigid(feature_pos),
    type_(Grass_type::cmn)
{
    if (rnd::one_in(5)) {type_ = Grass_type::withered;}
}

void Grass::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}


Tile_id Grass::tile() const
{
    return burn_state() == Burn_state::has_burned ?
           Tile_id::scorched_ground :
           data().tile;
}

std::string Grass::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the) {ret += "the ";}

    switch (burn_state())
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

Clr Grass::clr_default() const
{
    switch (type_)
    {
    case Grass_type::cmn:
        return clr_green;
        break;

    case Grass_type::withered:
        return clr_brown_drk;
        break;
    }

    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- BUSH
Bush::Bush(const P& feature_pos) :
    Rigid(feature_pos),
    type_(Grass_type::cmn)
{
    if (rnd::one_in(5)) {type_ = Grass_type::withered;}
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
    map::put(grass);
    return Was_destroyed::yes;
}

std::string Bush::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    switch (burn_state())
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

Clr Bush::clr_default() const
{
    switch (type_)
    {
    case Grass_type::cmn:
        return clr_green;
        break;

    case Grass_type::withered:
        return clr_brown_drk;
        break;
    }

    assert(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- TREE
Tree::Tree(const P& feature_pos) :
    Rigid(feature_pos) {}

void Tree::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::fire && dmg_method == Dmg_method::elemental)
    {
        (void)actor;

        if (rnd::one_in(3))
        {
            try_start_burning(false);
        }
    }
}

Was_destroyed Tree::on_finished_burning()
{
    if (utils::is_pos_inside_map(pos_, false))
    {
        Grass* const grass = new Grass(pos_);
        grass->set_has_burned();
        map::put(grass);
        return Was_destroyed::yes;
    }

    return Was_destroyed::no;
}

std::string Tree::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    switch (burn_state())
    {
    case Burn_state::not_burned:
        break;

    case Burn_state::burning:
        ret += "burning ";
        break;

    case Burn_state::has_burned:
        ret += "scorched ";
        break;
    }

    return ret + "tree";
}

Clr Tree::clr_default() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- BRAZIER
std::string Brazier::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "brazier";
}

void Brazier::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    if (dmg_type == Dmg_type::physical && dmg_method == Dmg_method::kick)
    {
        assert(actor);

        if (actor->has_prop(Prop_id::weakened))
        {
            msg_log::add("It wiggles a bit.");
            return;
        }

        const Alerts_mon alerts_mon = actor == map::player ?
                                      Alerts_mon::yes :
                                      Alerts_mon::no;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("It topples over.");
        }

        Snd snd("I hear a crash.",
                Sfx_id::END,
                Ignore_msg_if_origin_seen::yes,
                pos_,
                actor,
                Snd_vol::low,
                alerts_mon);

        snd_emit::run(snd);

        const P dst_pos = pos_ + (pos_ - actor->pos);

        const P my_pos = pos_;

        map::put(new Rubble_low(pos_));

        //NOTE: "this" is now deleted!

        map::player->update_fov();
        render::draw_map_and_interface();

        Rigid* const dst_rigid = map::cells[dst_pos.x][dst_pos.y].rigid;

        if (!dst_rigid->data().is_bottomless)
        {
            P expl_pos;

            int expl_d = 0;

            if (dst_rigid->is_projectile_passable())
            {
                expl_pos    = dst_pos;
                expl_d      = -1;
            }
            else
            {
                expl_pos    = my_pos;
                expl_d      = -2;
            }

            //TODO: Emit sound from explosion center

            explosion::run(expl_pos,
                           Expl_type::apply_prop,
                           Expl_src::misc,
                           Emit_expl_snd::no,
                           expl_d,
                           new Prop_burning(Prop_turns::std));
        }
    }
}

void Brazier::add_light_hook(bool light[MAP_W][MAP_H]) const
{
    for (const P& d : dir_utils::dir_list_w_center)
    {
        const P p(pos_ + d);

        light[p.x][p.y] = true;
    }
}

Clr Brazier::clr_default() const
{
    return clr_yellow;
}

//--------------------------------------------------------------------- ITEM CONTAINER
Item_container::Item_container()
{
    for (auto* item : items_)
    {
        delete item;
    }

    items_.clear();
}

Item_container::~Item_container()
{
    for (auto* item : items_)
    {
        delete item;
    }
}

void Item_container::init(const Feature_id feature_id, const int NR_ITEMS_TO_ATTEMPT)
{
    for (auto* item : items_)
    {
        delete item;
    }

    items_.clear();

    if (NR_ITEMS_TO_ATTEMPT > 0)
    {
        while (items_.empty())
        {
            std::vector<Item_id> item_bucket;

            for (size_t i = 0; i < size_t(Item_id::END); ++i)
            {
                Item_data_t& d = item_data::data[i];

                for (auto container_spawn_rule : d.container_spawn_rules)
                {
                    if (
                        container_spawn_rule.feature_id == feature_id &&
                        d.allow_spawn &&
                        rnd::percent() < container_spawn_rule.pct_chance_to_incl)
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

                const int     IDX = rnd::range(0, item_bucket.size() - 1);
                const Item_id  id  = item_bucket[IDX];

                //Check if this item is no longer allowed to spawn (e.g. a unique item)
                if (!item_data::data[int(id)].allow_spawn)
                {
                    item_bucket.erase(begin(item_bucket) + IDX);
                }
                else
                {
                    Item* item = item_factory::mk(item_bucket[IDX]);
                    item_factory::set_item_randomized_properties(item);
                    items_.push_back(item);
                }
            }
        }
    }
}

void Item_container::open(const P& feature_pos, Actor* const actor_opening)
{
    if (actor_opening)
    {
        for (auto* item : items_)
        {
            msg_log::clear();

            const std::string name = item->name(Item_ref_type::plural, Item_ref_inf::yes,
                                                Item_ref_att_inf::wpn_context);

            msg_log::add("Pick up " + name + "? [y/n]");

            const Item_data_t&  data = item->data();

            Wpn* wpn = data.ranged.is_ranged_wpn ? static_cast<Wpn*>(item) : nullptr;

            const bool IS_UNLOADABLE_WPN = wpn                      &&
                                           wpn->nr_ammo_loaded_ > 0 &&
                                           !data.ranged.has_infinite_ammo;

            if (IS_UNLOADABLE_WPN)
            {
                msg_log::add("Unload? [G]");
            }

            render::draw_map_and_interface();

            const Yes_no_answer answer = query::yes_or_no(IS_UNLOADABLE_WPN ? 'G' : -1);

            if (answer == Yes_no_answer::yes)
            {
                audio::play(Sfx_id::pickup);

                Inventory& inv = map::player->inv();

                Item* const thrown_item = inv.slots_[size_t(Slot_id::thrown)].item;

                if (thrown_item && thrown_item->id() == item->id())
                {
                    thrown_item->nr_items_ += item->nr_items_;
                    delete item;
                }
                else //Item does not stack with current thrown weapon
                {
                    inv.put_in_backpack(item);
                }
            }
            else if (answer == Yes_no_answer::no)
            {
                item_drop::drop_item_on_map(feature_pos, *item);
            }
            else //Special key (unload in this case)
            {
                assert(IS_UNLOADABLE_WPN);
                assert(wpn);
                assert(wpn->nr_ammo_loaded_ > 0);
                assert(!data.ranged.has_infinite_ammo);

                audio::play(Sfx_id::pickup);

                Ammo* const spawned_ammo = item_pickup::unload_ranged_wpn(*wpn);

                map::player->inv().put_in_backpack(spawned_ammo);

                item_drop::drop_item_on_map(feature_pos, *wpn);
            }
        }

        msg_log::clear();
        msg_log::add("There are no more items.");
        render::draw_map_and_interface();
    }
    else //Not opened by an actor (probably opened by a spell of opening)
    {
        for (auto* item : items_)
        {
            item_drop::drop_item_on_map(feature_pos, *item);
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
        const Item_data_t& d = item->data();

        if (d.type == Item_type::potion || d.id == Item_id::molotov)
        {
            delete item;
            items_.erase(items_.begin() + i);
            msg_log::add("I hear a muffled shatter.");
            break;
        }
    }
}

//--------------------------------------------------------------------- TOMB
Tomb::Tomb(const P& feature_pos) :
    Rigid                   (feature_pos),
    is_open_                (false),
    is_trait_known_         (false),
    push_lid_one_in_n_      (rnd::range(6, 14)),
    appearance_             (Tomb_appearance::common),
    is_random_appearance_   (false),
    trait_                  (Tomb_trait::END)
{
    //Contained items
    const int NR_ITEMS_MIN  = 0;
    const int NR_ITEMS_MAX  = player_bon::traits[size_t(Trait::treasure_hunter)] ? 2 : 1;

    item_container_.init(Feature_id::tomb, rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

    //Appearance
    if (rnd::one_in(12))
    {
        //Do not base appearance on items (random appearance)
        const int NR_APP    = int(Tomb_appearance::END);
        appearance_         = Tomb_appearance(rnd::range(0, NR_APP - 1));

        is_random_appearance_ = true;
    }
    else //Base appearance on value of contained items
    {
        //Appearance is based on items inside
        for (Item* item : item_container_.items_)
        {
            const Item_value item_value = item->data().value;

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

    if (appearance_ == Tomb_appearance::marvelous && !is_random_appearance_)
    {
        trait_ = Tomb_trait::ghost; //NOTE: Wraith is always chosen
    }
    else //Randomized trait
    {
        std::vector<int> weights(size_t(Tomb_trait::END) + 1, 0);

        weights[size_t(Tomb_trait::ghost)]          = 5;
        weights[size_t(Tomb_trait::other_undead)]   = 3;
        weights[size_t(Tomb_trait::stench)]         = 2;
        weights[size_t(Tomb_trait::cursed)]         = 1;
        weights[size_t(Tomb_trait::END)]            = 2;

        trait_ = Tomb_trait(rnd::weighted_choice(weights));
    }
}

void Tomb::on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Tomb::name(const Article article) const
{
    const bool          IS_EMPTY    = is_open_ && item_container_.items_.empty();
    const std::string   empty_str   = IS_EMPTY                 ? "empty " : "";
    const std::string   open_str    = (is_open_ && !IS_EMPTY)  ? "open "  : "";

    std::string a = "";

    if (article == Article::a)
    {
        a = (is_open_ || appearance_ == Tomb_appearance::ornate) ?  "an " : "a ";
    }
    else
    {
        a = "the ";
    }

    std::string appear_str = "";

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

Tile_id Tomb::tile() const
{
    return is_open_ ? Tile_id::tomb_open : Tile_id::tomb_closed;
}

Clr Tomb::clr_default() const
{
    switch (appearance_)
    {
    case Tomb_appearance::common:
        return clr_gray;

    case Tomb_appearance::ornate:
        return clr_white;

    case Tomb_appearance::marvelous:
        return clr_yellow;

    case Tomb_appearance::END:
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
            msg_log::add("The tomb is empty.");
        }
        else if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("There is a stone box here.");
            render::draw_map_and_interface();
        }
        else //Player can see
        {
            if (is_open_)
            {
                player_loot();
            }
            else //Not open
            {
                msg_log::add("I attempt to push the lid.");

                if (actor_bumping.has_prop(Prop_id::weakened))
                {
                    try_sprain_player();

                    msg_log::add("It seems futile.", clr_msg_note, false, More_prompt_on_msg::yes);
                }
                else //Not weakened
                {
                    const int BON =
                        player_bon::traits[size_t(Trait::unbreakable)]  ? 12  :
                        player_bon::traits[size_t(Trait::rugged)]       ? 8   :
                        player_bon::traits[size_t(Trait::tough)]        ? 4   : 0;

                    TRACE << "Base chance to push lid is: 1 in "
                          << push_lid_one_in_n_ << std::endl;

                    TRACE << "Bonus to roll: "
                          << BON << std::endl;

                    const int ROLL_TOT = rnd::range(1, push_lid_one_in_n_) + BON;

                    TRACE << "Roll + bonus = " << ROLL_TOT << std::endl;

                    bool is_success = false;

                    if (ROLL_TOT < push_lid_one_in_n_ - 9)
                    {
                        msg_log::add("It does not yield at all.");
                    }
                    else if (ROLL_TOT < push_lid_one_in_n_ - 1)
                    {
                        msg_log::add("It resists.");
                    }
                    else if (ROLL_TOT == push_lid_one_in_n_ - 1)
                    {
                        msg_log::add("It moves a little!");
                        push_lid_one_in_n_--;
                    }
                    else
                    {
                        is_success = true;
                    }

                    if (is_success)
                    {
                        open(map::player);
                    }
                    else
                    {
                        try_sprain_player();
                    }
                }
            }

            game_time::tick();
        }
    }
}

void Tomb::try_sprain_player()
{
    const int SPRAIN_ONE_IN_N =
        player_bon::traits[size_t(Trait::unbreakable)]  ? 10 :
        player_bon::traits[size_t(Trait::rugged)]       ? 8  :
        player_bon::traits[size_t(Trait::tough)]        ? 6  : 4;

    if (rnd::one_in(SPRAIN_ONE_IN_N))
    {
        msg_log::add("I sprain myself.", clr_msg_bad);
        map::player->hit(rnd::range(1, 3), Dmg_type::pure);
    }
}

void Tomb::player_loot()
{
    msg_log::add("I peer inside the tomb.");

    if (item_container_.items_.empty())
    {
        msg_log::add("There is nothing of value inside.");
    }
    else
    {
        msg_log::add("There are some items inside.",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        item_container_.open(pos_, map::player);
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

        snd_emit::run({"I hear heavy stone sliding.", Sfx_id::tomb_open,
                       Ignore_msg_if_origin_seen::yes, pos_, nullptr, Snd_vol::high,
                       Alerts_mon::yes
                      });

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            render::draw_map_and_interface();
            msg_log::add("The lid comes off.");
        }

        trigger_trap(actor_opening);

        render::draw_map_and_interface();
        return Did_open::yes;
    }
}

Did_trigger_trap Tomb::trigger_trap(Actor* const actor)
{
    (void)actor;

    Did_trigger_trap did_trigger_trap = Did_trigger_trap::no;

    Actor_id mon_to_spawn = Actor_id::END;

    switch (trait_)
    {
    case Tomb_trait::ghost:
    {
        //Tomb contains major treasure?
        if (appearance_ == Tomb_appearance::marvelous && !is_random_appearance_)
        {
            mon_to_spawn = Actor_id::wraith;
        }
        else //Not containing major treasure
        {
            std::vector<Actor_id> mon_bucket =
            {
                Actor_id::ghost,
                Actor_id::phantasm,
            };

            const size_t IDX = rnd::range(0, mon_bucket.size() - 1);

            mon_to_spawn = mon_bucket[IDX];
        }

        const std::string msg = "The air feels colder.";

        msg_log::add(msg,
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        did_trigger_trap = Did_trigger_trap::yes;
    }
    break;

    case Tomb_trait::other_undead:
    {
        std::vector<Actor_id> mon_bucket =
        {
            Actor_id::mummy,
            Actor_id::croc_head_mummy,
            Actor_id::zombie,
            Actor_id::zombie_axe,
            Actor_id::floating_head
        };

        const size_t IDX = rnd::range(0, mon_bucket.size() - 1);

        mon_to_spawn = mon_bucket[IDX];

        const std::string msg = "Something rises from the tomb!";

        msg_log::add(msg,
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);

        did_trigger_trap = Did_trigger_trap::yes;
    }
    break;

    case Tomb_trait::stench:
    {
        if (rnd::coin_toss())
        {
            if (map::cells[pos_.x][pos_.y].is_seen_by_player)
            {
                msg_log::add("Fumes burst out from the tomb!", clr_white, false,
                             More_prompt_on_msg::yes);
            }

            Snd snd("I hear a burst of gas.", Sfx_id::gas, Ignore_msg_if_origin_seen::yes, pos_,
                    nullptr, Snd_vol::low, Alerts_mon::yes);

            snd_emit::run(snd);

            Prop*       prop        = nullptr;
            Clr         fume_clr    = clr_magenta;
            const int   RND         = rnd::percent();

            if (map::dlvl >= MIN_DLVL_HARDER_TRAPS && RND < 20)
            {
                prop        = new Prop_poisoned(Prop_turns::std);
                fume_clr    = clr_green_lgt;
            }
            else if (RND < 40)
            {
                prop        = new Prop_diseased(Prop_turns::std);
                fume_clr    = clr_green;
            }
            else
            {
                prop = new Prop_paralyzed(Prop_turns::std);
                prop->set_nr_turns_left(prop->nr_turns_left() * 2);
            }

            explosion::run(pos_, Expl_type::apply_prop, Expl_src::misc,
                           Emit_expl_snd::no, 0, prop, &fume_clr);
        }
        else //Not fumes
        {
            std::vector<Actor_id> mon_bucket;

            for (size_t i = 0; i < size_t(Actor_id::END); ++i)
            {
                const Actor_data_t& d = actor_data::data[i];

                if (
                    d.natural_props[size_t(Prop_id::ooze)]  &&
                    d.is_auto_spawn_allowed                 &&
                    !d.is_unique)
                {
                    mon_bucket.push_back(Actor_id(i));
                }
            }

            const size_t IDX = rnd::range(0, mon_bucket.size() - 1);

            mon_to_spawn = mon_bucket[IDX];

            msg_log::add("Something repulsive creeps up from the tomb!",
                         clr_white,
                         false,
                         More_prompt_on_msg::yes);
        }

        did_trigger_trap = Did_trigger_trap::yes;
    }
    break;

    case Tomb_trait::cursed:
    {
        map::player->prop_handler().try_add_prop(new Prop_cursed(Prop_turns::std));
        did_trigger_trap = Did_trigger_trap::yes;
    }
    break;

    case Tomb_trait::END:
        break;
    }

    if (mon_to_spawn != Actor_id::END)
    {
        Actor* const actor = actor_factory::mk(mon_to_spawn, pos_);

        Mon* const mon = static_cast<Mon*>(actor);

        mon->become_aware(false);

        mon->prop_handler().try_add_prop(new Prop_disabled_attack(Prop_turns::specific, 1));
    }

    trait_        = Tomb_trait::END;
    is_trait_known_ = true;

    return did_trigger_trap;
}

//--------------------------------------------------------------------- CHEST
Chest::Chest(const P& feature_pos) :
    Rigid                   (feature_pos),
    is_open_                (false),
    is_locked_              (false),
    is_trapped_             (false),
    is_trap_status_known_   (false),
    matl_                   (Chest_matl::wood),
    TRAP_DET_LVL            (rnd::range(0, 2))
{
    if (map::dlvl >= 3 && rnd::coin_toss())
    {
        matl_ = Chest_matl::iron;
    }

    const bool  IS_TREASURE_HUNTER  = player_bon::traits[size_t(Trait::treasure_hunter)];
    const int   NR_ITEMS_MIN        = 0;
    const int   NR_ITEMS_MAX        = IS_TREASURE_HUNTER    ? 3 : 2;

    item_container_.init(Feature_id::chest, rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

    if (!item_container_.items_.empty())
    {
        is_locked_   = rnd::fraction(4, 10);
        is_trapped_  = rnd::fraction(6, 10);
    }
}

void Chest::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (item_container_.items_.empty() && is_open_)
        {
            msg_log::add("The chest is empty.");
            render::draw_map_and_interface();
        }
        else if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("There is a chest here.");
            render::draw_map_and_interface();
        }
        else //Player can see
        {
            if (is_locked_)
            {
                msg_log::add("The chest is locked.");
                render::draw_map_and_interface();
            }
            else //Not locked
            {
                if (is_open_)
                {
                    player_loot();
                }
                else
                {
                    open(map::player);
                }

                game_time::tick();
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
        player_bon::traits[size_t(Trait::perceptive)]  ||
        (TRAP_DET_LVL == 1 && player_bon::traits[size_t(Trait::observant)]);

    if (CAN_DET_TRAP)
    {
        is_trap_status_known_ = true;
        msg_log::add("There appears to be a hidden trap.");
        render::draw_map_and_interface();
    }
}

void Chest::try_sprain_player()
{
    const int SPRAIN_ONE_IN_N =
        player_bon::traits[size_t(Trait::unbreakable)]  ? 10 :
        player_bon::traits[size_t(Trait::rugged)]       ? 8  :
        player_bon::traits[size_t(Trait::tough)]        ? 6  : 4;

    if (rnd::one_in(SPRAIN_ONE_IN_N))
    {
        msg_log::add("I sprain myself.", clr_msg_bad);
        map::player->hit(rnd::range(1, 3), Dmg_type::pure);
    }
}

void Chest::player_loot()
{
    msg_log::add("I search the chest.");

    if (item_container_.items_.empty())
    {
        msg_log::add("There is nothing of value inside.");
    }
    else //Not empty
    {
        msg_log::add("There are some items inside.", clr_white, false, More_prompt_on_msg::yes);
        item_container_.open(pos_, map::player);
    }
}

Did_open Chest::open(Actor* const actor_opening)
{
    is_locked_              = false;
    is_trap_status_known_   = true;

    if (is_open_)
    {
        return Did_open::no;
    }
    else //Chest was not already open
    {
        is_open_ = true;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("The chest opens.");
        }

        render::draw_map_and_interface();

        trigger_trap(actor_opening);

        render::draw_map_and_interface();

        return Did_open::yes;
    }
}

void Chest::hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    switch (dmg_type)
    {
    case Dmg_type::physical:
    {
        switch (dmg_method)
        {
        case Dmg_method::kick:
        {
            if (!map::player->prop_handler().allow_see())
            {
                //If player is blind, call the parent hit function instead
                //(generic kicking)
                Rigid::hit(dmg_type, dmg_method, map::player);
            }
            else if (!is_locked_)
            {
                msg_log::add("It is not locked.");
            }
            else if (is_open_)
            {
                msg_log::add("It is already open.");
            }
            else
            {
                assert(actor);

                msg_log::add("I kick the lid.");

                if (actor->has_prop(Prop_id::weakened) || matl_ == Chest_matl::iron)
                {
                    try_sprain_player();
                    msg_log::add("It seems futile.", clr_msg_note, false, More_prompt_on_msg::yes);
                }
                else //Chest can be bashed open
                {
                    if (
                        !actor->has_prop(Prop_id::blessed) &&
                        (actor->has_prop(Prop_id::cursed) || rnd::one_in(3)))
                    {
                        item_container_.destroy_single_fragile();
                    }

                    const int OPEN_ONE_IN_N =
                        player_bon::traits[size_t(Trait::unbreakable)]  ? 1 :
                        player_bon::traits[size_t(Trait::rugged)]       ? 2 :
                        player_bon::traits[size_t(Trait::tough)]        ? 3 : 4;

                    if (rnd::one_in(OPEN_ONE_IN_N))
                    {
                        msg_log::add("The lock breaks!", clr_white, false,
                                     More_prompt_on_msg::yes);
                        is_locked_ = false;
                    }
                    else
                    {
                        msg_log::add("The lock resists.");
                        try_sprain_player();
                    }
                }

                game_time::tick();
            }
        }
        break; //Kick

        default:
            break;

        } //dmg_method

    } //Physical damage

    default:
        break;

    } //dmg_type

    //TODO: Force lock with weapon
//      Inventory& inv    = map::player->inv();
//      Item* const item  = inv.item_in_slot(Slot_id::wpn);
//
//      if(!item) {
//        msg_log::add(
//          "I attempt to punch the lock open, nearly breaking my hand.",
//          clr_msg_bad);
//        map::player->hit(1, Dmg_type::pure, false);
//      } else {
//        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);
//
//        if(rnd::percent() < CHANCE_TO_DMG_WPN) {
//          const std::string wpn_name = item_data::item_ref(
//                                   *item, Item_ref_type::plain, true);
//
//          Wpn* const wpn = static_cast<Wpn*>(item);
//
//          if(wpn->melee_dmg_plus == 0) {
//            msg_log::add("My " + wpn_name + " breaks!");
//            delete wpn;
//            inv.slot(Slot_id::wpn)->item = nullptr;
//          } else {
//            msg_log::add("My " + wpn_name + " is damaged!");
//            wpn->melee_dmg_plus--;
//          }
//          return;
//        }
//
//        if(IS_WEAK) {
//          msg_log::add("It seems futile.");
//        } else {
//          const int CHANCE_TO_OPEN = 40;
//          if(rnd::percent() < CHANCE_TO_OPEN) {
//            msg_log::add("I force the lock open!");
//            open();
//          } else {
//            msg_log::add("The lock resists.");
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
        msg_log::add("The chest is locked.");
        render::draw_map_and_interface();
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
                msg_log::add("Attempt to disarm it? [y/n]");
                render::draw_map_and_interface();
                const auto answer = query::yes_or_no();

                if (answer == Yes_no_answer::no)
                {
                    msg_log::clear();
                    msg_log::add("I leave the chest for now.");
                    render::draw_map_and_interface();
                    return;
                }
            }
        }

        //Try disarming trap
        if (is_trap_status_known_)
        {
            msg_log::add("I attempt to disarm the chest...", clr_white, false,
                         More_prompt_on_msg::yes);
            msg_log::clear();

            const Fraction disarm_chance(4, 5);

            if (rnd::fraction(disarm_chance))
            {
                msg_log::add("I successfully disarm it!");
                render::draw_map_and_interface();
                is_trapped_ = false;

                dungeon_master::incr_player_xp(XP_FOR_DISRM_TRAP);
            }
            else //Failed to disarm
            {
                trigger_trap(map::player); //NOTE: This will disable the trap
            }

            game_time::tick();
            return;
        }
    }

    //This point reached means the chest is not locked, but there were no attempt at
    //disarming a trap (either the trap is not known, or the chest is not trapped)
    if (!is_trap_status_known_)
    {
        //The chest could be trapped
        msg_log::add("The chest does not appear to be trapped.");
    }
    else if (!is_trapped_)
    {
        //Player knows for sure that the chest is *not* trapped
        msg_log::add("The chest is not trapped.");
    }

    render::draw_map_and_interface();
}

Did_trigger_trap Chest::trigger_trap(Actor* const actor)
{
    //TODO: This function seems to assume that an actor is opening (not nullptr). But could it not
    //be openened by e.g. by a spell of opening? Make sure this function handles that.

    assert(actor);

    if (!actor) //Robustness for release builds, but see "TODO" above.
    {
        is_trap_status_known_ = true;
        return Did_trigger_trap::no;
    }

    //Chest is not trapped? (Either already triggered, or chest were created wihout trap)
    if (!is_trapped_)
    {
        is_trap_status_known_ = true;
        return Did_trigger_trap::no;
    }

    const bool IS_SEEN = map::cells[pos_.x][pos_.y].is_seen_by_player;

    if (IS_SEEN)
    {
        msg_log::add("A hidden trap on the chest triggers...",
                     clr_white,
                     false,
                     More_prompt_on_msg::yes);
    }

    is_trap_status_known_   = true;
    is_trapped_             = false;

    //Nothing happens?
    const int TRAP_NO_ACTION_ONE_IN_N = actor->has_prop(Prop_id::blessed) ? 2 :
                                        actor->has_prop(Prop_id::cursed)  ? 20 : 4;

    if (rnd::one_in(TRAP_NO_ACTION_ONE_IN_N))
    {
        if (IS_SEEN)
        {
            msg_log::clear();
            msg_log::add("...but nothing happens.");
            return Did_trigger_trap::no;
        }
    }

    //Fire explosion?
    const int FIRE_EXPLOSION_ONE_IN_N = 5;

    if (map::dlvl >= MIN_DLVL_HARDER_TRAPS && rnd::one_in(FIRE_EXPLOSION_ONE_IN_N))
    {
        if (IS_SEEN)
        {
            msg_log::clear();
            msg_log::add("Flames burst out from the chest!",
                         clr_white,
                         false,
                         More_prompt_on_msg::yes);
        }

        explosion::run(pos_,
                       Expl_type::apply_prop,
                       Expl_src::misc,
                       Emit_expl_snd::yes,
                       0,
                       new Prop_burning(Prop_turns::std));

        return Did_trigger_trap::yes;
    }

    if (actor == map::player && rnd::coin_toss())
    {
        //Needle
        msg_log::add("A needle pierces my skin!",
                     clr_msg_bad,
                     true);

        actor->hit(rnd::range(1, 3), Dmg_type::physical);

        if (map::dlvl < MIN_DLVL_HARDER_TRAPS)
        {
            //Weak poison
            actor->prop_handler().try_add_prop(new Prop_poisoned(Prop_turns::specific,
                                               POISON_DMG_N_TURN * 3));
        }
        else //We're at the deep end of the pool now, apply strong poison
        {
            actor->prop_handler().try_add_prop(new Prop_poisoned(Prop_turns::std));
        }
    }
    else //Is monster, or cointoss is false
    {
        //Fumes
        if (IS_SEEN)
        {
            msg_log::clear();
            msg_log::add("Fumes burst out from the chest!",
                         clr_white,
                         false,
                         More_prompt_on_msg::yes);
        }

        Snd snd("I hear a burst of gas.",
                Sfx_id::gas,
                Ignore_msg_if_origin_seen::yes,
                pos_,
                nullptr,
                Snd_vol::low,
                Alerts_mon::yes);

        snd_emit::run(snd);

        Prop*       prop        = nullptr;
        Clr         fume_clr    = clr_magenta;
        const int   RND         = rnd::percent();

        if (map::dlvl >= MIN_DLVL_HARDER_TRAPS && RND < 20)
        {
            prop        = new Prop_poisoned(Prop_turns::std);
            fume_clr    = clr_green_lgt;
        }
        else if (RND < 40)
        {
            prop        = new Prop_diseased(Prop_turns::std);
            fume_clr    = clr_green;
        }
        else
        {
            prop = new Prop_paralyzed(Prop_turns::std);
            prop->set_nr_turns_left(prop->nr_turns_left() * 2);
        }

        explosion::run(pos_,
                       Expl_type::apply_prop,
                       Expl_src::misc,
                       Emit_expl_snd::no,
                       0,
                       prop,
                       &fume_clr);
    }

    return Did_trigger_trap::yes;
}

std::string Chest::name(const Article article) const
{
    const bool          IS_EMPTY        = item_container_.items_.empty() && is_open_;
    const bool          IS_KNOWN_TRAP   = is_trapped_ && is_trap_status_known_;
    const std::string   locked_str      = is_locked_                ? "locked "   : "";
    const std::string   empty_str       = IS_EMPTY                  ? "empty "    : "";
    const std::string   trap_str        = IS_KNOWN_TRAP             ? "trapped "  : "";
    const std::string   open_str        = (is_open_ && !IS_EMPTY)   ? "open "     : "";

    std::string a = "";

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

    const std::string matl_str = is_open_ ? "" : matl_ == Chest_matl::wood ? "wooden " : "iron ";

    return a + locked_str + empty_str + open_str + trap_str + matl_str + "chest";
}

Tile_id Chest::tile() const
{
    return is_open_ ? Tile_id::chest_open : Tile_id::chest_closed;
}

Clr Chest::clr_default() const
{
    return matl_ == Chest_matl::wood ? clr_brown_drk : clr_gray;
}

//--------------------------------------------------------------------- FOUNTAIN
Fountain::Fountain(const P& feature_pos) :
    Rigid               (feature_pos),
    fountain_effects_   (std::vector<Fountain_effect>()),
    fountain_matl_      (Fountain_matl::stone),
    nr_drinks_left_     (rnd::range(3, 4))
{
    if (rnd::one_in(16))
    {
        fountain_matl_ = Fountain_matl::gold;
    }

    switch (fountain_matl_)
    {
    case Fountain_matl::stone:
        if (rnd::fraction(5, 6))
        {
            fountain_effects_.push_back(Fountain_effect::refreshing);
        }
        else
        {
            const int   NR_TYPES  = int(Fountain_effect::END);
            const auto  effect    = Fountain_effect(rnd::range(0, NR_TYPES - 1));
            fountain_effects_.push_back(effect);
        }
        break;

    case Fountain_matl::gold:
        std::vector<Fountain_effect> effect_bucket
        {
            Fountain_effect::refreshing,
            Fountain_effect::spirit,
            Fountain_effect::vitality,
            Fountain_effect::rFire,
            Fountain_effect::rElec,
            Fountain_effect::rFear,
            Fountain_effect::rConf
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

Clr Fountain::clr_default() const
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

std::string Fountain::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

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
        msg_log::add("The fountain is dried out.");
    }
    else
    {
        Prop_handler& prop_hlr = map::player->prop_handler();

        if (!prop_hlr.allow_see())
        {
            msg_log::clear();
            msg_log::add("There is a fountain here. Drink from it? [y/n]");
            render::draw_map_and_interface();

            const auto answer = query::yes_or_no();

            if (answer == Yes_no_answer::no)
            {
                msg_log::clear();
                msg_log::add("I leave the fountain for now.");
                render::draw_map_and_interface();
                return;
            }
        }

        if (!prop_hlr.allow_eat(Verbosity::verbose))
        {
            return;
        }

        msg_log::clear();
        msg_log::add("I drink from the fountain...");

        audio::play(Sfx_id::fountain_drink);

        for (auto effect : fountain_effects_)
        {
            switch (effect)
            {
            case Fountain_effect::refreshing:
                msg_log::add("It's very refreshing.");
                map::player->restore_hp(1, false, Verbosity::silent);
                map::player->restore_spi(1, false, Verbosity::silent);
                map::player->restore_shock(5, false);
                break;

            case Fountain_effect::curse:
                prop_hlr.try_add_prop(new Prop_cursed(Prop_turns::std));
                break;

            case Fountain_effect::spirit:
                map::player->restore_spi(2, true);
                break;

            case Fountain_effect::vitality:
                map::player->restore_hp(2, true);
                break;

            case Fountain_effect::disease:
                prop_hlr.try_add_prop(new Prop_diseased(Prop_turns::std));
                break;

            case Fountain_effect::poison:
                prop_hlr.try_add_prop(new Prop_poisoned(Prop_turns::std));
                break;

            case Fountain_effect::frenzy:
                prop_hlr.try_add_prop(new Prop_frenzied(Prop_turns::std));
                break;

            case Fountain_effect::paralyze:
                prop_hlr.try_add_prop(new Prop_paralyzed(Prop_turns::std));
                break;

            case Fountain_effect::blind:
                prop_hlr.try_add_prop(new Prop_blind(Prop_turns::std));
                break;

            case Fountain_effect::faint:
                prop_hlr.try_add_prop(new Prop_fainted(Prop_turns::specific, 10));
                break;

            case Fountain_effect::rFire:
            {
                Prop* const prop = new Prop_rFire(Prop_turns::std);
                prop->set_nr_turns_left(prop->nr_turns_left() * 2);
                prop_hlr.try_add_prop(prop);
            }
            break;

            case Fountain_effect::rElec:
            {
                Prop* const prop = new Prop_rElec(Prop_turns::std);
                prop->set_nr_turns_left(prop->nr_turns_left() * 2);
                prop_hlr.try_add_prop(prop);
            } break;

            case Fountain_effect::rConf:
            {
                Prop* const prop = new Prop_rConf(Prop_turns::std);
                prop->set_nr_turns_left(prop->nr_turns_left() * 2);
                prop_hlr.try_add_prop(prop);
            }
            break;

            case Fountain_effect::rFear:
            {
                Prop* const prop = new Prop_rFear(Prop_turns::std);
                prop->set_nr_turns_left(prop->nr_turns_left() * 2);
                prop_hlr.try_add_prop(prop);
            }
            break;

            case Fountain_effect::END:
                break;
            }
        }

        --nr_drinks_left_;

        if (nr_drinks_left_ <= 0)
        {
            msg_log::add("The fountain dries out.");
        }

        render::draw_map_and_interface();
        game_time::tick();
    }
}

//--------------------------------------------------------------------- CABINET
Cabinet::Cabinet(const P& feature_pos) :
    Rigid       (feature_pos),
    is_open_    (false)
{
    const int IS_EMPTY_N_IN_10  = 5;
    const int NR_ITEMS_MIN      = rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
    const int NR_ITEMS_MAX      = player_bon::traits[size_t(Trait::treasure_hunter)] ? 2 : 1;

    item_container_.init(Feature_id::cabinet, rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
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
            msg_log::add("The cabinet is empty.");
        }
        else if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("There is a cabinet here.");
            render::draw_map_and_interface();
        }
        else //Can see
        {
            if (is_open_)
            {
                player_loot();
            }
            else
            {
                open(map::player);
            }
        }
    }
}

void Cabinet::player_loot()
{
    msg_log::add("I search the cabinet.");

    if (item_container_.items_.empty())
    {
        msg_log::add("There is nothing of value inside.");
    }
    else
    {
        msg_log::add("There are some items inside.", clr_white, false,
                     More_prompt_on_msg::yes);

        item_container_.open(pos_, map::player);
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

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            render::draw_map_and_interface();
            msg_log::add("The cabinet opens.");
        }

        render::draw_map_and_interface(true);

        return Did_open::yes;
    }
}

std::string Cabinet::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    if (burn_state() == Burn_state::burning)
    {
        ret += "burning ";
    }

    return ret + "cabinet";
}

Tile_id Cabinet::tile() const
{
    return is_open_ ? Tile_id::cabinet_open : Tile_id::cabinet_closed;
}

Clr Cabinet::clr_default() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- COCOON
Cocoon::Cocoon(const P& feature_pos) :
    Rigid(feature_pos),
    is_trapped_(rnd::fraction(6, 10)),
    is_open_(false)
{
    if (is_trapped_)
    {
        item_container_.init(Feature_id::cocoon, 0);
    }
    else
    {
        const bool IS_TREASURE_HUNTER = player_bon::traits[size_t(Trait::treasure_hunter)];

        const Fraction fraction_empty(6, 10);

        const int NR_ITEMS_MIN = rnd::fraction(fraction_empty) ? 0 : 1;

        const int NR_ITEMS_MAX = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);

        item_container_.init(Feature_id::cocoon, rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
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
            msg_log::add("The cocoon is empty.");
        }
        else if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("There is a cocoon here.");
            render::draw_map_and_interface();
        }
        else //Player can see
        {
            if (insanity::has_sympt(Ins_sympt_id::phobia_spider))
            {
                map::player->prop_handler().try_add_prop( new Prop_terrified(Prop_turns::std));
            }

            if (is_open_)
            {
                player_loot();
            }
            else
            {
                open(map::player);
            }
        }
    }
}

Did_trigger_trap Cocoon::trigger_trap(Actor* const actor)
{
    (void)actor;

    if (is_trapped_)
    {
        const int RND = rnd::percent();

        if (RND < 15)
        {
            //A dead body
            msg_log::add("There is a half-dissolved human body inside!");
            map::player->incr_shock(Shock_lvl::heavy, Shock_src::misc);
            is_trapped_ = false;
            return Did_trigger_trap::yes;
        }
        else if (RND < 50)
        {
            //Spiders
            TRACE << "Attempting to spawn spiders" << std::endl;
            std::vector<Actor_id> spawn_bucket;

            for (int i = 0; i < int(Actor_id::END); ++i)
            {
                const Actor_data_t& d = actor_data::data[i];

                if (
                    d.is_spider                         &&
                    d.actor_size == Actor_size::floor   &&
                    d.is_auto_spawn_allowed             &&
                    !d.is_unique)
                {
                    spawn_bucket.push_back(d.id);
                }
            }

            const int NR_CANDIDATES = spawn_bucket.size();

            if (NR_CANDIDATES > 0)
            {
                TRACE << "Spawn candidates found, attempting to place" << std::endl;
                msg_log::add("There are spiders inside!");
                const int NR_SPIDERS          = rnd::range(2, 5);
                const int IDX                 = rnd::range(0, NR_CANDIDATES - 1);
                const Actor_id actor_id_to_summon = spawn_bucket[IDX];

                const std::vector<Actor_id> ids_to_summon(NR_SPIDERS, actor_id_to_summon);

                actor_factory::summon(pos_, ids_to_summon, true);

                is_trapped_ = false;
                return Did_trigger_trap::yes;
            }
        }
    }

    return Did_trigger_trap::no;
}

void Cocoon::player_loot()
{
    msg_log::add("I search the Cocoon.");

    if (item_container_.items_.empty())
    {
        msg_log::add("It is empty.");
    }
    else
    {
        msg_log::add("There are some items inside.", clr_white, false, More_prompt_on_msg::yes);

        item_container_.open(pos_, map::player);
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

        render::draw_map_and_interface(true);

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("The cocoon opens.");
        }

        trigger_trap(actor_opening);

        return Did_open::yes;
    }
}

std::string Cocoon::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    if (burn_state() == Burn_state::burning)
    {
        ret += "burning ";
    }

    return ret + "cocoon";
}

Tile_id Cocoon::tile() const
{
    return is_open_ ? Tile_id::cocoon_open : Tile_id::cocoon_closed;
}

Clr Cocoon::clr_default() const
{
    return clr_white;
}
