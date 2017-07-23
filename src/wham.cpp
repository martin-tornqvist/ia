#include "wham.hpp"

#include "init.hpp"
#include "game_time.hpp"
#include "feature_rigid.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "io.hpp"
#include "map_parsing.hpp"
#include "text_format.hpp"
#include "item_factory.hpp"
#include "feature_door.hpp"

namespace wham
{

void try_sprain_player()
{
    if (map::player->has_prop(PropId::frenzied))
    {
        return;
    }

    int sprain_one_in_n;

    if (player_bon::traits[(size_t)Trait::rugged])
    {
        sprain_one_in_n = 12;
    }
    else if (player_bon::traits[(size_t)Trait::tough])
    {
        sprain_one_in_n = 8;
    }
    else // No bonus
    {
        sprain_one_in_n = 4;
    }

    if (rnd::one_in(sprain_one_in_n))
    {
        msg_log::add("I sprain myself.", clr_msg_bad);

        const int dmg = rnd::range(1, 2);

        map::player->hit(dmg, DmgType::pure);
    }
}

void run()
{
    TRACE_FUNC_BEGIN;

    msg_log::clear();

    //
    // Choose direction
    //
    msg_log::add("Which direction?" + cancel_info_str,
                 clr_white_lgt);

    const Dir input_dir = query::dir(AllowCenter::yes);

    msg_log::clear();

    if (input_dir == Dir::END)
    {
        // Invalid direction
        io::update_screen();

        TRACE_FUNC_END;

        return;
    }

    // The chosen direction is valid

    P att_pos(map::player->pos + dir_utils::offset(input_dir));

    //
    // Kick living actor?
    //
    TRACE << "Checking if player is kicking a living actor" << std::endl;

    if (input_dir != Dir::center)
    {
        Actor* living_actor =
            map::actor_at_pos(att_pos, ActorState::alive);

        if (living_actor)
        {
            TRACE << "Actor found at kick pos" << std::endl;

            const bool melee_allowed =
                map::player->prop_handler().allow_attack_melee(
                    Verbosity::verbose);

            if (melee_allowed)
            {
                TRACE << "Player is allowed to do melee attack"
                      << std::endl;

                bool blocked[map_w][map_h];

                map_parsers::BlocksLos()
                    .run(blocked);

                TRACE << "Player can see actor" << std::endl;

                map::player->kick_mon(*living_actor);

                try_sprain_player();

                game_time::tick();
            }

            TRACE_FUNC_END;
            return;
        }
    }

    //
    // Destroy corpse?
    //
    TRACE << "Checking if player is destroying a corpse" << std::endl;
    Actor* corpse = nullptr;

    // Check all corpses here, stop at any corpse which is prioritized for
    // bashing (Zombies)
    for (Actor* const actor : game_time::actors)
    {
        if ((actor->pos == att_pos) &&
            (actor->state() == ActorState::corpse))
        {
            corpse = actor;

            if (actor->data().prio_corpse_bash)
            {
                break;
            }
        }
    }

    const auto kick_wpn =
        std::unique_ptr<Wpn>(
            static_cast<Wpn*>(
                item_factory::mk(ItemId::player_kick)));

    const auto* const wielded_wpn =
        map::player->inv().item_in_slot(SlotId::wpn);

    if (corpse)
    {
        const bool is_seeing_cell =
            map::cells[att_pos.x][att_pos.y].is_seen_by_player;

        std::string corpse_name =
            is_seeing_cell ?
            corpse->corpse_name_the() :
            "a corpse";

        corpse_name = text_format::first_to_lower(corpse_name);

        // Decide if we should kick or use wielded weapon
        const bool can_wpn_att_corpse =
            wielded_wpn &&
            wielded_wpn->data().melee.att_corpse;

        const auto* const wpn_used =
            (wielded_wpn && can_wpn_att_corpse) ?
            wielded_wpn :
            kick_wpn.get();

        const std::string msg =
            "I " +
            wpn_used->data().melee.att_msgs.player + " "  +
            corpse_name + ".";

        msg_log::add(msg);

        const Dice dmg_dice =
            wpn_used->dmg(AttMode::melee, map::player);

        const int dmg = dmg_dice.roll();

        corpse->hit(dmg,
                    DmgType::physical,
                    wpn_used->data().melee.dmg_method);

        if (wpn_used == kick_wpn.get())
        {
            try_sprain_player();
        }

        // Print message if corpse was destroyed, and there are more corpses
        if (corpse->state() == ActorState::destroyed)
        {
            std::vector<Actor*> corpses_here;

            for (auto* const actor : game_time::actors)
            {
                if ((actor->pos == att_pos) &&
                    (actor->state() == ActorState::corpse))
                {
                    corpses_here.push_back(actor);
                }
            }

            if (!corpses_here.empty())
            {
                msg_log::more_prompt();

                for (auto* const corpse : corpses_here)
                {
                    const std::string name = corpse->corpse_name_a();

                    msg_log::add(name + ".");
                }
            }
        }

        game_time::tick();

        TRACE_FUNC_END;

        return;
    }

    //
    // Attack feature
    //
    TRACE << "Checking if player is kicking a feature" << std::endl;

    if (input_dir != Dir::center)
    {
        // Decide if we should kick or use wielded weapon
        auto* const feature = map::cells[att_pos.x][att_pos.y].rigid;

        bool allow_wielded_wpn_att_rigid =
            wielded_wpn &&
            wielded_wpn->data().melee.att_rigid;

        const auto wielded_wpn_dmg_method =
            wielded_wpn ?
            wielded_wpn->data().melee.dmg_method :
            DmgMethod::END;

        if (allow_wielded_wpn_att_rigid)
        {
            switch (feature->id())
            {
            case FeatureId::door:
            {
                const auto* const door = static_cast<const Door*>(feature);

                const auto door_type = door->type();

                if (door_type == DoorType::gate)
                {
                    // Only allow blunt weapons for gates (feels weird to attack
                    // a barred gate with an axe...)
                    allow_wielded_wpn_att_rigid =
                        wielded_wpn_dmg_method == DmgMethod::blunt;
                }
                else // Not gate (i.e. wooden, metal)
                {
                    allow_wielded_wpn_att_rigid = true;
                }
            }
            break;

            case FeatureId::wall:
            {
                allow_wielded_wpn_att_rigid = true;
            }
            break;

            default:
            {
                allow_wielded_wpn_att_rigid = false;
            }
            break;
            }
        }

        const auto* const wpn_used =
            (wielded_wpn && allow_wielded_wpn_att_rigid) ?
            wielded_wpn :
            kick_wpn.get();

        const Dice dmg_dice =
            wpn_used->dmg(AttMode::melee, map::player);

        const int dmg = dmg_dice.roll();

        feature->hit(dmg,
                     DmgType::physical,
                     wpn_used->data().melee.dmg_method,
                     map::player);

        game_time::tick();
    }

    TRACE_FUNC_END;
}

} // wham
