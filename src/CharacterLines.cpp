#include "CharacterLines.h"

#include "Engine.h"

#include "CommonTypes.h"

#include "Colors.h"
#include "ItemWeapon.h"
#include "ItemArmor.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "MapBuildBSP.h"
#include "PlayerBonuses.h"
#include "Inventory.h"

using namespace std;

void CharacterLines::drawLocationInfo() {
  if(eng->player->getStatusEffectsHandler()->allowSee()) {
    string str = "";

    const Pos& playerPos = eng->player->pos;

    const int DLVL = eng->map->getDungeonLevel();
    if(DLVL > 0 && DLVL < FIRST_CAVERN_LEVEL) {
      const vector<Room*>& roomList = eng->roomThemeMaker->roomList;
      for(unsigned int i = 0; i < roomList.size(); i++) {
        const Room* const room = roomList.at(i);
        const Pos& x0y0 = room->getX0Y0();
        const Pos& x1y1 = room->getX1Y1();
        if(eng->mapTests->isCellInside(playerPos, Rect(x0y0, x1y1))) {
          const string& roomDescr = room->roomDescr;
          if(roomDescr != "") {
            str += room->roomDescr + " ";
          }
        }
      }
    }

    const bool IS_DARK_AT_PLAYER = eng->map->darkness[playerPos.x][playerPos.y];
    const bool IS_LIGHT_AT_PLAYER = eng->map->light[playerPos.x][playerPos.y];
    if(IS_DARK_AT_PLAYER) {
      str += IS_LIGHT_AT_PLAYER ?
             "The darkness is lit up. " :
             "It is dark here. ";
    }

    if(str != "") {
      eng->renderer->drawText(str, panel_character, Pos(1, -1), clrWhite);
    }
  }
}

