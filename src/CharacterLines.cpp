#include "CharacterLines.h"

#include "CmnTypes.h"
#include "GameTime.h"
#include "Colors.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "MapGen.h"
#include "PlayerBon.h"
#include "Inventory.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"
#include "TextFormatting.h"
#include "Item.h"

using namespace std;

namespace CharacterLines {

void drawLocationInfo() {
  string str = "";

  if(Map::player->getPropHandler().allowSee()) {
    const Pos& p = Map::player->pos;

//    if(Map::dlvl > 0 && Map::dlvl < FIRST_CAVERN_LVL) {
//      const vector<Room*>& rooms = Map::roomList;
//      for(size_t i = 0; i < rooms.size(); ++i) {
//        const Room* const room = rooms.at(i);
//        const Pos& p0 = room->r_.p0;
//        const Pos& p1 = room->r_.p1;
//        if(Utils::isPosInside(p, Rect(p0, p1))) {
//          const string& roomDescr = room->descr_;
//          if(!roomDescr.empty()) {str += roomDescr + " ";}
//        }
//      }
//    }

    string featureName = "";

    //Describe mob
    const Mob* const mob = Utils::getFirstMobAtPos(p);
    if(mob) {
      featureName = mob->getName(Article::a);
      if(!featureName.empty()) {
        str += TextFormatting::firstToUpper(featureName) + ". ";
      }
    }

    //Describe rigid
    featureName = Map::cells[p.x][p.y].rigid->getName(Article::a);
    if(!featureName.empty()) {
      str += TextFormatting::firstToUpper(featureName) + ". ";
    }

    //Describe item
    Item* const item = Map::cells[p.x][p.y].item;
    if(item) {
      string itemName = item->getName(ItemRefType::a);
      str += TextFormatting::firstToUpper(itemName) + ". ";
    }

    //Light/darkness
    const auto& cell = Map::cells[p.x][p.y];
    if(cell.isDark) {
      str += cell.isLight ? "The darkness is lit up. " : "It is dark here. ";
    }
  }

  if(!str.empty()) {
    str.pop_back(); //Erase trailing space character
    Renderer::drawText(str, Panel::charLines, Pos(0, -1), clrWhite);

    if(int(str.size()) > MAP_W) {
      Renderer::drawText("(...)", Panel::charLines, Pos(MAP_W - 5, -1), clrWhite);
    }
  }
}

void drawInfoLines() {
  Renderer::coverPanel(Panel::charLines);

  const int CHARACTER_LINE_X0 = 0;
  const int CHARACTER_LINE_Y0 = 0;

  Pos pos(CHARACTER_LINE_X0, CHARACTER_LINE_Y0);

  const Clr clrGenDrk = clrNosfTealDrk;
  const Clr clrGenLgt = clrNosfTealLgt;
  const Clr clrGenMed = clrNosfTeal;

  Player& player = *Map::player;

  //Name
//  str = player->getNameA();
//  Renderer::drawText(str, Panel::charLines, pos, clrRedLgt);
//  pos.x += str.length() + 1;

  //Health
  const string hp = toStr(player.getHp());
  const string hpMax = toStr(player.getHpMax(true));
  Renderer::drawText("HP:", Panel::charLines, pos, clrGenDrk);
  pos.x += 3;
  string str = hp + "/" + hpMax;
  Renderer::drawText(str, Panel::charLines, pos, clrRedLgt);
  pos.x += str.length() + 1;

  //Spirit
  const string spi    = toStr(player.getSpi());
  const string spiMax = toStr(player.getSpiMax());
  Renderer::drawText("SPI:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  str = spi + "/" + spiMax;
  Renderer::drawText(str, Panel::charLines, pos, clrBlueLgt);
  pos.x += str.length() + 1;

  //Sanity
  const int SHOCK = player.getShockTotal();
  const int INS = player.getInsanity();
  Renderer::drawText("INS:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  const Clr shortSanClr =
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
  const Item* const armor = player.getInv().getItemInSlot(SlotId::body);
  if(armor) {
    str = static_cast<const Armor*>(armor)->getArmorDataLine(false);
    Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
    pos.x += str.length() + 1;
  } else {
    Renderer::drawText("N/A", Panel::charLines, pos, clrGenLgt);
    pos.x += 4;
  }

  //Wielded weapon
  pos.x += 6;
  const int X_POS_MISSILE = pos.x;

  Item* itemWielded = Map::player->getInv().getItemInSlot(SlotId::wielded);
  if(itemWielded) {
    const Clr itemClr = itemWielded->getClr();
    if(Config::isTilesMode()) {
      Renderer::drawTile(
        itemWielded->getTile(), Panel::charLines, pos, itemClr);
    } else {
      Renderer::drawGlyph(
        itemWielded->getGlyph(), Panel::charLines, pos, itemClr);
    }
    pos.x += 2;

    str = itemWielded->getName(ItemRefType::plain);
    Renderer::drawText(str, Panel::charLines, pos, clrGenMed);
    pos.x += str.length() + 1;
  } else {
    Renderer::drawText("Unarmed", Panel::charLines, pos, clrGenMed);
  }

  pos.x = CHARACTER_LINE_X0;
  pos.y += 1;

  // Level and xp
  Renderer::drawText("LVL:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  str = toStr(DungeonMaster::getCLvl());
  Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
  pos.x += str.length() + 1;
  Renderer::drawText("NXT:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  str = DungeonMaster::getCLvl() >= PLAYER_MAX_CLVL ? "-" :
        toStr(DungeonMaster::getXpToNextLvl());
  Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
  pos.x += str.length() + 1;

  //Dungeon level
  Renderer::drawText("DLVL:", Panel::charLines, pos, clrGenDrk);
  pos.x += 5;
  str = Map::dlvl >= 0 ? toStr(Map::dlvl) : "?";
  Renderer::drawText(str, Panel::charLines, pos, clrGenLgt);
  pos.x += str.length() + 1;

  //Encumbrance
  Renderer::drawText("ENC:", Panel::charLines, pos, clrGenDrk);
  pos.x += 4;
  const int ENC = Map::player->getEncPercent();
  str = toStr(ENC) + "%";
  const Clr encClr = ENC < 100 ? clrGreenLgt :
                           ENC < ENC_IMMOBILE_LVL ? clrYellow : clrRedLgt;
  Renderer::drawText(str, Panel::charLines, pos, encClr);
  pos.x += str.length() + 1;

  //Missile weapon
  pos.x = X_POS_MISSILE;

  auto* const itemMissiles =
    Map::player->getInv().getItemInSlot(SlotId::thrown);

  if(itemMissiles) {
    const Clr itemClr = itemMissiles->getClr();
    if(Config::isTilesMode()) {
      Renderer::drawTile(
        itemMissiles->getTile(), Panel::charLines, pos, itemClr);
    } else {
      Renderer::drawGlyph(
        itemMissiles->getGlyph(), Panel::charLines, pos, itemClr);
    }
    pos.x += 2;

    str = itemMissiles->getName(ItemRefType::plain);
    Renderer::drawText(str, Panel::charLines, pos, clrGenMed);
    pos.x += str.length() + 1;
  } else {
    Renderer::drawText("No missile weapon", Panel::charLines, pos, clrGenMed);
  }

  pos.y += 1;
  pos.x = CHARACTER_LINE_X0;

  vector<StrAndClr> propsLine;
  Map::player->getPropHandler().getPropsInterfaceLine(propsLine);
  const int NR_PROPS = propsLine.size();
  for(int i = 0; i < NR_PROPS; ++i) {
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
//  for(unsigned int i = 0; i < appliedProps.size(); ++i) {
//    Prop* const prop = appliedProps.at(i);
//    const PropAlignment alignment = prop->getAlignment();
//    const Clr statusColor =
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
  str = "T:" + toStr(GameTime::getTurn());
  pos.x = MAP_W - str.length() - 1;
  Renderer::drawText(str, Panel::charLines, pos, clrGenMed);
}

} //CharacterLines
