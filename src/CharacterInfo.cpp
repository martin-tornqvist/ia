#include "CharacterInfo.h"

#include "Engine.h"
#include "PlayerBonuses.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Input.h"
#include "ItemPotion.h"
#include "ItemScroll.h"
#include "ItemFactory.h"
#include "Item.h"
#include "ItemWeapon.h"

void CharacterInfo::makeLines() {
  lines.resize(0);

  const string offsetSpaces = " ";
  const SDL_Color clrHeader = clrCyanLight;
  const SDL_Color clrText = clrRedLight;
  const SDL_Color clrTextDark = clrRed;

  lines.push_back(StringAndColor("COMBAT SKILLS", clrHeader));
  const int BASE_MELEE = min(100, eng->player->getDef()->abilityVals.getVal(ability_accuracyMelee, true, *(eng->player)));
  const int BASE_RANGED = min(100, eng->player->getDef()->abilityVals.getVal(ability_accuracyRanged, true, *(eng->player)));
  const int BASE_DODGE_ATTACKS = min(100, eng->player->getDef()->abilityVals.getVal(ability_dodgeAttack, true, *(eng->player)));
  Weapon* kick = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerKick));
  string kickStr = eng->itemData->getItemInterfaceRef(kick, false, primaryAttackMode_melee);
  delete kick;
  for(unsigned int i = 0; i < kickStr.length(); i++) {
    if(kickStr.at(0) == ' ') {
      kickStr.erase(kickStr.begin());
    } else {
      break;
    }
  }
  Weapon* punch = dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerPunch));
  string punchStr = eng->itemData->getItemInterfaceRef(punch, false, primaryAttackMode_melee);
  delete punch;
  for(unsigned int i = 0; i < punchStr.length(); i++) {
    if(punchStr.at(0) == ' ') {
      punchStr.erase(punchStr.begin());
    } else {
      break;
    }
  }
  lines.push_back(StringAndColor(offsetSpaces + "Melee           : " + intToString(BASE_MELEE) + "%", clrText));
  lines.push_back(StringAndColor(offsetSpaces + "Ranged          : " + intToString(BASE_RANGED) + "%", clrText));
  lines.push_back(StringAndColor(offsetSpaces + "Dodging         : " + intToString(BASE_DODGE_ATTACKS) + "%", clrText));
  lines.push_back(StringAndColor(offsetSpaces + "Kicking         : " + kickStr, clrText));
  lines.push_back(StringAndColor(offsetSpaces + "Punching        : " + punchStr, clrText));
  lines.push_back(StringAndColor(" ", clrText));

  lines.push_back(StringAndColor("RESISTANCE TO STATUS EFFECTS", clrHeader));
  const int STATUS_RES_PHYSICAL = min(100, eng->player->getDef()->abilityVals.getVal(ability_resistStatusBody, true, *(eng->player)));
  const int STATUS_RES_MENTAL = min(100, eng->player->getDef()->abilityVals.getVal(ability_resistStatusMind, true, *(eng->player)));
  lines.push_back(StringAndColor(offsetSpaces + "Physical        : " + intToString(STATUS_RES_PHYSICAL) + "%", clrText));
  lines.push_back(StringAndColor(offsetSpaces + "Mental          : " + intToString(STATUS_RES_MENTAL) + "%", clrText));
  lines.push_back(StringAndColor(" ", clrText));

  lines.push_back(StringAndColor("MYTHOS KNOWLEDGE EFFECTS", clrHeader));
  const int MTH = eng->player->getMth();
  if(MTH == 0) {
    lines.push_back(StringAndColor(offsetSpaces + "No effects", clrText));
  } else {
    lines.push_back(StringAndColor(
                      offsetSpaces + "+" + intToString(MTH) +
                      "% damage against all bizarre, alien, mystical and shocking creatures",
                      clrText));
    const int CASTING_BON = eng->player->getMth() / CAST_FROM_MEMORY_MTH_BON_DIV;
    lines.push_back(StringAndColor(
                      offsetSpaces + "+" + intToString(CASTING_BON) +
                      "% chance to cast spells",
                      clrText));
  }
