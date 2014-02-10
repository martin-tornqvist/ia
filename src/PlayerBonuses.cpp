#include "PlayerBonuses.h"

#include <assert.h>

#include "Engine.h"
#include "TextFormatting.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ItemFactory.h"
#include "Inventory.h"
#include "PlayerSpellsHandler.h"

using namespace std;

PlayerBonHandler::PlayerBonHandler(Engine& engine) :
  bg_(endOfBgs), eng(engine) {
  traitsPicked_.resize(0);
}

void PlayerBonHandler::getBgTitle(const Bg id, string& strRef) const {
  strRef = "[BG TITLE MISSING]";
  switch(id) {
    case bgOccultist: strRef = "Occultist"; break;
    case bgRogue:     strRef = "Rogue";     break;
    case bgSoldier:   strRef = "Soldier";   break;
    case endOfBgs: {} break;
  }
}

void PlayerBonHandler::getTraitTitle(
  const TraitId id, string& strRef) const {

  strRef = "[TRAIT TITLE MISSING]";

  switch(id) {
    case traitAdeptMeleeFighter:    strRef = "Adept Melee Fighter";     break;
    case traitExpertMeleeFighter:   strRef = "Expert Melee Fighter";    break;
    case traitMasterMeleeFighter:   strRef = "Master Melee Fighter";    break;
    case traitCoolHeaded:           strRef = "Cool-headed";             break;
    case traitCourageous:           strRef = "Courageous";              break;
    case traitSelfPossessed:        strRef = "Self-possessed";          break;
    case traitMythologist:          strRef = "Mythologist";             break;
    case traitWarlock:              strRef = "Warlock";                 break;
    case traitBloodSorcerer:        strRef = "Blood Sorcerer";          break;
    case traitSpiritCannibal:       strRef = "Spirit Cannibal";         break;
    case traitSeer:                 strRef = "Seer";                    break;
    case traitDexterous:            strRef = "Dexterous";               break;
    case traitLithe:                strRef = "Lithe";                   break;
    case traitMobile:               strRef = "Mobile";                  break;
    case traitFearless:             strRef = "Fearless";                break;
    case traitHealer:               strRef = "Healer";                  break;
    case traitAdeptMarksman:        strRef = "Adept Marksman";          break;
    case traitExpertMarksman:       strRef = "Expert Marksman";         break;
    case traitMasterMarksman:       strRef = "Master Marksman";         break;
    case traitSteadyAimer:          strRef = "Steady Aimer";            break;
    case traitSharpShooter:         strRef = "Sharpshooter";            break;
    case traitObservant:            strRef = "Observant";               break;
    case traitPerceptive:           strRef = "Perceptive";              break;
    case traitVigilant:             strRef = "Vigilant";                break;
    case traitRapidRecoverer:       strRef = "Rapid Recoverer";         break;
    case traitSurvivalist:          strRef = "Survivalist";             break;
    case traitSelfAware:            strRef = "Self-aware";              break;
    case traitPotentSpirit:         strRef = "Potent Spirit";           break;
    case traitStrongSpirit:         strRef = "Strong Spirit";           break;
    case traitMightySpirit:         strRef = "Mighty Spirit";           break;
    case traitStealthy:             strRef = "Stealthy";                break;
    case traitImperceptible:        strRef = "Imperceptible";           break;
    case traitStrongBacked:         strRef = "Strong-backed";           break;
    case traitTough:                strRef = "Tough";                   break;
    case traitRugged:               strRef = "Rugged";                  break;
    case traitBreachExpert:         strRef = "Breach Expert";           break;
    case traitTreasureHunter:       strRef = "Treasure Hunter";         break;
    case traitDemolitionExpert:     strRef = "Demolition Expert";       break;
    case endOfTraits: break;
  }
}