void CharacterLines::drawInfoLines() {
  eng->renderer->coverPanel(panel_character);

  const int CHARACTER_LINE_X0 = 1;
  const int CHARACTER_LINE_Y0 = 0;

  Pos pos(CHARACTER_LINE_X0, CHARACTER_LINE_Y0);
  string str = "";

  const SDL_Color clrGenDrk = clrNosferatuTealDrk;
  const SDL_Color clrGenLgt = clrNosferatuTealLgt;
  const SDL_Color clrGenMed = clrNosferatuTeal;

  //Name
//  str = eng->player->getNameA();
//  eng->renderer->drawText(str, panel_character, pos, clrRedLgt);
//  pos.x += str.length() + 1;

  //Health
  const string hp = intToString(eng->player->getHp());
  const string hpMax = intToString(eng->player->getHpMax(true));
  eng->renderer->drawText("HP:", panel_character, pos, clrGenDrk);
  pos.x += 3;
  str = hp + "/" + hpMax;
  eng->renderer->drawText(str, panel_character, pos, clrRedLgt);
  pos.x += str.length() + 1;

  //Sanity
  const int SHOCK = eng->player->getShockTotal();
  const int INS = eng->player->getInsanity();
  eng->renderer->drawText("INS:", panel_character, pos, clrGenDrk);
  pos.x += 4;
  const SDL_Color shortSanClr = SHOCK < 50 ? clrGreenLgt : SHOCK < 75 ? clrYellow : clrMagenta;
  str = intToString(SHOCK) + "%/";
  eng->renderer->drawText(str, panel_character, pos, shortSanClr);
  pos.x += str.length();
  str = intToString(INS) + "%";
  eng->renderer->drawText(str, panel_character, pos, clrMagenta);
  pos.x += str.length() + 1;

  const int MTH = eng->player->getMth();
  eng->renderer->drawText("MTH:", panel_character, pos, clrGenDrk);
  pos.x += 4;
  str = intToString(MTH) + "%";
  eng->renderer->drawText(str, panel_character, pos, clrGenLgt);
  pos.x += str.length() + 1;

  //Encumbrance
  eng->renderer->drawText("ENC:", panel_character, pos, clrGenDrk);
  pos.x += 4;
  const int TOTAL_W = eng->player->getInventory()->getTotalItemWeight();
  const int MAX_W = eng->player->getCarryWeightLimit();
  const int ENC = int((double(TOTAL_W) / double(MAX_W)) * 100.0);
  str = intToString(ENC) + "%";
  eng->renderer->drawText(str, panel_character, pos, ENC >= 100 ? clrRedLgt : clrGenLgt);
  pos.x += str.length() + 1;

  //Wielded weapon
  pos.x += 4;
  //Store x position, because missile wpn info will be directly beaneath wielded wpn info
  const int X_POS_MISSILE = pos.x;

  Item* itemWielded = eng->player->getInventory()->getItemInSlot(slot_wielded);
  if(itemWielded == NULL) {
    eng->renderer->drawText(
      "Unarmed", panel_character, pos, clrGenMed);
  } else {
    str = eng->itemData->getItemInterfaceRef(*itemWielded, false);
    eng->renderer->drawText(str, panel_character, pos, clrGenMed);
    pos.x += str.length() + 1;
  }

  //Dungeon level
  pos.x = CHARACTER_LINE_X0;
  pos.y += 1;
  eng->renderer->drawText("DLVL:", panel_character, pos, clrGenDrk);
  pos.x += 5;
  const int DLVL = eng->map->getDungeonLevel();
  str = DLVL >= 0 ? intToString(DLVL) : "?";
  eng->renderer->drawText(str, panel_character, pos, clrGenLgt);
  pos.x += str.length() + 1;

  DungeonMaster* const dm = eng->dungeonMaster;

  //Level and xp
  str = "LVL:" + intToString(dm->getLevel());
  eng->renderer->drawText(str, panel_character, pos, clrGenDrk);
  pos.x += str.length() + 1;
  str = "NXT:";
  str += dm->getLevel() >= PLAYER_CLVL_MAX ? "-" : intToString(dm->getXpToNextLvl() - dm->getXp());
  eng->renderer->drawText(str, panel_character, pos, clrGenLgt);
  pos.x += str.length() + 1;

  //Armor
  eng->renderer->drawText("ARM:", panel_character, pos, clrGenDrk);
  pos.x += 4;
  const Item* const armor = eng->player->getInventory()->getItemInSlot(slot_armorBody);
  if(armor == NULL) {
    eng->renderer->drawText("N/A", panel_character, pos, clrGenLgt);
    pos.x += 4;
  } else {
    str = dynamic_cast<const Armor*>(armor)->getArmorDataLine(false);
    eng->renderer->drawText(str, panel_character, pos, clrGenLgt);
    pos.x += str.length() + 1;
  }

  //Missile weapon
  pos.x = X_POS_MISSILE;

  Item* const itemMissiles = eng->player->getInventory()->getItemInSlot(slot_missiles);
  if(itemMissiles == NULL) {
    eng->renderer->drawText("No missile weapon", panel_character, pos, clrGenMed);
  } else {
    str = eng->itemData->getItemInterfaceRef(*itemMissiles, false);
    eng->renderer->drawText(str, panel_character, pos, clrGenMed);
    pos.x += str.length() + 1;
  }

  pos.y += 1;
  pos.x = CHARACTER_LINE_X0;

  const bool IS_SELF_AWARE = eng->playerBonHandler->isBonPicked(playerBon_selfAware);
  const vector<StatusEffect*>& effects = eng->player->getStatusEffectsHandler()->effects;
  for(unsigned int i = 0; i < effects.size(); i++) {
    StatusEffect* const effect = effects.at(i);
    const SDL_Color statusColor = effect->isConsideredBeneficial() ? clrMessageGood : clrMessageBad;
    string statusText = effect->getInterfaceName();
    if(IS_SELF_AWARE) {
      if(effect->allowDisplayTurnsInInterface()) {
        // +1 to offset that the turn is also active on turn 0
        statusText += "(" + intToString(effect->turnsLeft + 1) + ")";
      }
    }
    eng->renderer->drawText(statusText, panel_character, pos, statusColor);
    pos.x += statusText.length() + 1;
  }

  // Turn number
  str = "TRN:" + intToString(eng->gameTime->getTurn());
  pos.x = MAP_X_CELLS - str.length() - 1;
  eng->renderer->drawText(str, panel_character, pos, clrGenLgt);
}

