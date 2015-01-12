#include "SaveHandling.h"

#include <fstream>
#include <iostream>

#include "Init.h"
#include "Log.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "PlayerBon.h"
#include "Map.h"
#include "MapTravel.h"
#include "Inventory.h"
#include "GameTime.h"
#include "PlayerSpellsHandling.h"
#include "ItemJewelry.h"

using namespace std;

namespace SaveHandling
{

namespace
{

void collectLinesFromGame(vector<string>& lines)
{
  lines.clear();
  lines.push_back(Map::player->getNameA());

  DungeonMaster::storeToSaveLines(lines);
  ScrollHandling::storeToSaveLines(lines);
  PotionHandling::storeToSaveLines(lines);
  ItemData::storeToSaveLines(lines);
  JewelryHandling::storeToSaveLines(lines);
  Map::player->getInv().storeToSaveLines(lines);
  Map::player->storeToSaveLines(lines);
  PlayerBon::storeToSaveLines(lines);
  MapTravel::storeToSaveLines(lines);
  Map::storeToSaveLines(lines);
  ActorData::storeToSaveLines(lines);
  GameTime::storeToSaveLines(lines);
  PlayerSpellsHandling::storeToSaveLines(lines);
}

void setupGameFromLines(vector<string>& lines)
{
  TRACE_FUNC_BEGIN;
  const string& playerName = lines.front();
  Map::player->getData().nameA = playerName;
  Map::player->getData().nameThe = playerName;
  lines.erase(begin(lines));

  DungeonMaster::setupFromSaveLines(lines);
  ScrollHandling::setupFromSaveLines(lines);
  PotionHandling::setupFromSaveLines(lines);
  ItemData::setupFromSaveLines(lines);
  JewelryHandling::setupFromSaveLines(lines);
  Map::player->getInv().setupFromSaveLines(lines);
  Map::player->setupFromSaveLines(lines);
  PlayerBon::setupFromSaveLines(lines);
  MapTravel::setupFromSaveLines(lines);
  Map::setupFromSaveLines(lines);
  ActorData::setupFromSaveLines(lines);
  GameTime::setupFromSaveLines(lines);
  PlayerSpellsHandling::setupFromSaveLines(lines);
  TRACE_FUNC_END;
}

void writeFile(const vector<string>& lines)
{
  ofstream file;
  file.open("data/save", ios::trunc);

  if (file.is_open())
  {
    for (size_t i = 0; i < lines.size(); ++i)
    {
      file << lines[i];
      if (i != lines.size() - 1) {file << endl;}
    }
    file.close();
  }
}

void readFile(vector<string>& lines)
{
  lines.clear();

  string curLine;
  ifstream file("data/save");
  if (file.is_open())
  {
    while (getline(file, curLine)) {lines.push_back(curLine);}
    file.close();

    vector<string> emptyLines;
    emptyLines.clear();
    writeFile(emptyLines);
  }
  else
  {
    assert(false && "Failed to open save file");
  }
}

} //namespace

void save()
{
  vector<string> lines;
  collectLinesFromGame(lines);
  writeFile(lines);
}

void load()
{
  vector<string> lines;
  readFile(lines);
  setupGameFromLines(lines);
}

bool isSaveAvailable()
{
  ifstream file("data/save");
  if (file.good())
  {
    const bool IS_EMPTY = file.peek() == std::ifstream::traits_type::eof();
    file.close();
    return !IS_EMPTY;
  }
  else
  {
    file.close();
    return false;
  }
}

} //SaveHandling
