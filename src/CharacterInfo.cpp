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

  const string offset = " ";
  const SDL_Color clrHeader = clrWhiteHigh;
  const SDL_Color clrText = clrWhite;
  const SDL_Color clrTextDark = clrGray;

  const AbilityValues& abilities = eng->player->getDef()->abilityVals;

  lines.push_back(StringAndClr("Combat skills", clrHeader));
  const int BASE_MELEE =
    min(100, abilities.getVal(ability_accuracyMelee, true, *(eng->player)));
  const int BASE_RANGED =
    min(100, abilities.getVal(ability_accuracyRanged, true, *(eng->player)));
  const int BASE_DODGE_ATTACKS =
    min(100, abilities.getVal(ability_dodgeAttack, true, *(eng->player)));
  Weapon* kick =
    dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerKick));
  string kickStr =
    eng->itemData->getItemInterfaceRef(*kick, false, primaryAttackMode_melee);
  delete kick;
  for(unsigned int i = 0; i < kickStr.length(); i++) {
    if(kickStr.at(0) == ' ') {
      kickStr.erase(kickStr.begin());
    } else {
      break;
    }
  }
  Weapon* punch =
    dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerPunch));
  string punchStr =
    eng->itemData->getItemInterfaceRef(*punch, false, primaryAttackMode_melee);
  delete punch;
  for(unsigned int i = 0; i < punchStr.length(); i++) {
    if(punchStr.at(0) == ' ') {
      punchStr.erase(punchStr.begin());
    } else {
      break;
    }
  }
  lines.push_back(StringAndClr(
                    offset + "Melee           : " +
                    intToString(BASE_MELEE) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Ranged          : " +
                    intToString(BASE_RANGED) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Dodging         : " +
                    intToString(BASE_DODGE_ATTACKS) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Kicking         : " +
                    kickStr, clrText));
  lines.push_back(StringAndClr(
                    offset + "Punching        : " +
                    punchStr, clrText));
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Resistance to status effects", clrHeader));
  const int STATUS_RES_PHYSICAL =
    min(100, abilities.getVal(ability_resistStatusBody, true, *(eng->player)));
  const int STATUS_RES_MENTAL =
    min(100, abilities.getVal(ability_resistStatusMind, true, *(eng->player)));
  lines.push_back(StringAndClr(
                    offset + "Physical        : " +
                    intToString(STATUS_RES_PHYSICAL) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Mental          : " +
                    intToString(STATUS_RES_MENTAL) + "%", clrText));
  lines.push_back(StringAndClr(" ", clrText));


  const int SHOCK_RESISTANCE = eng->player->getShockResistance();
  lines.push_back(StringAndClr(
                    "Shock resistance : " +
                    intToString(SHOCK_RESISTANCE) + "%", clrHeader));
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Mythos knowledge effects", clrHeader));
  const int MTH = eng->player->getMth();
  if(MTH == 0) {
    lines.push_back(StringAndClr(offset + "No effects", clrText));
  } else {
    lines.push_back(StringAndClr(
                      offset + "+" + intToString(MTH) +
                      "% damage against all bizarre, " +
                      "alien, mystical and shocking creatures",
                      clrText));
    const int CASTING_BON = eng->player->getMth() / CAST_FROM_MEMORY_MTH_BON_DIV;
    lines.push_back(StringAndClr(
                      offset + "+" + intToString(CASTING_BON) +
                      "% chance to cast spells",
                      clrText));
  }
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Mental conditions", clrHeader));
  const int NR_LINES_BEFORE_MENTAL = lines.size();
  if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
    lines.push_back(StringAndClr(offset + "Phobia of enclosed spaces", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_dog])
    lines.push_back(StringAndClr(offset + "Phobia of dogs", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_rat])
    lines.push_back(StringAndClr(offset + "Phobia of rats", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_undead])
    lines.push_back(StringAndClr(offset + "Phobia of the dead", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_openPlace])
    lines.push_back(StringAndClr(offset + "Phobia of open places", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_spider])
    lines.push_back(StringAndClr(offset + "Phobia of spiders", clrText));
  if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
    lines.push_back(StringAndClr(offset + "Phobia of deep places", clrText));

  if(eng->player->insanityObsessions[insanityObsession_masochism])
    lines.push_back(StringAndClr(offset + "Masochistic obsession", clrText));
  if(eng->player->insanityObsessions[insanityObsession_sadism])
    lines.push_back(StringAndClr(offset + "Sadistic obsession", clrText));
  const int NR_LINES_AFTER_MENTAL = lines.size();

  if(NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL) {
    lines.push_back(StringAndClr(offset + "No special symptoms", clrText));
  }
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Abilities gained", clrHeader));
  string abilitiesLine = "";
  bool isAnyBonusPicked = false;
  for(unsigned int i = 0; i < endOfPlayerBonuses; i++) {
    const PlayerBonuses_t bonus = static_cast<PlayerBonuses_t>(i);
    if(eng->playerBonusHandler->isBonusPicked(bonus)) {
      isAnyBonusPicked = true;
      const string currentTitle =
        eng->playerBonusHandler->getBonusTitle(bonus);
      lines.push_back(StringAndClr(offset + currentTitle, clrText));
      const string currentDescr =
        eng->playerBonusHandler->getBonusDescription(bonus);
      lines.push_back(StringAndClr(offset + currentDescr, clrTextDark));
    }
  }
  if(isAnyBonusPicked == false) {
    lines.push_back(StringAndClr(offset + "None", clrText));
  }
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Potion knowledge", clrHeader));
  vector<StringAndClr> potionList;
  vector<StringAndClr> manuscriptList;
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isQuaffable && (d->isTried || d->isIdentified)) {
      Item* item = eng->itemFactory->spawnItem(d->id);
      potionList.push_back(
        StringAndClr(
          offset + eng->itemData->getItemRef(*item, itemRef_plain),
          d->color));
      delete item;
    } else {
      if(d->isReadable && (d->isTried || d->isIdentified)) {
        Item* item = eng->itemFactory->spawnItem(d->id);
        manuscriptList.push_back(
          StringAndClr(
            offset + eng->itemData->getItemRef(*item, itemRef_plain),
            item->getInterfaceClr()));
        delete item;
      }
    }
  }

  if(potionList.size() == 0) {
    lines.push_back(StringAndClr(offset + "No known potions", clrText));
  } else {
    eng->basicUtils->lexicographicalSortStringAndClrVector(potionList);
    for(unsigned int i = 0; i < potionList.size(); i++) {
      lines.push_back(potionList.at(i));
    }
  }
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Manuscript knowledge", clrHeader));
  if(manuscriptList.size() == 0) {
    lines.push_back(StringAndClr(offset + "No known manuscripts", clrText));
  } else {
    eng->basicUtils->lexicographicalSortStringAndClrVector(manuscriptList);
    for(unsigned int i = 0; i < manuscriptList.size(); i++) {
      lines.push_back(manuscriptList.at(i));
    }
  }
  lines.push_back(StringAndClr(" ", clrText));

}

