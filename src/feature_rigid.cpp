#include "feature_rigid.hpp"

#include <string>

#include "init.hpp"
#include "msg_log.hpp"
#include "render.hpp"
#include "map.hpp"
#include "popup.hpp"
#include "map_travel.hpp"
#include "saving.hpp"
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
Rigid::Rigid(const P& p) :
    Feature                     (p),
    gore_tile_                  (TileId::empty),
    gore_glyph_                 (0),
    is_bloody_                  (false),
    burn_state_                 (BurnState::not_burned),
    nr_turns_color_corrupted_   (-1) {}

void Rigid::on_new_turn()
{
    if (nr_turns_color_corrupted_ > 0)
    {
        --nr_turns_color_corrupted_;
    }

    if (burn_state_ == BurnState::burning)
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

            actor.hit(1, DmgType::fire);
        };

        //TODO: Hit dead actors

        //Hit actor standing on feature
        auto* actor = map::actor_at_pos(pos_);

        if (actor)
        {
            //Occasionally try to set actor on fire, otherwise just do small fire damage
            if (rnd::one_in(4))
            {
                auto& prop_handler = actor->prop_handler();
                prop_handler.try_add(new PropBurning(PropTurns::std));
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
            burn_state_ = BurnState::has_burned;

            if (on_finished_burning() == WasDestroyed::yes)
            {
                return;
            }
        }

        //Hit actors and adjacent features?
        if (rnd::one_in(hit_adjacent_one_in_n))
        {
            actor = map::actor_at_pos(pos_);

            if (actor)
            {
                scorch_actor(*actor);
            }

            const P p(dir_utils::rnd_adj_pos(pos_, false));

            if (map::is_pos_inside_map(p))
            {
                map::cells[p.x][p.y].rigid->hit(DmgType::fire, DmgMethod::elemental);
            }
        }

        //Create smoke?
        if (rnd::one_in(20))
        {
            const P p(dir_utils::rnd_adj_pos(pos_, true));

            if (map::is_pos_inside_map(p))
            {
                if (!cell_check::BlocksMoveCmn(false).check(map::cells[p.x][p.y]))
                {
                    game_time::add_mob(new Smoke(p, 10));
                }
            }
        }
    }

    //Run specialized new turn actions
    on_new_turn_hook();
}

void Rigid::try_start_burning(const bool is_msg_allowed)
{
    clear_gore();

    if (burn_state_ == BurnState::not_burned)
    {
        if (map::is_pos_seen_by_player(pos_) && is_msg_allowed)
        {
            std::string str = name(Article::the) + " catches fire.";
            str[0] = toupper(str[0]);
            msg_log::add(str);
        }

        burn_state_ = BurnState::burning;
    }
}

WasDestroyed Rigid::on_finished_burning()
{
    return WasDestroyed::no;
}

void Rigid::disarm()
{
    msg_log::add(msg_disarm_no_trap);
    render::draw_map_state();
}

DidOpen Rigid::open(Actor* const actor_opening)
{
    (void)actor_opening;
    return DidOpen::no;
}

DidTriggerTrap Rigid::trigger_trap(Actor* const actor)
{
    (void)actor;
    return DidTriggerTrap::no;
}

void Rigid::hit(const DmgType dmg_type, const DmgMethod dmg_method, Actor* actor)
{
    bool is_feature_hit = true;

    if (actor == map::player && dmg_method == DmgMethod::kick)
    {
        const bool can_see_feature  = !map::cells[pos_.x][pos_.y].is_seen_by_player;
        const bool is_blocking      = !can_move_cmn() && id() != FeatureId::stairs;

        if (is_blocking)
        {
            const std::string rigid_name = can_see_feature ? "something" : name(Article::a);

            msg_log::add("I kick " + rigid_name + "!");

            if (!actor->has_prop(PropId::frenzied) && rnd::one_in(4))
            {
                msg_log::add("I sprain myself.", clr_msg_bad);
                actor->hit(rnd::range(1, 3), DmgType::pure);
            }

            if (rnd::one_in(4))
            {
                msg_log::add("I am off-balance.");

                Prop* prop = new PropParalyzed(PropTurns::specific, rnd::range(1, 3));

                actor->prop_handler().try_add(prop, PropSrc::intr, false, Verbosity::silent);
            }

        }
        else //Not blocking
        {
            is_feature_hit = false;
            msg_log::add("I kick the air!");
            audio::play(SfxId::miss_medium);
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
    int shock = base_shock_when_adj();

    if (nr_turns_color_corrupted_ > 0)
    {
        shock += 1;
    }

    return shock;
}

int Rigid::base_shock_when_adj() const
{
    return data().shock_when_adjacent;
}

void Rigid::try_put_gore()
{
    if (data().can_have_gore)
    {
        const int roll_glyph = rnd::dice(1, 4);

        switch (roll_glyph)
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

    const int roll_tile = rnd::dice(1, 8);

    switch (roll_tile)
    {
    case 1:
        gore_tile_ = TileId::gore1;
        break;

    case 2:
        gore_tile_ = TileId::gore2;
        break;

    case 3:
        gore_tile_ = TileId::gore3;
        break;

    case 4:
        gore_tile_ = TileId::gore4;
        break;

    case 5:
        gore_tile_ = TileId::gore5;
        break;

    case 6:
        gore_tile_ = TileId::gore6;
        break;

    case 7:
        gore_tile_ = TileId::gore7;
        break;

    case 8:
        gore_tile_ = TileId::gore8;
        break;
    }
}

void Rigid::corrupt_color()
{
    nr_turns_color_corrupted_ = rnd::range(200, 220);
}

Clr Rigid::clr() const
{
    if (burn_state_ == BurnState::burning)
    {
        return clr_orange;
    }
    else
    {
        if (nr_turns_color_corrupted_ > 0)
        {
            Clr clr = clr_magenta_lgt;

            clr.r = rnd::range(40, 255);
            clr.g = rnd::range(40, 255);
            clr.b = rnd::range(40, 255);

            return clr;
        }
        else if (is_bloody_)
        {
            return clr_red_lgt;
        }
        else
        {
            return burn_state_ == BurnState::not_burned ?
                clr_default() : clr_gray_drk;
        }
    }
}

Clr Rigid::clr_bg() const
{
    switch (burn_state_)
    {
    case BurnState::not_burned:
        return clr_bg_default();

    case BurnState::burning:
        return Clr {Uint8(rnd::range(32, 255)), 0, 0, 0};

    case BurnState::has_burned:
        return clr_bg_default();
    }

    ASSERT(false && "Failed to set color");
    return clr_yellow;
}

void Rigid::clear_gore()
{
    gore_tile_   = TileId::empty;
    gore_glyph_  = ' ';
    is_bloody_   = false;
}

void Rigid::add_light(bool light[map_w][map_h]) const
{
    if (burn_state_ == BurnState::burning)
    {
        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P p(pos_ + d);

            if (map::is_pos_inside_map(p, true))
            {
                light[p.x][p.y] = true;
            }
        }
    }

    add_light_hook(light);
}

//--------------------------------------------------------------------- FLOOR
Floor::Floor(const P& p) :
    Rigid   (p),
    type_   (FloorType::cmn) {}

void Floor::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
{
    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        (void)actor;

        if (rnd::one_in(3))
        {
            try_start_burning(false);
        }
    }
}

TileId Floor::tile() const
{
    return burn_state() == BurnState::has_burned ?
        TileId::scorched_ground :
        data().tile;
}

std::string Floor::name(const Article article) const
{
    std::string ret = article == Article::a ? "" : "the ";

    if (burn_state() == BurnState::burning)
    {
        ret += "flames";
    }
    else
    {
        if (burn_state() == BurnState::has_burned)
        {
            ret += "scorched ";
        }

        switch (type_)
        {
        case FloorType::cmn:
            ret += "stone floor";
            break;

        case FloorType::cave:
            ret += "cavern floor";
            break;

        case FloorType::stone_path:
            if (article == Article::a)
            {
                ret.insert(0, "a ");
            }

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
Wall::Wall(const P& p) :
    Rigid(p),
    type_(WallType::cmn),
    is_mossy_(false) {}

void Wall::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    (void)actor;

    auto destr_adj_doors = [&]()
        {
            for (const P& d : dir_utils::cardinal_list)
            {
                const P p(pos_ + d);

                if (map::is_pos_inside_map(p))
                {
                    if (map::cells[p.x][p.y].rigid->id() == FeatureId::door)
                    {
                        map::put(new RubbleLow(p));
                    }
                }
            }
        };

    auto mk_low_rubble_and_rocks = [&]()
        {
            const P p(pos_);
            map::put(new RubbleLow(p));

            //NOTE: object is now deleted!

            if (rnd::coin_toss())
            {
                item_factory::mk_item_on_floor(ItemId::rock, p);
            }
        };

    if (dmg_type == DmgType::physical)
    {
        if (dmg_method == DmgMethod::forced)
        {
            destr_adj_doors();
            mk_low_rubble_and_rocks();
        }

        if (dmg_method == DmgMethod::explosion)
        {
            destr_adj_doors();

            if (rnd::coin_toss())
            {
                mk_low_rubble_and_rocks();
            }
            else
            {
                map::put(new RubbleHigh(pos_));
            }
        }

        if (dmg_method == DmgMethod::blunt_heavy)
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
                    map::put(new RubbleHigh(pos_));
                }
            }
        }
    }
}

