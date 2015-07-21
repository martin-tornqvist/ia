#include "player_bon.hpp"

#include <assert.h>

#include "text_format.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "item_factory.hpp"
#include "inventory.hpp"
#include "player_spells_handling.hpp"
#include "map.hpp"
#include "map_parsing.hpp"

using namespace std;

namespace player_bon
{

bool traits[int(Trait::END)];

namespace
{

Bg bg_ = Bg::END;

} //Namespace

void init()
{
    for (int i = 0; i < int(Trait::END); ++i) {traits[i] = false;}

    bg_ = Bg::END;
}

void store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(int(bg_)));

    for (int i = 0; i < int(Trait::END); ++i)
    {
        lines.push_back(traits[i] ? "1" : "0");
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    bg_ = Bg(to_int(lines.front()));
    lines.erase(begin(lines));

    for (int i = 0; i < int(Trait::END); ++i)
    {
        traits[i] = lines.front() == "1";
        lines.erase(begin(lines));
    }
}

void bg_title(const Bg id, string& out)
{
    out = "[BG TITLE MISSING]";

    switch (id)
    {
    case Bg::occultist: out = "Occultist";   break;

    case Bg::rogue:     out = "Rogue";       break;

    case Bg::war_vet:    out = "War Veteran"; break;

    case Bg::END: {} break;
    }
}

void trait_title(const Trait id, string& out)
{
    out = "[TRAIT TITLE MISSING]";

    switch (id)
    {
    case Trait::adept_melee_fighter:    out = "Adept Melee Fighter";   break;

    case Trait::expert_melee_fighter:   out = "Expert Melee Fighter";  break;

    case Trait::master_melee_fighter:   out = "Master Melee Fighter";  break;

    case Trait::cool_headed:           out = "Cool-headed";           break;

    case Trait::courageous:           out = "Courageous";            break;

    case Trait::warlock:              out = "Warlock";               break;

    case Trait::summoner:             out = "Summoner";              break;

    case Trait::blood_sorcerer:        out = "Blood Sorcerer";        break;

    case Trait::seer:                 out = "Seer";                  break;

    case Trait::dexterous:            out = "Dexterous";             break;

    case Trait::lithe:                out = "Lithe";                 break;

    case Trait::mobile:               out = "Mobile";                break;

    case Trait::fearless:             out = "Fearless";              break;

    case Trait::healer:               out = "Healer";                break;

    case Trait::adept_marksman:        out = "Adept Marksman";        break;

    case Trait::expert_marksman:       out = "Expert Marksman";       break;

    case Trait::master_marksman:       out = "Master Marksman";       break;

    case Trait::steady_aimer:          out = "Steady Aimer";          break;

    case Trait::sharp_shooter:         out = "Sharpshooter";          break;

    case Trait::observant:            out = "Observant";             break;

    case Trait::perceptive:           out = "Perceptive";            break;

    case Trait::vigilant:             out = "Vigilant";              break;

    case Trait::rapid_recoverer:       out = "Rapid Recoverer";       break;

    case Trait::survivalist:          out = "Survivalist";           break;

    case Trait::perseverant:          out = "Perseverant";           break;

    case Trait::self_aware:            out = "Self-aware";            break;

    case Trait::stout_spirit:          out = "Stout Spirit";          break;

    case Trait::strong_spirit:         out = "Strong Spirit";         break;

    case Trait::mighty_spirit:         out = "Mighty Spirit";         break;

    case Trait::stealthy:             out = "Stealthy";              break;

    case Trait::imperceptible:        out = "Imperceptible";         break;

    case Trait::vicious:              out = "Vicious";               break;

    case Trait::strong_backed:         out = "Strong-backed";         break;

    case Trait::tough:                out = "Tough";                 break;

    case Trait::rugged:               out = "Rugged";                break;

    case Trait::unbreakable:          out = "Unbreakable";           break;

    case Trait::treasure_hunter:       out = "Treasure Hunter";       break;

    case Trait::dem_expert:            out = "Demolition Expert";     break;

    case Trait::undead_bane:           out = "Bane of the Undead";    break;

    case Trait::END: break;
    }
}

