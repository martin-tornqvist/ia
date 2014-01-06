#include "SaveHandler.h"

#include <fstream>
#include <iostream>

#include "Engine.h"
#include "Log.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "PlayerBonuses.h"
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
  setGameParamsFromLines(lines);
}

void SaveHandler::collectLinesFromGame(vector<string>& lines) {
  lines.resize(0);
  lines.push_back(eng.player->getNameA());

  eng.dungeonMaster->addSaveLines(lines);
  eng.scrollNameHandler->addSaveLines(lines);
  eng.potionNameHandler->addSaveLines(lines);
  eng.itemDataHandler->addSaveLines(lines);
  eng.player->getInv().addSaveLines(lines);
  eng.player->addSaveLines(lines);
  eng.playerBonHandler->addSaveLines(lines);
  eng.map->addSaveLines(lines);
  eng.actorDataHandler->addSaveLines(lines);
  eng.gameTime->addSaveLines(lines);
  eng.playerSpellsHandler->addSaveLines(lines);
}

void SaveHandler::setGameParamsFromLines(vector<string>& lines) const {
  trace << "SaveHandler::setGameParamsFromLines()..." << endl;
  trace << "SaveHandler: Nr lines: " << lines.size() << endl;
  const string& playerName = lines.front();
  trace << "SaveHandler: playerName: " << playerName << endl;
  eng.player->getData().name_a = playerName;
  eng.player->getData().name_the = playerName;
  lines.erase(lines.begin());

  eng.dungeonMaster->setParamsFromSaveLines(lines);
  eng.scrollNameHandler->setParamsFromSaveLines(lines);
  eng.potionNameHandler->setParamsFromSaveLines(lines);
  eng.itemDataHandler->setParamsFromSaveLines(lines);
  eng.player->getInv().setParamsFromSaveLines(lines, eng);
  eng.player->setParamsFromSaveLines(lines);
  eng.playerBonHandler->setParamsFromSaveLines(lines);
  eng.map->setParamsFromSaveLines(lines);
  eng.actorDataHandler->setParamsFromSaveLines(lines);
  eng.gameTime->setParamsFromSaveLines(lines);
  eng.playerSpellsHandler->setParamsFromSaveLines(lines);
  trace << "SaveHandler::setGameParamsFromLines() [DONE]" << endl;
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
