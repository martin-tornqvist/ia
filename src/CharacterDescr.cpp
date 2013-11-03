#include "CharacterDescr.h"

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

void CharacterDescr::makeLines() {
  lines.resize(0);

  const string offset = " ";
  const SDL_Color clrHeader = clrWhiteHigh;
  const SDL_Color clrText = clrWhite;
  const SDL_Color clrTextDark = clrGray;

  const AbilityValues& abilities = eng->player->getData()->abilityVals;

  lines.push_back(StringAndClr("Combat skills", clrHeader));
  const int BASE_MELEE =
    min(100, abilities.getVal(ability_accuracyMelee, true, *(eng->player)));
  const int BASE_RANGED =
    min(100, abilities.getVal(ability_accuracyRanged, true, *(eng->player)));
  const int BASE_DODGE_ATTACKS =
    min(100, abilities.getVal(ability_dodgeAttack, true, *(eng->player)));
  Weapon* kick =
    dynamic_cast<Weapon*>(eng->itemFactory->spawnItem(item_playerKick));
  string kickStr = eng->itemDataHandler->getItemInterfaceRef(
                     *kick, false, primaryAttackMode_melee);
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
  string punchStr = eng->itemDataHandler->getItemInterfaceRef(
                      *punch, false, primaryAttackMode_melee);
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
                    toString(BASE_MELEE) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Ranged          : " +
                    toString(BASE_RANGED) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Dodging         : " +
                    toString(BASE_DODGE_ATTACKS) + "%", clrText));
  lines.push_back(StringAndClr(
                    offset + "Kicking         : " +
                    kickStr, clrText));
  lines.push_back(StringAndClr(
                    offset + "Punching        : " +
                    punchStr, clrText));
  lines.push_back(StringAndClr(" ", clrText));


  const int SHOCK_RESISTANCE = eng->player->getShockResistance();
  lines.push_back(StringAndClr(
                    "Shock resistance : " +
                    toString(SHOCK_RESISTANCE) + "%", clrHeader));
  lines.push_back(StringAndClr(" ", clrText));


  lines.push_back(StringAndClr("Mythos knowledge effects", clrHeader));
  const int MTH = eng->player->getMth();
  if(MTH == 0) {
    lines.push_back(StringAndClr(offset + "No effects", clrText));
  } else {
    lines.push_back(StringAndClr(
                      offset + "+" + toString(MTH) +
                      "% damage against all bizarre, " +
                      "alien, mystical and shocking creatures",
                      clrText));
    const int CASTING_BON =
      eng->player->getMth() / CAST_FROM_MEMORY_MTH_BON_DIV;
    if(CASTING_BON > 0) {
      lines.push_back(StringAndClr(
                        offset + "-" + toString(CASTING_BON) +
                        " spirit required to cast spells",
                        clrText));
    }
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
  for(unsigned int i = 0; i < endOfPlayerBons; i++) {
    const PlayerBon_t bon = static_cast<PlayerBon_t>(i);
    if(eng->playerBonHandler->isBonPicked(bon)) {
      isAnyBonusPicked = true;
      const string currentTitle = eng->playerBonHandler->getBonTitle(bon);
      lines.push_back(StringAndClr(offset + currentTitle, clrText));
      const string curDescr = eng->playerBonHandler->getBonEffectDescr(bon);
      lines.push_back(StringAndClr(offset + curDescr, clrTextDark));
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
    const ItemData* const d = eng->itemDataHandler->dataList[i];
    if(d->isPotion && (d->isTried || d->isIdentified)) {
      Item* item = eng->itemFactory->spawnItem(d->id);
      potionList.push_back(
        StringAndClr(
          offset + eng->itemDataHandler->getItemRef(*item, itemRef_plain),
          d->color));
      delete item;
    } else {
      if(d->isScroll && (d->isTried || d->isIdentified)) {
        Item* item = eng->itemFactory->spawnItem(d->id);
        manuscriptList.push_back(
          StringAndClr(
            offset + eng->itemDataHandler->getItemRef(*item, itemRef_plain),
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

void CharacterDescr::drawInterface() {
  const string decorationLine(MAP_X_CELLS - 2, '-');

  eng->renderer->coverArea(panel_screen, Pos(0, 1), Pos(MAP_X_CELLS, 2));
  eng->renderer->drawText(
    decorationLine, panel_screen, Pos(1, 1), clrWhite);
  eng->renderer->drawText(
    " Displaying character description ", panel_screen, Pos(3, 1), clrWhite);
  eng->renderer->drawText(
    decorationLine, panel_character, Pos(1, 1), clrWhite);
  eng->renderer->drawText(
    " 2/8, down/up to navigate | space/esc to exit ",
    panel_character, Pos(3, 1), clrWhite);
}

void CharacterDescr::run() {
  makeLines();

  eng->renderer->coverPanel(panel_screen);

  StringAndClr currentLine;

  int topElement = 0;
  int btmElement = min(topElement + MAP_Y_CELLS - 1, int(lines.size()) - 1);

  drawInterface();

  Pos pos(1, 2);
  for(int i = topElement; i <= btmElement; i++) {
    eng->renderer->drawText(
      lines.at(i).str , panel_screen, pos, lines.at(i).clr);
    pos.y++;
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
      eng->renderer->coverArea(panel_screen, Pos(0, 2),
                               Pos(MAP_X_CELLS, MAP_Y_CELLS));
      drawInterface();
      pos.y = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(
          lines.at(i).str , panel_screen, pos, lines.at(i).clr);
        pos.y++;
      }
      eng->renderer->updateScreen();
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement = min(
                     topElement - int(MAP_Y_CELLS / 5),
                     int(lines.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_Y_CELLS - 1, int(lines.size()) - 1);
      eng->renderer->coverArea(panel_screen, Pos(0, 2),
                               Pos(MAP_X_CELLS, MAP_Y_CELLS));
      drawInterface();
      pos.y = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng->renderer->drawText(
          lines.at(i).str , panel_screen, pos, lines.at(i).clr);
        pos.y++;
      }
      eng->renderer->updateScreen();
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }

  }
  eng->renderer->coverPanel(panel_screen);
  eng->renderer->drawMapAndInterface();
}
