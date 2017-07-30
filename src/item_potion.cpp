#include "item_potion.hpp"

#include "init.hpp"
#include "properties.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "actor_mon.hpp"
#include "player_spells.hpp"
#include "item_scroll.hpp"
#include "game_time.hpp"
#include "audio.hpp"
#include "io.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "item_factory.hpp"
#include "saving.hpp"
#include "game.hpp"

ConsumeItem Potion::activate(Actor* const actor)
{
    ASSERT(actor);

    if (!actor->prop_handler().allow_eat(Verbosity::verbose))
    {
        return ConsumeItem::no;
    }

    if (actor->is_player())
    {
        data_->is_tried = true;

        audio::play(SfxId::potion_quaff);

        if (data_->is_identified)
        {
            const std::string potion_name =
                name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I drink " + potion_name + "...");
        }
        else // Not identified
        {
            const std::string potion_name =
                name(ItemRefType::plain, ItemRefInf::none);

            msg_log::add("I drink an unknown " + potion_name + "...");
        }

        map::player->incr_shock(ShockLvl::terrifying,
                                ShockSrc::use_strange_item);

        if (!map::player->is_alive())
        {
            return ConsumeItem::yes;
        }
    }

    quaff_impl(*actor);

    if (map::player->is_alive())
    {
        game_time::tick();
    }

    return ConsumeItem::yes;
}

void Potion::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_after =
                name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I have identified " + name_after + ".");

            game::add_history_event("Identified " + name_after + ".");

            give_xp_for_identify();
        }
    }
}

std::vector<std::string> Potion::descr() const
{
    if (data_->is_identified)
    {
        return
        {
            descr_identified()
        };
    }
    else // Not identified
    {
        return data_->base_descr;
    }
}

void Potion::on_collide(const P& pos, Actor* const actor)
{
    const auto& cell = map::cells[pos.x][pos.y];

    if (!cell.rigid->is_bottomless() || actor)
    {
        // Render and print message
        const bool player_see_cell = cell.is_seen_by_player;

        if (player_see_cell)
        {
            if (actor)
            {
                if (actor->is_alive())
                {
                    const std::string actor_name =
                        map::player->can_see_actor(*actor) ?
                        actor->name_the() :
                        "it";

                    msg_log::add("The potion shatters on " + actor_name + ".");
                }
            }
            else // No actor here
            {
                msg_log::add("The potion shatters on " +
                             cell.rigid->name(Article::the) + ".");
            }
        }

        if (actor)
        {
            // If the blow from the bottle didn't kill the actor, apply effect
            if (actor->is_alive())
            {
                collide_hook(pos, actor);

                if (actor->is_alive() &&
                    !data_->is_identified &&
                    player_see_cell)
                {
                    // This did not identify the potion
                    msg_log::add("It had no apparent effect...");
                }
            }
        }
    }
}

std::string Potion::name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

