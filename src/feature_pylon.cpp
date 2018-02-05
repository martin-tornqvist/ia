#include "feature_pylon.hpp"

#include "map_parsing.hpp"
#include "game_time.hpp"
#include "actor.hpp"
#include "map.hpp"
#include "random.hpp"
#include "msg_log.hpp"
#include "sound.hpp"
#include "knockback.hpp"

// -----------------------------------------------------------------------------
// Pylon
// -----------------------------------------------------------------------------
Pylon::Pylon(const P& p, PylonId id) :
    Rigid(p),
    pylon_impl_(nullptr),
    is_activated_(false),
    nr_turns_active_(0)
{
    if (id == PylonId::any)
    {
        if (rnd::coin_toss())
        {
            id = PylonId::burning;
        }
        else // Pick randomly
        {
            id = (PylonId)rnd::range(0, (int)PylonId::END - 1);
        }
    }

    pylon_impl_.reset(mk_pylon_impl_from_id(id));
}

PylonImpl* Pylon::mk_pylon_impl_from_id(const PylonId id)
{
    switch(id)
    {
    case PylonId::burning:
        return new PylonBurning(pos_, this);

    case PylonId::invis:
        return new PylonInvis(pos_, this);

    case PylonId::slow:
        return new PylonSlow(pos_, this);

    case PylonId::knockback:
        return new PylonKnockback(pos_, this);

    case PylonId::teleport:
        return new PylonTeleport(pos_, this);

    case PylonId::terrify:
        return new PylonTerrify(pos_, this);

    case PylonId::any:
    case PylonId::END:
        break;
    }

    TRACE << "Bad PylonId: " << (int)id << std::endl;

    ASSERT(false);

    return nullptr;
}

std::string Pylon::name(const Article article) const
{
    std::string str =
        ((article == Article::a) ?
         (is_activated_ ? "an " : "a ") :
         "the ");

    str +=
        is_activated_ ?
        "activated " :
        "deactivated ";

    str += "Pylon";

    return str;
}

Color Pylon::color_default() const
{
    return
        is_activated_ ?
        colors::light_red() :
        colors::gray();
}