void bg_descr(const Bg id, vector<string>& lines_ref)
{
    lines_ref.clear();
    string s = "";

    switch (id)
    {
    case Bg::occultist:
        lines_ref.push_back("Can learn spells cast from manuscripts");
        lines_ref.push_back(" ");
        lines_ref.push_back("-50% shock taken from using and identifying "
                            "strange items (e.g. potions)");
        lines_ref.push_back(" ");
        lines_ref.push_back("Can dispel magic traps");
        lines_ref.push_back(" ");
        lines_ref.push_back("-2 Hit Points");
        lines_ref.push_back(" ");
        lines_ref.push_back("Starts with the following trait(s):");
        lines_ref.push_back(" ");
        trait_title(Trait::stout_spirit, s); lines_ref.push_back("* " + s);
        trait_descr(Trait::stout_spirit, s); lines_ref.push_back(s);
        break;

    case Bg::rogue:
        lines_ref.push_back("Has an arcane ability to cloud the minds of "
                            "enemies, causing them to forget their pursuit "
                            "(press [x])");
        lines_ref.push_back(" ");
        lines_ref.push_back("+25% hit chance with ranged attacks vs unaware targets");
        lines_ref.push_back(" ");
        lines_ref.push_back("The rate of shock recieved passively over time "
                            "is reduced by half");
        lines_ref.push_back(" ");
        lines_ref.push_back("Starts with the following trait(s):");
        lines_ref.push_back(" ");
        trait_title(Trait::observant, s); lines_ref.push_back("* " + s);
        trait_descr(Trait::observant, s); lines_ref.push_back(s);
        lines_ref.push_back(" ");
        trait_title(Trait::stealthy, s);  lines_ref.push_back("* " + s);
        trait_descr(Trait::stealthy, s);  lines_ref.push_back(s);
        break;

    case Bg::war_vet:
        lines_ref.push_back("Can switch to prepared weapon instantly");
        lines_ref.push_back(" ");
        lines_ref.push_back("Maintains armor twice as long before it breaks");
        lines_ref.push_back(" ");
        lines_ref.push_back("Starts with 10% insanity");
        lines_ref.push_back(" ");
        lines_ref.push_back("Starts with the following trait(s):");
        lines_ref.push_back(" ");
        trait_title(Trait::adept_marksman, s);     lines_ref.push_back("* " + s);
        trait_descr(Trait::adept_marksman, s);     lines_ref.push_back(s);
        lines_ref.push_back(" ");
        trait_title(Trait::adept_melee_fighter, s); lines_ref.push_back("* " + s);
        trait_descr(Trait::adept_melee_fighter, s); lines_ref.push_back(s);
        lines_ref.push_back(" ");
        trait_title(Trait::tough, s);             lines_ref.push_back("* " + s);
        trait_descr(Trait::tough, s);             lines_ref.push_back(s);
        break;

    case Bg::END: {}
        break;
    }
}

