#include "CharacterLines.h"

#include "CmnTypes.h"
#include "GameTime.h"
#include "Colors.h"
#include "Render.h"
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

namespace CharacterLines
{

void drawLocationInfo()
{
  string str = "";

  if(Map::player->getPropHandler().allowSee())
  {
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
    if(mob)
    {
      featureName = mob->getName(Article::a);
      if(!featureName.empty())
      {
        str += TextFormatting::firstToUpper(featureName) + ". ";
      }
    }

    //Describe rigid
    featureName = Map::cells[p.x][p.y].rigid->getName(Article::a);
    if(!featureName.empty())
    {
      str += TextFormatting::firstToUpper(featureName) + ". ";
    }

    //Light/darkness
    const auto& cell = Map::cells[p.x][p.y];
    if(cell.isDark)
    {
      str += cell.isLit ? "The darkness is lit up. " : "It is dark here. ";
    }
  }

  if(!str.empty())
  {
    str.pop_back(); //Erase trailing space character
    Render::drawText(str, Panel::charLines, Pos(0, -1), clrWhite);

    if(int(str.size()) > MAP_W)
    {
      Render::drawText("(...)", Panel::charLines, Pos(MAP_W - 5, -1), clrWhite);
    }
  }
}

void drawInfoLines()
{
  Render::coverPanel(Panel::charLines);

  const int CHARACTER_LINE_X0 = 0;
  const int CHARACTER_LINE_Y0 = 0;

  Pos pos(CHARACTER_LINE_X0, CHARACTER_LINE_Y0);

  Player& player = *Map::player;

  //Name
//  str = player->getNameA();
//  Render::drawText(str, Panel::charLines, pos, clrRedLgt);
//  pos.x += str.length() + 1;

  //Health
  const string hp = toStr(player.getHp());
  const string hpMax = toStr(player.getHpMax(true));
  Render::drawText("HP:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 3;
  string str = hp + "/" + hpMax;
  Render::drawText(str, Panel::charLines, pos, clrRedLgt);
  pos.x += str.length() + 1;

  //Spirit
  const string spi    = toStr(player.getSpi());
  const string spiMax = toStr(player.getSpiMax());
  Render::drawText("SPI:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 4;
  str = spi + "/" + spiMax;
  Render::drawText(str, Panel::charLines, pos, clrBlueLgt);
  pos.x += str.length() + 1;

  //Sanity
  const int SHOCK = player.getShockTotal();
  const int INS = player.getInsanity();
  Render::drawText("INS:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 4;
  const Clr shortSanClr =
    SHOCK < 50  ? clrGreenLgt :
    SHOCK < 75  ? clrYellow   :
    SHOCK < 100 ? clrMagenta  : clrRedLgt;
  str = toStr(SHOCK) + "%/";
  Render::drawText(str, Panel::charLines, pos, shortSanClr);
  pos.x += str.length();
  str = toStr(INS) + "%";
  Render::drawText(str, Panel::charLines, pos, clrMagenta);
  pos.x += str.length() + 1;

  //Armor
  Render::drawText("ARM:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 4;
  const Item* const armor = player.getInv().getItemInSlot(SlotId::body);
  if(armor)
  {
    str = static_cast<const Armor*>(armor)->getArmorDataLine(false);
    Render::drawText(str, Panel::charLines, pos, clrWhite);
    pos.x += str.length() + 1;
  }
  else
  {
    Render::drawText("N/A", Panel::charLines, pos, clrWhite);
    pos.x += 4;
  }

  //Wielded weapon
  pos.x += 6;
  const int X_POS_MISSILE = pos.x;

  Item* itemWielded = Map::player->getInv().getItemInSlot(SlotId::wielded);
  if(itemWielded)
  {
    const Clr itemClr = itemWielded->getClr();
    if(Config::isTilesMode())
    {
      Render::drawTile(
        itemWielded->getTile(), Panel::charLines, pos, itemClr);
    }
    else
    {
      Render::drawGlyph(
        itemWielded->getGlyph(), Panel::charLines, pos, itemClr);
    }
    pos.x += 2;

    const auto& data = itemWielded->getData();
    //If thrown weapon, force melee info - otherwise use weapon context.
    const ItemRefAttInf attInf = data.mainAttMode == MainAttMode::thrown ?
                                 ItemRefAttInf::melee : ItemRefAttInf::wpnContext;

    str = itemWielded->getName(ItemRefType::plain, ItemRefInf::yes, attInf);
    Render::drawText(str, Panel::charLines, pos, clrWhite);
    pos.x += str.length() + 1;
  }
  else
  {
    Render::drawText("Unarmed", Panel::charLines, pos, clrWhite);
  }

  pos.x = CHARACTER_LINE_X0;
  pos.y += 1;

  // Level and xp
  Render::drawText("LVL:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 4;
  str = toStr(DungeonMaster::getCLvl());
  Render::drawText(str, Panel::charLines, pos, clrWhite);
  pos.x += str.length() + 1;
  Render::drawText("NXT:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 4;
  str = DungeonMaster::getCLvl() >= PLAYER_MAX_CLVL ? "-" :
        toStr(DungeonMaster::getXpToNextLvl());
  Render::drawText(str, Panel::charLines, pos, clrWhite);
  pos.x += str.length() + 1;

  //Dungeon level
  Render::drawText("DLVL:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 5;
  str = Map::dlvl >= 0 ? toStr(Map::dlvl) : "?";
  Render::drawText(str, Panel::charLines, pos, clrWhite);
  pos.x += str.length() + 1;

  //Encumbrance
  Render::drawText("ENC:", Panel::charLines, pos, clrMenuDrk);
  pos.x += 4;
  const int ENC = Map::player->getEncPercent();
  str = toStr(ENC) + "%";
  const Clr encClr = ENC < 100 ? clrGreenLgt :
                     ENC < ENC_IMMOBILE_LVL ? clrYellow : clrRedLgt;
  Render::drawText(str, Panel::charLines, pos, encClr);
  pos.x += str.length() + 1;

  //Thrown weapon
  pos.x = X_POS_MISSILE;

  auto* const itemMissiles =
    Map::player->getInv().getItemInSlot(SlotId::thrown);

  if(itemMissiles)
  {
    const Clr itemClr = itemMissiles->getClr();
    if(Config::isTilesMode())
    {
      Render::drawTile(itemMissiles->getTile(), Panel::charLines, pos, itemClr);
    }
    else
    {
      Render::drawGlyph(itemMissiles->getGlyph(), Panel::charLines, pos, itemClr);
    }
    pos.x += 2;

    str = itemMissiles->getName(ItemRefType::plural, ItemRefInf::yes,
                                ItemRefAttInf::thrown);
    Render::drawText(str, Panel::charLines, pos, clrWhite);
    pos.x += str.length() + 1;
  }
  else
  {
    Render::drawText("No thrown weapon", Panel::charLines, pos, clrWhite);
  }

  pos.y += 1;
  pos.x = CHARACTER_LINE_X0;

  vector<StrAndClr> propsLine;
  Map::player->getPropHandler().getPropsInterfaceLine(propsLine);
  const int NR_PROPS = propsLine.size();
  for(int i = 0; i < NR_PROPS; ++i)
  {
    const StrAndClr& curPropLabel = propsLine.at(i);
    Render::drawText(curPropLabel.str, Panel::charLines, pos, curPropLabel.clr);
    pos.x += curPropLabel.str.length() + 1;
  }

  //Turn number
  str = "T:" + toStr(GameTime::getTurn());
  pos.x = MAP_W - str.length() - 1;
  Render::drawText(str, Panel::charLines, pos, clrWhite);
}

} //CharacterLines