void Pylon::on_hit(const int dmg,
                   const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
{
    (void)dmg;
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;

    //
    // TODO
    //
}

void Pylon::on_new_turn_hook()
{
    if (is_activated_)
    {
        pylon_impl_->on_new_turn_activated();

        ++nr_turns_active_;

        // After a being active for a while, deactivate the pylon by toggling
        // the linked lever
        const int max_nr_turns_active = 300;

        if (nr_turns_active_ >= max_nr_turns_active)
        {
            TRACE << "Pylon timed out, deactivating by toggling lever"
                  << std::endl;

            for (int x = 0; x < map_w; ++x)
            {
                for (int y = 0; y < map_h; ++y)
                {
                    auto* const rigid = map::cells[x][y].rigid;

                    if (rigid && (rigid->id() == FeatureId::lever))
                    {
                        auto* const lever = static_cast<Lever*>(rigid);

                        if (lever->is_linked_to(*this))
                        {
                            lever->toggle();

                            break;
                        }
                    }
                }
            }
        }
    }
}

void Pylon::on_lever_pulled(Lever* const lever)
{
    (void)lever;

    is_activated_ = !is_activated_;

    nr_turns_active_ = 0;

    const bool is_seen_by_player =
        map::cells[pos_.x][pos_.y].is_seen_by_player;

    if (is_activated_)
    {
        std::string msg =
            is_seen_by_player ?
            "The pylon makes " :
            "I hear ";

        msg += "a droning sound.";

        Snd snd(msg,
                SfxId::END, // TODO: Add a sound effect
                IgnoreMsgIfOriginSeen::no,
                pos_,
                nullptr,
                SndVol::low,
                AlertsMon::no);

        snd.run();
    }
    // Deactivated
    else if (is_seen_by_player)
    {
        msg_log::add("The Pylon shuts down");
    }
}

void Pylon::add_light_hook(bool light[map_w][map_h]) const
{
    if (is_activated_)
    {
        for (const P& d : dir_utils::dir_list_w_center)
        {
            const P p(pos_ + d);

            light[p.x][p.y] = true;
        }
    }
 }

// -----------------------------------------------------------------------------
// Pylon implementation
// -----------------------------------------------------------------------------
// void PylonImpl::emit_trigger_snd() const
// {
//     const std::string msg =
//         map::cells[pos_.x][pos_.y].is_seen_by_player ?
//         "The pylon makes a buzzing sound." :
//         "I hear a buzzing sound.";

//     Snd snd(msg,
//             SfxId::END, // TODO: Add a sound effect
//             IgnoreMsgIfOriginSeen::no,
//             pos_,
//             nullptr,
//             SndVol::high,
//             AlertsMon::no);

//     snd.run();
// }

std::vector<Actor*> PylonImpl::living_actors_reached() const
{
    std::vector<Actor*> actors;

    for (auto* const actor : game_time::actors)
    {
        // Actor is dead?
        if (actor->state() != ActorState::alive)
        {
            continue;
        }

        const P& p = actor->pos;

        const int d = 1;

        // Actor is out of range?
        if (king_dist(pos_, p) > d)
        {
            continue;
        }

        actors.push_back(actor);
    }

    return actors;
}

Actor* PylonImpl::rnd_reached_living_actor() const
{
    auto actors = living_actors_reached();

    if (actors.empty())
    {
        return nullptr;
    }

    Actor* actor = rnd::element(living_actors_reached());

    return actor;
}

// -----------------------------------------------------------------------------
// Burning pylon
// -----------------------------------------------------------------------------
void PylonBurning::on_new_turn_activated()
{
    //
    // Do a floodfill from the pylon, and apply burning on each feature reached.
    // The flood distance is increased every N turn.
    //
    bool blocks_flood[map_w][map_h];

    map_parsers::BlocksProjectiles()
        .run(blocks_flood);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const auto r = map::cells[x][y].rigid;

            if (r->is_bottomless())
            {
                blocks_flood[x][y] = true;
            }
            // Doors must not block the floodfill, otherwise if a door is
            // opened, the flood can suddenly reach much further and completely
            // surround the player - which is very unfair.
            // Note however that this could also happen if e.g. a piece of wall
            // is destroyed, then it will seem like the fire is rushing in.
            // It's questionable if this is a good behavior or not, but keeping
            // it like this for now...
            else if (r->id() == FeatureId::door)
            {
                blocks_flood[x][y] = false;
            }
        }
    }

    int flood[map_w][map_h];

    const int nr_turns_active = pylon_->nr_turns_active();

    const int nr_turns_per_flood_step = 10;

    //
    // NOTE: The distance may also be limited by the number of turns which the
    //       Pylon remains active (it shuts itself down after a while)
    //
    const int flood_max_dist = 10;

    const int flood_dist =
        std::min(
            flood_max_dist,
            (nr_turns_active / nr_turns_per_flood_step) + 1);

    floodfill(pos_,
              blocks_flood,
              flood,
              flood_dist);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (flood[x][y] > 0)
            {
                map::cells[x][y].rigid->hit(
                    1, // Doesn't matter
                    DmgType::fire,
                    DmgMethod::elemental);
            }
        }
    }

    //
    // Occasionally also directly burn adjacent actors
    //
    if (!rnd::fraction(2, 3))
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        actor->apply_prop(
            new PropBurning(PropTurns::std));
    }
}

// -----------------------------------------------------------------------------
// Invisibility Pylon
// -----------------------------------------------------------------------------
void PylonInvis::on_new_turn_activated()
{
    auto* const actor = rnd_reached_living_actor();

    if (!actor)
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        actor->apply_prop(
            new PropInvisible(PropTurns::std));
    }
}

// -----------------------------------------------------------------------------
// Burning pylon
// -----------------------------------------------------------------------------
void PylonSlow::on_new_turn_activated()
{
    auto* const actor = rnd_reached_living_actor();

    if (!actor)
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        actor->apply_prop(
            new PropSlowed(PropTurns::std));
    }
}

// -----------------------------------------------------------------------------
// Knockback pylon
// -----------------------------------------------------------------------------
void PylonKnockback::on_new_turn_activated()
{
    if (!rnd::fraction(2, 3))
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        knockback::run(*actor,
                       pos_,
                       false,               // Not spike gun
                       Verbosity::verbose,
                       2);                  // Extra paralyze turns
    }
}

// -----------------------------------------------------------------------------
// Knockback pylon
// -----------------------------------------------------------------------------
void PylonTeleport::on_new_turn_activated()
{
    if (rnd::coin_toss())
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        actor->teleport();
    }
}

// -----------------------------------------------------------------------------
// Invisibility Pylon
// -----------------------------------------------------------------------------
void PylonTerrify::on_new_turn_activated()
{
    auto* const actor = rnd_reached_living_actor();

    if (!actor)
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        actor->apply_prop(
            new PropTerrified(PropTurns::std));
    }
}