void trait_descr(const Trait id, string& out)
{
    out = "[TRAIT DESCRIPTION MISSING]";

    switch (id)
    {
    case Trait::adept_melee_fighter:
        out = "+10% hit chance with melee attacks";
        break;

    case Trait::expert_melee_fighter:
        out = "+10% hit chance with melee attacks";
        break;

    case Trait::master_melee_fighter:
        out = "+10% hit chance with melee attacks";
        break;

    case Trait::adept_marksman:
        out = "+10% hit chance with firearms and thrown weapons";
        break;

    case Trait::expert_marksman:
        out = "+10% hit chance with firearms and thrown weapons, you occasionally "
              "reload instantly";
        break;

    case Trait::master_marksman:
        out = "+10% hit chance with firearms and thrown weapons";
        break;

    case Trait::steady_aimer:
        out = "Standing still gives ranged attacks +20% hit chance on the following turn";
        break;

    case Trait::sharp_shooter:
        out = "Standing still for three turns gives ranged attacks maximum hit chance "
              "and damage on the following turn";
        break;

    case Trait::dem_expert:
        out = "+1 radius for explosives (be careful), you are not harmed by your own "
              "Molotov Cocktails, you occasionally light explosives instantly";
        break;

    case Trait::cool_headed:
        out = "+20% shock resistance";
        break;

    case Trait::courageous:
        out = "+20% shock resistance";
        break;

    case Trait::warlock:
        out = "-1 Spirit cost for damage dealing spells, casting any spell has a chance "
              "to make you \"Charged\" for one turn, causing attack spells to do "
              "maximum damage";
        break;

    case Trait::summoner:
        out = "-1 Spirit cost for summoning spells, halved risk that called creatures "
              "are hostile";
        break;

    case Trait::blood_sorcerer:
        out = "-1 Spirit cost for all spells, casting a spell drains 2 Hit Points";
        break;

    case Trait::seer:
        out = "Detection spells have decreased Spirit costs, and the spell "
              "\"Detect Monsters\" has tripple duration";
        break;

    case Trait::tough:
        out = "+2 hit points, +10% carry weight limit, better results for object "
              "interactions requiring strength (e.g. bashing doors or pushing a lid)";
        break;

    case Trait::rugged:
        out = "+2 hit points, +10% carry weight limit, better results for object "
              "interactions requiring strength (such as bashing doors, or moving the "
              "lid from a stone coffin)";
        break;

    case Trait::unbreakable:
        out = "+2 hit points, +10% carry weight limit, better results for object "
              "interactions requiring strength (such as bashing doors, or moving the "
              "lid from a stone coffin)";
        break;

    case Trait::strong_backed:
        out = "+30% carry weight limit";
        break;

    case Trait::dexterous:
        out = "+25% chance to dodge melee attacks, better chances to evade traps, "
              "every fifth move is a free action";
        break;

    case Trait::lithe:
        out = "+25% chance to dodge melee attacks, better chances to evade traps, "
              "every fourth move is a free action";
        break;

    case Trait::mobile:
        out = "Every second move is a free action";
        break;

    case Trait::fearless:
        out = "You cannot become terrified, +5% shock resistance";
        break;

    case Trait::healer:
        out = "Using medical equipment takes half the normal time and resources";
        break;

    case Trait::observant:
        out = "You can spot hidden traps and doors from two cells away, and you are "
              "more likely to spot hidden monsters and objects";
        break;

    case Trait::perceptive:
        out = "You can spot hidden traps and doors from three cells away, and you are "
              "more likely to spot hidden monsters and objects";
        break;

    case Trait::vigilant:
        out = "You cannot be backstabbed - hidden monsters gets no melee attack bonus "
              "against you, and their attacks can be dodged";
        break;

    case Trait::rapid_recoverer:
        out = "Increased Hit Point regeneration rate";
        break;

    case Trait::survivalist:
        out = "Increased Hit Point regeneration rate, disease only lowers your Hit "
              "Points by 25% (instead of 50%)";
        break;

    case Trait::perseverant:
        out = "When your Hit Points are reduced to 25% or less, you gain +50% chance to "
              "dodge melee attacks, and +30% hit chance with melee and ranged attacks";
        break;

    case Trait::self_aware:
        out = "You cannot become confused, the number of remaining turns for status "
              "effects are displayed";
        break;

    case Trait::stout_spirit:
        out = "+2 Spirit Points, increased Spirit regeneration rate";
        break;

    case Trait::strong_spirit:
        out = "+2 Spirit Points, increased Spirit regeneration rate";
        break;

    case Trait::mighty_spirit:
        out = "+2 Spirit Points, increased Spirit regeneration rate";
        break;

    case Trait::stealthy:
        out = "You are more likely to avoid detection";
        break;

    case Trait::imperceptible:
        out = "You are more likely to avoid detection";
        break;

    case Trait::vicious:
        out = "+50% backstab damage";
        break;

    case Trait::treasure_hunter:
        out = "You tend to find more items";
        break;

    case Trait::undead_bane:
        out = "+2 melee and ranged attack damage against undead monsters. Attacks "
              "against ethereal undead monsters (e.g. Ghosts) never pass through them "
              "(although you can still miss in the ordinary way)";
        break;

    case Trait::END: {}
        break;
    }
}

void trait_prereqs(const Trait id, vector<Trait>& traits_ref, Bg& bg_ref)
{
    traits_ref.clear();
    bg_ref = Bg::END;

    switch (id)
    {
    case Trait::adept_melee_fighter: {}
        break;

    case Trait::expert_melee_fighter:
        traits_ref.push_back(Trait::adept_melee_fighter);
        break;

    case Trait::master_melee_fighter:
        traits_ref.push_back(Trait::expert_melee_fighter);
        break;

    case Trait::adept_marksman: {}
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

    case Trait::dem_expert: {}
        break;

    case Trait::cool_headed: {}
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

    case Trait::tough: {}
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

    case Trait::dexterous: {}
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

    case Trait::healer: {}
        break;

    case Trait::observant: {}
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

    case Trait::stout_spirit: {}
        break;

    case Trait::strong_spirit:
        traits_ref.push_back(Trait::stout_spirit);
        break;

    case Trait::mighty_spirit:
        traits_ref.push_back(Trait::strong_spirit);
        bg_ref = Bg::occultist;
        break;

    case Trait::stealthy: {}
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

    case Trait::END: {}
        break;
    }

    //Sort lexicographically
    sort(traits_ref.begin(), traits_ref.end(), [](const Trait & t1, const Trait & t2)
    {
        string str1 = ""; trait_title(t1, str1);
        string str2 = ""; trait_title(t2, str2);
        return str1 < str2;
    });
}