void PotionVitality::quaff_impl(Actor& actor)
{
    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
        PropId::wound
    };

    for (PropId prop_id : props_can_heal)
    {
        actor.prop_handler().end_prop(prop_id);
    }

    // HP is always restored at least up to maximum hp, but can go beyond
    const int hp = actor.hp();
    const int hp_max = actor.hp_max(true);
    const int hp_restored = std::max(20, hp_max - hp);

    actor.restore_hp(hp_restored, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionVitality::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionSpirit::quaff_impl(Actor& actor)
{
    // SPI is always restored at least up to maximum spi, but can go beyond
    const int spi = actor.spi();
    const int spi_max = actor.spi_max();
    const int spi_restored = std::max(10, spi_max - spi);

    actor.restore_spi(spi_restored, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionSpirit::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionBlindness::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropBlind(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionBlindness::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionParal::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropParalyzed(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionParal::collide_hook(const P& pos, Actor* const actor)
{

    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionDisease::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropDiseased(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionConf::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropConfused(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionConf::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionFortitude::quaff_impl(Actor& actor)
{
    PropHandler& prop_handler = actor.prop_handler();

    PropRFear* const r_fear =
        new PropRFear(PropTurns::std);

    const int nr_turns_left =
        r_fear->nr_turns_left();

    PropRConf* const r_conf =
        new PropRConf(PropTurns::specific,  nr_turns_left);

    PropRSleep* const r_sleep =
        new PropRSleep(PropTurns::specific, nr_turns_left);

    prop_handler.apply(r_fear);
    prop_handler.apply(r_conf);
    prop_handler.apply(r_sleep);

    prop_handler.end_prop(PropId::frenzied);

    // Remove a random insanity symptom if this is the player
    if (actor.is_player())
    {
        const std::vector<const InsSympt*> sympts = insanity::active_sympts();

        if (!sympts.empty())
        {
            const size_t idx = rnd::range(0, sympts.size() - 1);

            const InsSymptId id = sympts[idx]->id();

            insanity::end_sympt(id);
        }

        map::player->restore_shock(999, false);

        msg_log::add("I feel more at ease.");
    }

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionFortitude::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionPoison::quaff_impl(Actor& actor)
{
    const Range dmg_range(8, 11);

    actor.prop_handler().apply(
        new PropPoisoned(
            PropTurns::specific,
            poison_dmg_n_turn * dmg_range.roll()));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionPoison::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionRFire::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropRFire(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionRFire::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionCuring::quaff_impl(Actor& actor)
{
    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
    };

    bool is_noticable = false;

    for (PropId prop_id : props_can_heal)
    {
        if (actor.prop_handler().end_prop(prop_id))
        {
            is_noticable = true;
        }
    }

    if (actor.restore_hp(3, false /* Not allowed above max */))
    {
        is_noticable = true;
    }

    if (!is_noticable &&
        actor.is_player())
    {
        msg_log::add("I feel fine.");

        is_noticable = true;
    }

    if (is_noticable &&
        map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionCuring::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionRElec::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropRElec(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionRElec::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionRAcid::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropRAcid(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionRAcid::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void PotionDescent::quaff_impl(Actor& actor)
{
    (void)actor;

    if (map::dlvl < dlvl_last - 1)
    {
        if (!map::player->has_prop(PropId::descend))
        {
            map::player->prop_handler().apply(new PropDescend(PropTurns::std));
        }
    }
    else // Dungeon level is near the end
    {
        msg_log::add("I feel a faint sinking sensation, "
                     "but it soon disappears...");
    }

    identify(Verbosity::verbose);
}

void PotionInvis::quaff_impl(Actor& actor)
{
    actor.prop_handler().apply(new PropInvisible(PropTurns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void PotionInvis::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

//
// TODO: Implement sacrifice life/spirit as potions
//

// void SpellSacrLife::run_effect(Actor* const caster,
//                                const SpellSkill skill) const
// {
//     (void)caster;

//     // Convert every 2 HP to 1 SP

//     const int player_hp_cur = map::player->hp();

//     if (player_hp_cur > 2)
//     {
//         const int hp_drained = ((player_hp_cur - 1) / 2) * 2;

//         map::player->hit(hp_drained, DmgType::pure);

//         map::player->restore_spi(hp_drained, true);

//         if (skill >= 50)
//         {
//             SpellAzaWrath spell;

//             spell.run_effect(caster, skill);
//         }

//         if (skill >= 100)
//         {
//             SpellMayhem  spell;

//             spell.run_effect(caster, skill);
//         }
//     }
//     else // Not enough HP
//     {
//         msg_log::add("I feel like I have very little to offer.");
//     }
// }

// std::vector<std::string> SpellSacrLife::descr_specific(
//     const SpellSkill skill) const
// {
//     std::vector<std::string> descr;

//     descr.push_back(
//         "Brings the caster to the brink of death in order to restore the "
//         "spirit. The amount restored is proportional to the life "
//         "sacrificed.");

//     if (skill >= 50)
//     {
//         SpellAzaWrath spell;

//         const std::string wrath_name = spell.name();

//         descr.push_back(
//             "If any health was sacrificed, the spell \"" + wrath_name + "\""
//             "is also cast.");
//     }

//     if (skill >= 100)
//     {
//         SpellMayhem spell;

//         const std::string name = spell.name();

//         descr.push_back("Also casts the spell \"" + name + "\".");
//     }

//     return descr;
// }

// void SpellSacrSpi::run_effect(Actor* const caster,
//                               const SpellSkill skill) const
// {
//     (void)caster;

//     // Convert all SPI to HP

//     const int player_spi_cur = map::player->spi();

//     if (player_spi_cur > 0)
//     {
//         const int spi_drained = player_spi_cur - 1;

//         map::player->hit_spi(spi_drained);

//         map::player->restore_hp(hp_drained);

//         std::vector<PropId> props_can_heal;

//         if (skill >= 50)
//         {
//             props_can_heal.push_back(PropId::poisoned);
//         }

//         if (skill >= 100)
//         {
//             props_can_heal.push_back(PropId::blind);
//             props_can_heal.push_back(PropId::infected);
//             props_can_heal.push_back(PropId::diseased);
//             props_can_heal.push_back(PropId::weakened);
//         }

//         for (PropId prop_id : props_can_heal)
//         {
//             caster->prop_handler().end_prop(prop_id);
//         }
//     }
//     else // Not enough spirit
//     {
//         msg_log::add("I feel like I have very little to offer.");
//     }
// }

// std::vector<std::string> SpellSacrSpi::descr_specific(
//     const SpellSkill skill) const
// {
//     std::vector<std::string> descr;

//     descr.push_back(
//         "Brings the caster to the brink of spiritual death in order to "
//         "restore health. The amount restored is proportional to the "
//         "spirit sacrificed.");

//     if (skill >= 100)
//     {
//         descr.push_back(
//             "If any spirit was sacrificed, cures blindness, poisoning, "
//             "infections, disease, and weakening.");
//     }
//     // skill < 100
//     else if (skill >= 50)
//     {
//         descr.push_back("If any spirit was sacrificed, cures poisoning.");
//     }

//     return descr;
// }

namespace potion_handling
{

namespace
{

std::vector<PotionLook> potion_looks_;

} // namespace

void init()
{
    TRACE_FUNC_BEGIN;

    // Init possible potion colors and fake names
    potion_looks_.assign(
    {
        {"Golden",    "a Golden",     clr_yellow},
        {"Yellow",    "a Yellow",     clr_yellow},
        {"Dark",      "a Dark",       clr_gray},
        {"Black",     "a Black",      clr_gray},
        {"Oily",      "an Oily",      clr_gray},
        {"Smoky",     "a Smoky",      clr_white},
        {"Slimy",     "a Slimy",      clr_green},
        {"Green",     "a Green",      clr_green_lgt},
        {"Fiery",     "a Fiery",      clr_red_lgt},
        {"Murky",     "a Murky",      clr_brown_drk},
        {"Muddy",     "a Muddy",      clr_brown},
        {"Violet",    "a Violet",     clr_violet},
        {"Orange",    "an Orange",    clr_orange},
        {"Watery",    "a Watery",     clr_blue_lgt},
        {"Metallic",  "a Metallic",   clr_gray},
        {"Clear",     "a Clear",      clr_white_lgt},
        {"Misty",     "a Misty",      clr_white_lgt},
        {"Bloody",    "a Bloody",     clr_red},
        {"Magenta",   "a Magenta",    clr_magenta},
        {"Clotted",   "a Clotted",    clr_green},
        {"Moldy",     "a Moldy",      clr_brown},
        {"Frothy",    "a Frothy",     clr_white}
    });

    for (auto& d : item_data::data)
    {
        if (d.type == ItemType::potion)
        {
            // Color and false name
            const size_t idx = rnd::range(0, potion_looks_.size() - 1);

            PotionLook& look = potion_looks_[idx];

            d.base_name_un_id.names[(size_t)ItemRefType::plain]
                = look.name_plain + " Potion";

            d.base_name_un_id.names[(size_t)ItemRefType::plural] =
                look.name_plain + " Potions";

            d.base_name_un_id.names[(size_t)ItemRefType::a] =
                look.name_a     + " Potion";

            d.clr = look.clr;

            potion_looks_.erase(potion_looks_.begin() + idx);

            // True name
            const Potion* const potion =
                static_cast<const Potion*>(item_factory::mk(d.id, 1));

            const std::string real_type_name = potion->real_name();

            delete potion;

            const std::string real_name =
                "Potion of " + real_type_name;

            const std::string real_name_plural =
                "Potions of " + real_type_name;

            const std::string real_name_a =
                "a Potion of " + real_type_name;

            d.base_name.names[(size_t)ItemRefType::plain]  = real_name;
            d.base_name.names[(size_t)ItemRefType::plural] = real_name_plural;
            d.base_name.names[(size_t)ItemRefType::a]      = real_name_a;
        }
    }

    TRACE_FUNC_END;
}

void save()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        ItemDataT& d = item_data::data[i];

        if (d.type == ItemType::potion)
        {
            saving::put_str(
                d.base_name_un_id.names[(size_t)ItemRefType::plain]);

            saving::put_str(
                d.base_name_un_id.names[(size_t)ItemRefType::plural]);

            saving::put_str(
                d.base_name_un_id.names[(size_t)ItemRefType::a]);

            saving::put_int(d.clr.r);
            saving::put_int(d.clr.g);
            saving::put_int(d.clr.b);
        }
    }
}

void load()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        ItemDataT& d = item_data::data[i];

        if (d.type == ItemType::potion)
        {
            d.base_name_un_id.names[(size_t)ItemRefType::plain] =
                saving::get_str();

            d.base_name_un_id.names[(size_t)ItemRefType::plural] =
                saving::get_str();

            d.base_name_un_id.names[(size_t)ItemRefType::a] =
                saving::get_str();

            d.clr.r = saving::get_int();
            d.clr.g = saving::get_int();
            d.clr.b = saving::get_int();
        }
    }
}

} // potion_handling
