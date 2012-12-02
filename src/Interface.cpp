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

using namespace std;

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

  //Health
  xPos += 1 + str.length();
  const string hp = intToString(eng->player->getHp());
  const string hpMax = intToString(eng->player->getHpMax());
  eng->renderer->drawText("HP:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 3;
  str = hp + "/" + hpMax;
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrRedLight);

  //Sanity
  const int INS_SHORT = eng->player->insanityShort + eng->player->insanityShortTemp;
  const int INS_LONG = eng->player->insanityLong;
  xPos += 1 + str.length();
  eng->renderer->drawText("INS:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 4;
  str = intToString(INS_LONG) + "%";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrMagenta);
  xPos += str.length();
  const sf::Color shortSanClr = INS_SHORT < 50 ? clrGreenLight : INS_SHORT < 75 ? clrYellow : clrMagenta;
  str = "(" + intToString(INS_SHORT) + "%)";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, shortSanClr);
  xPos += 1 + str.length();

  //Encumbrance
  eng->renderer->drawText("ENC:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 4;
  const int TOTAL_W = eng->player->getInventory()->getTotalItemWeight();
  const int MAX_W = PLAYER_CARRY_WEIGHT_STANDARD;
  const int ENC = static_cast<int>((static_cast<double>(TOTAL_W) / static_cast<double>(MAX_W)) * 100.0);
  str = intToString(ENC) + "%";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, ENC >= 100 ? clrRedLight : clrWhite);
  xPos += 1 + str.length();

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
    xPos += 1 + str.length();
  }

  //Dungeon level
  xPos = CHARACTER_LINE_X0;
  yPos += 1;
  eng->renderer->drawText("DLVL:", renderArea_characterLines, xPos, yPos, clrGray);
  xPos += 5;
  const int DLVL = eng->map->getDungeonLevel();
  str = DLVL >= 0 ? intToString(DLVL) : "?";
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);

  DungeonMaster* const dm = eng->dungeonMaster;

  //Level and xp
  xPos += str.length() + 1;
  str = "LVL:" + intToString(dm->getLevel());
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrGreenLight);
  xPos += str.length() + 1;
  str = "NXT:";
  str += dm->getLevel() >= PLAYER_MAX_LEVEL ? "-" : intToString(dm->getXpToNextLvl() - dm->getXp());
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
    str = dynamic_cast<const Armor*>(armor)->getArmorDataLine();
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

  const string statusLine = eng->player->getStatusEffectsHandler()->getStatusLine();
  eng->renderer->drawText(statusLine, renderArea_characterLines, xPos, yPos, clrWhite);

  // Turn number
  str = "TRN:" + intToString(eng->gameTime->getTurn());
  xPos = MAP_X_CELLS - str.length() - 1;
  eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
}

