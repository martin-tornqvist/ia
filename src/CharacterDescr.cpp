#include "CharacterDescr.h"

#include <algorithm>

#include "Engine.h"
#include "PlayerBonuses.h"
#include "ActorPlayer.h"
#include "Renderer.h"
#include "TextFormatting.h"
#include "Input.h"
#include "ItemPotion.h"
#include "ItemScroll.h"
#include "ItemFactory.h"
#include "Item.h"
#include "ItemWeapon.h"

void CharacterDescr::makeLines() {
  lines.resize(0);

  const string offset = "   ";
  const SDL_Color clrHeading  = clrWhiteHigh;
  const SDL_Color clrText     = clrWhite;
  const SDL_Color clrTextDark = clrGray;

  const AbilityValues& abilities = eng.player->getData().abilityVals;

  lines.push_back(StrAndClr("Combat skills", clrHeading));
  const int BASE_MELEE =
    min(100, abilities.getVal(ability_accuracyMelee, true, *(eng.player)));
  const int BASE_RANGED =
    min(100, abilities.getVal(ability_accuracyRanged, true, *(eng.player)));
  const int BASE_DODGE_ATTACKS =
    min(100, abilities.getVal(ability_dodgeAttack, true, *(eng.player)));
  Weapon* kick =
    dynamic_cast<Weapon*>(eng.itemFactory->spawnItem(item_playerKick));
  string kickStr = eng.itemDataHandler->getItemInterfaceRef(
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
    dynamic_cast<Weapon*>(eng.itemFactory->spawnItem(item_playerPunch));
  string punchStr = eng.itemDataHandler->getItemInterfaceRef(
                      *punch, false, primaryAttackMode_melee);
  delete punch;
  for(unsigned int i = 0; i < punchStr.length(); i++) {
    if(punchStr.at(0) == ' ') {
      punchStr.erase(punchStr.begin());
    } else {
      break;
    }
  }
  lines.push_back(StrAndClr(
                    offset + "Melee    : " +
                    toString(BASE_MELEE) + "%", clrText));
  lines.push_back(StrAndClr(
                    offset + "Ranged   : " +
                    toString(BASE_RANGED) + "%", clrText));
  lines.push_back(StrAndClr(
                    offset + "Dodging  : " +
                    toString(BASE_DODGE_ATTACKS) + "%", clrText));
  lines.push_back(StrAndClr(
                    offset + "Kicking  : " +
                    kickStr, clrText));
  lines.push_back(StrAndClr(
                    offset + "Punching : " +
                    punchStr, clrText));
  lines.push_back(StrAndClr(" ", clrText));

  lines.push_back(StrAndClr("Shock resistances", clrHeading));
  vector<string> shockResTitles;
  int lenOfLongestShockResTitle = 0;
  for(int i = 0; i < endOfShockSrc; i++) {
    string shockResTitle = "";
    getShockResSrcTitle(ShockSrc_t(i), shockResTitle);
    shockResTitles.push_back(shockResTitle);
    const int CUR_LEN = shockResTitle.size();
    if(CUR_LEN > lenOfLongestShockResTitle) {
      lenOfLongestShockResTitle = CUR_LEN;
    }
  }
  for(int i = 0; i < endOfShockSrc; i++) {
    string& curTitle = shockResTitles.at(i);
    curTitle.resize(lenOfLongestShockResTitle, ' ');
    const int SHOCK_RESISTANCE = eng.player->getShockResistance(ShockSrc_t(i));
    string str = offset + curTitle + " : " + toString(SHOCK_RESISTANCE) + "%";
    lines.push_back(StrAndClr(str, clrText));
  }
  lines.push_back(StrAndClr(" ", clrText));


  lines.push_back(StrAndClr("Mythos knowledge effects", clrHeading));
  const int MTH = eng.player->getMth();
  if(MTH == 0) {
    lines.push_back(StrAndClr(offset + "No effects", clrText));
  } else {
    lines.push_back(StrAndClr(
                      offset + "+" + toString(MTH) +
                      "% damage against all strange creatures", clrText));
    const bool IS_SPI_BON = eng.player->getMth() >= MTH_LVL_SPELLS_SPI_BON;
    if(IS_SPI_BON) {
      lines.push_back(StrAndClr(
                        offset + "-1 Spirit cost for all spells", clrText));
    }
  }
  lines.push_back(StrAndClr(" ", clrText));


  lines.push_back(StrAndClr("Mental conditions", clrHeading));
  const int NR_LINES_BEFORE_MENTAL = lines.size();
  if(eng.player->insanityPhobias[insanityPhobia_closedPlace])
    lines.push_back(StrAndClr(offset + "Phobia of enclosed spaces", clrText));
  if(eng.player->insanityPhobias[insanityPhobia_dog])
    lines.push_back(StrAndClr(offset + "Phobia of dogs", clrText));
  if(eng.player->insanityPhobias[insanityPhobia_rat])
    lines.push_back(StrAndClr(offset + "Phobia of rats", clrText));
  if(eng.player->insanityPhobias[insanityPhobia_undead])
    lines.push_back(StrAndClr(offset + "Phobia of the dead", clrText));
  if(eng.player->insanityPhobias[insanityPhobia_openPlace])
    lines.push_back(StrAndClr(offset + "Phobia of open places", clrText));
  if(eng.player->insanityPhobias[insanityPhobia_spider])
    lines.push_back(StrAndClr(offset + "Phobia of spiders", clrText));
  if(eng.player->insanityPhobias[insanityPhobia_deepPlaces])
    lines.push_back(StrAndClr(offset + "Phobia of deep places", clrText));

  if(eng.player->insanityObsessions[insanityObsession_masochism])
    lines.push_back(StrAndClr(offset + "Masochistic obsession", clrText));
  if(eng.player->insanityObsessions[insanityObsession_sadism])
    lines.push_back(StrAndClr(offset + "Sadistic obsession", clrText));
  const int NR_LINES_AFTER_MENTAL = lines.size();

  if(NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL) {
    lines.push_back(StrAndClr(offset + "No special symptoms", clrText));
  }
  lines.push_back(StrAndClr(" ", clrText));

  lines.push_back(StrAndClr("Potion knowledge", clrHeading));
  vector<StrAndClr> potionList;
  vector<StrAndClr> manuscriptList;
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    const ItemData* const d = eng.itemDataHandler->dataList[i];
    if(d->isPotion && (d->isTried || d->isIdentified)) {
      Item* item = eng.itemFactory->spawnItem(d->id);
      potionList.push_back(
        StrAndClr(
          offset + eng.itemDataHandler->getItemRef(*item, itemRef_plain),
          d->color));
      delete item;
    } else {
      if(d->isScroll && (d->isTried || d->isIdentified)) {
        Item* item = eng.itemFactory->spawnItem(d->id);
        manuscriptList.push_back(
          StrAndClr(
            offset + eng.itemDataHandler->getItemRef(*item, itemRef_plain),
            item->getInterfaceClr()));
        delete item;
      }
    }
  }

  auto strAndClrSort = [](const StrAndClr & e1, const StrAndClr & e2) {
    return e1.str < e2.str;
  };

  if(potionList.empty()) {
    lines.push_back(StrAndClr(offset + "No known potions", clrText));
  } else {
    sort(potionList.begin(), potionList.end(), strAndClrSort);
    for(StrAndClr & e : potionList) {lines.push_back(e);}
  }
  lines.push_back(StrAndClr(" ", clrText));


  lines.push_back(StrAndClr("Manuscript knowledge", clrHeading));
  if(manuscriptList.size() == 0) {
    lines.push_back(StrAndClr(offset + "No known manuscripts", clrText));
  } else {
    sort(manuscriptList.begin(), manuscriptList.end(), strAndClrSort);
    for(StrAndClr & e : manuscriptList) {lines.push_back(e);}
  }
  lines.push_back(StrAndClr(" ", clrText));

  lines.push_back(StrAndClr("Traits gained", clrHeading));
  string abilitiesLine = "";
  vector<Trait_t>& traits = eng.playerBonHandler->traitsPicked_;
  if(traits.empty()) {
    lines.push_back(StrAndClr(offset + "None", clrText));
    lines.push_back(StrAndClr(" ", clrText));
  } else {
    const int MAX_W_DESCR = (MAP_W * 2) / 3;

    for(Trait_t trait : traits) {
      string title = "";
      eng.playerBonHandler->getTraitTitle(trait, title);
      lines.push_back(StrAndClr(offset + title, clrText));
      string descr = "";
      eng.playerBonHandler->getTraitDescr(trait, descr);
      vector<string> descrLines;
      TextFormatting::lineToLines(descr, MAX_W_DESCR, descrLines);
      for(string & descrLine : descrLines) {
        lines.push_back(StrAndClr(offset + descrLine, clrTextDark));
      }
      lines.push_back(StrAndClr(" ", clrText));
    }
  }
}