bool Wall::is_tile_any_wall_front(const TileId tile)
{
    return
        tile == TileId::wall_front         ||
        tile == TileId::wall_front_alt1    ||
        tile == TileId::wall_front_alt2    ||
        tile == TileId::cave_wall_front    ||
        tile == TileId::egypt_wall_front;
}

bool Wall::is_tile_any_wall_top(const TileId tile)
{
    return
        tile == TileId::wall_top       ||
        tile == TileId::cave_wall_top  ||
        tile == TileId::egypt_wall_top ||
        tile == TileId::rubble_high;
}

std::string Wall::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    if (is_mossy_) {ret += "moss-grown ";}

    switch (type_)
    {
    case WallType::cmn:
    case WallType::cmn_alt:
    case WallType::leng_monestary:
    case WallType::egypt:
        ret += "stone wall";
        break;

    case WallType::cave:
        ret += "cavern wall";
        break;

    case WallType::cliff:
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
    case WallType::cliff:
        return clr_gray_drk;

    case WallType::egypt:
    case WallType::cave:
        return clr_brown_gray;

    case WallType::cmn:
    case WallType::cmn_alt:
        return map::wall_clr;

    case WallType::leng_monestary:
        return clr_red;
    }

    ASSERT(false && "Failed to set color");
    return clr_yellow;
}

char Wall::glyph() const
{
    return config::is_text_mode_wall_full_square() ? 10 : '#';
}

TileId Wall::front_wall_tile() const
{
    if (config::is_tiles_wall_full_square())
    {
        switch (type_)
        {
        case WallType::cmn:
        case WallType::cmn_alt:
            return TileId::wall_top;

        case WallType::cliff:
        case WallType::cave:
            return TileId::cave_wall_top;

        case WallType::leng_monestary:
        case WallType::egypt:
            return TileId::egypt_wall_top;
        }
    }
    else
    {
        switch (type_)
        {
        case WallType::cmn:
            return TileId::wall_front;

        case WallType::cmn_alt:
            return TileId::wall_front_alt1;

        case WallType::cliff:
        case WallType::cave:
            return TileId::cave_wall_front;

        case WallType::leng_monestary:
        case WallType::egypt:
            return TileId::egypt_wall_front;
        }
    }

    ASSERT(false && "Failed to set front wall tile");
    return TileId::empty;
}

TileId Wall::top_wall_tile() const
{
    switch (type_)
    {
    case WallType::cmn:
    case WallType::cmn_alt:
        return TileId::wall_top;

    case WallType::cliff:
    case WallType::cave:
        return TileId::cave_wall_top;

    case WallType::leng_monestary:
    case WallType::egypt:
        return TileId::egypt_wall_top;
    }

    ASSERT(false && "Failed to set top wall tile");
    return TileId::empty;
}

void Wall::set_rnd_cmn_wall()
{
    const int rnd = rnd::range(1, 6);

    switch (rnd)
    {
    case 1:   type_ = WallType::cmn_alt; break;

    default:  type_ = WallType::cmn;    break;
    }
}

void Wall::set_random_is_moss_grown()
{
    is_mossy_ = rnd::one_in(40);
}

//--------------------------------------------------------------------- HIGH RUBBLE
RubbleHigh::RubbleHigh(const P& p) :
    Rigid(p) {}

void RubbleHigh::on_hit(const DmgType dmg_type,
                        const DmgMethod dmg_method,
                        Actor* const actor)
{
    (void)actor;

    auto mk_low_rubble_and_rocks = [&]()
        {
            const P p(pos_);
            map::put(new RubbleLow(p)); //NOTE: "this" is now deleted!

            if (rnd::coin_toss()) {item_factory::mk_item_on_floor(ItemId::rock, p);}
        };

    if (dmg_type == DmgType::physical)
    {

        if (dmg_method == DmgMethod::forced)
        {
            mk_low_rubble_and_rocks();
        }

        if (dmg_method == DmgMethod::explosion)
        {
            mk_low_rubble_and_rocks();
        }

        if (dmg_method == DmgMethod::blunt_heavy)
        {
            if (rnd::fraction(2, 4)) {mk_low_rubble_and_rocks();}
        }
    }
}

std::string RubbleHigh::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "big pile of debris";
}

Clr RubbleHigh::clr_default() const
{
    return map::wall_clr;
}

//--------------------------------------------------------------------- LOW RUBBLE
RubbleLow::RubbleLow(const P& p) :
    Rigid(p) {}

void RubbleLow::on_hit(const DmgType dmg_type,
                       const DmgMethod dmg_method,
                       Actor* const actor)
{
    (void)actor;

    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        try_start_burning(false);
    }
}

std::string RubbleLow::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the)
    {
        ret += "the ";
    }

    if (burn_state() == BurnState::burning)
    {
        ret += "burning ";
    }

    return ret + "rubble";
}

Clr RubbleLow::clr_default() const
{
    return map::wall_clr;
}

//--------------------------------------------------------------------- BONES
Bones::Bones(const P& p) :
    Rigid(p) {}

void Bones::on_hit(const DmgType dmg_type, const DmgMethod dmg_method,
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
    return clr_gray_drk;
}

//--------------------------------------------------------------------- GRAVE
GraveStone::GraveStone(const P& p) :
    Rigid(p) {}

void GraveStone::on_hit(const DmgType dmg_type,
                        const DmgMethod dmg_method,
                        Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

void GraveStone::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        msg_log::add(inscr_);
    }
}

std::string GraveStone::name(const Article article) const
{
    const std::string ret = article == Article::a ?
        "a " : "the ";

    return ret + "gravestone; " + inscr_;
}

Clr GraveStone::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- CHURCH BENCH
ChurchBench::ChurchBench(const P& p) : Rigid(p) {}