//  const int INS_FROM_MTH = eng->player->getSanityPenaltyFromMythosKnowledge();
//  lines.push_back(StringAndColor(offsetSpaces + "Sanity penalty  : " + intToString(INS_FROM_MTH) + "%", clrText));
  lines.push_back(StringAndColor(" ", clrText));

  const int SHOCK_RESISTANCE = eng->player->getShockResistance();
  lines.push_back(StringAndColor("SHOCK RESISTANCE : " + intToString(SHOCK_RESISTANCE) + "%", clrHeader));
  lines.push_back(StringAndColor(" ", clrText));

  lines.push_back(StringAndColor("MENTAL CONDITIONS", clrHeader));
  const int NR_LINES_BEFORE_MENTAL = lines.size();
  if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of enclosed spaces", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_dog])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of dogs", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_rat])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of rats", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_undead])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of the dead", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_openPlace])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of open places", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_spider])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of spiders", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
    lines.push_back(StringAndColor(offsetSpaces + "Phobia of deep places", clrText));

  if(eng->player->insanityObsessions[insanityObsession_masochism])
    lines.push_back(StringAndColor(offsetSpaces + "Masochistic obsession", clrText));
  if(eng->player->insanityObsessions[insanityObsession_sadism])
    lines.push_back(StringAndColor(offsetSpaces + "Sadistic obsession", clrText));
  const int NR_LINES_AFTER_MENTAL = lines.size();

  if(NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL) {
    lines.push_back(StringAndColor(offsetSpaces + "No special symptoms", clrText));
  }
  lines.push_back(StringAndColor(" ", clrText));

  lines.push_back(StringAndColor("ABILITIES GAINED", clrHeader));
  string abilitiesLine = "";
  bool isAnyBonusPicked = false;
  for(unsigned int i = 0; i < endOfPlayerBonuses; i++) {
    const PlayerBonuses_t bonus = static_cast<PlayerBonuses_t>(i);
    if(eng->playerBonusHandler->isBonusPicked(bonus)) {
      isAnyBonusPicked = true;
      const string currentTitle = eng->playerBonusHandler->getBonusTitle(bonus);
      lines.push_back(StringAndColor(offsetSpaces + currentTitle, clrText));
      const string currentDescr = eng->playerBonusHandler->getBonusDescription(bonus);
      lines.push_back(StringAndColor(offsetSpaces + currentDescr, clrTextDark));
    }
  }
  if(isAnyBonusPicked == false) {
    lines.push_back(StringAndColor(offsetSpaces + "None", clrText));
  }
  lines.push_back(StringAndColor(" ", clrText));

  lines.push_back(StringAndColor("POTION KNOWLEDGE", clrHeader));
  vector<StringAndColor> potionList;
  vector<StringAndColor> manuscriptList;
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isQuaffable && (d->isTried || d->isIdentified)) {
      Item* item = eng->itemFactory->spawnItem(d->id);
      potionList.push_back(StringAndColor(offsetSpaces + eng->itemData->getItemRef(item, itemRef_plain), d->color));
      delete item;
    } else {
      if(d->isReadable && (d->isTried || d->isIdentified)) {
        Item* item = eng->itemFactory->spawnItem(d->id);
        manuscriptList.push_back(StringAndColor(offsetSpaces + eng->itemData->getItemRef(item, itemRef_plain), item->getInterfaceClr()));
        delete item;
      }
    }
  }

  if(potionList.size() == 0) {
    lines.push_back(StringAndColor(offsetSpaces + "No known potions", clrText));
  } else {
    eng->basicUtils->lexicographicalSortStringAndColorVector(potionList);
    for(unsigned int i = 0; i < potionList.size(); i++) {
      lines.push_back(potionList.at(i));
    }
  }
  lines.push_back(StringAndColor(" ", clrText));

  lines.push_back(StringAndColor("MANUSCRIPT KNOWLEDGE", clrHeader));
  if(manuscriptList.size() == 0) {
    lines.push_back(StringAndColor(offsetSpaces + "No known manuscripts", clrText));
  } else {
    eng->basicUtils->lexicographicalSortStringAndColorVector(manuscriptList);
    for(unsigned int i = 0; i < manuscriptList.size(); i++) {
      lines.push_back(manuscriptList.at(i));
    }
  }
  lines.push_back(StringAndColor(" ", clrText));

}

void CharacterInfo::drawInterface() {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverArea(renderArea_screen, coord(0, 1), MAP_X_CELLS, 2);
  eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);

  eng->renderer->drawText(" Displaying character info ", renderArea_screen, 3, 1, clrWhite);

  eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);

  eng->renderer->drawText(" 2/8, down/up to navigate | space/esc to exit ", renderArea_characterLines, 3, 1, clrWhite);
}

void CharacterInfo::run() {
  makeLines();

  eng->renderer->coverRenderArea(renderArea_screen);

  StringAndColor currentLine;

  int topElement = 0;
  int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

  drawInterface();

  int yCell = 2;
  for(int i = topElement; i <= btmElement; i++) {
    eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
    yCell++;
  }

  eng->renderer->updateScreen();

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, coord(0, 2), MAP_X_CELLS, MAP_Y_CELLS);
      drawInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
        yCell++;
      }
      eng->renderer->updateScreen();
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size()) - static_cast<int>(MAP_Y_CELLS)));
      btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, coord(0, 2), MAP_X_CELLS, MAP_Y_CELLS);
      drawInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
        yCell++;
      }
      eng->renderer->updateScreen();
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }

  }
  eng->renderer->coverRenderArea(renderArea_screen);
  eng->renderer->drawMapAndInterface();
}
