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

PlayerBonHandler::PlayerBonHandler(Engine* engine) :
  bg_(endOfBgs), eng(engine) {

  for(int i = 0; i < endOfTraits; i++) {
    traitsPicked_[i] = false;
  }
}

void PlayerBonHandler::getTraitTitle(
  const Trait_t id, string& strToSet) const {

  strToSet = "";

  switch(id) {
    case traitAdeptMeleeCombatant:  strToSet = "Adept Melee Combatant"; break;
    case traitAlchemist:            strToSet = "Alchemist";             break;
    case traitCoolHeaded:           strToSet = "Cool-headed";           break;
    case traitDexterous:            strToSet = "Dexterous";             break;
    case traitFearless:             strToSet = "Fearless";              break;
    case traitHealer:               strToSet = "Healer";                break;
    case traitMarksman:             strToSet = "Marksman";              break;
    case traitObservant:            strToSet = "Observant";             break;
    case traitRapidRecoverer:       strToSet = "Rapid Recoverer";       break;
    case traitSelfAware:            strToSet = "Self-aware";            break;
    case traitSpirited:             strToSet = "Spirited";              break;
    case traitStealthy:             strToSet = "Stealthy";              break;
    case traitStrongBacked:         strToSet = "Strong-backed";         break;
    case traitTough:                strToSet = "Tough";                 break;
    case traitTreasureHunter:       strToSet = "Treasure Hunter";       break;
    case endOfTraits: break;
  }
}

void PlayerBonHandler::getTraitDescr(
  const Trait_t id, string& strToSet) const {

  strToSet = "";

  switch(id) {
    case traitAdeptMeleeCombatant: {
      strToSet  = "+15% hit chance with melee weapons";
    } break;

    case traitAlchemist: {
      strToSet  = "All potions are identified, you start with five potions";
    } break;

    case traitCoolHeaded: {
      strToSet  = "+20% shock resistance";

    } break;

    case traitDexterous: {
      strToSet  = "+25% chance to evade attacks and traps, ";
      strToSet += "every fourth move is a free action";
    } break;

    case traitFearless: {
      strToSet  = "You cannot become terrified, +5% shock resistance";
    } break;

    case traitHealer: {
      strToSet  = "Healing takes half the normal time and resources";
    } break;

    case traitMarksman: {
      strToSet  = "+15% hit chance with firearms and thrown weapons, ";
      strToSet += "standing still gives another 10% hit chance, ";
      strToSet += "you occasionally reload instantly";
    } break;

    case traitObservant: {
      strToSet  = "You occasionally spot clues about hidden passages, ";
      strToSet += "your attentiveness is higher when examining objects ";
      strToSet += "(e.g. when searching a chest for traps), and you cannot ";
      strToSet += "be backstabbed - hidden monsters gets no melee attack ";
      strToSet += "bonus against you, and their attacks can be dodged";
    } break;

    case traitRapidRecoverer: {
      strToSet  = "Greatly increased hit point and spirit point ";
      strToSet += "regeneration rate";
    } break;

    case traitSelfAware: {
      strToSet  = "You cannot become confused, the number of remaining turns ";
      strToSet += "for status effects are displayed";

    } break;

    case traitSpirited: {
      strToSet  = "+3 spirit points";
    } break;

    case traitStealthy: {
      strToSet  = "You are more likely to avoid detection";
    } break;

    case traitStrongBacked: {
      strToSet  = "+30% carry weight limit";
    } break;

    case traitTough: {
      strToSet  = "+3 hit points, +10% carry weight limit, better results ";
      strToSet += "for object interactions requiring strength (such as ";
      strToSet += "bashing doors, or moving the lid from a stone coffin)";
    } break;

    case traitTreasureHunter: {
      strToSet  = "You tend to find more items";
    } break;

    case endOfTraits: {} break;
  }
}

void PlayerBonHandler::getTraitPrereqs(const Trait_t id,
                                       vector<Trait_t>& traitsToFill) {
  traitsToFill.resize(0);
  //TODO Add prereqs
}

void PlayerBonHandler::getAllPickableTraits(vector<Trait_t>& traitsToSet) {
  traitsToSet.resize(0);

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
        traitsToSet.push_back(Trait_t(i));
      }
    }
  }
}

void PlayerBonHandler::pickTrait(const Trait_t id) {
  traitsPicked_[id] = true;

  switch(id) {
    case traitTough: {
      eng->player->changeMaxHp(3, false);
    } break;

    case traitSpirited: {
      eng->player->changeMaxSpi(3, false);
    } break;

    case traitSelfAware: {
      eng->player->getPropHandler()->tryApplyProp(
        new PropRConfusion(eng, propTurnsIndefinite), true, true, true);
    } break;

    case traitFearless: {
      eng->player->getPropHandler()->tryApplyProp(
        new PropRFear(eng, propTurnsIndefinite), true, true, true);
    } break;

//    case traitOccultist: {
//      const int NR_SCROLLS_TO_START_WITH = 2;
//      for(int i = 0; i < NR_SCROLLS_TO_START_WITH; i++) {
//        Item* const item =
//          eng->itemFactory->spawnRandomScrollOrPotion(true, false);
//
//        Spell_t spellId = item->getData().spellCastFromScroll;
//        Spell* const spell = eng->spellHandler->getSpellFromId(spellId);
//        const bool IS_SPELL_LEARNABLE = spell->isLearnableForPlayer();
//        delete spell;
//
//        if(IS_SPELL_LEARNABLE && spellId != spell_pestilence) {
//          Scroll* const scroll = dynamic_cast<Scroll*>(item);
//          scroll->identify(true);
//          eng->player->getInventory()->putItemInGeneral(scroll);
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
        ItemData* const d = eng->itemDataHandler->dataList[i];
        if(d->isPotion) {
          Item* const item      = eng->itemFactory->spawnItem(d->id);
          Potion* const potion  = dynamic_cast<Potion*>(item);
          potion->identify(true);
          delete potion;
        }
      }
      const int NR_POTIONS_TO_START_WITH = 5;
      for(int i = 0; i < NR_POTIONS_TO_START_WITH; i++) {
        eng->player->getInventory()->putItemInGeneral(
          eng->itemFactory->spawnRandomScrollOrPotion(false, true));
      }
    } break;

    default: {} break;
  }
}

void PlayerBonHandler::getAllPickedTraitsTitlesList(
  vector<string>& titlesToSet) {

  titlesToSet.resize(0);
  for(int i = 0; i < endOfTraits; i++) {
    if(traitsPicked_[i]) {
      string title = "";
      getTraitTitle(Trait_t(i), title);
      titlesToSet.push_back(title);
    }
  }
}

void PlayerBonHandler::getAllPickedTraitsTitlesLine(string& strToSet) {
  strToSet = "";
  for(int i = 0; i < endOfTraits; i++) {
    if(traitsPicked_[i]) {
      string title = "";
      getTraitTitle(Trait_t(i), title);
      title = "\"" + title + "\"";
      strToSet += strToSet.empty() ? title : (", " + title);
    }
  }
}
