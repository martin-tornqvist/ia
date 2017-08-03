#include "player_bon.hpp"

#include "init.hpp"
#include "text_format.hpp"
#include "actor_player.hpp"
#include "game.hpp"
#include "item_factory.hpp"
#include "inventory.hpp"
#include "player_spells.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "create_character.hpp"
#include "saving.hpp"

namespace player_bon
{

bool traits[(size_t)Trait::END];

namespace
{

Bg bg_ = Bg::END;

bool is_trait_blocked_for_bg(const Trait trait, const Bg bg)
{
    switch (trait)
    {
    case Trait::adept_melee_fighter:
    case Trait::expert_melee_fighter:
    case Trait::master_melee_fighter:
    case Trait::adept_marksman:
    case Trait::expert_marksman:
    case Trait::master_marksman:
    case Trait::steady_aimer:
    case Trait::sharpshooter:
    case Trait::fast_shooter:
    case Trait::cool_headed:
    case Trait::perseverant:
    case Trait::observant:
    case Trait::ravenous:
    case Trait::foul:
    case Trait::toxic:
    case Trait::indomitable_fury:
    case Trait::vigilant:
    case Trait::self_aware:
    case Trait::stout_spirit:
    case Trait::strong_spirit:
    case Trait::mighty_spirit:
    case Trait::stealthy:
    case Trait::silent:
    case Trait::imperceptible:
    case Trait::vicious:
    case Trait::treasure_hunter:
    case Trait::undead_bane:
    case Trait::courageous:
    case Trait::lesser_invoc:
    case Trait::greater_invoc:
    case Trait::lesser_summoning:
    case Trait::greater_summoning:
    case Trait::lesser_clairv:
    case Trait::greater_clairv:
    case Trait::lesser_ench:
    case Trait::greater_ench:
    case Trait::lesser_alter:
    case Trait::greater_alter:
    case Trait::blood_sorc:
    case Trait::tough:
    case Trait::rugged:
    case Trait::thick_skinned:
    case Trait::strong_backed:
    case Trait::dexterous:
    case Trait::lithe:
    case Trait::mobile:
    case Trait::fearless:
        break;

    case Trait::healer:
        // Cannot use Medial Bag
        return bg == Bg::ghoul;

    case Trait::dem_expert:
        // Too much character theme mismatch
        return
            (bg == Bg::occultist) ||
            (bg == Bg::ghoul);

    case Trait::rapid_recoverer:
        // Cannot regen hp passively
        return bg == Bg::ghoul;

    case Trait::survivalist:
        // Has RDISEASE already + a bit of theme mismatch
        return bg == Bg::ghoul;

    case Trait::elec_incl:
        return bg == Bg::ghoul;
        break;

    case Trait::END:
        break;
    }

    return false;
}

} // namespace

void init()
{
    for (size_t i = 0; i < (size_t)Trait::END; ++i)
    {
        traits[i] = false;
    }

    bg_ = Bg::END;
}

void save()
{
    saving::put_int((int)bg_);

    for (size_t i = 0; i < (size_t)Trait::END; ++i)
    {
        saving::put_bool(traits[i]);
    }
}

void load()
{
    bg_ = Bg(saving::get_int());

    for (size_t i = 0; i < (size_t)Trait::END; ++i)
    {
        traits[i] = saving::get_bool();
    }
}

std::string bg_title(const Bg id)
{
    switch (id)
    {
    case Bg::ghoul:
        return "Ghoul";

    case Bg::occultist:
        return "Occultist";

    case Bg::rogue:
        return "Rogue";

    case Bg::war_vet:
        return "War Veteran";

    case Bg::END:
        break;
    }

    ASSERT(false);

    return "[BG TITLE MISSING]";
}

std::string trait_title(const Trait id)
{
    switch (id)
    {
    case Trait::adept_melee_fighter:
        return "Adept Melee Fighter";

    case Trait::expert_melee_fighter:
        return "Expert Melee Fighter";

    case Trait::master_melee_fighter:
        return "Master Melee Fighter";

    case Trait::cool_headed:
        return "Cool-headed";

    case Trait::courageous:
        return "Courageous";

    case Trait::lesser_invoc:
        return "Lesser Invocation";

    case Trait::greater_invoc:
        return "Greater Invocation";

    case Trait::lesser_summoning:
        return "Lesser Summoning";

    case Trait::greater_summoning:
        return "Greater Summoning";

    case Trait::lesser_clairv:
        return "Lesser Clairvoyance";

    case Trait::greater_clairv:
        return "Greater Clairvoyance";

    case Trait::lesser_ench:
        return "Lesser Enchantment";

    case Trait::greater_ench:
        return "Greater Enchantment";

    case Trait::lesser_alter:
        return "Lesser Alteration";

    case Trait::greater_alter:
        return "Greater Alteration";

    case Trait::blood_sorc:
        return "Blood Sorcerer";

    case Trait::dexterous:
        return "Dexterous";

    case Trait::lithe:
        return "Lithe";

    case Trait::mobile:
        return "Mobile";

    case Trait::fearless:
        return "Fearless";

    case Trait::healer:
        return "Healer";

    case Trait::adept_marksman:
        return "Adept Marksman";

    case Trait::expert_marksman:
        return "Expert Marksman";

    case Trait::master_marksman:
        return "Master Marksman";

    case Trait::steady_aimer:
        return "Steady Aimer";

    case Trait::sharpshooter:
        return "Sharpshooter";

    case Trait::fast_shooter:
        return "Fast Shooter";

    case Trait::observant:
        return "Observant";

    case Trait::vigilant:
        return "Vigilant";

    case Trait::rapid_recoverer:
        return "Rapid Recoverer";

    case Trait::survivalist:
        return "Survivalist";

    case Trait::perseverant:
        return "Perseverant";

    case Trait::self_aware:
        return "Self-aware";

    case Trait::stout_spirit:
        return "Stout Spirit";

    case Trait::strong_spirit:
        return "Strong Spirit";

    case Trait::mighty_spirit:
        return "Mighty Spirit";

    case Trait::stealthy:
        return "Stealthy";

    case Trait::silent:
        return "Silent";

    case Trait::imperceptible:
        return "Imperceptible";

    case Trait::vicious:
        return "Vicious";

    case Trait::strong_backed:
        return "Strong-backed";

    case Trait::tough:
        return "Tough";

    case Trait::rugged:
        return "Rugged";

    case Trait::thick_skinned:
        return "Thick Skinned";

    case Trait::treasure_hunter:
        return "Treasure Hunter";

    case Trait::dem_expert:
        return "Demolition Expert";

    case Trait::undead_bane:
        return "Bane of the Undead";

    case Trait::elec_incl:
        return "Electrically Inclined";

    case Trait::ravenous:
        return "Ravenous";

    case Trait::foul:
        return "Foul";

    case Trait::toxic:
        return "Toxic";

    case Trait::indomitable_fury:
        return "Indomitable Fury";

    case Trait::END:
        break;
    }

    ASSERT(false);

    return "[TRAIT TITLE MISSING]";
}

std::vector<StrAndClr> bg_descr(const Bg id)
{
    std::vector<StrAndClr> descr;

    auto put = [&descr](const std::string& str)
    {
        descr.push_back({str, clr_white});
    };

    auto put_trait = [&descr](const Trait id)
    {
        descr.push_back({trait_title(id), clr_white});
        descr.push_back({trait_descr(id), clr_gray});
    };

    switch (id)
    {
    case Bg::ghoul:
        put("Does not regenerate Hit Points and cannot use medical equipment - "
            "heals by feeding on corpses (press '5' or '.' while standing on "
            "a corpse to feed)");
        put("");
        put("Can incite Frenzy (+100% speed, +10% melee hit chance, +1 melee "
            "damage, must move towards enemies)");
        put("");
        put("Does not become Weakened when Frenzy ends");
        put("");
        put("+10% speed");
        put("");
        put("+6 Hit Points");
        put("");
        put("+1 Armor Point (physical damage reduced by 1 point)");
        put("");
        put("Is immune to Disease and Infections");
        put("");
        put("Has Infravision");
        put("");
        put("-50% shock taken from seeing monsters");
        put("");
        put("-15% hit chance with firearms and thrown weapons");
        put("");
        put("All Ghouls are allied");
        break;

    case Bg::occultist:
        put("Has access to traits which allows casting more powerful versions "
            "of spells");
        put("");
        put("Learns spells by heart immediately when casting from unidentified "
            "manuscripts (other backgrounds only learns spells when casting "
            "from known manuscripts)");
        put("");
        put("Starts with a few known spells");
        put("");
        put("-50% shock taken from casting spells, and from carrying, using "
            "or identifying strange items (e.g. drinking a potion or carrying "
            "a disturbing artifact)");
        put("");
        put("Can dispel magic traps");
        put("");
        put("+2 Spirit Points (in addition to \"Stout Spirit\")");
        put("");
        put("-2 Hit Points");
        put("");
        put_trait(Trait::stout_spirit);
        break;

    case Bg::rogue:
        put("The rate of shock received passively over time is halved");
        put("");
        put("Has acquired an artifact which can cloud the minds of all "
            "enemies, causing them to forget the presence of the user");
        put("");
        put("Remains aware of the presence of other creatures much longer");
        put("");
        put_trait(Trait::observant);
        put("");
        put_trait(Trait::stealthy);
        break;

    case Bg::war_vet:
        put("Switches to prepared weapon instantly");
        put("");
        put("Starts with a Flak Jacket");
        put("");
        put("Maintains armor twice as long before it breaks");
        put("");
        put_trait(Trait::adept_marksman);
        put("");
        put_trait(Trait::adept_melee_fighter);
        put("");
        put_trait(Trait::tough);
        put("");
        put_trait(Trait::healer);
        break;

    case Bg::END:
        ASSERT(false);
        break;
    }

    return descr;
}

std::string trait_descr(const Trait id)
{
    switch (id)
    {
    case Trait::adept_melee_fighter:
        return
            "+10% hit chance, +10% attack speed, and +1 damage with melee "
            "attacks, +5% chance to evade melee attacks";

    case Trait::expert_melee_fighter:
        return
            "+10% hit chance, +10% attack speed, and +1 damage with melee "
            "attacks, +5% chance to evade melee attacks";

    case Trait::master_melee_fighter:
        return
            "+10% hit chance, +10% attack speed, and +1 damage with melee "
            "attacks, +5% chance to evade melee attacks";

    case Trait::adept_marksman:
        return
            "+10% hit chance, and +25% attack speed with firearms and thrown "
            "weapons, +50% reload speed";

    case Trait::expert_marksman:
        return
            "+10% hit chance, and +25% attack speed with firearms and thrown "
            "weapons, +50% reload speed";

    case Trait::master_marksman:
        return
            "+10% hit chance, and +25% attack speed with firearms and thrown "
            "weapons, +50% reload speed";

    case Trait::steady_aimer:
        return
            "Standing still gives ranged attacks +20% hit chance on the "
            "following turn";

    case Trait::sharpshooter:
        return
            "Standing still for three turns gives ranged attacks maximum hit "
            "chance and damage on the following turn";

    case Trait::fast_shooter:
        return
            "+100% firing speed (not applicable to thrown weapons)";

    case Trait::dem_expert:
        return
            "+1 radius for explosives, you are not harmed by your own "
            "Molotov Cocktails, +100% speed when lighting explosives";

    case Trait::cool_headed:
        return
            "+20% shock resistance";

    case Trait::courageous:
        return
            "+20% shock resistance";

    case Trait::lesser_invoc:
        return
            "Attack spells are cast at expert level (Darkbolt, "
            "Azathoth's Wrath, Mayhem)";

    case Trait::greater_invoc:
        return
            "Attack spells are cast at master level (Darkbolt, "
            "Azathoth's Wrath, Mayhem)";

    case Trait::lesser_summoning:
        return
            "Summoning spells are cast at expert level (Summoning, Pestilence)";

    case Trait::greater_summoning:
        return
            "Summoning spells are cast at master level (Summoning, Pestilence)";

    case Trait::lesser_clairv:
        return
            "Spells related to detection and information are cast at expert "
            "level (Searching, Insight, See Invisible)";

    case Trait::greater_clairv:
        return
            "Spells related to detection and information are cast at master "
            "level (Searching, Insight, See Invisible)";

    case Trait::lesser_ench:
        return
            "Spells related to bestowing properties, aiding, or debilitating "
            "are cast at expert level (Resistance, Light, Bless, Healing, "
            "Enfeeble)";

    case Trait::greater_ench:
        return
            "Spells related to bestowing properties, aiding, or debilitating "
            "are cast at master level (Resistance, Light, Bless, Healing, "
            "Enfeeble)";

    case Trait::lesser_alter:
        return
            "Spells related to moving, shifting, or changing the nature of "
            "things are cast at expert level (Teleport, Animate Weapons, "
            "Opening, Transmutation)";

    case Trait::greater_alter:
        return
            "Spells related to moving, shifting, or changing the nature of "
            "things are cast at master level (Teleport, Animate Weapons, "
            "Opening, Transmutation)";

    case Trait::blood_sorc:
        return
            "-1 Spirit cost for all spells, casting a spell drains 2 "
            "Hit Points";

    case Trait::tough:
        return
            "+4 Hit Points, +10% carry weight limit, less likely to sprain "
            "when kicking, more likely to succeed with object interactions "
            "requiring strength (e.g. bashing things open)";

    case Trait::rugged:
        return
            "+4 Hit Points, +10% carry weight limit, less likely to sprain "
            "when kicking, more likely to succeed with object interactions "
            "requiring strength (e.g. bashing things open)";

    case Trait::thick_skinned:
        return
            "+1 Armor Point (physical damage reduced by 1 point)";

    case Trait::strong_backed:
        return
            "+30% carry weight limit";

    case Trait::dexterous:
        return
            "+10% speed, +15% chance to evade attacks";

    case Trait::lithe:
        return
            "+10% speed, +15% chance to evade attacks";

    case Trait::mobile:
        return
            "+20% movement speed (i.e. only when moving between different "
            "cells on the map)";

    case Trait::fearless:
        return
            "You cannot become terrified, +5% shock resistance";

    case Trait::healer:
        return
            "Using medical equipment takes half the normal time and resources";

    case Trait::observant:
        return
            "+10% chance to spot hidden monsters, doors, and traps, and 80% "
            "chance to sense unique monsters or artifacts (when entering a "
            "new level, or when new monsters appear)";

    case Trait::vigilant:
        return
            "You cannot be backstabbed - monsters that you are unaware of get "
            "no melee attack bonus against you, and their attacks can be "
            "evaded";

    case Trait::rapid_recoverer:
        return
            "Four times faster Hit Point regeneration base rate";

    case Trait::survivalist:
        return
            "You cannot become diseased, negative effects from wounds reduced "
            "by 50%";

    case Trait::perseverant:
        return
            "While your Hit Points are reduced below 50%, you gain +20% speed, "
            "+20% hit chance with melee and ranged attacks, and +30% chance "
            "to evade attacks";

    case Trait::self_aware:
        return
            "You cannot become confused, the number of remaining turns for "
            "status effects are displayed";

    case Trait::stout_spirit:
        return
            "+2 Spirit Points, increased Spirit regeneration rate, you can "
            "defy harmful spells (it takes a certain number of turns to regain "
            "spell resistance)";

    case Trait::strong_spirit:
        return
            "+2 Spirit Points, increased Spirit regeneration rate, you can "
            "defy harmful spells - the number of turns to regain spell "
            "resistance is reduced";

    case Trait::mighty_spirit:
        return
            "+2 Spirit Points, increased Spirit regeneration rate, you can "
            "defy harmful spells - the number of turns to regain spell "
            "resistance is reduced";

    case Trait::stealthy:
        return
            "You are more likely to avoid detection";

    case Trait::imperceptible:
        return
            "You are more likely to avoid detection";

    case Trait::silent:
        return
            "All your melee attacks are silent, opening or closing doors does "
            "not alert other creatures";

    case Trait::vicious:
        return
            "+150% backstab damage (in addition to the normal +50% damage from "
            "stealth attacks)";

    case Trait::treasure_hunter:
        return
            "You tend to find more items";

    case Trait::undead_bane:
        return
            "+2 melee and ranged attack damage against undead monsters. "
            "Attacks against ethereal undead monsters (e.g. Ghosts) never pass "
            "through them (although you can still miss in the ordinary way)";

    case Trait::elec_incl:
        return
            "Rods recharge twice as fast, Strange Devices are less likely to "
            "malfunction or break, Electric Lanterns last twice as long, and "
            "are less likely to flicker, +1 damage with electricity weapons";

    case Trait::ravenous:
        return
            "You occasionally feed on living victims when attacking with your "
            "claws";

    case Trait::foul:
        return
            "You have particularly filthy and verminous claws - vicious worms "
            "occasionally burst out from the corpses of your victims to attack "
            "your enemies";

    case Trait::toxic:
        return
            "Attacks with your claws occasionally poisons your victims. "
            "You are immune to Poison";

    case Trait::indomitable_fury:
        return
            "While Frenzied, you are immune to Wounds, and your attacks causes "
            "fear";

    case Trait::END:
        break;
    }

    ASSERT(false);

    return "[TRAIT DESCRIPTION MISSING]";
}

void trait_prereqs(const Trait trait,
                   const Bg bg,
                   std::vector<Trait>& traits_out,
                   Bg& bg_out,
                   int& clvl_out)
{
    traits_out.clear();

    bg_out = Bg::END;

    clvl_out = -1;

    switch (trait)
    {
    case Trait::adept_melee_fighter:
        break;

    case Trait::expert_melee_fighter:
        traits_out.push_back(Trait::adept_melee_fighter);
        break;

    case Trait::master_melee_fighter:
        traits_out.push_back(Trait::expert_melee_fighter);
        break;

    case Trait::adept_marksman:
        break;

    case Trait::expert_marksman:
        traits_out.push_back(Trait::adept_marksman);
        break;

    case Trait::master_marksman:
        traits_out.push_back(Trait::expert_marksman);
        break;

    case Trait::steady_aimer:
        bg_out = Bg::war_vet;
        break;

    case Trait::sharpshooter:
        traits_out.push_back(Trait::steady_aimer);
        traits_out.push_back(Trait::expert_marksman);
        bg_out = Bg::war_vet;
        break;

    case Trait::fast_shooter:
        traits_out.push_back(Trait::expert_marksman);
        traits_out.push_back(Trait::dexterous);
        bg_out = Bg::war_vet;
        break;

    case Trait::dem_expert:
        break;

    case Trait::cool_headed:
        break;

    case Trait::courageous:
        traits_out.push_back(Trait::cool_headed);
        break;

    case Trait::lesser_invoc:
        bg_out = Bg::occultist;
        clvl_out = 3;
        break;

    case Trait::greater_invoc:
        traits_out.push_back(Trait::lesser_invoc);
        bg_out = Bg::occultist;
        clvl_out = 6;
        break;

    case Trait::lesser_summoning:
        bg_out = Bg::occultist;
        clvl_out = 3;
        break;

    case Trait::greater_summoning:
        traits_out.push_back(Trait::lesser_summoning);
        bg_out = Bg::occultist;
        clvl_out = 6;
        break;

    case Trait::lesser_clairv:
        bg_out = Bg::occultist;
        clvl_out = 3;
        break;

    case Trait::greater_clairv:
        traits_out.push_back(Trait::lesser_clairv);
        bg_out = Bg::occultist;
        clvl_out = 6;
        break;

    case Trait::lesser_ench:
        bg_out = Bg::occultist;
        clvl_out = 3;
        break;

    case Trait::greater_ench:
        traits_out.push_back(Trait::lesser_ench);
        bg_out = Bg::occultist;
        clvl_out = 6;
        break;

    case Trait::lesser_alter:
        bg_out = Bg::occultist;
        clvl_out = 3;
        break;

    case Trait::greater_alter:
        traits_out.push_back(Trait::lesser_alter);
        bg_out = Bg::occultist;
        clvl_out = 6;
        break;

    case Trait::blood_sorc:
        traits_out.push_back(Trait::tough);
        traits_out.push_back(Trait::cool_headed);
        bg_out = Bg::occultist;
        break;

    case Trait::tough:
        break;

    case Trait::rugged:
        traits_out.push_back(Trait::tough);
        break;

    case Trait::thick_skinned:
        break;

    case Trait::strong_backed:
        traits_out.push_back(Trait::tough);
        break;

    case Trait::dexterous:
        break;

    case Trait::lithe:
        traits_out.push_back(Trait::dexterous);
        break;

    case Trait::mobile:
        traits_out.push_back(Trait::lithe);
        break;

    case Trait::fearless:
        traits_out.push_back(Trait::cool_headed);
        break;

    case Trait::healer:
        break;

    case Trait::observant:
        break;

    case Trait::vigilant:
        traits_out.push_back(Trait::observant);
        break;

    case Trait::rapid_recoverer:
        traits_out.push_back(Trait::tough);
        traits_out.push_back(Trait::healer);
        break;

    case Trait::survivalist:
        traits_out.push_back(Trait::tough);
        traits_out.push_back(Trait::healer);
        break;

    case Trait::perseverant:
        traits_out.push_back(Trait::tough);
        traits_out.push_back(Trait::fearless);
        break;

    case Trait::self_aware:
        traits_out.push_back(Trait::stout_spirit);
        traits_out.push_back(Trait::observant);
        break;

    case Trait::stout_spirit:
        break;

    case Trait::strong_spirit:
        traits_out.push_back(Trait::stout_spirit);
        break;

    case Trait::mighty_spirit:
        traits_out.push_back(Trait::strong_spirit);
        bg_out = Bg::occultist;
        break;

    case Trait::stealthy:
        break;

    case Trait::imperceptible:
        traits_out.push_back(Trait::stealthy);
        break;

    case Trait::silent:
        traits_out.push_back(Trait::dexterous);
        traits_out.push_back(Trait::stealthy);
        break;

    case Trait::vicious:
        traits_out.push_back(Trait::stealthy);
        traits_out.push_back(Trait::dexterous);
        bg_out = Bg::rogue;
        break;

    case Trait::treasure_hunter:
        traits_out.push_back(Trait::observant);
        break;

    case Trait::undead_bane:
        traits_out.push_back(Trait::tough);
        traits_out.push_back(Trait::fearless);
        traits_out.push_back(Trait::stout_spirit);
        break;

    case Trait::elec_incl:
        break;

    case Trait::ravenous:
        traits_out.push_back(Trait::adept_melee_fighter);
        bg_out = Bg::ghoul;
        break;

    case Trait::foul:
        bg_out = Bg::ghoul;
        break;

    case Trait::toxic:
        traits_out.push_back(Trait::foul);
        bg_out = Bg::ghoul;
        break;

    case Trait::indomitable_fury:
        traits_out.push_back(Trait::adept_melee_fighter);
        traits_out.push_back(Trait::tough);
        bg_out = Bg::ghoul;
        break;

    case Trait::END:
        break;
    }

    // Remove traits which are blocked for this background (prerequisites are
    // considered fulfilled)
    for (auto it = begin(traits_out); it != end(traits_out); /* No increment */)
    {
        const Trait trait = *it;

        if (is_trait_blocked_for_bg(trait, bg))
        {
            it = traits_out.erase(it);
        }
        else // Not blocked
        {
            ++it;
        }
    }

    // Sort lexicographically
    sort(traits_out.begin(),
         traits_out.end(),
         [](const Trait & t1, const Trait & t2)
    {
        const std::string str1 = trait_title(t1);
        const std::string str2 = trait_title(t2);
        return str1 < str2;
    });
}

Bg bg()
{
    return bg_;
}

std::vector<Bg> pickable_bgs()
{
    std::vector<Bg> ret;

    for (int i = 0; i < (int)Bg::END; ++i)
    {
        ret.push_back(Bg(i));
    }

    // Sort lexicographically
    sort(ret.begin(), ret.end(), [](const Bg & bg1, const Bg & bg2)
    {
        const std::string str1 = bg_title(bg1);
        const std::string str2 = bg_title(bg2);
        return str1 < str2;
    });

    return ret;
}

void unpicked_traits_for_bg(const Bg bg,
                            std::vector<Trait>& traits_can_be_picked_out,
                            std::vector<Trait>& traits_prereqs_not_met_out)
{
    for (size_t i = 0; i < (size_t)Trait::END; ++i)
    {
        //
        // Already picked?
        //
        if (traits[i])
        {
            continue;
        }

        const auto trait = (Trait)i;

        //
        // Check if trait is explicitly blocked for this background
        //
        const bool is_blocked_for_bg = is_trait_blocked_for_bg(trait, bg);

        if (is_blocked_for_bg)
        {
            continue;
        }

        //
        // Check trait prerequisites (traits and background)
        //

        std::vector<Trait> trait_prereq_list;

        Bg bg_prereq = Bg::END;

        int clvl_prereq = -1;

        //
        // NOTE: Traits blocked for the current background are not
        //       considered prerequisites
        //
        trait_prereqs(trait,
                      bg,
                      trait_prereq_list,
                      bg_prereq,
                      clvl_prereq);

        const bool is_bg_ok =
            (bg_ == bg_prereq) ||
            (bg_prereq == Bg::END);

        if (!is_bg_ok)
        {
            continue;
        }

        bool is_trait_prereqs_ok = true;

        for (const auto& prereq : trait_prereq_list)
        {
            if (!traits[(size_t)prereq])
            {
                is_trait_prereqs_ok = false;

                break;
            }
        }

        is_trait_prereqs_ok =
            is_trait_prereqs_ok &&
            (game::clvl() >= clvl_prereq);

        if (is_trait_prereqs_ok)
        {
            traits_can_be_picked_out.push_back(trait);
        }
        else // Prerequisites not met
        {
            traits_prereqs_not_met_out.push_back(trait);
        }

    } // Trait loop

    // Sort lexicographically
    sort(traits_can_be_picked_out.begin(),
         traits_can_be_picked_out.end(),
         [](const Trait & t1, const Trait & t2)
    {
        const std::string str1 = trait_title(t1);
        const std::string str2 = trait_title(t2);
        return str1 < str2;
    });

    sort(traits_prereqs_not_met_out.begin(),
         traits_prereqs_not_met_out.end(),
         [](const Trait & t1, const Trait & t2)
    {
        const std::string str1 = trait_title(t1);
        const std::string str2 = trait_title(t2);
        return str1 < str2;
    });
}

void pick_bg(const Bg bg)
{
    ASSERT(bg != Bg::END);

    bg_ = bg;

    switch (bg_)
    {
    case Bg::ghoul:
        map::player->prop_handler()
            .apply(new PropRDisease(PropTurns::indefinite),
                     PropSrc::intr,
                     true,
                     Verbosity::silent);

        map::player->prop_handler()
            .apply(new PropInfravis(PropTurns::indefinite),
                     PropSrc::intr,
                     true,
                     Verbosity::silent);

        player_spells::learn_spell(SpellId::frenzy,
                                   Verbosity::silent);

        map::player->change_max_hp(6, Verbosity::silent);
        break;

    case Bg::occultist:
        pick_trait(Trait::stout_spirit);
        map::player->change_max_spi(2, Verbosity::silent);
        map::player->change_max_hp(-2, Verbosity::silent);
        break;

    case Bg::rogue:
        pick_trait(Trait::observant);
        pick_trait(Trait::stealthy);
        break;

    case Bg::war_vet:
        pick_trait(Trait::adept_melee_fighter);
        pick_trait(Trait::adept_marksman);
        pick_trait(Trait::tough);
        pick_trait(Trait::healer);
        break;

    case Bg::END:
        break;
    }
}

void set_all_traits_to_picked()
{
    for (int i = 0; i < (int)Trait::END; ++i)
    {
        traits[i] = true;
    }
}

void pick_trait(const Trait id)
{
    ASSERT(id != Trait::END);

    traits[(size_t)id] = true;

    switch (id)
    {
    case Trait::lesser_invoc:
    case Trait::greater_invoc:
    {
        player_spells::incr_spell_skill(SpellId::darkbolt);
        player_spells::incr_spell_skill(SpellId::aza_wrath);
        player_spells::incr_spell_skill(SpellId::mayhem);
    }
    break;

    case Trait::lesser_summoning:
    case Trait::greater_summoning:
    {
        player_spells::incr_spell_skill(SpellId::summon);
        player_spells::incr_spell_skill(SpellId::pest);
    }
    break;

    case Trait::lesser_clairv:
    case Trait::greater_clairv:
    {
        player_spells::incr_spell_skill(SpellId::searching);
        player_spells::incr_spell_skill(SpellId::insight);
        player_spells::incr_spell_skill(SpellId::see_invis);
    }
    break;

    case Trait::lesser_ench:
    case Trait::greater_ench:
    {
        player_spells::incr_spell_skill(SpellId::heal);
        player_spells::incr_spell_skill(SpellId::res);
        player_spells::incr_spell_skill(SpellId::light);
        player_spells::incr_spell_skill(SpellId::bless);
        player_spells::incr_spell_skill(SpellId::enfeeble);
    }
    break;

    case Trait::lesser_alter:
    case Trait::greater_alter:
    {
        player_spells::incr_spell_skill(SpellId::teleport);
        player_spells::incr_spell_skill(SpellId::opening);
        player_spells::incr_spell_skill(SpellId::anim_wpns);
        player_spells::incr_spell_skill(SpellId::transmut);
    }
    break;

    case Trait::tough:
    {
        const int hp_incr = 4;

        map::player->change_max_hp(hp_incr,
                                   Verbosity::silent);

        map::player->restore_hp(hp_incr,
                                false, // Not allowed above max
                                Verbosity::silent);
    }
    break;

    case Trait::rugged:
    {
        const int hp_incr = 4;

        map::player->change_max_hp(hp_incr,
                                   Verbosity::silent);

        map::player->restore_hp(hp_incr,
                                false, // Not allowed above max
                                Verbosity::silent);
    }
    break;

    case Trait::stout_spirit:
    {
        const int spi_incr = 2;

        map::player->change_max_spi(spi_incr,
                                    Verbosity::silent);

        map::player->restore_spi(spi_incr,
                                 false, // Not allowed above max
                                 Verbosity::silent);

        map::player->prop_handler().apply(
            new PropRSpell(PropTurns::indefinite),
            PropSrc::intr,
            true,
            Verbosity::silent);
    }
    break;

    case Trait::strong_spirit:
    case Trait::mighty_spirit:
    {
        const int spi_incr = 2;

        map::player->change_max_spi(spi_incr,
                                    Verbosity::silent);

        map::player->restore_spi(spi_incr,
                                 false, // Not allowed above max
                                 Verbosity::silent);
    }
    break;

    case Trait::self_aware:
    {
        map::player->prop_handler()
            .apply(new PropRConf(PropTurns::indefinite),
                     PropSrc::intr,
                     true,
                     Verbosity::silent);
    }
    break;

    case Trait::survivalist:
    {
        map::player->prop_handler()
            .apply(new PropRDisease(PropTurns::indefinite),
                     PropSrc::intr,
                     true,
                     Verbosity::silent);
    }
    break;

    case Trait::fearless:
    {
        map::player->prop_handler()
            .apply(new PropRFear(PropTurns::indefinite),
                     PropSrc::intr,
                     true,
                     Verbosity::silent);
    }
    break;

    case Trait::toxic:
    {
        map::player->prop_handler()
            .apply(new PropRPoison(PropTurns::indefinite),
                     PropSrc::intr,
                     true,
                     Verbosity::silent);
    }
    break;

    default:
        break;
    }
}

std::string all_picked_traits_titles_line()
{
    std::string out = "";

    for (int i = 0; i < (int)Trait::END; ++i)
    {
        if (traits[i])
        {
            const std::string title = trait_title(Trait(i));

            out += (out.empty() ? "" : ", ") + title;
        }
    }

    return out;
}

bool gets_undead_bane_bon(const ActorDataT& actor_data)
{
    return
        player_bon::traits[(size_t)Trait::undead_bane] &&
        actor_data.is_undead;
}

} // player_bon