void CharacterInfo::drawInterface() {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, 2);
  eng->renderer->drawText(
    decorationLine, renderArea_screen, 1, 1, clrWhite);
  eng->renderer->drawText(
    " Displaying character info ", renderArea_screen, 3, 1, clrWhite);
  eng->renderer->drawText(
    decorationLine, renderArea_characterLines, 1, 1, clrWhite);
  eng->renderer->drawText(
    " 2/8, down/up to navigate | space/esc to exit ",
    renderArea_characterLines, 3, 1, clrWhite);
}

void CharacterInfo::run() {
  makeLines();

  eng->renderer->coverRenderArea(renderArea_screen);

  StringAndClr currentLine;

  int topElement = 0;
  int btmElement = min(topElement + MAP_Y_CELLS - 1, int(lines.size()) - 1);

  drawInterface();

  int yCell = 2;
  for(int i = topElement; i <= btmElement; i++) {
    eng->renderer->drawText(
      lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).clr);
    yCell++;
  }

  eng->renderer->updateScreen();

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement = min(
                     topElement + int(MAP_Y_CELLS / 5),
                     int(lines.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_Y_CELLS - 1, int(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(
          lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).clr);
        yCell++;
      }
      eng->renderer->updateScreen();
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement = min(
                     topElement - int(MAP_Y_CELLS / 5),
                     int(lines.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_Y_CELLS - 1, int(lines.size()) - 1);
      eng->renderer->coverArea(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
      drawInterface();
      yCell = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(
          lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).clr);
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
