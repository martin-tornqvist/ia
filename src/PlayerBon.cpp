#include "PlayerBon.h"

#include <assert.h>

#include "TextFormatting.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemFactory.h"
#include "Inventory.h"
#include "PlayerSpellsHandling.h"
#include "Map.h"
#include "MapParsing.h"

using namespace std;

namespace PlayerBon
{

bool traitsPicked[int(Trait::END)];

namespace
{

Bg bg_ = Bg::END;

} //Namespace

void init()
{
  for (int i = 0; i < int(Trait::END); ++i) {traitsPicked[i] = false;}
  bg_ = Bg::END;
}

void storeToSaveLines(vector<string>& lines)
{
  lines.push_back(toStr(int(bg_)));
  for (int i = 0; i < int(Trait::END); ++i)
  {
    lines.push_back(traitsPicked[i] ? "1" : "0");
  }
}

void setupFromSaveLines(vector<string>& lines)
{
  bg_ = Bg(toInt(lines.front()));
  lines.erase(begin(lines));

  for (int i = 0; i < int(Trait::END); ++i)
  {
    traitsPicked[i] = lines.front() == "1";
    lines.erase(begin(lines));
  }
}

void getBgTitle(const Bg id, string& out)
{
  out = "[BG TITLE MISSING]";
  switch (id)
  {
    case Bg::occultist: out = "Occultist";   break;
    case Bg::rogue:     out = "Rogue";       break;
    case Bg::warVet:    out = "War Veteran"; break;
    case Bg::END: {} break;
  }
}

void getTraitTitle(const Trait id, string& out)
{
  out = "[TRAIT TITLE MISSING]";

  switch (id)
  {
    case Trait::adeptMeleeFighter:    out = "Adept Melee Fighter";   break;
    case Trait::expertMeleeFighter:   out = "Expert Melee Fighter";  break;
    case Trait::masterMeleeFighter:   out = "Master Melee Fighter";  break;
    case Trait::coolHeaded:           out = "Cool-headed";           break;
    case Trait::courageous:           out = "Courageous";            break;
    case Trait::warlock:              out = "Warlock";               break;
    case Trait::summoner:             out = "Summoner";              break;
    case Trait::bloodSorcerer:        out = "Blood Sorcerer";        break;
    case Trait::seer:                 out = "Seer";                  break;
    case Trait::dexterous:            out = "Dexterous";             break;
    case Trait::lithe:                out = "Lithe";                 break;
    case Trait::mobile:               out = "Mobile";                break;
    case Trait::fearless:             out = "Fearless";              break;
    case Trait::healer:               out = "Healer";                break;
    case Trait::adeptMarksman:        out = "Adept Marksman";        break;
    case Trait::expertMarksman:       out = "Expert Marksman";       break;
    case Trait::masterMarksman:       out = "Master Marksman";       break;
    case Trait::steadyAimer:          out = "Steady Aimer";          break;
    case Trait::sharpShooter:         out = "Sharpshooter";          break;
    case Trait::observant:            out = "Observant";             break;
    case Trait::perceptive:           out = "Perceptive";            break;
    case Trait::vigilant:             out = "Vigilant";              break;
    case Trait::rapidRecoverer:       out = "Rapid Recoverer";       break;
    case Trait::survivalist:          out = "Survivalist";           break;
    case Trait::perseverant:          out = "Perseverant";           break;
    case Trait::selfAware:            out = "Self-aware";            break;
    case Trait::stoutSpirit:          out = "Stout Spirit";          break;
    case Trait::strongSpirit:         out = "Strong Spirit";         break;
    case Trait::mightySpirit:         out = "Mighty Spirit";         break;
    case Trait::stealthy:             out = "Stealthy";              break;
    case Trait::imperceptible:        out = "Imperceptible";         break;
    case Trait::vicious:              out = "Vicious";               break;
    case Trait::strongBacked:         out = "Strong-backed";         break;
    case Trait::tough:                out = "Tough";                 break;
    case Trait::rugged:               out = "Rugged";                break;
    case Trait::unbreakable:          out = "Unbreakable";           break;
    case Trait::treasureHunter:       out = "Treasure Hunter";       break;
    case Trait::demExpert:            out = "Demolition Expert";     break;
    case Trait::undeadBane:           out = "Bane of the Undead";    break;
    case Trait::END: break;
  }
}

void getBgDescr(const Bg id, vector<string>& linesRef)
{
  linesRef.clear();
  string s = "";

  switch (id)
  {
    case Bg::occultist:
      linesRef.push_back("Can learn spells cast from manuscripts");
      linesRef.push_back(" ");
      linesRef.push_back("-50% shock taken from using and identifying "
                         "strange items (e.g. potions)");
      linesRef.push_back(" ");
      linesRef.push_back("Can dispel magic traps");
      linesRef.push_back(" ");
      linesRef.push_back("-2 Hit Points");
      linesRef.push_back(" ");
      linesRef.push_back("Starts with the following trait(s):");
      linesRef.push_back(" ");
      getTraitTitle(Trait::stoutSpirit, s); linesRef.push_back("* " + s);
      getTraitDescr(Trait::stoutSpirit, s); linesRef.push_back(s);
      break;

    case Bg::rogue:
      linesRef.push_back("Has an arcane ability to cloud the minds of "
                         "enemies, causing them to forget their pursuit "
                         "(press [x])");
      linesRef.push_back(" ");
      //TODO: Should this be a trait instead?
      linesRef.push_back("+25% hit chance with ranged attacks vs unaware targets");
      linesRef.push_back(" ");
      linesRef.push_back("The rate of shock recieved passively over time "
                         "is reduced by half");
      linesRef.push_back(" ");
      linesRef.push_back("Starts with the following trait(s):");
      linesRef.push_back(" ");
      getTraitTitle(Trait::observant, s); linesRef.push_back("* " + s);
      getTraitDescr(Trait::observant, s); linesRef.push_back(s);
      linesRef.push_back(" ");
      getTraitTitle(Trait::stealthy, s);  linesRef.push_back("* " + s);
      getTraitDescr(Trait::stealthy, s);  linesRef.push_back(s);
      break;

    case Bg::warVet:
      linesRef.push_back("Can switch to prepared weapon instantly");
      linesRef.push_back(" ");
      linesRef.push_back("Maintains armor twice as long before it breaks");
      linesRef.push_back(" ");
      linesRef.push_back("Starts with 10% insanity");
      linesRef.push_back(" ");
      linesRef.push_back("Starts with the following trait(s):");
      linesRef.push_back(" ");
      getTraitTitle(Trait::adeptMarksman, s);     linesRef.push_back("* " + s);
      getTraitDescr(Trait::adeptMarksman, s);     linesRef.push_back(s);
      linesRef.push_back(" ");
      getTraitTitle(Trait::adeptMeleeFighter, s); linesRef.push_back("* " + s);
      getTraitDescr(Trait::adeptMeleeFighter, s); linesRef.push_back(s);
      linesRef.push_back(" ");
      getTraitTitle(Trait::tough, s);             linesRef.push_back("* " + s);
      getTraitDescr(Trait::tough, s);             linesRef.push_back(s);
      break;

    case Bg::END: {}
      break;
  }
}

void getTraitDescr(const Trait id, string& out)
{
  out = "[TRAIT DESCRIPTION MISSING]";

  switch (id)
  {
    case Trait::adeptMeleeFighter:
      out = "+10% hit chance with melee attacks";
      break;

    case Trait::expertMeleeFighter:
      out = "+10% hit chance with melee attacks";
      break;

    case Trait::masterMeleeFighter:
      out = "+10% hit chance with melee attacks";
      break;

    case Trait::adeptMarksman:
      out = "+10% hit chance with firearms and thrown weapons";
      break;

    case Trait::expertMarksman:
      out = "+10% hit chance with firearms and thrown weapons, you occasionally reload "
            "instantly";
      break;

    case Trait::masterMarksman:
      out = "+10% hit chance with firearms and thrown weapons";
      break;

    case Trait::steadyAimer:
      out = "No recoil penalty when using firearms. Standing still gives ranged "
            "attacks +10% hit chance on the following turn";
      break;

    case Trait::sharpShooter:
      out = "Standing still for three turns gives ranged attacks maximum hit chance and "
            "damage on the following turn";
      break;

    case Trait::demExpert:
      out = "+1 radius for explosives (be careful), you are not harmed by your own "
            "Molotov Cocktails, you occasionally light explosives instantly";
      break;

    case Trait::coolHeaded:
      out = "+20% shock resistance";
      break;

    case Trait::courageous:
      out = "+20% shock resistance";
      break;

    case Trait::warlock:
      out = "-1 Spirit cost for damage dealing spells, casting any spell has a chance "
            "to make you \"Charged\" for one turn, causing attack spells to do maximum "
            "damage";
      break;

    case Trait::summoner:
      out = "-1 Spirit cost for summoning spells, halved risk that called creatures are "
            "hostile";
      break;

    case Trait::bloodSorcerer:
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
            "interactions requiring strength (such as bashing doors, or moving the lid "
            "from a stone coffin)";
      break;

    case Trait::unbreakable:
      out = "+2 hit points, +10% carry weight limit, better results for object "
            "interactions requiring strength (such as bashing doors, or moving the lid "
            "from a stone coffin)";
      break;

    case Trait::strongBacked:
      out = "+30% carry weight limit";
      break;

    case Trait::dexterous:
      out = "+25% chance to dodge melee attacks, better chances to evade traps, "
            "every fifth move is a free action";
      break;

    case Trait::lithe:
      out = "+25% chance to dodge melee attacks, better chances to evade traps, every "
            "fourth move is a free action";
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
      out = "You can spot hidden traps and doors from two cells away, and you are more "
            "likely to spot hidden monsters and objects";
      break;

    case Trait::perceptive:
      out = "You can spot hidden traps and doors from three cells away, and you are "
            "more likely to spot hidden monsters and objects";
      break;

    case Trait::vigilant:
      out = "You cannot be backstabbed - hidden monsters gets no melee attack bonus "
            "against you, and their attacks can be dodged";
      break;

    case Trait::rapidRecoverer:
      out = "Increased Hit Point regeneration rate";
      break;

    case Trait::survivalist:
      out = "Increased Hit Point regeneration rate, disease only lowers your Hit Points "
            "by 25% (instead of 50%)";
      break;

    case Trait::perseverant:
      out = "When your Hit Points are reduced to 25% or less, you gain +50% chance to "
            "dodge melee attacks, and +30% hit chance with melee and ranged attacks";
      break;

    case Trait::selfAware:
      out = "You cannot become confused, the number of remaining turns for status "
            "effects are displayed";
      break;

    case Trait::stoutSpirit:
      out = "+2 Spirit Points, increased Spirit regeneration rate";
      break;

    case Trait::strongSpirit:
      out = "+2 Spirit Points, increased Spirit regeneration rate";
      break;

    case Trait::mightySpirit:
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

    case Trait::treasureHunter:
      out = "You tend to find more items";
      break;

    case Trait::undeadBane:
      out = "+2 melee and ranged attack damage against undead monsters. Attacks "
            "against ethereal undead monsters (e.g. Ghosts) never pass through them "
            "(although you can still miss in the ordinary way)";
      break;

    case Trait::END: {}
      break;
  }
}

void getTraitPrereqs(const Trait id, vector<Trait>& traitsRef, Bg& bgRef)
{
  traitsRef.clear();
  bgRef = Bg::END;

  switch (id)
  {
    case Trait::adeptMeleeFighter: {}
      break;

    case Trait::expertMeleeFighter:
      traitsRef.push_back(Trait::adeptMeleeFighter);
      break;

    case Trait::masterMeleeFighter:
      traitsRef.push_back(Trait::expertMeleeFighter);
      break;

    case Trait::adeptMarksman: {}
      break;

    case Trait::expertMarksman:
      traitsRef.push_back(Trait::adeptMarksman);
      break;

    case Trait::masterMarksman:
      traitsRef.push_back(Trait::expertMarksman);
      bgRef = Bg::warVet;
      break;

    case Trait::steadyAimer:
      traitsRef.push_back(Trait::adeptMarksman);
      break;

    case Trait::sharpShooter:
      traitsRef.push_back(Trait::steadyAimer);
      bgRef = Bg::warVet;
      break;

    case Trait::demExpert: {}
      break;

    case Trait::coolHeaded: {}
      break;

    case Trait::courageous:
      traitsRef.push_back(Trait::coolHeaded);
      bgRef = Bg::warVet;
      break;

    case Trait::warlock:
      traitsRef.push_back(Trait::fearless);
      traitsRef.push_back(Trait::strongSpirit);
      bgRef = Bg::occultist;
      break;

    case Trait::summoner:
      traitsRef.push_back(Trait::fearless);
      traitsRef.push_back(Trait::strongSpirit);
      bgRef = Bg::occultist;
      break;

    case Trait::bloodSorcerer:
      traitsRef.push_back(Trait::tough);
      bgRef = Bg::occultist;
      break;

    case Trait::seer:
      traitsRef.push_back(Trait::observant);
      bgRef = Bg::occultist;
      break;

    case Trait::tough: {}
      break;

    case Trait::rugged:
      traitsRef.push_back(Trait::tough);
      break;

    case Trait::unbreakable:
      traitsRef.push_back(Trait::rugged);
      break;

    case Trait::strongBacked:
      traitsRef.push_back(Trait::tough);
      break;

    case Trait::dexterous: {}
      break;

    case Trait::lithe:
      traitsRef.push_back(Trait::dexterous);
      break;

    case Trait::mobile:
      traitsRef.push_back(Trait::lithe);
      break;

    case Trait::fearless:
      traitsRef.push_back(Trait::coolHeaded);
      break;

    case Trait::healer: {}
      break;

    case Trait::observant: {}
      break;

    case Trait::perceptive:
      traitsRef.push_back(Trait::observant);
      break;

    case Trait::vigilant:
      traitsRef.push_back(Trait::observant);
      break;

    case Trait::rapidRecoverer:
      traitsRef.push_back(Trait::tough);
      traitsRef.push_back(Trait::healer);
      break;

    case Trait::survivalist:
      traitsRef.push_back(Trait::rapidRecoverer);
      break;

    case Trait::perseverant:
      traitsRef.push_back(Trait::tough);
      traitsRef.push_back(Trait::fearless);
      break;

    case Trait::selfAware:
      traitsRef.push_back(Trait::stoutSpirit);
      traitsRef.push_back(Trait::observant);
      break;

    case Trait::stoutSpirit: {}
      break;

    case Trait::strongSpirit:
      traitsRef.push_back(Trait::stoutSpirit);
      break;

    case Trait::mightySpirit:
      traitsRef.push_back(Trait::strongSpirit);
      bgRef = Bg::occultist;
      break;

    case Trait::stealthy: {}
      break;

    case Trait::imperceptible:
      traitsRef.push_back(Trait::stealthy);
      break;

    case Trait::vicious:
      traitsRef.push_back(Trait::stealthy);
      traitsRef.push_back(Trait::dexterous);
      break;

    case Trait::treasureHunter:
      traitsRef.push_back(Trait::observant);
      break;

    case Trait::undeadBane:
      traitsRef.push_back(Trait::tough);
      traitsRef.push_back(Trait::fearless);
      traitsRef.push_back(Trait::stoutSpirit);
      break;

    case Trait::END: {}
      break;
  }

  //Sort lexicographically
  sort(traitsRef.begin(), traitsRef.end(), [](const Trait & t1, const Trait & t2)
  {
    string str1 = ""; getTraitTitle(t1, str1);
    string str2 = ""; getTraitTitle(t2, str2);
    return str1 < str2;
  });
}

Bg getBg() {return bg_;}

void getPickableBgs(vector<Bg>& bgsRef)
{
  bgsRef.clear();

  for (int i = 0; i < int(Bg::END); ++i) {bgsRef.push_back(Bg(i));}

  //Sort lexicographically
  sort(bgsRef.begin(), bgsRef.end(), [](const Bg & bg1, const Bg & bg2)
  {
    string str1 = ""; getBgTitle(bg1, str1);
    string str2 = ""; getBgTitle(bg2, str2);
    return str1 < str2;
  });
}

void getPickableTraits(vector<Trait>& traitsRef)
{
  traitsRef.clear();

  for (int i = 0; i < int(Trait::END); ++i)
  {
    if (!traitsPicked[i])
    {
      vector<Trait> traitPrereqs;
      Bg bgPrereq = Bg::END;
      getTraitPrereqs(Trait(i), traitPrereqs, bgPrereq);

      bool isPickable = true;
      for (Trait prereq : traitPrereqs)
      {
        if (!traitsPicked[int(prereq)])
        {
          isPickable = false;
          break;
        }
      }

      isPickable = isPickable && (bg_ == bgPrereq || bgPrereq == Bg::END);

      if (isPickable) {traitsRef.push_back(Trait(i));}
    }
  }

  //Limit the number of trait choices (due to screen space constraints)
  random_shuffle(traitsRef.begin(), traitsRef.end());
  const int MAX_NR_TRAIT_CHOICES = 16;
  traitsRef.resize(min(int(traitsRef.size()), MAX_NR_TRAIT_CHOICES));

  //Sort lexicographically
  sort(traitsRef.begin(), traitsRef.end(),
       [](const Trait & t1, const Trait & t2)
  {
    string str1 = ""; getTraitTitle(t1, str1);
    string str2 = ""; getTraitTitle(t2, str2);
    return str1 < str2;
  });
}

void pickBg(const Bg bg)
{
  assert(bg != Bg::END);

  bg_ = bg;

  switch (bg_)
  {
    case Bg::occultist:
      pickTrait(Trait::stoutSpirit);
      Map::player->changeMaxHp(-2, false); //Occultist starts with fewer HP
      break;

    case Bg::rogue:
      PlayerSpellsHandling::learnSpellIfNotKnown(SpellId::cloudMinds);
      pickTrait(Trait::observant);
      pickTrait(Trait::stealthy);
      break;

    case Bg::warVet:
      pickTrait(Trait::adeptMeleeFighter);
      pickTrait(Trait::adeptMarksman);
      pickTrait(Trait::tough);
      Map::player->ins_ += 10; //War Veteran starts a little bit insane
      break;

    case Bg::END: {}
      break;
  }
}

void setAllTraitsToPicked()
{
  for (int i = 0; i < int(Trait::END); ++i) {traitsPicked[i] = true;}
}

void pickTrait(const Trait id)
{
  assert(id != Trait::END);

  traitsPicked[int(id)] = true;

  switch (id)
  {
    case Trait::tough:
      Map::player->changeMaxHp(2, false);
      break;

    case Trait::rugged:
      Map::player->changeMaxHp(2, false);
      break;

    case Trait::unbreakable:
      Map::player->changeMaxHp(2, false);
      break;

    case Trait::stoutSpirit:
      Map::player->changeMaxSpi(2, false);
      break;

    case Trait::strongSpirit:
      Map::player->changeMaxSpi(2, false);
      break;

    case Trait::mightySpirit:
      Map::player->changeMaxSpi(2, false);
      break;

    case Trait::selfAware:
      Map::player->getPropHandler().tryApplyProp(
        new PropRConfusion(PropTurns::indefinite), true, true, true, false);
      break;

    case Trait::fearless:
      Map::player->getPropHandler().tryApplyProp(
        new PropRFear(PropTurns::indefinite), true, true, true, false);
      break;

    case Trait::steadyAimer:
      Map::player->getPropHandler().endAppliedProp(PropId::recoil);
      break;

    default: {}
      break;
  }
}

void getAllPickedTraitsTitlesLine(string& out)
{
  out = "";

  for (int i = 0; i < int(Trait::END); ++i)
  {
    if (traitsPicked[i])
    {
      string title = "";
      getTraitTitle(Trait(i), title);
      out += (out.empty() ? "" : ", ") + title;
    }
  }
}

int getSpiOccultistCanCastAtLvl(const int LVL)
{
  assert(LVL > 0);
  const int SPI_FROM_START_TRAIT  = 2;
  const int SPI_FROM_LVLS         = (LVL - 1) * SPI_PER_LVL;
  return PLAYER_START_SPI + SPI_FROM_LVLS + SPI_FROM_START_TRAIT - 1;
}

bool getsUndeadBaneBon(const Actor& attacker, const ActorDataT& actorData)
{
  return attacker.isPlayer()                              &&
         PlayerBon::traitsPicked[int(Trait::undeadBane)]  &&
         actorData.isUndead;
}

} //PlayerBon