void ChurchBench::on_hit(const DmgType dmg_type, const DmgMethod dmg_method,
                         Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string ChurchBench::name(const Article article) const
{
    const std::string ret = article == Article::a ?
        "a " : "the ";

    return ret + "church bench";
}

Clr ChurchBench::clr_default() const
{
    return clr_brown;
}

//--------------------------------------------------------------------- STATUE
Statue::Statue(const P& p) :
    Rigid   (p),
    type_   (rnd::one_in(8) ? StatueType::ghoul : StatueType::cmn) {}

int Statue::base_shock_when_adj() const
{
    //Non-ghoul players are scared of Ghoul statues
    if (type_ == StatueType::ghoul && player_bon::bg() != Bg::ghoul)
    {
        return 2;
    }

    return 0;
}

void Statue::on_hit(const DmgType dmg_type,
                    const DmgMethod dmg_method,
                    Actor* const actor)
{
    if (dmg_type == DmgType::physical && dmg_method == DmgMethod::kick)
    {
        ASSERT(actor);

        if (actor->has_prop(PropId::weakened))
        {
            msg_log::add("It wiggles a bit.");
            return;
        }

        const AlertsMon alerts_mon = actor == map::player ?
            AlertsMon::yes :
            AlertsMon::no;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("It topples over.");
        }

        Snd snd("I hear a crash.",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                pos_,
                actor,
                SndVol::low,
                alerts_mon);

        snd_emit::run(snd);

        const P dst_pos = pos_ + (pos_ - actor->pos);

        map::put(new RubbleLow(pos_)); //NOTE: "this" is now deleted!

        map::player->update_fov();

        render::draw_map_state();

        Actor* const actor_behind = map::actor_at_pos(dst_pos);

        if (actor_behind && actor_behind->is_alive())
        {
            if (!actor_behind->has_prop(PropId::ethereal))
            {
                if (actor_behind == map::player)
                {
                    msg_log::add("It falls on me!");
                }
                else if (map::player->can_see_actor(*actor_behind))
                {
                    msg_log::add("It falls on " + actor_behind->name_a() + ".");
                }

                actor_behind->hit(rnd::dice(3, 5), DmgType::physical);
            }
        }

        const auto rigid_id = map::cells[dst_pos.x][dst_pos.y].rigid->id();

        //NOTE: This is kinda hacky, but the rubble is mostly just for decoration anyway,
        //so it doesn't really matter.
        if (
            rigid_id == FeatureId::floor ||
            rigid_id == FeatureId::grass ||
            rigid_id == FeatureId::carpet)
        {
            map::put(new RubbleLow(dst_pos));
        }
    }
}

std::string Statue::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    switch (type_)
    {
    case StatueType::cmn:
        ret += "statue";
        break;

    case StatueType::ghoul:
        ret += "statue of a ghoulish creature";
        break;
    }

    return ret;
}

TileId Statue::tile() const
{
    return type_ == StatueType::cmn ?
        TileId::witch_or_warlock : TileId::ghoul;
}

Clr Statue::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- PILLAR
Pillar::Pillar(const P& p) :
    Rigid(p) {}

void Pillar::on_hit(const DmgType dmg_type,
                    const DmgMethod dmg_method,
                    Actor* const actor)
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

//--------------------------------------------------------------------- STALAGMITE
Stalagmite::Stalagmite(const P& p) :
    Rigid(p) {}

void Stalagmite::on_hit(const DmgType dmg_type,
                        const DmgMethod dmg_method,
                        Actor* const actor)
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
Stairs::Stairs(const P& p) :
    Rigid(p) {}

void Stairs::on_hit(const DmgType dmg_type,
                    const DmgMethod dmg_method,
                    Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

void Stairs::on_new_turn_hook()
{
    ASSERT(!map::cells[pos_.x][pos_.y].item);
}

void Stairs::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        const std::vector<std::string> choices
        {
            "Descend",
                "Save and quit",
                "Cancel"
                };

        const std::string   title   = "A staircase leading downwards";
        const int           choice  =  popup::show_menu_msg("", true, choices, title);

        switch (choice)
        {
        case 0:
            map::player->pos = pos_;
            msg_log::clear();
            msg_log::add("I descend the stairs.");
            render::draw_map_state();
            map_travel::go_to_nxt();
            break;

        case 1:
            map::player->pos = pos_;
            saving::save_game();
            init::quit_to_main_menu = true;
            break;

        default:
            msg_log::clear();
            render::draw_map_state();
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
TileId Bridge::tile() const
{
    return axis_ == Axis::hor ? TileId::hangbridge_hor : TileId::hangbridge_ver;
}

void Bridge::on_hit(const DmgType dmg_type,
                    const DmgMethod dmg_method,
                    Actor* const actor)
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
LiquidShallow::LiquidShallow(const P& p) :
    Rigid   (p),
    type_   (LiquidType::water) {}

void LiquidShallow::on_hit(const DmgType dmg_type,
                           const DmgMethod dmg_method,
                           Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

void LiquidShallow::bump(Actor& actor_bumping)
{
    if (
        !actor_bumping.has_prop(PropId::ethereal) &&
        !actor_bumping.has_prop(PropId::flying))
    {
        actor_bumping.prop_handler().try_add(new PropWaiting(PropTurns::std));

        if (actor_bumping.is_player())
        {
            Snd snd("*Glop*",
                    SfxId::END,
                    IgnoreMsgIfOriginSeen::no,
                    actor_bumping.pos,
                    &actor_bumping,
                    SndVol::low,
                    AlertsMon::yes);

            snd_emit::run(snd);
        }
    }
}

std::string LiquidShallow::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the)
    {
        ret += "the ";
    }

    ret += "a pool of ";

    switch (type_)
    {
    case LiquidType::water:
        ret += "water";
        break;

    case LiquidType::acid:
        ret += "acid";
        break;

    case LiquidType::blood:
        ret += "blood";
        break;

    case LiquidType::lava:
        ret += "lava";
        break;

    case LiquidType::mud:
        ret += "mud";
        break;
    }

    return ret;
}

Clr LiquidShallow::clr_default() const
{
    switch (type_)
    {
    case LiquidType::water:
        return clr_blue_lgt;
        break;

    case LiquidType::acid:
        return clr_green_lgt;
        break;

    case LiquidType::blood:
        return clr_red_lgt;
        break;

    case LiquidType::lava:
        return clr_orange;
        break;

    case LiquidType::mud:
        return clr_brown;
        break;
    }

    ASSERT(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- DEEP LIQUID
LiquidDeep::LiquidDeep(const P& p) :
    Rigid(p),
    type_(LiquidType::water) {}

void LiquidDeep::on_hit(const DmgType dmg_type, const DmgMethod dmg_method,
                        Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

void LiquidDeep::bump(Actor& actor_bumping)
{
    (void)actor_bumping;
}

std::string LiquidDeep::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the) {ret += "the ";}

    ret += "deep ";

    switch (type_)
    {
    case LiquidType::water:
        ret += "water";
        break;

    case LiquidType::acid:
        ret += "acid";
        break;

    case LiquidType::blood:
        ret += "blood";
        break;

    case LiquidType::lava:
        ret += "lava";
        break;

    case LiquidType::mud:
        ret += "mud";
        break;
    }

    return ret;
}

Clr LiquidDeep::clr_default() const
{
    switch (type_)
    {
    case LiquidType::water:
        return clr_blue;
        break;

    case LiquidType::acid:
        return clr_green;
        break;

    case LiquidType::blood:
        return clr_red;
        break;

    case LiquidType::lava:
        return clr_orange;
        break;

    case LiquidType::mud:
        return clr_brown_drk;
        break;
    }

    ASSERT(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- CHASM
Chasm::Chasm(const P& p) :
    Rigid(p) {}

void Chasm::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
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
Lever::Lever(const P& p) :
    Rigid(p),
    is_position_left_(true),
    door_linked_to_(nullptr)  {}

void Lever::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
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

TileId Lever::tile() const
{
    return is_position_left_ ? TileId::lever_left : TileId::lever_right;
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
    render::draw_map_state();
    TRACE_FUNC_END;
}

//--------------------------------------------------------------------- ALTAR
Altar::Altar(const P& p) :
    Rigid(p) {}

void Altar::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

std::string Altar::name(const Article article) const
{
    std::string ret = article == Article::a ? "an " : "the ";
    return ret + "altar";
}

Clr Altar::clr_default() const
{
    return clr_white;
}

//--------------------------------------------------------------------- CARPET
Carpet::Carpet(const P& p) :
    Rigid(p) {}

void Carpet::on_hit(const DmgType dmg_type,
                    const DmgMethod dmg_method,
                    Actor* const actor)
{
    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}

WasDestroyed Carpet::on_finished_burning()
{
    Floor* const floor = new Floor(pos_);
    floor->set_has_burned();
    map::put(floor);
    return WasDestroyed::yes;
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
Grass::Grass(const P& p) :
    Rigid(p),
    type_(GrassType::cmn)
{
    if (rnd::one_in(5)) {type_ = GrassType::withered;}
}

void Grass::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
{
    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}


TileId Grass::tile() const
{
    return burn_state() == BurnState::has_burned ?
        TileId::scorched_ground :
        data().tile;
}

std::string Grass::name(const Article article) const
{
    std::string ret = "";

    if (article == Article::the) {ret += "the ";}

    switch (burn_state())
    {
    case BurnState::not_burned:
        switch (type_)
        {
        case GrassType::cmn:
            return ret + "grass";

        case GrassType::withered:
            return ret + "withered grass";
        }
        break;

    case BurnState::burning:
        return ret + "burning grass";

    case BurnState::has_burned:
        return ret + "scorched ground";
    }

    ASSERT("Failed to set name" && false);
    return "";
}

Clr Grass::clr_default() const
{
    switch (type_)
    {
    case GrassType::cmn:
        return clr_green;
        break;

    case GrassType::withered:
        return clr_brown_drk;
        break;
    }

    ASSERT(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- BUSH
Bush::Bush(const P& p) :
    Rigid(p),
    type_(GrassType::cmn)
{
    if (rnd::one_in(5))
    {
        type_ = GrassType::withered;
    }
}

void Bush::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}

WasDestroyed Bush::on_finished_burning()
{
    Grass* const grass = new Grass(pos_);
    grass->set_has_burned();
    map::put(grass);
    return WasDestroyed::yes;
}

std::string Bush::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    switch (burn_state())
    {
    case BurnState::not_burned:
        switch (type_)
        {
        case GrassType::cmn:
            return ret + "shrub";

        case GrassType::withered:
            return ret + "withered shrub";
        }
        break;

    case BurnState::burning:
        return ret + "burning shrub";

    case BurnState::has_burned:
        //Should not happen
        break;
    }

    ASSERT("Failed to set name" && false);
    return "";
}

Clr Bush::clr_default() const
{
    switch (type_)
    {
    case GrassType::cmn:
        return clr_green;
        break;

    case GrassType::withered:
        return clr_brown_drk;
        break;
    }

    ASSERT(false && "Failed to set color");
    return clr_yellow;
}

//--------------------------------------------------------------------- VINES
Vines::Vines(const P& p) :
    Rigid(p) {}

void Vines::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        (void)actor;
        try_start_burning(false);
    }
}

WasDestroyed Vines::on_finished_burning()
{
    Floor* const floor = new Floor(pos_);
    floor->set_has_burned();
    map::put(floor);
    return WasDestroyed::yes;
}

std::string Vines::name(const Article article) const
{
    std::string ret = article == Article::a ? "" : "the ";

    switch (burn_state())
    {
    case BurnState::not_burned:
        return ret + "hanging vines";

    case BurnState::burning:
        return ret + "burning vines";

    case BurnState::has_burned:
        //Should not happen
        break;
    }

    ASSERT("Failed to set name" && false);
    return "";
}

Clr Vines::clr_default() const
{
    return clr_green;
}

//--------------------------------------------------------------------- TREE
Tree::Tree(const P& p) :
    Rigid(p) {}

void Tree::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    if (dmg_type == DmgType::fire && dmg_method == DmgMethod::elemental)
    {
        (void)actor;

        if (rnd::fraction(2, 3))
        {
            try_start_burning(false);
        }
    }
}

WasDestroyed Tree::on_finished_burning()
{
    if (map::is_pos_inside_map(pos_, false))
    {
        Grass* const grass = new Grass(pos_);
        grass->set_has_burned();
        map::put(grass);
        return WasDestroyed::yes;
    }

    return WasDestroyed::no;
}

std::string Tree::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    switch (burn_state())
    {
    case BurnState::not_burned:
        break;

    case BurnState::burning:
        ret += "burning ";
        break;

    case BurnState::has_burned:
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

void Brazier::on_hit(const DmgType dmg_type,
                     const DmgMethod dmg_method,
                     Actor* const actor)
{
    if (dmg_type == DmgType::physical && dmg_method == DmgMethod::kick)
    {
        ASSERT(actor);

        if (actor->has_prop(PropId::weakened))
        {
            msg_log::add("It wiggles a bit.");
            return;
        }

        const AlertsMon alerts_mon = actor == map::player ?
            AlertsMon::yes :
            AlertsMon::no;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("It topples over.");
        }

        Snd snd("I hear a crash.",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                pos_,
                actor,
                SndVol::low,
                alerts_mon);

        snd_emit::run(snd);

        const P dst_pos = pos_ + (pos_ - actor->pos);

        const P my_pos = pos_;

        map::put(new RubbleLow(pos_));

        //NOTE: "this" is now deleted!

        map::player->update_fov();
        render::draw_map_state();

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
                           ExplType::apply_prop,
                           ExplSrc::misc,
                           EmitExplSnd::no,
                           expl_d,
                           new PropBurning(PropTurns::std));
        }
    }
}

