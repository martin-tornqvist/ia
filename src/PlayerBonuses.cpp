#include "PlayerBonuses.h"

#include <algorithm>

#include "Engine.h"
#include "TextFormatting.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ItemFactory.h"
#include "Inventory.h"

using namespace std;

PlayerBonHandler::PlayerBonHandler(Engine& engine) :
  bg_(endOfBgs), eng(engine) {

  for(int i = 0; i < endOfTraits; i++) {
    traitsPicked_[i] = false;
  }
}

void PlayerBonHandler::getTraitTitle(
  const Trait_t id, string& strRef) const {

  strRef = "";

  switch(id) {
    case traitAdeptMeleeCombatant:  strRef = "Adept Melee Combatant"; break;
    case traitAlchemist:            strRef = "Alchemist";             break;
    case traitCoolHeaded:           strRef = "Cool-headed";           break;
    case traitDexterous:            strRef = "Dexterous";             break;
    case traitFearless:             strRef = "Fearless";              break;
    case traitHealer:               strRef = "Healer";                break;
    case traitMarksman:             strRef = "Marksman";              break;
    case traitObservant:            strRef = "Observant";             break;
    case traitRapidRecoverer:       strRef = "Rapid Recoverer";       break;
    case traitSelfAware:            strRef = "Self-aware";            break;
    case traitSpirited:             strRef = "Spirited";              break;
    case traitStealthy:             strRef = "Stealthy";              break;
    case traitStrongBacked:         strRef = "Strong-backed";         break;
    case traitTough:                strRef = "Tough";                 break;
    case traitTreasureHunter:       strRef = "Treasure Hunter";       break;
    case endOfTraits: break;
  }
}

void PlayerBonHandler::getTraitDescr(
  const Trait_t id, string& strRef) const {

  strRef = "";

  switch(id) {
    case traitAdeptMeleeCombatant: {
      strRef  = "+15% hit chance with melee weapons";
    } break;

    case traitAlchemist: {
      strRef  = "All potions are identified, you start with five potions";
    } break;

    case traitCoolHeaded: {
      strRef  = "+20% shock resistance";

    } break;

    case traitDexterous: {
      strRef  = "+25% chance to evade attacks and traps, ";
      strRef += "every fourth move is a free action";
    } break;

    case traitFearless: {
      strRef  = "You cannot become terrified, +5% shock resistance";
    } break;

    case traitHealer: {
      strRef  = "Healing takes half the normal time and resources";
    } break;

    case traitMarksman: {
      strRef  = "+15% hit chance with firearms and thrown weapons, ";
      strRef += "standing still gives another 10% hit chance, ";
      strRef += "you occasionally reload instantly";
    } break;

    case traitObservant: {
      strRef  = "You occasionally spot clues about hidden passages, ";
      strRef += "your attentiveness is higher when examining objects ";
      strRef += "(e.g. when searching a chest for traps), and you cannot ";
      strRef += "be backstabbed - hidden monsters gets no melee attack ";
      strRef += "bonus against you, and their attacks can be dodged";
    } break;

    case traitRapidRecoverer: {
      strRef  = "Greatly increased hit point and spirit point ";
      strRef += "regeneration rate";
    } break;

    case traitSelfAware: {
      strRef  = "You cannot become confused, the number of remaining turns ";
      strRef += "for status effects are displayed";

    } break;

    case traitSpirited: {
      strRef  = "+3 spirit points";
    } break;

    case traitStealthy: {
      strRef  = "You are more likely to avoid detection";
    } break;

    case traitStrongBacked: {
      strRef  = "+30% carry weight limit";
    } break;

    case traitTough: {
      strRef  = "+3 hit points, +10% carry weight limit, better results ";
      strRef += "for object interactions requiring strength (such as ";
      strRef += "bashing doors, or moving the lid from a stone coffin)";
    } break;

    case traitTreasureHunter: {
      strRef  = "You tend to find more items";
    } break;

    case endOfTraits: {} break;
  }
}