void PlayerBonHandler::getBgDescr(
  const Bg id, vector<string>& linesRef) const {

  linesRef.resize(0);
  string s = "";

  switch(id) {
    case bgOccultist: {
      linesRef.push_back("Can memorize spells cast from manuscripts");
      linesRef.push_back("");
//      linesRef.push_back("Starts with some manuscripts and potions");
//      linesRef.push_back("");
      linesRef.push_back("-2 Hit Points");
      linesRef.push_back("");
      linesRef.push_back("Starts with the following trait(s):");
      linesRef.push_back("");
      getTraitTitle(traitPotentSpirit, s); linesRef.push_back("* " + s);
      getTraitDescr(traitPotentSpirit, s); linesRef.push_back(s);
    } break;

    case bgRogue: {
      s =  "Has an arcane ability to cloud the minds of enemies, causing ";
      s += "them to forget their pursuit (press [x])";
      linesRef.push_back(s);
      linesRef.push_back("");
      s = "+25% hit chance with ranged attacks vs unaware targets";
      linesRef.push_back(s);
      linesRef.push_back("");
      s = "Takes no shock from seeing monsters if they are unaware";
      linesRef.push_back(s);
      linesRef.push_back("");
      linesRef.push_back("Starts with the following trait(s):");
      linesRef.push_back("");
      getTraitTitle(traitObservant, s);           linesRef.push_back("* " + s);
      getTraitDescr(traitObservant, s);           linesRef.push_back(s);
      linesRef.push_back("");
      getTraitTitle(traitStealthy, s);            linesRef.push_back("* " + s);
      getTraitDescr(traitStealthy, s);            linesRef.push_back(s);
    } break;

    case bgSoldier: {
//      linesRef.push_back("Starts with more combat equipment");
//      linesRef.push_back("");
      linesRef.push_back("Starts with the following trait(s):");
      linesRef.push_back("");
      getTraitTitle(traitAdeptMarksman, s);       linesRef.push_back("* " + s);
      getTraitDescr(traitAdeptMarksman, s);       linesRef.push_back(s);
      linesRef.push_back("");
      getTraitTitle(traitAdeptMeleeFighter, s);   linesRef.push_back("* " + s);
      getTraitDescr(traitAdeptMeleeFighter, s);   linesRef.push_back(s);
      linesRef.push_back("");
      getTraitTitle(traitTough, s);               linesRef.push_back("* " + s);
      getTraitDescr(traitTough, s);               linesRef.push_back(s);
    } break;

    case endOfBgs: {} break;
  }
}