Bg bg() {return bg_;}

void pickable_bgs(vector<Bg>& bgs_ref)
{
    bgs_ref.clear();

    for (int i = 0; i < int(Bg::END); ++i) {bgs_ref.push_back(Bg(i));}

    //Sort lexicographically
    sort(bgs_ref.begin(), bgs_ref.end(), [](const Bg & bg1, const Bg & bg2)
    {
        string str1 = ""; bg_title(bg1, str1);
        string str2 = ""; bg_title(bg2, str2);
        return str1 < str2;
    });
}

void pickable_traits(vector<Trait>& traits_ref)
{
    traits_ref.clear();

    for (int i = 0; i < int(Trait::END); ++i)
    {
        if (!traits[i])
        {
            vector<Trait> trait_prereq_list;
            Bg bg_prereq = Bg::END;

            trait_prereqs(Trait(i), trait_prereq_list, bg_prereq);

            bool is_pickable = true;

            for (Trait prereq : trait_prereq_list)
            {
                if (!traits[int(prereq)])
                {
                    is_pickable = false;
                    break;
                }
            }

            is_pickable = is_pickable && (bg_ == bg_prereq || bg_prereq == Bg::END);

            if (is_pickable) {traits_ref.push_back(Trait(i));}
        }
    }

    //Limit the number of trait choices (due to screen space constraints)
    random_shuffle(traits_ref.begin(), traits_ref.end());
    const int MAX_NR_TRAIT_CHOICES = 16;
    traits_ref.resize(min(int(traits_ref.size()), MAX_NR_TRAIT_CHOICES));

    //Sort lexicographically
    sort(traits_ref.begin(), traits_ref.end(),
         [](const Trait & t1, const Trait & t2)
    {
        string str1 = ""; trait_title(t1, str1);
        string str2 = ""; trait_title(t2, str2);
        return str1 < str2;
    });
}

void pick_bg(const Bg bg)
{
    assert(bg != Bg::END);

    bg_ = bg;

    switch (bg_)
    {
    case Bg::occultist:
        pick_trait(Trait::stout_spirit);
        map::player->change_max_hp(-2, false); //Occultist starts with fewer HP
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
        map::player->ins_ += 10; //War Veteran starts a little bit insane
        break;

    case Bg::END: {}
        break;
    }
}

void set_all_traits_to_picked()
{
    for (int i = 0; i < int(Trait::END); ++i) {traits[i] = true;}
}

void pick_trait(const Trait id)
{
    assert(id != Trait::END);

    traits[int(id)] = true;

    switch (id)
    {
    case Trait::tough:
        map::player->change_max_hp(2, false);
        break;

    case Trait::rugged:
        map::player->change_max_hp(2, false);
        break;

    case Trait::unbreakable:
        map::player->change_max_hp(2, false);
        break;

    case Trait::stout_spirit:
        map::player->change_max_spi(2, false);
        break;

    case Trait::strong_spirit:
        map::player->change_max_spi(2, false);
        break;

    case Trait::mighty_spirit:
        map::player->change_max_spi(2, false);
        break;

    case Trait::self_aware:
        map::player->prop_handler().try_apply_prop(
            new Prop_rConfusion(Prop_turns::indefinite), true, Verbosity::silent, true, false);
        break;

    case Trait::fearless:
        map::player->prop_handler().try_apply_prop(
            new Prop_rFear(Prop_turns::indefinite), true, Verbosity::silent, true, false);
        break;

    default: {}
        break;
    }
}

void all_picked_traits_titles_line(string& out)
{
    out = "";

    for (int i = 0; i < int(Trait::END); ++i)
    {
        if (traits[i])
        {
            string title = "";
            trait_title(Trait(i), title);
            out += (out.empty() ? "" : ", ") + title;
        }
    }
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