void Brazier::add_light_hook(bool light[map_w][map_h]) const
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
ItemContainer::ItemContainer()
{
    for (auto* item : items_)
    {
        delete item;
    }

    items_.clear();
}

ItemContainer::~ItemContainer()
{
    for (auto* item : items_)
    {
        delete item;
    }
}

void ItemContainer::init(const FeatureId feature_id, const int nr_items_to_attempt)
{
    for (auto* item : items_)
    {
        delete item;
    }

    items_.clear();

    if (nr_items_to_attempt > 0)
    {
        while (items_.empty())
        {
            std::vector<ItemId> item_bucket;

            for (size_t i = 0; i < size_t(ItemId::END); ++i)
            {
                ItemDataT& d = item_data::data[i];

                for (auto container_spawn_rule : d.container_spawn_rules)
                {
                    if (
                        container_spawn_rule.feature_id == feature_id &&
                        d.allow_spawn &&
                        rnd::percent() < container_spawn_rule.pct_chance_to_incl)
                    {
                        item_bucket.push_back(ItemId(i));
                        break;
                    }
                }
            }

            for (int i = 0; i < nr_items_to_attempt; ++i)
            {
                if (item_bucket.empty())
                {
                    break;
                }

                const int     idx = rnd::range(0, item_bucket.size() - 1);
                const ItemId id  = item_bucket[idx];

                if (item_data::data[(size_t)id].allow_spawn)
                {
                    Item* item = item_factory::mk(item_bucket[idx]);
                    item_factory::set_item_randomized_properties(item);
                    items_.push_back(item);
                }
                else //Not allowed to spawn
                {
                    item_bucket.erase(begin(item_bucket) + idx);
                }
            }
        }
    }
}

