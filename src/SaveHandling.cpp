#include "SaveHandling.h"

#include <fstream>
#include <iostream>

#include "Init.h"
#include "Log.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "PlayerBon.h"
#include "Map.h"
#include "DungeonClimb.h"
#include "Inventory.h"
#include "GameTime.h"
#include "PlayerSpellsHandling.h"

using namespace std;

namespace SaveHandling {

namespace {

void collectLinesFromGame(vector<string>& lines) {
  lines.resize(0);
  lines.push_back(Map::player->getNameA());

  DungeonMaster::storeToSaveLines(lines);
  ScrollNameHandling::storeToSaveLines(lines);
  PotionNameHandling::storeToSaveLines(lines);
  ItemData::storeToSaveLines(lines);
  Map::player->getInv().storeToSaveLines(lines);
  Map::player->storeToSaveLines(lines);
  PlayerBon::storeToSaveLines(lines);
  Map::storeToSaveLines(lines);
  ActorData::storeToSaveLines(lines);
  GameTime::storeToSaveLines(lines);
  PlayerSpellsHandling::storeToSaveLines(lines);
}

void setupGameFromLines(vector<string>& lines) {
  TRACE_FUNC_BEGIN;
  TRACE << "SaveHandler: Nr lines: " << lines.size() << endl;
  const string& playerName = lines.front();
  TRACE << "SaveHandler: playerName: " << playerName << endl;
  Map::player->getData().name_a = playerName;
  Map::player->getData().name_the = playerName;
  lines.erase(lines.begin());

  DungeonMaster::setupFromSaveLines(lines);
  ScrollNameHandling::setupFromSaveLines(lines);
  PotionNameHandling::setupFromSaveLines(lines);
  ItemData::setupFromSaveLines(lines);
  Map::player->getInv().setupFromSaveLines(lines);
  Map::player->setupFromSaveLines(lines);
  PlayerBon::setupFromSaveLines(lines);
  Map::setupFromSaveLines(lines);
  ActorData::setupFromSaveLines(lines);
  GameTime::setupFromSaveLines(lines);
  PlayerSpellsHandling::setupFromSaveLines(lines);
  TRACE_FUNC_END;
}

void writeFile(const vector<string>& lines) {
  ofstream file;
  file.open("data/save", ios::trunc);

  if(file.is_open()) {
    for(unsigned int i = 0; i < lines.size(); i++) {
      file << lines.at(i);
      if(i != lines.size() - 1) {
        file << endl;
      }
    }
    file.close();
  }
}

void readFile(vector<string>& lines) {
  lines.resize(0);

  string curLine;
  ifstream file("data/save");
  if(file.is_open()) {
    while(getline(file, curLine)) {
      lines.push_back(curLine);
    }
    file.close();

    vector<string> emptyLines;
    emptyLines.resize(0);
    writeFile(emptyLines);
  } else {
    TRACE << "[WARNING] Could not open save file, ";
    TRACE << "in SaveHandler::readFile()" << endl;
  }
}

} //namespace

void save() {
  vector<string> lines;
  collectLinesFromGame(lines);
  writeFile(lines);
}

void load() {
  vector<string> lines;
  readFile(lines);
  setupGameFromLines(lines);
}

bool isSaveAvailable() {
  ifstream file("data/save");
  if(file.good()) {
    const bool IS_EMPTY = file.peek() == std::ifstream::traits_type::eof();
    file.close();
    return !IS_EMPTY;
  } else {
    file.close();
    return false;
  }
}

} //SaveHandling
