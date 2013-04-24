#include "Interface.h"

#include "Engine.h"

#include "ConstTypes.h"

#include "Colors.h"
#include "ItemWeapon.h"
#include "ItemArmor.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "MapBuildBSP.h"
#include "PlayerBonuses.h"

using namespace std;

void Interface::drawLocationInfo() {
  if(eng->player->getStatusEffectsHandler()->allowSee()) {
    string str = "";

    const coord& playerPos = eng->player->pos;

    const int DLVL = eng->map->getDungeonLevel();
    if(DLVL > 0 && DLVL < FIRST_CAVERN_LEVEL) {
      const vector<Room*>& roomList = eng->roomThemeMaker->roomList;
      for(unsigned int i = 0; i < roomList.size(); i++) {
        const Room* const room = roomList.at(i);
        const coord& x0y0 = room->getX0Y0();
        const coord& x1y1 = room->getX1Y1();
        if(eng->mapTests->isCellInside(playerPos, x0y0, x1y1)) {
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
      str += IS_LIGHT_AT_PLAYER ? "The darkness is lit up. " : "It is dark here. ";
    }

    if(str != "") {
      eng->renderer->drawText(str, renderArea_characterLines, 1, -1, clrWhite);
    }
  }
}

void Interface::drawInfoLines() {
  eng->renderer->coverRenderArea(renderArea_characterLines);

  const int CHARACTER_LINE_X0 = 1;
  const int CHARACTER_LINE_Y0 = 0;

  int xPos = CHARACTER_LINE_X0;
  int yPos = CHARACTER_LINE_Y0;
  string str = "";

  //Name
  str = eng->player->getNameA();
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrRedLight);
  xPos += str.length() + 1;

  //Health
  const string hp = intToString(eng->player->getHp());
  const string hpMax = intToString(eng->player->getHpMax(true));
  eng->renderer->drawText("HP:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 3;
  str = hp + "/" + hpMax;
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrRedLight);
  xPos += str.length() + 1;

  //Sanity
  const int SHOCK = eng->player->getShockTotal();
  const int INS = eng->player->getInsanity();
  eng->renderer->drawText("INS:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 4;
  const SDL_Color shortSanClr = SHOCK < 50 ? clrGreenLight : SHOCK < 75 ? clrYellow : clrMagenta;
  str = intToString(SHOCK) + "%/";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, shortSanClr);
  xPos += str.length();
  str = intToString(INS) + "%";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrMagenta);
  xPos += str.length() + 1;

  const int MTH = eng->player->getMth();
  eng->renderer->drawText("MTH:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 4;
  str = intToString(MTH) + "%";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrMagenta);
  xPos += str.length() + 1;

  //Encumbrance
  eng->renderer->drawText("ENC:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 4;
  const int TOTAL_W = eng->player->getInventory()->getTotalItemWeight();
  const int MAX_W = eng->player->getCarryWeightLimit();
  const int ENC = static_cast<int>((static_cast<double>(TOTAL_W) / static_cast<double>(MAX_W)) * 100.0);
  str = intToString(ENC) + "%";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, ENC >= 100 ? clrRedLight : clrWhite);
  xPos += str.length() + 1;

  //Wielded weapon
  xPos += 4;
  //Store x position, because missile wpn info will be directly beaneath wielded wpn info
  const int X_POS_MISSILE = xPos;

  Item* itemWielded = eng->player->getInventory()->getItemInSlot(slot_wielded);
  if(itemWielded == NULL) {
    eng->renderer->drawText("Unarmed", renderArea_characterLines, xPos, yPos, clrWhite);
  } else {
    str = eng->itemData->getItemInterfaceRef(itemWielded, false);
    eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
    xPos += str.length() + 1;
  }

  //Dungeon level
  xPos = CHARACTER_LINE_X0;
  yPos += 1;
  eng->renderer->drawText("DLVL:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 5;
  const int DLVL = eng->map->getDungeonLevel();
  str = DLVL >= 0 ? intToString(DLVL) : "?";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
  xPos += str.length() + 1;

  DungeonMaster* const dm = eng->dungeonMaster;

  //Level and xp
  str = "LVL:" + intToString(dm->getLevel());
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrGreenLight);
  xPos += str.length() + 1;
  str = "NXT:";
  str += dm->getLevel() >= PLAYER_CLVL_MAX ? "-" : intToString(dm->getXpToNextLvl() - dm->getXp());
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrGreenLight);
  xPos += str.length() + 1;

  //Armor
  eng->renderer->drawText("ARM:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 4;
  const Item* const armor = eng->player->getInventory()->getItemInSlot(slot_armorBody);
  if(armor == NULL) {
    eng->renderer->drawText("N/A", renderArea_characterLines, xPos, yPos, clrWhite);
    xPos += 4;
  } else {
    str = dynamic_cast<const Armor*>(armor)->getArmorDataLine(false);
    eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
    xPos += str.length() + 1;
  }

  //Missile weapon
  xPos = X_POS_MISSILE;

  Item* const itemMissiles = eng->player->getInventory()->getItemInSlot(slot_missiles);
  if(itemMissiles == NULL) {
    eng->renderer->drawText("No missile weapon", renderArea_characterLines, xPos, yPos, clrWhite);
  } else {
    str = eng->itemData->getItemInterfaceRef(itemMissiles, false);
    eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
    xPos += str.length() + 1;
  }

  yPos += 1;
  xPos = CHARACTER_LINE_X0;

  const bool IS_SELF_AWARE = eng->playerBonusHandler->isBonusPicked(playerBonus_selfAware);
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
    eng->renderer->drawText(statusText, renderArea_characterLines, xPos, yPos, statusColor);
    xPos += statusText.length() + 1;
  }

  // Turn number
  str = "TRN:" + intToString(eng->gameTime->getTurn());
  xPos = MAP_X_CELLS - str.length() - 1;
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
}