void PlayerBonHandler::getTraitDescr(
  const TraitId id, string& strRef) const {

  strRef = "[TRAIT DESCRIPTION MISSING]";

  switch(id) {
    case traitAdeptMeleeFighter: {
      strRef  = "+10% hit chance with melee weapons";
    } break;

    case traitExpertMeleeFighter: {
      strRef  = "+10% hit chance with melee weapons";
    } break;

    case traitMasterMeleeFighter: {
      strRef  = "+10% hit chance with melee weapons";
    } break;

    case traitAdeptMarksman: {
      strRef  = "+10% hit chance with firearms and thrown weapons";
    } break;

    case traitExpertMarksman: {
      strRef  = "+10% hit chance with firearms and thrown weapons, ";
      strRef += "you occasionally reload instantly";
    } break;

    case traitMasterMarksman: {
      strRef  = "+10% hit chance with firearms and thrown weapons";
    } break;

    case traitSteadyAimer: {
      strRef  = "Standing still gives ranged attacks +10% hit chance on the ";
      strRef += "following turn";
    } break;

    case traitSharpShooter: {
      strRef  = "Standing still for three turns gives ranged attacks maximum ";
      strRef += "hit chance and damage on the following turn";
    } break;

    case traitDemolitionExpert: {
      strRef  = "+1 radius for explosives (be careful), you are not harmed ";
      strRef += "by your own Molotov Cocktails, you occasionally light ";
      strRef += "explosives instantly";
    } break;

    case traitCoolHeaded: {
      strRef  = "+20% shock resistance";
    } break;

    case traitCourageous: {
      strRef  = "+30% shock resistance";
    } break;

    case traitSelfPossessed: {
      strRef  = "-50% passive shock received over time (does not affect ";
      strRef += "shock from seeing terrifying creatures, using magic, etc)";
    } break;

    case traitMythologist: {
      strRef  = "-50% shock taken from seeing terrifying creatures, for ";
      strRef += "casting spells and for using and identifying strange items ";
      strRef += "(e.g. potions)";
    } break;

    case traitWarlock: {
      strRef  = "-1 Spirit cost for damage dealing spells, casting any spell ";
      strRef += "can make you \"Charged\" for one turn, causing attack spells ";
      strRef += "to do maximum damage";
      strRef += "[UNIMPLEMENTED]";
    } break;

    case traitBloodSorcerer: {
      strRef  = "-1 Spirit cost for all spells, casting a spell drains 2 ";
      strRef += "Hit points";
      strRef += "[UNIMPLEMENTED]";
    } break;

    case traitSpiritCannibal: {
      strRef  = "For every life you take, you gain 1 Spirit Point by ";
      strRef += "absorbing a part of that creatures spirit (does not work ";
      strRef += "on insignificant creatures like a small spider or a rat)";
      strRef += "[UNIMPLEMENTED]";
    } break;

    case traitSeer: {
      strRef  = "The spell \"Clairvoyance\" has double duration, and ";
      strRef += "detection spells have increased range - spirit costs for ";
      strRef += "these spells are reduced";
      strRef += "[UNIMPLEMENTED]";
    } break;

    case traitTough: {
      strRef  = "+2 hit points, +10% carry weight limit, better results ";
      strRef += "for object interactions requiring strength (e.g. bashing ";
      strRef += "doors or pushing a lid)";
    } break;

    case traitRugged: {
      strRef  = "+2 hit points, +10% carry weight limit, better results ";
      strRef += "for object interactions requiring strength (such as ";
      strRef += "bashing doors, or moving the lid from a stone coffin)";
    } break;

    case traitStrongBacked: {
      strRef  = "+30% carry weight limit";
    } break;

    case traitBreachExpert: {
      strRef  = "Increased chance of success when bashing doors, breaking ";
      strRef += "a door is a free turn, any creature adjacent to the door ";
      strRef += "on the opposite side is damaged and stunned";
      strRef += "[UNIMPLEMENTED]";
    } break;

    case traitDexterous: {
      strRef  = "+20% chance to dodge melee attacks, better chances to evade ";
      strRef += "traps, every fifth move is a free action";
    } break;

    case traitLithe: {
      strRef  = "+20% chance to dodge melee attacks, better chances to evade ";
      strRef += "traps, every fifth move is a free action";
    } break;

    case traitMobile: {
      strRef  = "Every second move is a free action";
    } break;

    case traitFearless: {
      strRef  = "You cannot become terrified, +5% shock resistance";
    } break;

    case traitHealer: {
      strRef  = "Healing takes half the normal time and resources";
    } break;

    case traitObservant: {
      strRef  = "You can spot hidden traps and doors from two cells away, ";
      strRef += "you are more likely to spot hidden things and monsters, and ";
      strRef += "your attentiveness is higher when examining objects";
    } break;

    case traitPerceptive: {
      strRef  = "You can spot hidden traps and doors from three cells away, ";
      strRef += "you are more likely to spot hidden things and monsters, and ";
      strRef += "your attentiveness is higher when examining objects";
    } break;

    case traitVigilant: {
      strRef  = "You cannot be backstabbed - hidden monsters gets no melee ";
      strRef += "attack bonus against you, and their attacks can be dodged";
    } break;

    case traitRapidRecoverer: {
      strRef  = "Increased Hit Point regeneration rate";
    } break;

    case traitSurvivalist: {
      strRef  = "Increased Hit Point regeneration rate, negative effects ";
      strRef += "from wounds and disease reduced by 50%";
      strRef += "[UNIMPLEMENTED]";
    } break;

    case traitSelfAware: {
      strRef  = "You cannot become confused, the number of remaining turns ";
      strRef += "for status effects are displayed";
    } break;

    case traitPotentSpirit: {
      strRef  = "+2 Spirit Points, increased Spirit regeneration rate";
    } break;

    case traitStrongSpirit: {
      strRef  = "+2 Spirit Points, increased Spirit regeneration rate";
    } break;

    case traitMightySpirit: {
      strRef  = "+2 Spirit Points, increased Spirit regeneration rate";
    } break;

    case traitStealthy: {
      strRef  = "You are more likely to avoid detection";
    } break;

    case traitImperceptible: {
      strRef  = "You are more likely to avoid detection";
    } break;

    case traitTreasureHunter: {
      strRef  = "You tend to find more items";
    } break;

    case endOfTraits: {} break;
  }
}

