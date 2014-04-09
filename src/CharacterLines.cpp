#include "CharacterLines.h"

#include "Engine.h"

#include "CommonTypes.h"
#include "GameTime.h"
#include "Colors.h"
#include "ItemWeapon.h"
#include "ItemArmor.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "MapGen.h"
#include "PlayerBon.h"
#include "Inventory.h"

using namespace std;

namespace CharacterLines {

void drawLocationInfo(Engine& eng) {
  Player* const player = eng.player;

  if(player->getPropHandler().allowSee()) {
    string str = "";

    const Pos& playerPos = player->pos;

    const int DLVL = eng.map->getDlvl();
    if(DLVL > 0 && DLVL < FIRST_CAVERN_LEVEL) {
      const vector<Room*>& rooms = eng.map->rooms;
      for(unsigned int i = 0; i < rooms.size(); i++) {
        const Room* const room = rooms.at(i);
        const Pos& x0y0 = room->getX0Y0();
        const Pos& x1y1 = room->getX1Y1();
        if(Utils::isPosInside(playerPos, Rect(x0y0, x1y1))) {
          const string& roomDescr = room->roomDescr;
          if(roomDescr.empty() == false) {
            str += room->roomDescr + " ";
          }
        }
      }
    }

    const bool IS_DRK_AT_PLAYER =
      eng.map->cells[playerPos.x][playerPos.y].isDark;
    const bool IS_LGT_AT_PLAYER =
      eng.map->cells[playerPos.x][playerPos.y].isLight;
    if(IS_DRK_AT_PLAYER) {
      str += IS_LGT_AT_PLAYER ?
             "The darkness is lit up. " :
             "It is dark here. ";
    }

    if(str.empty() == false) {
      Renderer::drawText(str, Panel::charLines, Pos(0, -1), clrWhite);
    }
  }
}

void drawInfoLines(Engine& eng) {
  Player* const player = eng.player;

  Renderer::coverPanel(Panel::charLines);

  const int CHARACTER_LINE_X0 = 0;
  const int CHARACTER_LINE_Y0 = 0;

  Pos pos(CHARACTER_LINE_X0, CHARACTER_LINE_Y0);

  const SDL_Color clrGenDrk = clrNosfTealDrk;
  const SDL_Color clrGenLgt = clrNosfTealLgt;
  const SDL_Color clrGenMed = clrNosfTeal;

  //Name
//  str = player->getNameA();
//  Renderer::drawText(str, Panel::charLines, pos, clrRedLgt);
//  pos.x += str.length() + 1;

  //Health
  const string hp = toStr(player->getHp());
  const string hpMax = toStr(player->getHpMax(true));
  Renderer::drawText("HP:", Panel::charLines, pos, clrGenDrk);
  pos.x += 3;
  string str = hp + "/" + hpMax;
  Renderer::drawText(str, Panel::charLines, pos, clrRedLgt);
  pos.x += str.length() + 1;

  //Spirit
  const string spi    = toStr(player->getSpi());
  const string spiMax = toStr(player->getSpiMax());
  Renderer::drawText("SPI:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  str = spi + "/" + spiMax;
  Renderer::drawText(str, Panel::charLines, pos, clrBlueLgt);
  pos.x += str.length() + 1;

  //Sanity
  const int SHOCK = player->getShockTotal();
  const int INS = player->getInsanity();
  Renderer::drawText("INS:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  const SDL_Color shortSanClr =
    SHOCK < 50  ? clrGreenLgt :
    SHOCK < 75  ? clrYellow   :
    SHOCK < 100 ? clrMagenta  : clrRedLgt;
  str = toStr(SHOCK) + "%/";
  Renderer::drawText(str, Panel::charLines, pos, shortSanClr);
  pos.x += str.length();
  str = toStr(INS) + "%";
  Renderer::drawText(str, Panel::charLines, pos, clrMagenta);
  pos.x += str.length() + 1;

  //Armor
  Renderer::drawText("ARM:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  const Item* const armor =
    player->getInv().getItemInSlot(slot_armorBody);
  if(armor == NULL) {
    Renderer::drawText("N/A", Panel::charLines, pos, clrGenLgt);
    pos.x += 4;
  } else {
    str = dynamic_cast<const Armor*>(armor)->getArmorDataLine(false);
    Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
    pos.x += str.length() + 1;
  }

  //Wielded weapon
  pos.x += 6;
  const int X_POS_MISSILE = pos.x;

  Item* itemWielded = player->getInv().getItemInSlot(slot_wielded);
  if(itemWielded == NULL) {
    Renderer::drawText(
      "Unarmed", Panel::charLines, pos, clrGenMed);
  } else {
    const SDL_Color itemClr = itemWielded->getClr();
    if(Config::isTilesMode()) {
      Renderer::drawTile(
        itemWielded->getTile(), Panel::charLines, pos, itemClr);
    } else {
      Renderer::drawGlyph(
        itemWielded->getGlyph(), Panel::charLines, pos, itemClr);
    }
    pos.x += 2;

    str = eng.itemDataHandler->getItemInterfaceRef(*itemWielded, false);
    Renderer::drawText(str, Panel::charLines, pos, clrGenMed);
    pos.x += str.length() + 1;
  }

  pos.x = CHARACTER_LINE_X0;
  pos.y += 1;

  // Level and xp
  DungeonMaster* const dm = eng.dungeonMaster;
  Renderer::drawText("LVL:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  str = toStr(dm->getCLvl());
  Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
  pos.x += str.length() + 1;
  Renderer::drawText("NXT:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  str = dm->getCLvl() >= PLAYER_MAX_CLVL ? "-" :
        toStr(dm->getXpToNextLvl());
  Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
  pos.x += str.length() + 1;

  //Dungeon level
  Renderer::drawText("DLVL:", Panel::charLines, pos, clrGenDrk);
  pos.x += 5;
  const int DLVL = eng.map->getDlvl();
  str = DLVL >= 0 ? toStr(DLVL) : "?";
  Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
  pos.x += str.length() + 1;

  //Encumbrance
  Renderer::drawText("ENC:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  const int ENC = player->getEncPercent();
  str = toStr(ENC) + "%";
  const SDL_Color encClr = ENC < 100 ? clrGreenLgt :
                           ENC < ENC_IMMOBILE_LVL ? clrYellow : clrRedLgt;
  Renderer::drawText(str, Panel::charLines, pos, encClr);
  pos.x += str.length() + 1;

  //Missile weapon
  pos.x = X_POS_MISSILE;

  Item* const itemMissiles = player->getInv().getItemInSlot(slot_missiles);
  if(itemMissiles == NULL) {
    Renderer::drawText("No missile weapon", Panel::charLines, pos, clrGenMed);
  } else {
    const SDL_Color itemClr = itemMissiles->getClr();
    if(Config::isTilesMode()) {
      Renderer::drawTile(
        itemMissiles->getTile(), Panel::charLines, pos, itemClr);
    } else {
      Renderer::drawGlyph(
        itemMissiles->getGlyph(), Panel::charLines, pos, itemClr);
    }
    pos.x += 2;

    str = eng.itemDataHandler->getItemInterfaceRef(
            *itemMissiles, false, PrimaryAttackMode::missile);
    Renderer::drawText(str, Panel::charLines, pos, clrGenMed);
    pos.x += str.length() + 1;
  }

  pos.y += 1;
  pos.x = CHARACTER_LINE_X0;

  vector<StrAndClr> propsLine;
  player->getPropHandler().getPropsInterfaceLine(propsLine);
  const int NR_PROPS = propsLine.size();
  for(int i = 0; i < NR_PROPS; i++) {
    const StrAndClr& curPropLabel = propsLine.at(i);
    Renderer::drawText(
      curPropLabel.str, Panel::charLines, pos, curPropLabel.clr);
    pos.x += curPropLabel.str.length() + 1;
  }

//  const bool IS_SELF_AWARE =
//    playerBonHandler->hasTrait(Trait::traitselfAware);
//  //TODO This should be collected from applied and inventory by the Property handler
//  const vector<Prop*>& appliedProps =
//    player->getPropHandler().appliedProps_;
//  for(unsigned int i = 0; i < appliedProps.size(); i++) {
//    Prop* const prop = appliedProps.at(i);
//    const PropAlignment alignment = prop->getAlignment();
//    const SDL_Color statusColor =
//      alignment == propAlignmentGood ? clrMsgGood :
//      alignment == propAlignmentBad  ? clrMsgBad  : clrWhite;
//    string propText = prop->getNameShort();
//    if(IS_SELF_AWARE && prop->allowDisplayTurns()) {
//      // +1 to offset that the turn is also active on turn 0
//      propText += "(" + toStr(prop->turnsLeft_ + 1) + ")";
//    }
//    Renderer::drawText(propText, Panel::charLines, pos, statusColor);
//    pos.x += propText.length() + 1;
//  }

// Turn number
  str = "T:" + toStr(eng.gameTime->getTurn());
  pos.x = MAP_W - str.length() - 1;
  Renderer::drawText(str, Panel::charLines, pos, clrGenMed);
}

} //CharacterLines
