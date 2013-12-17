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

void SaveHandler::save() {
  vector<string> lines;
  collectLinesFromGame(lines);
  writeFile(lines);
}

void SaveHandler::load() {
  vector<string> lines;
  readFile(lines);
  setGameParametersFromLines(lines);
}

void SaveHandler::collectLinesFromGame(vector<string>& lines) {
  lines.resize(0);
  lines.push_back("@" + eng.player->getNameA());

  eng.dungeonMaster->addSaveLines(lines);
  eng.scrollNameHandler->addSaveLines(lines);
  eng.potionNameHandler->addSaveLines(lines);
  eng.itemDataHandler->addSaveLines(lines);
  eng.player->getInventory()->addSaveLines(lines);
  eng.player->addSaveLines(lines);
  eng.playerBonHandler->addSaveLines(lines);
  eng.map->addSaveLines(lines);
  eng.actorDataHandler->addSaveLines(lines);
  eng.gameTime->addSaveLines(lines);
}

void SaveHandler::setGameParametersFromLines(vector<string>& lines) const {
  string playerName = lines.front();
  lines.erase(lines.begin());
  playerName.erase(playerName.begin());
  eng.player->getData()->name_a = playerName;
  eng.player->getData()->name_the = playerName;

  eng.dungeonMaster->setParametersFromSaveLines(lines);
  eng.scrollNameHandler->setParametersFromSaveLines(lines);
  eng.potionNameHandler->setParametersFromSaveLines(lines);
  eng.itemDataHandler->setParametersFromSaveLines(lines);
  eng.player->getInventory()->setParametersFromSaveLines(lines, eng);
  eng.player->setParametersFromSaveLines(lines);
  eng.playerBonHandler->setParametersFromSaveLines(lines);
  eng.map->setParametersFromSaveLines(lines);
  eng.actorDataHandler->setParametersFromSaveLines(lines);
  eng.gameTime->setParametersFromSaveLines(lines);
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