void PlayerBonHandler::getTraitPrereqs(const TraitId id,
                                       vector<TraitId>& traitsRef,
                                       Bg& bgRef) const {
  traitsRef.resize(0);
  bgRef = endOfBgs;

  //TODO Add background prereqs
  //TODO Add CLVL prereqs

  switch(id) {
    case traitAdeptMeleeFighter: {
    } break;

    case traitExpertMeleeFighter: {
      traitsRef.push_back(traitAdeptMeleeFighter);
    } break;

    case traitMasterMeleeFighter: {
      traitsRef.push_back(traitExpertMeleeFighter);
    } break;

    case traitAdeptMarksman: {
    } break;

    case traitExpertMarksman: {
      traitsRef.push_back(traitAdeptMarksman);
    } break;

    case traitMasterMarksman: {
      traitsRef.push_back(traitExpertMarksman);
      bgRef = bgSoldier;
    } break;

    case traitSteadyAimer: {
      traitsRef.push_back(traitAdeptMarksman);
    } break;

    case traitSharpShooter: {
      traitsRef.push_back(traitSteadyAimer);
    } break;

    case traitDemolitionExpert: {
    } break;

    case traitCoolHeaded: {
    } break;

    case traitCourageous: {
      traitsRef.push_back(traitCoolHeaded);
      bgRef = bgSoldier;
    } break;

    case traitMythologist: {
      traitsRef.push_back(traitCoolHeaded);
      bgRef = bgOccultist;
    } break;

    case traitWarlock: {
      traitsRef.push_back(traitMythologist);
      traitsRef.push_back(traitFearless);
      bgRef = bgOccultist;
    } break;

    case traitBloodSorcerer: {
      traitsRef.push_back(traitMythologist);
      traitsRef.push_back(traitTough);
      bgRef = bgOccultist;
    } break;

    case traitSpiritCannibal: {
      traitsRef.push_back(traitMythologist);
      traitsRef.push_back(traitStrongSpirit);
      bgRef = bgOccultist;
    } break;

    case traitSeer: {
      traitsRef.push_back(traitMythologist);
      traitsRef.push_back(traitObservant);
      bgRef = bgOccultist;
    } break;

    case traitSelfPossessed: {
      traitsRef.push_back(traitCoolHeaded);
      bgRef = bgRogue;
    } break;

    case traitTough: {
    } break;

    case traitRugged: {
      traitsRef.push_back(traitTough);
    } break;

    case traitStrongBacked: {
      traitsRef.push_back(traitTough);
    } break;

    case traitBreachExpert: {
      traitsRef.push_back(traitTough);
      bgRef = bgSoldier;
    } break;

    case traitDexterous: {
    } break;

    case traitLithe: {
      traitsRef.push_back(traitDexterous);
    } break;

    case traitMobile: {
      traitsRef.push_back(traitLithe);
    } break;

    case traitFearless: {
      traitsRef.push_back(traitCoolHeaded);
    } break;

    case traitHealer: {
    } break;

    case traitObservant: {
    } break;

    case traitPerceptive: {
      traitsRef.push_back(traitObservant);
    } break;

    case traitVigilant: {
      traitsRef.push_back(traitObservant);
    } break;

    case traitRapidRecoverer: {
      traitsRef.push_back(traitTough);
      traitsRef.push_back(traitHealer);
    } break;

    case traitSurvivalist: {
      traitsRef.push_back(traitRapidRecoverer);
    } break;

    case traitSelfAware: {
      traitsRef.push_back(traitPotentSpirit);
      traitsRef.push_back(traitObservant);
    } break;

    case traitPotentSpirit: {
    } break;

    case traitStrongSpirit: {
      traitsRef.push_back(traitPotentSpirit);
    } break;

    case traitMightySpirit: {
      traitsRef.push_back(traitStrongSpirit);
      bgRef = bgOccultist;
    } break;

    case traitStealthy: {
    } break;

    case traitImperceptible: {
      traitsRef.push_back(traitStealthy);
      traitsRef.push_back(traitDexterous);
    } break;

    case traitTreasureHunter: {
      traitsRef.push_back(traitObservant);
    } break;

    case endOfTraits: {} break;
  }

  //Sort lexicographically
  sort(traitsRef.begin(), traitsRef.end(),
  [this](const TraitId & t1, const TraitId & t2) {
    string str1 = ""; getTraitTitle(t1, str1);
    string str2 = ""; getTraitTitle(t2, str2);
    return str1 < str2;
  });
}

void PlayerBonHandler::getPickableBgs(vector<Bg>& bgsRef) const {
  bgsRef.resize(0);

  for(int i = 0; i < endOfBgs; i++) {bgsRef.push_back(Bg(i));}

  //Sort lexicographically
  sort(bgsRef.begin(), bgsRef.end(),
  [this](const Bg & bg1, const Bg & bg2) {
    string str1 = ""; getBgTitle(bg1, str1);
    string str2 = ""; getBgTitle(bg2, str2);
    return str1 < str2;
  });
}

