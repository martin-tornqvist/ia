#include "SaveHandler.h"

#include <fstream>
#include <iostream>

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
#include "PlayerSpellsHandler.h"

void SaveHandler::save() {
  vector<string> lines;
  collectLinesFromGame(lines);
  writeFile(lines);
}

void SaveHandler::load() {
  vector<string> lines;
  readFile(lines);
  setupGameFromLines(lines);
}

void SaveHandler::collectLinesFromGame(vector<string>& lines) {
  lines.resize(0);
  lines.push_back(Map::player->getNameA());

  eng.dungeonMaster->storeToSaveLines(lines);
  eng.scrollNameHandler->storeToSaveLines(lines);
  eng.potionNameHandler->storeToSaveLines(lines);
  eng.itemDataHandler->storeToSaveLines(lines);
  Map::player->getInv().storeToSaveLines(lines);
  Map::player->storeToSaveLines(lines);
  PlayerBon::storeToSaveLines(lines);
  Map::storeToSaveLines(lines);
  ActorData::storeToSaveLines(lines);
  GameTime::storeToSaveLines(lines);
  Map::playerSpellsHandler->storeToSaveLines(lines);
}

void SaveHandler::setupGameFromLines(vector<string>& lines) const {
  trace << "SaveHandler::setupGameFromLines()..." << endl;
  trace << "SaveHandler: Nr lines: " << lines.size() << endl;
  const string& playerName = lines.front();
  trace << "SaveHandler: playerName: " << playerName << endl;
  Map::player->getData().name_a = playerName;
  Map::player->getData().name_the = playerName;
  lines.erase(lines.begin());

  eng.dungeonMaster->setupFromSaveLines(lines);
  eng.scrollNameHandler->setupFromSaveLines(lines);
  eng.potionNameHandler->setupFromSaveLines(lines);
  eng.itemDataHandler->setupFromSaveLines(lines);
  Map::player->getInv().setupFromSaveLines(lines, eng);
  Map::player->setupFromSaveLines(lines);
  PlayerBon::setupFromSaveLines(lines);
  Map::setupFromSaveLines(lines);
  ActorData::setupFromSaveLines(lines);
  GameTime::setupFromSaveLines(lines);
  Map::playerSpellsHandler->setupFromSaveLines(lines);
  trace << "SaveHandler::setupGameFromLines() [DONE]" << endl;
}

bool SaveHandler::isSaveAvailable() {
  ifstream file("data/save");
  if(file.good()) {
    const bool IS_EMPTY = file.peek() == std::ifstream::traits_type::eof();
    file.close();
    return IS_EMPTY == false;
  } else {
    file.close();
    return false;
  }
}

void SaveHandler::readFile(vector<string>& lines) {
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
    trace << "[WARNING] Could not open save file, ";
    trace << "in SaveHandler::readFile()" << endl;
  }
}

void SaveHandler::writeFile(const vector<string>& lines) const {
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