void ItemContainer::open(const P& feature_pos, Actor* const actor_opening)
{
    if (actor_opening)
    {
        for (auto* item : items_)
        {
            msg_log::clear();

            const std::string name = item->name(ItemRefType::plural, ItemRefInf::yes,
                                                ItemRefAttInf::wpn_context);

            msg_log::add("Pick up " + name + "? [y/n]");

            const ItemDataT&  data = item->data();

            Wpn* wpn = data.ranged.is_ranged_wpn ? static_cast<Wpn*>(item) : nullptr;

            const bool is_unloadable_wpn = wpn  &&
                wpn->nr_ammo_loaded_ > 0        &&
                !data.ranged.has_infinite_ammo;

            if (is_unloadable_wpn)
            {
                msg_log::add("Unload? [G]");
            }

            render::draw_map_state();

            const YesNoAnswer answer = query::yes_or_no(is_unloadable_wpn ? 'G' : -1);

            if (answer == YesNoAnswer::yes)
            {
                audio::play(SfxId::pickup);

                Inventory& inv = map::player->inv();

                Item* const thrown_item = inv.slots_[(size_t)SlotId::thrown].item;

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
            else if (answer == YesNoAnswer::no)
            {
                item_drop::drop_item_on_map(feature_pos, *item);
            }
            else //Special key (unload in this case)
            {
                ASSERT(is_unloadable_wpn);
                ASSERT(wpn);
                ASSERT(wpn->nr_ammo_loaded_ > 0);
                ASSERT(!data.ranged.has_infinite_ammo);

                audio::play(SfxId::pickup);

                Ammo* const spawned_ammo = item_pickup::unload_ranged_wpn(*wpn);

                map::player->inv().put_in_backpack(spawned_ammo);

                item_drop::drop_item_on_map(feature_pos, *wpn);
            }
        }

        msg_log::clear();
        msg_log::add("There are no more items.");
        render::draw_map_state();
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

void ItemContainer::destroy_single_fragile()
{
    //TODO: Generalize this (perhaps something like "is_fragile" item data value)

    for (size_t i = 0; i < items_.size(); ++i)
    {
        Item* const item = items_[i];
        const ItemDataT& d = item->data();

        if (d.type == ItemType::potion || d.id == ItemId::molotov)
        {
            delete item;
            items_.erase(items_.begin() + i);
            msg_log::add("I hear a muffled shatter.");
            break;
        }
    }
}

//--------------------------------------------------------------------- TOMB
Tomb::Tomb(const P& p) :
    Rigid                   (p),
    is_open_                (false),
    is_trait_known_         (false),
    push_lid_one_in_n_      (rnd::range(6, 14)),
    appearance_             (TombAppearance::common),
    is_random_appearance_   (false),
    trait_                  (TombTrait::END)
{
    //Contained items
    const int nr_items_min  = 0;
    const int nr_items_max  = player_bon::traits[(size_t)Trait::treasure_hunter] ? 2 : 1;

    item_container_.init(FeatureId::tomb, rnd::range(nr_items_min, nr_items_max));

    //Appearance
    if (rnd::one_in(12))
    {
        //Do not base appearance on items (random appearance)
        const int nr_app    = int(TombAppearance::END);
        appearance_         = TombAppearance(rnd::range(0, nr_app - 1));

        is_random_appearance_ = true;
    }
    else //Base appearance on value of contained items
    {
        //Appearance is based on items inside
        for (Item* item : item_container_.items_)
        {
            const ItemValue item_value = item->data().value;

            if (item_value == ItemValue::major_treasure)
            {
                appearance_ = TombAppearance::marvelous;
                break;
            }
            else if (item_value == ItemValue::minor_treasure)
            {
                appearance_ = TombAppearance::ornate;
            }
        }
    }

    if (appearance_ == TombAppearance::marvelous && !is_random_appearance_)
    {
        trait_ = TombTrait::ghost; //NOTE: Wraith is always chosen
    }
    else //Randomized trait
    {
        std::vector<int> weights(size_t(TombTrait::END) + 1, 0);

        weights[(size_t)TombTrait::ghost]          = 5;
        weights[(size_t)TombTrait::other_undead]   = 3;
        weights[(size_t)TombTrait::stench]         = 2;
        weights[(size_t)TombTrait::cursed]         = 1;
        weights[(size_t)TombTrait::END]            = 2;

        trait_ = TombTrait(rnd::weighted_choice(weights));
    }
}

void Tomb::on_hit(const DmgType dmg_type,
                  const DmgMethod dmg_method,
                  Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Tomb::name(const Article article) const
{
    const bool          is_empty    = is_open_ && item_container_.items_.empty();
    const std::string   empty_str   = is_empty                 ? "empty " : "";
    const std::string   open_str    = (is_open_ && !is_empty)  ? "open "  : "";

    std::string a = "";

    if (article == Article::a)
    {
        a = (is_open_ || appearance_ == TombAppearance::ornate) ?  "an " : "a ";
    }
    else
    {
        a = "the ";
    }

    std::string appear_str = "";

    if (!is_empty)
    {
        switch (appearance_)
        {
        case TombAppearance::common:
        case TombAppearance::END: {}
            break;

        case TombAppearance::ornate:
            appear_str = "ornate ";
            break;

        case TombAppearance::marvelous:
            appear_str = "marvelous ";
            break;
        }
    }

    return a + empty_str + open_str + appear_str + "tomb";
}

TileId Tomb::tile() const
{
    return is_open_ ? TileId::tomb_open : TileId::tomb_closed;
}

Clr Tomb::clr_default() const
{
    switch (appearance_)
    {
    case TombAppearance::common:
        return clr_gray;

    case TombAppearance::ornate:
        return clr_white;

    case TombAppearance::marvelous:
        return clr_yellow;

    case TombAppearance::END:
        break;
    }

    ASSERT("Failed to set Tomb color" && false);
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
        else if (!map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("There is a stone box here.");
            render::draw_map_state();
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

                if (actor_bumping.has_prop(PropId::weakened))
                {
                    try_sprain_player();

                    msg_log::add("It seems futile.", clr_msg_note, false, MorePromptOnMsg::yes);
                }
                else //Not weakened
                {
                    const int bon =
                        player_bon::traits[(size_t)Trait::rugged]   ? 8   :
                        player_bon::traits[(size_t)Trait::tough]    ? 4   : 0;

                    TRACE << "Base chance to push lid is: 1 in "
                          << push_lid_one_in_n_ << std::endl;

                    TRACE << "Bonus to roll: "
                          << bon << std::endl;

                    const int roll_tot = rnd::range(1, push_lid_one_in_n_) + bon;

                    TRACE << "Roll + bonus = " << roll_tot << std::endl;

                    bool is_success = false;

                    if (roll_tot < push_lid_one_in_n_ - 9)
                    {
                        msg_log::add("It does not yield at all.");
                    }
                    else if (roll_tot < push_lid_one_in_n_ - 1)
                    {
                        msg_log::add("It resists.");
                    }
                    else if (roll_tot == push_lid_one_in_n_ - 1)
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
    const int sprain_one_in_n =
        player_bon::traits[(size_t)Trait::rugged]   ? 8  :
        player_bon::traits[(size_t)Trait::tough]    ? 6  : 4;

    if (rnd::one_in(sprain_one_in_n))
    {
        msg_log::add("I sprain myself.", clr_msg_bad);
        map::player->hit(rnd::range(1, 3), DmgType::pure);
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
                     MorePromptOnMsg::yes);

        item_container_.open(pos_, map::player);
    }
}

DidOpen Tomb::open(Actor* const actor_opening)
{
    if (is_open_)
    {
        return DidOpen::no;
    }
    else //Was not already open
    {
        is_open_ = true;

        snd_emit::run(Snd("I hear heavy stone sliding.",
                          SfxId::tomb_open,
                          IgnoreMsgIfOriginSeen::yes,
                          pos_,
                          nullptr,
                          SndVol::high,
                          AlertsMon::yes
                          ));

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            render::draw_map_state();
            msg_log::add("The lid comes off.");
        }

        trigger_trap(actor_opening);

        render::draw_map_state();
        return DidOpen::yes;
    }
}

DidTriggerTrap Tomb::trigger_trap(Actor* const actor)
{
    TRACE_FUNC_BEGIN;

    (void)actor;

    DidTriggerTrap did_trigger_trap = DidTriggerTrap::no;

    ActorId id_to_spawn = ActorId::END;

    const bool is_seen = map::cells[pos_.x][pos_.y].is_seen_by_player;

    switch (trait_)
    {
    case TombTrait::ghost:
    {
        //Tomb contains major treasure?
        if (appearance_ == TombAppearance::marvelous && !is_random_appearance_)
        {
            id_to_spawn = ActorId::wraith;
        }
        else //Not containing major treasure
        {
            std::vector<ActorId> mon_bucket =
                {
                    ActorId::ghost,
                    ActorId::phantasm,
                };

            const size_t idx = rnd::range(0, mon_bucket.size() - 1);

            id_to_spawn = mon_bucket[idx];
        }

        const std::string msg = "The air suddenly feels colder.";

        msg_log::add(msg,
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        did_trigger_trap = DidTriggerTrap::yes;
    }
    break;

    case TombTrait::other_undead:
    {
        std::vector<ActorId> mon_bucket =
            {
                ActorId::mummy,
                ActorId::croc_head_mummy,
                ActorId::zombie,
                ActorId::zombie_axe,
                ActorId::floating_skull
            };

        const size_t idx = rnd::range(0, mon_bucket.size() - 1);

        id_to_spawn = mon_bucket[idx];

        const std::string msg = "Something rises from the tomb!";

        msg_log::add(msg,
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        did_trigger_trap = DidTriggerTrap::yes;
    }
    break;

    case TombTrait::stench:
    {
        if (rnd::coin_toss())
        {
            if (is_seen)
            {
                msg_log::add("Fumes burst out from the tomb!",
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);
            }

            Snd snd("I hear a burst of gas.",
                    SfxId::gas,
                    IgnoreMsgIfOriginSeen::yes,
                    pos_,
                    nullptr,
                    SndVol::low,
                    AlertsMon::yes);

            snd_emit::run(snd);

            Prop*       prop        = nullptr;
            Clr         fume_clr    = clr_magenta;
            const int   rnd         = rnd::percent();

            if (map::dlvl >= min_dlvl_harder_traps && rnd < 20)
            {
                prop        = new PropPoisoned(PropTurns::std);
                fume_clr    = clr_green_lgt;
            }
            else if (rnd < 40)
            {
                prop        = new PropDiseased(PropTurns::std);
                fume_clr    = clr_green;
            }
            else
            {
                prop = new PropParalyzed(PropTurns::std);
                prop->set_nr_turns_left(prop->nr_turns_left() * 2);
            }

            explosion::run(pos_,
                           ExplType::apply_prop,
                           ExplSrc::misc,
                           EmitExplSnd::no,
                           0,
                           prop,
                           &fume_clr);
        }
        else //Not fumes
        {
            std::vector<ActorId> mon_bucket;

            for (size_t i = 0; i < size_t(ActorId::END); ++i)
            {
                const ActorDataT& d = actor_data::data[i];

                if (
                    d.natural_props[(size_t)PropId::ooze]   &&
                    d.is_auto_spawn_allowed                 &&
                    !d.is_unique)
                {
                    mon_bucket.push_back(ActorId(i));
                }
            }

            const size_t idx = rnd::range(0, mon_bucket.size() - 1);

            id_to_spawn = mon_bucket[idx];

            if (is_seen)
            {
                msg_log::add("Something repulsive creeps up from the tomb!",
                             clr_white,
                             false,
                             MorePromptOnMsg::yes);
            }
        }

        did_trigger_trap = DidTriggerTrap::yes;
    }
    break;

    case TombTrait::cursed:
    {
        map::player->prop_handler().try_add(new PropCursed(PropTurns::std));
        did_trigger_trap = DidTriggerTrap::yes;
    }
    break;

    case TombTrait::END:
        break;
    }

    if (id_to_spawn != ActorId::END)
    {
        TRACE << "Summoning monster" << std::endl;

        //First, try to spawn monster in an adjacent cell
        P spawn_pos(-1, -1);

        bool blocked[map_w][map_h];

        map_parse::run(cell_check::BlocksMoveCmn(true),
                       blocked,
                       MapParseMode::overwrite,
                       R(pos_ - 1, pos_ + 1));

        auto positions_to_try = dir_utils::dir_list;

        random_shuffle(begin(positions_to_try), end(positions_to_try));

        Mon* mon_spawned = nullptr;

        for (const P& d : positions_to_try)
        {
            const P p(pos_ + d);

            if (!blocked[p.x][p.y])
            {
                Actor* actor_spawned = actor_factory::mk(id_to_spawn, p);

                mon_spawned = static_cast<Mon*>(actor_spawned);

                mon_spawned->become_aware_player(false);

                break;
            }
        }

        if (!mon_spawned)
        {
            //All adjacent space is occupied, use the summon method as fallback
            //(Although the monster could spawn pretty far from the tomb...)

            TRACE << "Could not spawn adjacent monster, using summon method instead"
                  << std::endl;

            std::vector<Mon*> spawned_list;

            actor_factory::summon(pos_,
                                  std::vector<ActorId>(1, id_to_spawn),
                                  MakeMonAware::yes,
                                  nullptr,
                                  &spawned_list,
                                  Verbosity::silent);

            ASSERT(spawned_list.size() == 1);

            if (!spawned_list.empty())
            {
                mon_spawned = spawned_list[0];
            }
        }

        ASSERT(mon_spawned);

        if (mon_spawned)
        {
            mon_spawned->prop_handler().try_add(
                new PropWaiting(PropTurns::specific, 1));
        }
    }

    trait_          = TombTrait::END;
    is_trait_known_ = true;

    TRACE_FUNC_END;

    return did_trigger_trap;
}

//--------------------------------------------------------------------- CHEST
Chest::Chest(const P& p) :
    Rigid                   (p),
    is_open_                (false),
    is_locked_              (false),
    is_trapped_             (false),
    is_trap_status_known_   (false),
    matl_                   (ChestMatl::wood),
    trap_det_lvl            (rnd::range(0, 2))
{
    if (map::dlvl >= 3 && rnd::coin_toss())
    {
        matl_ = ChestMatl::iron;
    }

    const bool  is_treasure_hunter  = player_bon::traits[(size_t)Trait::treasure_hunter];
    const int   nr_items_min        = 0;
    const int   nr_items_max        = is_treasure_hunter ? 3 : 2;

    item_container_.init(FeatureId::chest, rnd::range(nr_items_min, nr_items_max));

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
            render::draw_map_state();
        }
        else if (!map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("There is a chest here.");
            render::draw_map_state();
        }
        else //Player can see
        {
            if (is_locked_)
            {
                msg_log::add("The chest is locked.");
                render::draw_map_state();
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
    ASSERT(is_trapped_);
    ASSERT(!is_open_);

    const bool can_det_trap =
        trap_det_lvl == 0                               ||
        player_bon::traits[(size_t)Trait::perceptive]   ||
        (trap_det_lvl == 1 && player_bon::traits[(size_t)Trait::observant]);

    if (can_det_trap)
    {
        is_trap_status_known_ = true;
        msg_log::add("There appears to be a hidden trap.");
        render::draw_map_state();
    }
}

void Chest::try_sprain_player()
{
    const int sprain_one_in_n =
        player_bon::traits[(size_t)Trait::rugged]   ? 8  :
        player_bon::traits[(size_t)Trait::tough]    ? 6  : 4;

    if (rnd::one_in(sprain_one_in_n))
    {
        msg_log::add("I sprain myself.", clr_msg_bad);
        map::player->hit(rnd::range(1, 3), DmgType::pure);
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
        msg_log::add("There are some items inside.",
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);

        item_container_.open(pos_, map::player);
    }
}

DidOpen Chest::open(Actor* const actor_opening)
{
    is_locked_              = false;
    is_trap_status_known_   = true;

    if (is_open_)
    {
        return DidOpen::no;
    }
    else //Chest is closed
    {
        is_open_ = true;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("The chest opens.");
        }

        render::draw_map_state();

        trigger_trap(actor_opening);

        render::draw_map_state();

        return DidOpen::yes;
    }
}

void Chest::hit(const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor)
{
    switch (dmg_type)
    {
    case DmgType::physical:
    {
        switch (dmg_method)
        {
        case DmgMethod::kick:
        {
            if (!map::cells[pos_.x][pos_.y].is_seen_by_player)
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
                ASSERT(actor);

                msg_log::add("I kick the lid.");

                if (actor->has_prop(PropId::weakened) || matl_ == ChestMatl::iron)
                {
                    try_sprain_player();
                    msg_log::add("It seems futile.", clr_msg_note, false, MorePromptOnMsg::yes);
                }
                else //Chest can be bashed open
                {
                    if (
                        !actor->has_prop(PropId::blessed) &&
                        (actor->has_prop(PropId::cursed) || rnd::one_in(3)))
                    {
                        item_container_.destroy_single_fragile();
                    }

                    const int open_one_in_n =
                        player_bon::traits[(size_t)Trait::rugged]   ? 2 :
                        player_bon::traits[(size_t)Trait::tough]    ? 3 : 4;

                    if (rnd::one_in(open_one_in_n))
                    {
                        msg_log::add("The lock breaks!",
                                     clr_text,
                                     false,
                                     MorePromptOnMsg::yes);

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
//      Item* const item  = inv.item_in_slot(SlotId::wpn);
//
//      if(!item) {
//        msg_log::add(
//          "I attempt to punch the lock open, nearly breaking my hand.",
//          clr_msg_bad);
//        map::player->hit(1, DmgType::pure, false);
//      } else {
//        const int chance_to_dmg_wpn = is_blessed ? 1 : (is_cursed ? 80 : 15);
//
//        if(rnd::percent() < chance_to_dmg_wpn) {
//          const std::string wpn_name = item_data::item_ref(
//                                   *item, ItemRefType::plain, true);
//
//          Wpn* const wpn = static_cast<Wpn*>(item);
//
//          if(wpn->melee_dmg_plus == 0) {
//            msg_log::add("My " + wpn_name + " breaks!");
//            delete wpn;
//            inv.slot(SlotId::wpn)->item = nullptr;
//          } else {
//            msg_log::add("My " + wpn_name + " is damaged!");
//            wpn->melee_dmg_plus--;
//          }
//          return;
//        }
//
//        if(is_weak) {
//          msg_log::add("It seems futile.");
//        } else {
//          const int chance_to_open = 40;
//          if(rnd::percent() < chance_to_open) {
//            msg_log::add("I force the lock open!");
//            open();
//          } else {
//            msg_log::add("The lock resists.");
//          }
//        }
//      }
}

void Chest::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
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
        render::draw_map_state();
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
                render::draw_map_state();
                const auto answer = query::yes_or_no();

                if (answer == YesNoAnswer::no)
                {
                    msg_log::clear();
                    msg_log::add("I leave the chest for now.");
                    render::draw_map_state();
                    return;
                }
            }
        }

        //Try disarming trap
        if (is_trap_status_known_)
        {
            msg_log::add("I attempt to disarm the chest...",
                         clr_text,
                         false,
                         MorePromptOnMsg::yes);
            msg_log::clear();

            const Fraction disarm_chance(4, 5);

            if (disarm_chance.roll())
            {
                msg_log::add("I successfully disarm it!");
                render::draw_map_state();
                is_trapped_ = false;
            }
            else //Failed to disarm
            {
                //NOTE: This will disable the trap
                trigger_trap(map::player);
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

    render::draw_map_state();
}

DidTriggerTrap Chest::trigger_trap(Actor* const actor)
{
    is_trap_status_known_ = true;

    //Chest is not trapped?
    if (!is_trapped_)
    {
        return DidTriggerTrap::no;
    }

    is_trapped_ = false;

    const bool is_seen = map::cells[pos_.x][pos_.y].is_seen_by_player;

    if (actor && is_seen)
    {
        msg_log::add("A hidden trap on the chest triggers...",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);
    }

    //Nothing happens?
    int trap_no_action_one_in_n = 4;

    if (actor)
    {
        if (actor->has_prop(PropId::blessed))
        {
            trap_no_action_one_in_n = 2;
        }
        else if (actor->has_prop(PropId::cursed))
        {
            trap_no_action_one_in_n = 20;
        }
    }

    if (rnd::one_in(trap_no_action_one_in_n))
    {
        if (actor && is_seen)
        {
            msg_log::clear();
            msg_log::add("...but nothing happens.");
        }

        return DidTriggerTrap::no;
    }

    //Fire explosion?
    const int fire_explosion_one_in_n = 5;

    if (map::dlvl >= min_dlvl_harder_traps && rnd::one_in(fire_explosion_one_in_n))
    {
        if (is_seen)
        {
            msg_log::clear();
            msg_log::add("Flames burst out from the chest!",
                         clr_white,
                         false,
                         MorePromptOnMsg::yes);
        }

        explosion::run(pos_,
                       ExplType::apply_prop,
                       ExplSrc::misc,
                       EmitExplSnd::yes,
                       0,
                       new PropBurning(PropTurns::std));

        return DidTriggerTrap::yes;
    }

    //If player is opening, the trap can be a needle
    if (actor == map::player && rnd::coin_toss())
    {
        //Needle
        msg_log::add("A needle pierces my skin!",
                     clr_msg_bad,
                     true);

        actor->hit(rnd::range(1, 3), DmgType::physical);

        if (map::dlvl < min_dlvl_harder_traps)
        {
            //Weak poison
            actor->prop_handler().try_add(
                new PropPoisoned(PropTurns::specific, poison_dmg_n_turn * 3));
        }
        else //We're at the deep end of the pool now, apply strong poison
        {
            actor->prop_handler().try_add(new PropPoisoned(PropTurns::std));
        }
    }
    else //Not needle trap
    {
        //Fumes
        if (is_seen)
        {
            msg_log::clear();
            msg_log::add("Fumes burst out from the chest!",
                         clr_white,
                         false,
                         MorePromptOnMsg::yes);
        }

        Snd snd("I hear a burst of gas.",
                SfxId::gas,
                IgnoreMsgIfOriginSeen::yes,
                pos_,
                nullptr,
                SndVol::low,
                AlertsMon::yes);

        snd_emit::run(snd);

        Prop*       prop        = nullptr;
        Clr         fume_clr    = clr_magenta;
        const int   rnd         = rnd::percent();

        if (map::dlvl >= min_dlvl_harder_traps && rnd < 20)
        {
            prop        = new PropPoisoned(PropTurns::std);
            fume_clr    = clr_green_lgt;
        }
        else if (rnd < 40)
        {
            prop        = new PropDiseased(PropTurns::std);
            fume_clr    = clr_green;
        }
        else
        {
            prop = new PropParalyzed(PropTurns::std);
            prop->set_nr_turns_left(prop->nr_turns_left() * 2);
        }

        explosion::run(pos_,
                       ExplType::apply_prop,
                       ExplSrc::misc,
                       EmitExplSnd::no,
                       0,
                       prop,
                       &fume_clr);
    }

    return DidTriggerTrap::yes;
}

std::string Chest::name(const Article article) const
{
    const bool          is_empty        = item_container_.items_.empty() && is_open_;
    const bool          is_known_trap   = is_trapped_ && is_trap_status_known_;
    const std::string   locked_str      = is_locked_                ? "locked "   : "";
    const std::string   empty_str       = is_empty                  ? "empty "    : "";
    const std::string   trap_str        = is_known_trap             ? "trapped "  : "";
    const std::string   open_str        = (is_open_ && !is_empty)   ? "open "     : "";

    std::string a = "";

    if (article == Article::a)
    {
        a = (
            is_locked_ ||
            (!is_open_ && (matl_ == ChestMatl::wood || is_known_trap))
            ) ?
            "a " : "an ";
    }
    else
    {
        a = "the ";
    }

    const std::string matl_str =
        is_open_ ? "" :
        matl_ == ChestMatl::wood ? "wooden " : "iron ";

    return a + locked_str + empty_str + open_str + trap_str + matl_str + "chest";
}

TileId Chest::tile() const
{
    return is_open_ ? TileId::chest_open : TileId::chest_closed;
}

Clr Chest::clr_default() const
{
    return matl_ == ChestMatl::wood ? clr_brown_drk : clr_gray;
}

//--------------------------------------------------------------------- FOUNTAIN
Fountain::Fountain(const P& p) :
    Rigid               (p),
    fountain_effect_    (FountainEffect::END),
    fountain_type_      (FountainType::normal),
    has_drinks_left_    (true)
{
    std::vector<int> weights =
    {
        16, //Normal
        4,  //Blessed
        1,  //Cursed
    };

    const int choice = rnd::weighted_choice(weights);

    FountainType type = FountainType::normal;

    switch (choice)
    {
    case 0: type = FountainType::normal;    break;
    case 1: type = FountainType::blessed;   break;
    case 2: type = FountainType::cursed;    break;
    }

    set_type(type);
}

void Fountain::set_type(const FountainType type)
{
    fountain_type_ = type;

    //Setup effect
    switch (fountain_type_)
    {
    case FountainType::normal:
    {
        fountain_effect_ = FountainEffect::refreshing;
    }
    break;

    case FountainType::blessed:
    {
        fountain_effect_ = FountainEffect::xp;
    }
    break;

    case FountainType::cursed:
    {
        const int min = (int)FountainEffect::START_OF_BAD_EFFECTS + 1;
        const int max = (int)FountainEffect::END - 1;

        fountain_effect_ = (FountainEffect)rnd::range(min, max);
    }
    }
}

void Fountain::on_hit(const DmgType dmg_type,
                      const DmgMethod dmg_method,
                      Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

Clr Fountain::clr_default() const
{
    return has_drinks_left_ ? clr_blue_lgt : clr_gray;

    ASSERT("Failed to get fountain color" && false);
    return clr_black;
}

std::string Fountain::name(const Article article) const
{
    std::string a = article == Article::a ? "a " : "the ";

    return a + "fountain";
}

void Fountain::bump(Actor& actor_bumping)
{
    if (!actor_bumping.is_player())
    {
        return;
    }

    if (has_drinks_left_)
    {
        PropHandler& prop_hlr = map::player->prop_handler();

        if (!map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::clear();
            msg_log::add("There is a fountain here. Drink from it? [y/n]");
            render::draw_map_state();

            const auto answer = query::yes_or_no();

            if (answer == YesNoAnswer::no)
            {
                msg_log::clear();
                msg_log::add("I leave the fountain for now.");
                render::draw_map_state();
                return;
            }
        }

        if (!prop_hlr.allow_eat(Verbosity::verbose))
        {
            return;
        }

        msg_log::clear();
        msg_log::add("I drink from the fountain...");

        audio::play(SfxId::fountain_drink);

        switch (fountain_effect_)
        {
        case FountainEffect::refreshing:
            msg_log::add("It's very refreshing.");
            map::player->restore_hp(1, false, Verbosity::silent);
            map::player->restore_spi(1, false, Verbosity::silent);
            map::player->restore_shock(5, true);
            break;

        case FountainEffect::xp:
            msg_log::add("I feel more powerful!");
            dungeon_master::incr_player_xp(3);
            break;

        case FountainEffect::curse:
            prop_hlr.try_add(new PropCursed(PropTurns::std));
            break;

        case FountainEffect::disease:
            prop_hlr.try_add(new PropDiseased(PropTurns::std));
            break;

        case FountainEffect::poison:
            prop_hlr.try_add(new PropPoisoned(PropTurns::std));
            break;

        case FountainEffect::frenzy:
            prop_hlr.try_add(new PropFrenzied(PropTurns::std));
            break;

        case FountainEffect::paralyze:
            prop_hlr.try_add(new PropParalyzed(PropTurns::std));
            break;

        case FountainEffect::blind:
            prop_hlr.try_add(new PropBlind(PropTurns::std));
            break;

        case FountainEffect::faint:
            prop_hlr.try_add(new PropFainted(PropTurns::specific, 10));
            break;

        case FountainEffect::START_OF_BAD_EFFECTS:
        case FountainEffect::END:
            break;
        }

        const int dry_one_in_n = 3;

        if (rnd::one_in(dry_one_in_n))
        {
            has_drinks_left_ = false;

            msg_log::add("The fountain dries up.");
        }

        render::draw_map_state();
        game_time::tick();
    }
    else //Dried up
    {
        msg_log::add("The fountain is dried-up.");
    }
}

//--------------------------------------------------------------------- CABINET
Cabinet::Cabinet(const P& p) :
    Rigid       (p),
    is_open_    (false)
{
    const int is_empty_N_IN_10  = 5;
    const int nr_items_min      = rnd::fraction(is_empty_N_IN_10, 10) ? 0 : 1;
    const int nr_items_max      = player_bon::traits[(size_t)Trait::treasure_hunter] ? 2 : 1;

    item_container_.init(FeatureId::cabinet,
                         rnd::range(nr_items_min, nr_items_max));
}

void Cabinet::on_hit(const DmgType dmg_type, const DmgMethod dmg_method,
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
        else if (!map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("There is a cabinet here.");
            render::draw_map_state();
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
        msg_log::add("There are some items inside.",
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);

        item_container_.open(pos_, map::player);
    }
}

DidOpen Cabinet::open(Actor* const actor_opening)
{
    (void)actor_opening;

    if (is_open_)
    {
        return DidOpen::no;
    }
    else //Was not already open
    {
        is_open_ = true;

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            render::draw_map_state();
            msg_log::add("The cabinet opens.");
        }

        render::draw_map_state(UpdateScreen::yes);

        return DidOpen::yes;
    }
}

std::string Cabinet::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    if (burn_state() == BurnState::burning)
    {
        ret += "burning ";
    }

    return ret + "cabinet";
}

TileId Cabinet::tile() const
{
    return is_open_ ? TileId::cabinet_open : TileId::cabinet_closed;
}

Clr Cabinet::clr_default() const
{
    return clr_brown_drk;
}

//--------------------------------------------------------------------- COCOON
Cocoon::Cocoon(const P& p) :
    Rigid(p),
    is_trapped_(rnd::fraction(6, 10)),
    is_open_(false)
{
    if (is_trapped_)
    {
        item_container_.init(FeatureId::cocoon, 0);
    }
    else
    {
        const bool is_treasure_hunter = player_bon::traits[(size_t)Trait::treasure_hunter];

        const Fraction fraction_empty(6, 10);

        const int nr_items_min = fraction_empty.roll() ? 0 : 1;

        const int nr_items_max = nr_items_min + (is_treasure_hunter ? 1 : 0);

        item_container_.init(FeatureId::cocoon, rnd::range(nr_items_min, nr_items_max));
    }
}

void Cocoon::on_hit(const DmgType dmg_type,
                    const DmgMethod dmg_method,
                    Actor* const actor)
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
        else if (!map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("There is a cocoon here.");
            render::draw_map_state();
        }
        else //Player can see
        {
            if (insanity::has_sympt(InsSymptId::phobia_spider))
            {
                map::player->prop_handler().try_add( new PropTerrified(PropTurns::std));
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

DidTriggerTrap Cocoon::trigger_trap(Actor* const actor)
{
    (void)actor;

    if (is_trapped_)
    {
        const int rnd = rnd::percent();

        if (rnd < 15)
        {
            //A dead body
            msg_log::add("There is a half-dissolved human body inside!");
            map::player->incr_shock(ShockLvl::heavy, ShockSrc::misc);
            is_trapped_ = false;
            return DidTriggerTrap::yes;
        }
        else if (rnd < 50)
        {
            //Spiders
            TRACE << "Attempting to spawn spiders" << std::endl;
            std::vector<ActorId> spawn_bucket;

            for (int i = 0; i < int(ActorId::END); ++i)
            {
                const ActorDataT& d = actor_data::data[i];

                if (
                    d.is_spider                         &&
                    d.actor_size == ActorSize::floor    &&
                    d.is_auto_spawn_allowed             &&
                    !d.is_unique)
                {
                    spawn_bucket.push_back(d.id);
                }
            }

            const int nr_candidates = spawn_bucket.size();

            if (nr_candidates > 0)
            {
                TRACE << "Spawn candidates found, attempting to place" << std::endl;
                msg_log::add("There are spiders inside!");
                const int nr_spiders          = rnd::range(2, 5);
                const int idx                 = rnd::range(0, nr_candidates - 1);
                const ActorId actor_id_to_summon = spawn_bucket[idx];

                const std::vector<ActorId> ids_to_summon(nr_spiders, actor_id_to_summon);

                actor_factory::summon(pos_, ids_to_summon, MakeMonAware::yes);

                is_trapped_ = false;
                return DidTriggerTrap::yes;
            }
        }
    }

    return DidTriggerTrap::no;
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
        msg_log::add("There are some items inside.",
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);

        item_container_.open(pos_, map::player);
    }
}

DidOpen Cocoon::open(Actor* const actor_opening)
{
    if (is_open_)
    {
        return DidOpen::no;
    }
    else //Was not already open
    {
        is_open_ = true;

        render::draw_map_state(UpdateScreen::yes);

        if (map::cells[pos_.x][pos_.y].is_seen_by_player)
        {
            msg_log::add("The cocoon opens.");
        }

        trigger_trap(actor_opening);

        return DidOpen::yes;
    }
}

std::string Cocoon::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";

    if (burn_state() == BurnState::burning)
    {
        ret += "burning ";
    }

    return ret + "cocoon";
}

TileId Cocoon::tile() const
{
    return is_open_ ? TileId::cocoon_open : TileId::cocoon_closed;
}

Clr Cocoon::clr_default() const
{
    return clr_white;
}