void PlayerBonHandler::getPickableTraits(vector<TraitId>& traitsRef) const {
  traitsRef.resize(0);

  for(int i = 0; i < endOfTraits; i++) {

    const TraitId trait = TraitId(i);

    if(hasTrait(trait) == false) {

      vector<TraitId> traitPrereqs;
      Bg bgPrereq = endOfBgs;
      getTraitPrereqs(TraitId(i), traitPrereqs, bgPrereq);

      bool isPickable = true;
      for(TraitId prereq : traitPrereqs) {

        if(hasTrait(prereq) == false) {
          isPickable = false;
          break;
        }
      }

      isPickable = isPickable && (bg_ == bgPrereq || bgPrereq == endOfBgs);

      if(isPickable) {
        traitsRef.push_back(TraitId(i));
      }
    }
  }

  //Limit the number of trait choices (due to screen space constraints)
  random_shuffle(traitsRef.begin(), traitsRef.end());
  const int MAX_NR_TRAIT_CHOICES = 16;
  traitsRef.resize(min(int(traitsRef.size()), MAX_NR_TRAIT_CHOICES));

  //Sort lexicographically
  sort(traitsRef.begin(), traitsRef.end(),
  [this](const TraitId & t1, const TraitId & t2) {
    string str1 = ""; getTraitTitle(t1, str1);
    string str2 = ""; getTraitTitle(t2, str2);
    return str1 < str2;
  });
}

void PlayerBonHandler::pickBg(const Bg bg) {
  assert(bg != endOfBgs);

  bg_ = bg;

  switch(bg_) {
    case bgOccultist: {
      pickTrait(traitPotentSpirit);

      eng.player->changeMaxHp(-2, false);

      //Player starts with a scroll of Darkbolt, and one other random scroll
      //Both are identified
      Item* scroll = eng.itemFactory->spawnItem(item_scrollOfDarkbolt);
      dynamic_cast<Scroll*>(scroll)->identify(true);
      eng.player->getInv().putItemInGeneral(scroll);
      while(true) {
        scroll = eng.itemFactory->spawnRandomScrollOrPotion(true, false);

        SpellId id = scroll->getData().spellCastFromScroll;
        Spell* const spell = eng.spellHandler->getSpellFromId(id);
        const bool IS_AVAIL = spell->isAvailForPlayer();
        delete spell;

        if(IS_AVAIL && id != spell_darkbolt) {
          dynamic_cast<Scroll*>(scroll)->identify(true);
          eng.player->getInv().putItemInGeneral(scroll);
          break;
        }
      }

      //Potions
      const int NR_POTIONS = 2;
      for(int i = 0; i < NR_POTIONS; i++) {
        Item* const potion =
          eng.itemFactory->spawnRandomScrollOrPotion(false, true);
        dynamic_cast<Potion*>(potion)->identify(true);
        eng.player->getInv().putItemInGeneral(potion);
      }

    } break;

    case bgRogue: {
      eng.playerSpellsHandler->learnSpellIfNotKnown(spell_cloudMinds);
      pickTrait(traitObservant);
      pickTrait(traitStealthy);
    } break;

    case bgSoldier: {
      pickTrait(traitAdeptMeleeFighter);
      pickTrait(traitAdeptMarksman);
      pickTrait(traitTough);
    } break;

    case endOfBgs: {} break;
  }
}

void PlayerBonHandler::pickTrait(const TraitId id) {
  assert(id != endOfTraits);

  traitsPicked_.push_back(id);

  switch(id) {
    case traitTough: {
      eng.player->changeMaxHp(2, false);
    } break;

    case traitRugged: {
      eng.player->changeMaxHp(2, false);
    } break;

    case traitPotentSpirit: {
      eng.player->changeMaxSpi(2, false);
    } break;

    case traitStrongSpirit: {
      eng.player->changeMaxSpi(2, false);
    } break;

    case traitMightySpirit: {
      eng.player->changeMaxSpi(2, false);
    } break;

    case traitSelfAware: {
      eng.player->getPropHandler().tryApplyProp(
        new PropRConfusion(eng, propTurnsIndefinite), true, true, true, false);
    } break;

    case traitFearless: {
      eng.player->getPropHandler().tryApplyProp(
        new PropRFear(eng, propTurnsIndefinite), true, true, true, false);
    } break;

    default: {} break;
  }
}

void PlayerBonHandler::getAllPickedTraitsTitlesLine(string& strRef) const {
  strRef = "";

  for(TraitId t : traitsPicked_) {
    string title = ""; getTraitTitle(t, title);
    strRef += (strRef.empty() ? "" : ", ") + title;
  }
}
