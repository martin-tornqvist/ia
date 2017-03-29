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
    is_activated_(false)
{
    if (id == PylonId::any)
    {
        // Make most pylons of the burning type
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

    str += " Pylon";

    return str;
}

Clr Pylon::clr_default() const
{
    return
        is_activated_ ?
        clr_red_lgt :
        clr_gray;
}

void Pylon::on_hit(const DmgType dmg_type,
                   const DmgMethod dmg_method,
                   Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;

    //
    // TODO:
    //
}

void Pylon::on_new_turn_hook()
{
    if (is_activated_)
    {
        pylon_impl_->on_new_turn_activated();
    }
}

void Pylon::on_lever_pulled(Lever* const lever)
{
    (void)lever;

    is_activated_ = !is_activated_;

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
    for (P d : dir_utils::dir_list_w_center)
    {
        const P p_adj(pos_ + d);

        map::cells[p_adj.x][p_adj.y].rigid->hit(
            DmgType::fire,
            DmgMethod::elemental);
    }

    if (!rnd::fraction(2, 3))
    {
        return;
    }

    // emit_trigger_snd();

    auto actors = living_actors_reached();

    for (auto actor : actors)
    {
        actor->prop_handler().try_add(
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
        actor->prop_handler().try_add(
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
        actor->prop_handler().try_add(
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
        actor->prop_handler().try_add(
            new PropTerrified(PropTurns::std));
    }
}
