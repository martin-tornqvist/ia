#include "feature_monolith.hpp"

#include "msg_log.hpp"
#include "game.hpp"
#include "map.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_factory.hpp"

Monolith::Monolith(const P& p) :
    Rigid           (p),
    is_activated_   (false) {}

void Monolith::on_hit(const int dmg,
                      const DmgType dmg_type,
                      const DmgMethod dmg_method,
                      Actor* const actor)
{
    (void)dmg;
    (void)dmg_type;
    (void)dmg_method;
    (void)actor;
}

std::string Monolith::name(const Article article) const
{
    std::string ret =
        article == Article::a ?
        "a " : "the ";

    return ret + "carved monolith";
}

Clr Monolith::clr_default() const
{
    return
        is_activated_ ?
        clr_gray :
        clr_cyan_lgt;
}

void Monolith::bump(Actor& actor_bumping)
{
    if (actor_bumping.is_player())
    {
        if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("There is a carved rock here.");

            return;
        }

        msg_log::add("I recite the inscriptions on the Monolith...");

        if (is_activated_)
        {
            msg_log::add("Nothing happens.");
        }
        else // Not yet activated
        {
            activate();

            map::player->incr_shock(ShockLvl::terrifying,
                                    ShockSrc::misc);
        }
    }
}

void Monolith::activate()
{
    msg_log::add("I feel powerful!");

    game::incr_player_xp(15);

    is_activated_ = true;
}