void PlayerBonHandler::getTraitPrereqs(const Trait_t id,
                                       vector<Trait_t>& traitsToFill) {
  traitsToFill.resize(0);
  //TODO Add prereqs
}

void PlayerBonHandler::getAllPickableTraits(vector<Trait_t>& traitsRef) {
  traitsRef.resize(0);

  for(int i = 0; i < endOfTraits; i++) {
    if(traitsPicked_[i] == false) {

      vector<Trait_t> traitPrereqs;
      getTraitPrereqs(Trait_t(i), traitPrereqs);

      bool isPickable = true;
      for(unsigned int ii = 0; ii < traitPrereqs.size(); ii++) {
        if(traitsPicked_[traitPrereqs.at(ii)] == false) {
          isPickable = false;
          break;
        }
      }

      //TODO Check player background prereq

      if(isPickable) {
        traitsRef.push_back(Trait_t(i));
      }
    }
  }
}

void PlayerBonHandler::pickTrait(const Trait_t id) {
  traitsPicked_[id] = true;

  switch(id) {
    case traitTough: {
      eng.player->changeMaxHp(3, false);
    } break;

    case traitSpirited: {
      eng.player->changeMaxSpi(3, false);
    } break;

    case traitSelfAware: {
      eng.player->getPropHandler()->tryApplyProp(
        new PropRConfusion(eng, propTurnsIndefinite), true, true, true);
    } break;

    case traitFearless: {
      eng.player->getPropHandler()->tryApplyProp(
        new PropRFear(eng, propTurnsIndefinite), true, true, true);
    } break;

//    case traitOccultist: {
//      const int NR_SCROLLS_TO_START_WITH = 2;
//      for(int i = 0; i < NR_SCROLLS_TO_START_WITH; i++) {
//        Item* const item =
//          eng.itemFactory->spawnRandomScrollOrPotion(true, false);
//
//        Spell_t spellId = item->getData().spellCastFromScroll;
//        Spell* const spell = eng.spellHandler->getSpellFromId(spellId);
//        const bool IS_SPELL_LEARNABLE = spell->isLearnableForPlayer();
//        delete spell;
//
//        if(IS_SPELL_LEARNABLE && spellId != spell_pestilence) {
//          Scroll* const scroll = dynamic_cast<Scroll*>(item);
//          scroll->identify(true);
//          eng.player->getInventory()->putItemInGeneral(scroll);
//
//          if(item->nrItems == 2) {
//            item->nrItems = 1;
//            i--;
//          }
//        } else {
//          delete item;
//          i--;
//        }
//      }
//    } break;

    case traitAlchemist: {
      for(int i = 1; i < endOfItemIds; i++) {
        ItemData* const d = eng.itemDataHandler->dataList[i];
        if(d->isPotion) {
          Item* const item      = eng.itemFactory->spawnItem(d->id);
          Potion* const potion  = dynamic_cast<Potion*>(item);
          potion->identify(true);
          delete potion;
        }
      }
      const int NR_POTIONS_TO_START_WITH = 5;
      for(int i = 0; i < NR_POTIONS_TO_START_WITH; i++) {
        eng.player->getInventory()->putItemInGeneral(
          eng.itemFactory->spawnRandomScrollOrPotion(false, true));
      }
    } break;

    default: {} break;
  }
}

void PlayerBonHandler::getAllPickedTraitsTitlesList(
  vector<string>& titlesRef) {

  titlesRef.resize(0);
  for(int i = 0; i < endOfTraits; i++) {
    if(traitsPicked_[i]) {
      string title = "";
      getTraitTitle(Trait_t(i), title);
      titlesRef.push_back(title);
    }
  }
}

void PlayerBonHandler::getAllPickedTraitsTitlesLine(string& strRef) {
  strRef = "";
  for(int i = 0; i < endOfTraits; i++) {
    if(traitsPicked_[i]) {
      string title = "";
      getTraitTitle(Trait_t(i), title);
      title = "\"" + title + "\"";
      strRef += strRef.empty() ? title : (", " + title);
    }
  }
}
