#include "player_bon.hpp"

#include <cassert>

#include "init.hpp"
#include "text_format.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "item_factory.hpp"
#include "inventory.hpp"
#include "player_spells_handling.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "create_character.hpp"
#include "save_handling.hpp"

namespace player_bon
{

bool traits[int(Trait::END)];

namespace
{

Bg bg_ = Bg::END;

bool is_trait_blocked_for_bg(const Trait trait, const Bg bg)
{
    switch (trait)
    {
    case Trait::adept_melee_fighter:
        break;

    case Trait::expert_melee_fighter:
        break;

    case Trait::master_melee_fighter:
        break;

    case Trait::adept_marksman:
        break;

    case Trait::expert_marksman:
        break;

    case Trait::master_marksman:
        break;

    case Trait::steady_aimer:
        break;

    case Trait::sharp_shooter:
        break;

    case Trait::dem_expert:
        //Too much character theme mismatch
        return bg == Bg::occultist || bg == Bg::ghoul;

    case Trait::cool_headed:
        break;

    case Trait::courageous:
        break;

    case Trait::warlock:
        break;

    case Trait::summoner:
        break;

    case Trait::blood_sorcerer:
        break;

    case Trait::seer:
        break;

    case Trait::tough:
        break;

    case Trait::rugged:
        break;

    case Trait::unbreakable:
        break;

    case Trait::strong_backed:
        break;

    case Trait::dexterous:
        break;

    case Trait::lithe:
        break;

    case Trait::mobile:
        break;

    case Trait::fearless:
        break;

    case Trait::healer:
        //Cannot use Medial Bag
        return bg == Bg::ghoul;

    case Trait::observant:
        break;

    case Trait::perceptive:
        break;

    case Trait::vigilant:
        break;

    case Trait::rapid_recoverer:
        break;

    case Trait::survivalist:
        //Has RDISEASE already, so trait is partly useless
        return bg == Bg::ghoul;

    case Trait::perseverant:
        break;

    case Trait::self_aware:
        break;

    case Trait::stout_spirit:
        break;

    case Trait::strong_spirit:
        break;

    case Trait::mighty_spirit:
        break;

    case Trait::stealthy:
        break;

    case Trait::imperceptible:
        break;

    case Trait::vicious:
        break;

    case Trait::treasure_hunter:
        break;

    case Trait::undead_bane:
        break;

    case Trait::END:
        break;
    }

    return false;
}

} //Namespace

void init()
{
    for (int i = 0; i < int(Trait::END); ++i)
    {
        traits[i] = false;
    }

    bg_ = Bg::END;
}

void save()
{
    save_handling::put_int(int(bg_));

    for (int i = 0; i < int(Trait::END); ++i)
    {
        save_handling::put_bool(traits[i]);
    }
}

void load()
{
    bg_ = Bg(save_handling::get_int());

    for (int i = 0; i < int(Trait::END); ++i)
    {
        traits[i] = save_handling::get_bool();
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

    assert(false);

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

    case Trait::warlock:
        return "Warlock";

    case Trait::summoner:
        return "Summoner";

    case Trait::blood_sorcerer:
        return "Blood Sorcerer";

    case Trait::seer:
        return "Seer";

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

    case Trait::sharp_shooter:
        return "Sharpshooter";

    case Trait::observant:
        return "Observant";

    case Trait::perceptive:
        return "Perceptive";

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

    case Trait::unbreakable:
        return "Unbreakable";

    case Trait::treasure_hunter:
        return "Treasure Hunter";

    case Trait::dem_expert:
        return "Demolition Expert";

    case Trait::undead_bane:
        return "Bane of the Undead";

    case Trait::END:
        break;
    }

    assert(false);

    return "[TRAIT TITLE MISSING]";
}

void bg_descr(const Bg id, std::vector<std::string>& out)
{
    out.clear();

    switch (id)
    {
    case Bg::ghoul:
    {
        Item_data_t& claw_data = item_data::data[size_t(Item_id::player_ghoul_claw)];

        const std::string dmg_str = to_str(claw_data.melee.dmg.first) + "d" +
                                    to_str(claw_data.melee.dmg.second);

        const std::string att_mod_str = to_str(claw_data.melee.hit_chance_mod) + "%";

        out.push_back("Cannot use medical equipment, but can instead regenerate Hit Points by "
                      "feeding on corpses (press [E])");
        out.push_back(" ");
        out.push_back("Has powerful claws (" + dmg_str + ", +" + att_mod_str + " to hit)");
        out.push_back(" ");
        out.push_back("-15% hit chance with firearms and thrown weapons");
        out.push_back(" ");
        out.push_back("Starts with +4 Hit Points");
        out.push_back(" ");
        out.push_back("Is immune to Disease and Infections");
        out.push_back(" ");
        out.push_back("Has Infravision");
        out.push_back(" ");
        out.push_back("-50% shock taken from seeing monsters");
        out.push_back(" ");
        out.push_back("All Ghouls are allied to you");
    }
    break;

    case Bg::occultist:
        out.push_back("Can learn spells by heart when casting from manuscripts");
        out.push_back(" ");
        out.push_back("-50% shock taken from using and identifying "
                      "strange items (e.g. potions)");
        out.push_back(" ");
        out.push_back("Can dispel magic traps");
        out.push_back(" ");
        out.push_back("-2 Hit Points");
        out.push_back(" ");
        out.push_back("Starts with the following trait(s):");
        out.push_back(" ");
        out.push_back("* " + trait_title(Trait::stout_spirit));
        out.push_back(trait_descr(Trait::stout_spirit));
        break;

    case Bg::rogue:
        out.push_back("Has an arcane ability to cloud the minds of "
                      "enemies, causing them to forget their pursuit "
                      "(press [x])");
        out.push_back(" ");
        out.push_back("+25% hit chance with ranged attacks vs unaware targets");
        out.push_back(" ");
        out.push_back("The rate of shock recieved passively over time "
                      "is reduced by half");
        out.push_back(" ");
        out.push_back("Starts with the following trait(s):");
        out.push_back(" ");
        out.push_back("* " + trait_title(Trait::observant));
        out.push_back(trait_descr(Trait::observant));
        out.push_back(" ");
        out.push_back("* " + trait_title(Trait::stealthy));
        out.push_back(trait_descr(Trait::stealthy));
        break;

    case Bg::war_vet:
        out.push_back("Can switch to prepared weapon instantly");
        out.push_back(" ");
        out.push_back("Maintains armor twice as long before it breaks");
        out.push_back(" ");
        out.push_back("Starts with 10% insanity");
        out.push_back(" ");
        out.push_back("Starts with the following trait(s):");
        out.push_back(" ");
        out.push_back("* " + trait_title(Trait::adept_marksman));
        out.push_back(trait_descr(Trait::adept_marksman));
        out.push_back(" ");
        out.push_back("* " + trait_title(Trait::adept_melee_fighter));
        out.push_back(trait_descr(Trait::adept_melee_fighter));
        out.push_back(" ");
        out.push_back("* " + trait_title(Trait::tough));
        out.push_back(trait_descr(Trait::tough));
        break;

    case Bg::END:
        break;
    }
}

std::string trait_descr(const Trait id)
{
    switch (id)
    {
    case Trait::adept_melee_fighter:
        return "+10% hit chance with melee attacks";

    case Trait::expert_melee_fighter:
        return "+10% hit chance with melee attacks";

    case Trait::master_melee_fighter:
        return "+10% hit chance with melee attacks";

    case Trait::adept_marksman:
        return "+10% hit chance with firearms and thrown weapons";

    case Trait::expert_marksman:
        return "+10% hit chance with firearms and thrown weapons, you occasionally "
               "reload instantly";

    case Trait::master_marksman:
        return "+10% hit chance with firearms and thrown weapons";

    case Trait::steady_aimer:
        return "Standing still gives ranged attacks +20% hit chance on the following turn";

    case Trait::sharp_shooter:
        return "Standing still for three turns gives ranged attacks maximum hit chance "
               "and damage on the following turn";

    case Trait::dem_expert:
        return "+1 radius for explosives (be careful), you are not harmed by your own "
               "Molotov Cocktails, you occasionally light explosives instantly";

    case Trait::cool_headed:
        return "+20% shock resistance";

    case Trait::courageous:
        return "+20% shock resistance";

    case Trait::warlock:
        return "-1 Spirit cost for damage dealing spells, casting any spell has a chance "
               "to make you \"Charged\" for one turn, causing attack spells to do "
               "maximum damage";

    case Trait::summoner:
        return "-1 Spirit cost for summoning spells, halved risk that called creatures "
               "are hostile";

    case Trait::blood_sorcerer:
        return "-1 Spirit cost for all spells, casting a spell drains 2 Hit Points";

    case Trait::seer:
        return "Detection spells have decreased Spirit costs, and the spell "
               "\"Detect Monsters\" has triple duration";

    case Trait::tough:
        return "+2 hit points, +10% carry weight limit, better results for object "
               "interactions requiring strength (e.g. bashing doors or pushing a lid)";

    case Trait::rugged:
        return "+2 hit points, +10% carry weight limit, better results for object "
               "interactions requiring strength (such as bashing doors, or moving the "
               "lid from a stone coffin)";

    case Trait::unbreakable:
        return "+2 hit points, +10% carry weight limit, better results for object "
               "interactions requiring strength (such as bashing doors, or moving the "
               "lid from a stone coffin)";

    case Trait::strong_backed:
        return "+30% carry weight limit";

    case Trait::dexterous:
        return "+25% chance to dodge melee attacks, better chances to evade traps, "
               "every fifth step is a free action";

    case Trait::lithe:
        return "+25% chance to dodge melee attacks, better chances to evade traps, "
               "every fourth step is a free action";

    case Trait::mobile:
        return "Every second step is a free action";

    case Trait::fearless:
        return "You cannot become terrified, +5% shock resistance";

    case Trait::healer:
        return "Using medical equipment takes half the normal time and resources";

    case Trait::observant:
        return "You can spot hidden traps and doors from two cells away, and you are "
               "more likely to spot hidden monsters and objects";

    case Trait::perceptive:
        return "You can spot hidden traps and doors from three cells away, and you are "
               "more likely to spot hidden monsters and objects";

    case Trait::vigilant:
        return "You cannot be backstabbed - monsters that you are unaware of gets no melee attack "
               "bonus against you, and their attacks can be dodged";

    case Trait::rapid_recoverer:
        return "Increased Hit Point regeneration rate";

    case Trait::survivalist:
        return "Increased Hit Point regeneration rate, disease only lowers your Hit "
               "Points by 25% (instead of 50%)";

    case Trait::perseverant:
        return "When your Hit Points are reduced to 25% or less, you gain +50% chance to "
               "dodge melee attacks, and +30% hit chance with melee and ranged attacks";

    case Trait::self_aware:
        return "You cannot become confused, the number of remaining turns for status "
               "effects are displayed";

    case Trait::stout_spirit:
        return "+2 Spirit Points, increased Spirit regeneration rate";

    case Trait::strong_spirit:
        return "+2 Spirit Points, increased Spirit regeneration rate";

    case Trait::mighty_spirit:
        return "+2 Spirit Points, increased Spirit regeneration rate";

    case Trait::stealthy:
        return "You are more likely to avoid detection";

    case Trait::imperceptible:
        return "You are more likely to avoid detection";

    case Trait::vicious:
        return "+50% backstab damage";

    case Trait::treasure_hunter:
        return "You tend to find more items";

    case Trait::undead_bane:
        return "+2 melee and ranged attack damage against undead monsters. Attacks "
               "against ethereal undead monsters (e.g. Ghosts) never pass through them "
               "(although you can still miss in the ordinary way)";

    case Trait::END:
        break;
    }

    assert(false);

    return "[TRAIT DESCRIPTION MISSING]";
}

void trait_prereqs(const Trait trait,
                   const Bg bg,
                   std::vector<Trait>& traits_ref,
                   Bg& bg_ref)
{
    traits_ref.clear();
    bg_ref = Bg::END;

    switch (trait)
    {
    case Trait::adept_melee_fighter:
        break;

    case Trait::expert_melee_fighter:
        traits_ref.push_back(Trait::adept_melee_fighter);
        break;

    case Trait::master_melee_fighter:
        traits_ref.push_back(Trait::expert_melee_fighter);
        break;

    case Trait::adept_marksman:
        break;

    case Trait::expert_marksman:
        traits_ref.push_back(Trait::adept_marksman);
        break;

    case Trait::master_marksman:
        traits_ref.push_back(Trait::expert_marksman);
        bg_ref = Bg::war_vet;
        break;

    case Trait::steady_aimer:
        traits_ref.push_back(Trait::adept_marksman);
        break;

    case Trait::sharp_shooter:
        traits_ref.push_back(Trait::steady_aimer);
        traits_ref.push_back(Trait::expert_marksman);
        bg_ref = Bg::war_vet;
        break;

    case Trait::dem_expert:
        break;

    case Trait::cool_headed:
        break;

    case Trait::courageous:
        traits_ref.push_back(Trait::cool_headed);
        bg_ref = Bg::war_vet;
        break;

    case Trait::warlock:
        traits_ref.push_back(Trait::fearless);
        traits_ref.push_back(Trait::strong_spirit);
        bg_ref = Bg::occultist;
        break;

    case Trait::summoner:
        traits_ref.push_back(Trait::fearless);
        traits_ref.push_back(Trait::strong_spirit);
        bg_ref = Bg::occultist;
        break;

    case Trait::blood_sorcerer:
        traits_ref.push_back(Trait::tough);
        bg_ref = Bg::occultist;
        break;

    case Trait::seer:
        traits_ref.push_back(Trait::observant);
        bg_ref = Bg::occultist;
        break;

    case Trait::tough:
        break;

    case Trait::rugged:
        traits_ref.push_back(Trait::tough);
        break;

    case Trait::unbreakable:
        traits_ref.push_back(Trait::rugged);
        break;

    case Trait::strong_backed:
        traits_ref.push_back(Trait::tough);
        break;

    case Trait::dexterous:
        break;

    case Trait::lithe:
        traits_ref.push_back(Trait::dexterous);
        break;

    case Trait::mobile:
        traits_ref.push_back(Trait::lithe);
        break;

    case Trait::fearless:
        traits_ref.push_back(Trait::cool_headed);
        break;

    case Trait::healer:
        break;

    case Trait::observant:
        break;

    case Trait::perceptive:
        traits_ref.push_back(Trait::observant);
        break;

    case Trait::vigilant:
        traits_ref.push_back(Trait::observant);
        break;

    case Trait::rapid_recoverer:
        traits_ref.push_back(Trait::tough);
        traits_ref.push_back(Trait::healer);
        break;

    case Trait::survivalist:
        traits_ref.push_back(Trait::rapid_recoverer);
        break;

    case Trait::perseverant:
        traits_ref.push_back(Trait::tough);
        traits_ref.push_back(Trait::fearless);
        break;

    case Trait::self_aware:
        traits_ref.push_back(Trait::stout_spirit);
        traits_ref.push_back(Trait::observant);
        break;

    case Trait::stout_spirit:
        break;

    case Trait::strong_spirit:
        traits_ref.push_back(Trait::stout_spirit);
        break;

    case Trait::mighty_spirit:
        traits_ref.push_back(Trait::strong_spirit);
        bg_ref = Bg::occultist;
        break;

    case Trait::stealthy:
        break;

    case Trait::imperceptible:
        traits_ref.push_back(Trait::stealthy);
        break;

    case Trait::vicious:
        traits_ref.push_back(Trait::stealthy);
        traits_ref.push_back(Trait::dexterous);
        break;

    case Trait::treasure_hunter:
        traits_ref.push_back(Trait::observant);
        break;

    case Trait::undead_bane:
        traits_ref.push_back(Trait::tough);
        traits_ref.push_back(Trait::fearless);
        traits_ref.push_back(Trait::stout_spirit);
        break;

    case Trait::END:
        break;
    }

    //Remove traits which are blocked for this background (prerequisites are considered fulfilled)
    for (auto it = begin(traits_ref); it != end(traits_ref); /* No increment */)
    {
        const Trait trait = *it;

        if (is_trait_blocked_for_bg(trait, bg))
        {
            it = traits_ref.erase(it);
        }
        else //Not blocked
        {
            ++it;
        }
    }

    //Sort lexicographically
    sort(traits_ref.begin(), traits_ref.end(), [](const Trait & t1, const Trait & t2)
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

void pickable_bgs(std::vector<Bg>& bgs_ref)
{
    bgs_ref.clear();

    for (int i = 0; i < int(Bg::END); ++i)
    {
        bgs_ref.push_back(Bg(i));
    }

    //Sort lexicographically
    sort(bgs_ref.begin(), bgs_ref.end(), [](const Bg & bg1, const Bg & bg2)
    {
        const std::string str1 = bg_title(bg1);
        const std::string str2 = bg_title(bg2);
        return str1 < str2;
    });
}

void pickable_traits(const Bg bg, std::vector<Trait>& traits_ref)
{
    traits_ref.clear();

    for (size_t i = 0; i < size_t(Trait::END); ++i)
    {
        const Trait trait = Trait(i);

        //Only allow picking trait if not already picked, and not blocked for this background
        const bool IS_PICKED            = traits[i];
        const bool IS_BLOCKED_FOR_BG    = is_trait_blocked_for_bg(trait, bg);

        if (!IS_PICKED && !IS_BLOCKED_FOR_BG)
        {
            //Check trait prerequisites (traits and background)
            //NOTE: Traits blocked for the current background are not considered prerequisites

            std::vector<Trait> trait_prereq_list;
            Bg bg_prereq = Bg::END;

            trait_prereqs(trait, bg, trait_prereq_list, bg_prereq);

            bool is_pickable = true;

            for (Trait prereq : trait_prereq_list)
            {
                if (!traits[size_t(prereq)])
                {
                    is_pickable = false;
                    break;
                }
            }

            is_pickable = is_pickable && (bg_ == bg_prereq || bg_prereq == Bg::END);

            if (is_pickable)
            {
                traits_ref.push_back(trait);
            }
        }
    }

    //Limit the number of trait choices (due to screen space constraints)
    const size_t NR_PICKABLE    = size_t(traits_ref.size());
    const size_t MAX_NR_CHOICES = create_character::OPT_H;

    if (NR_PICKABLE > MAX_NR_CHOICES)
    {
        //Limit the traits by random removal
        random_shuffle(traits_ref.begin(), traits_ref.end());

        traits_ref.resize(MAX_NR_CHOICES);
    }

    //Sort lexicographically
    sort(traits_ref.begin(), traits_ref.end(), [](const Trait & t1, const Trait & t2)
    {
        const std::string str1 = trait_title(t1);
        const std::string str2 = trait_title(t2);
        return str1 < str2;
    });
}

void pick_bg(const Bg bg)
{
    assert(bg != Bg::END);

    bg_ = bg;

    switch (bg_)
    {
    case Bg::ghoul:
        map::player->prop_handler().try_add_prop(
            new Prop_rDisease(Prop_turns::indefinite), Prop_src::intr, true, Verbosity::silent);

        map::player->prop_handler().try_add_prop(
            new Prop_infravis(Prop_turns::indefinite), Prop_src::intr, true, Verbosity::silent);

        map::player->change_max_hp(4, Verbosity::silent);
        break;

    case Bg::occultist:
        pick_trait(Trait::stout_spirit);
        map::player->change_max_hp(-2, Verbosity::silent);
        break;

    case Bg::rogue:
        player_spells_handling::learn_spell_if_not_known(Spell_id::cloud_minds);
        pick_trait(Trait::observant);
        pick_trait(Trait::stealthy);
        break;

    case Bg::war_vet:
        pick_trait(Trait::adept_melee_fighter);
        pick_trait(Trait::adept_marksman);
        pick_trait(Trait::tough);
        map::player->ins_ += 10;
        break;

    case Bg::END:
        break;
    }
}

void set_all_traits_to_picked()
{
    for (int i = 0; i < int(Trait::END); ++i)
    {
        traits[i] = true;
    }
}

void pick_trait(const Trait id)
{
    assert(id != Trait::END);

    traits[int(id)] = true;

    switch (id)
    {
    case Trait::tough:
        map::player->change_max_hp(2, Verbosity::silent);
        break;

    case Trait::rugged:
        map::player->change_max_hp(2, Verbosity::silent);
        break;

    case Trait::unbreakable:
        map::player->change_max_hp(2, Verbosity::silent);
        break;

    case Trait::stout_spirit:
        map::player->change_max_spi(2, Verbosity::silent);
        break;

    case Trait::strong_spirit:
        map::player->change_max_spi(2, Verbosity::silent);
        break;

    case Trait::mighty_spirit:
        map::player->change_max_spi(2, Verbosity::silent);
        break;

    case Trait::self_aware:
        map::player->prop_handler().try_add_prop(
            new Prop_rConf(Prop_turns::indefinite), Prop_src::intr, true, Verbosity::silent);
        break;

    case Trait::fearless:
        map::player->prop_handler().try_add_prop(
            new Prop_rFear(Prop_turns::indefinite), Prop_src::intr, true, Verbosity::silent);
        break;

    default:
        break;
    }
}

std::string all_picked_traits_titles_line()
{
    std::string out = "";

    for (int i = 0; i < int(Trait::END); ++i)
    {
        if (traits[i])
        {
            const std::string title = trait_title(Trait(i));

            out += (out.empty() ? "" : ", ") + title;
        }
    }

    return out;
}

int spi_occultist_can_cast_at_lvl(const int LVL)
{
    assert(LVL > 0);
    const int SPI_FROM_START_TRAIT  = 2;
    const int SPI_FROM_LVLS         = (LVL - 1) * SPI_PER_LVL;
    return PLAYER_START_SPI + SPI_FROM_LVLS + SPI_FROM_START_TRAIT - 1;
}

bool gets_undead_bane_bon(const Actor_data_t& actor_data)
{
    return player_bon::traits[int(Trait::undead_bane)] && actor_data.is_undead;
}

} //player_bon