void CharacterDescr::getShockResSrcTitle(
  const ShockSrc_t shockSrc, string& strRef) {

  strRef = "";
  switch(shockSrc) {
    case shockSrc_time:           strRef = "Time";                    break;
    case shockSrc_castIntrSpell:  strRef = "Casting learned spells";  break;
    case shockSrc_seeMonster:     strRef = "Seeing monsters";         break;
    case shockSrc_useStrangeItem: strRef = "Using strange items";     break;
    case shockSrc_misc:           strRef = "Other";                   break;
    case endOfShockSrc: {} break;
  }
}

void CharacterDescr::drawInterface() {
  const string decorationLine(MAP_W - 2, '-');

  eng.renderer->coverArea(panel_screen, Pos(0, 1), Pos(MAP_W, 2));
  eng.renderer->drawText(
    decorationLine, panel_screen, Pos(1, 0), clrWhite);
  eng.renderer->drawText(
    " Displaying character description ", panel_screen, Pos(3, 0), clrWhite);
  eng.renderer->drawText(
    decorationLine, panel_char, Pos(1, 2), clrWhite);
  eng.renderer->drawText(
    " 2/8, down/up to navigate | space/esc to exit ",
    panel_char, Pos(3, 2), clrWhite);
}

void CharacterDescr::run() {
  makeLines();

  eng.renderer->coverPanel(panel_screen);

  StrAndClr currentLine;

  int topElement = 0;
  int btmElement = min(topElement + MAP_H - 1, int(lines.size()) - 1);

  drawInterface();

  Pos pos(1, 2);
  for(int i = topElement; i <= btmElement; i++) {
    eng.renderer->drawText(
      lines.at(i).str , panel_screen, pos, lines.at(i).clr);
    pos.y++;
  }

  eng.renderer->updateScreen();

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng.input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement = min(
                     topElement + int(MAP_H / 5),
                     int(lines.size()) - int(MAP_H));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_H - 1, int(lines.size()) - 1);
      eng.renderer->coverArea(panel_screen, Pos(0, 2),
                              Pos(MAP_W, MAP_H));
      drawInterface();
      pos.y = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng.renderer->drawText(
          lines.at(i).str , panel_screen, pos, lines.at(i).clr);
        pos.y++;
      }
      eng.renderer->updateScreen();
    } else if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement = min(
                     topElement - int(MAP_H / 5),
                     int(lines.size()) - int(MAP_H));
      topElement = max(0, topElement);
      btmElement = min(topElement + MAP_H - 1, int(lines.size()) - 1);
      eng.renderer->coverArea(panel_screen, Pos(0, 2),
                              Pos(MAP_W, MAP_H));
      drawInterface();
      pos.y = 2;
      for(int i = topElement; i <= btmElement; i++) {
        eng.renderer->drawText(
          lines.at(i).str , panel_screen, pos, lines.at(i).clr);
        pos.y++;
      }
      eng.renderer->updateScreen();
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }

  }
  eng.renderer->coverPanel(panel_screen);
  eng.renderer->drawMapAndInterface();
}
