#include "feature_monolith.hpp"

#include "msg_log.hpp"
#include "dungeon_master.hpp"
#include "map.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_factory.hpp"

Monolith::Monolith(const P& p) :
    Rigid           (p),
    is_activated_   (false) {}

void Monolith::on_hit(const DmgType dmg_type,
                      const DmgMethod dmg_method,
                      Actor* const actor)
{
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Monolith::name(const Article article) const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "carved monolith";
}

Clr Monolith::clr_default() const
{
    return is_activated_ ? clr_red : clr_cyan_lgt;
}

void Monolith::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        msg_log::add("I recite the inscriptions carved on the Monolith!");

        if (is_activated_)
        {
            msg_log::add("Nothing happens.");
        }
        else //Not yet activated
        {
            activate();
        }

        render::draw_map_state();
    }
}

void Monolith::activate()
{
    dungeon_master::incr_player_xp(25);

    is_activated_ = true;

    for (const P& d : dir_utils::dir_list)
    {
        const P p(pos_ + d);

        map::mk_blood(p);

        if (rnd::fraction(2, 3))
        {
            map::mk_gore(p);
        }
    }

    //Spawn monsters?
    if (rnd::coin_toss())
    {
        spawn_monsters();
    }
}

void Monolith::spawn_monsters()
{
    std::vector<ActorId> summon_bucket;

    for (int i = 0; i < (int)ActorId::END; ++i)
    {
        const ActorDataT& data = actor_data::data[i];

        if (data.can_be_summoned)
        {
            if (data.spawn_min_dlvl <= (map::dlvl + 8))
            {
                summon_bucket.push_back(ActorId(i));
            }
        }
    }

    if (summon_bucket.empty())
    {
        TRACE << "No elligible monsters found for spawning" << std::endl;
        ASSERT(false);
        return;
    }

    const ActorId mon_id = rnd::element(summon_bucket);

    std::vector<Mon*> mon_summoned;

    const size_t nr_mon = rnd::range(2, 3);

    actor_factory::summon(pos_,
                          {nr_mon, mon_id},
                          MakeMonAware::yes,
                          nullptr,
                          &mon_summoned);

    for (Mon* mon : mon_summoned)
    {
        mon->become_aware_player(false);

        mon->prop_handler().try_add(new PropWaiting(PropTurns::specific, 1));
    }
}
