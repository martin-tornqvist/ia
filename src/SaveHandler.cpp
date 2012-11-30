#include "SaveHandler.h"

#include <fstream>
#include <iostream>

#include "Engine.h"
#include "Log.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "PlayerBonuses.h"
#include "Map.h"
#include "DungeonClimb.h"
#include "Inventory.h"

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
  lines.push_back("@" + eng->player->getNameA());

  eng->dungeonMaster->addSaveLines(lines);
  eng->scrollNameHandler->addSaveLines(lines);
  eng->potionNameHandler->addSaveLines(lines);
  eng->itemData->addSaveLines(lines);
  eng->player->getInventory()->addSaveLines(lines);
  eng->player->addSaveLines(lines);
  eng->playerBonusHandler->addSaveLines(lines);
  eng->map->addSaveLines(lines);
  eng->actorData->addSaveLines(lines);
  eng->gameTime->addSaveLines(lines);
}

void SaveHandler::setGameParametersFromLines(vector<string>& lines) const {
  string playerName = lines.front();
  lines.erase(lines.begin());
  playerName.erase(playerName.begin());
  eng->player->getDef()->name_a	= playerName;
  eng->player->getDef()->name_the	= playerName;

  eng->dungeonMaster->setParametersFromSaveLines(lines);
  eng->scrollNameHandler->setParametersFromSaveLines(lines);
  eng->potionNameHandler->setParametersFromSaveLines(lines);
  eng->itemData->setParametersFromSaveLines(lines);
  eng->player->getInventory()->setParametersFromSaveLines(lines, eng);
  eng->player->setParametersFromSaveLines(lines);
  eng->playerBonusHandler->setParametersFromSaveLines(lines);
  eng->map->setParametersFromSaveLines(lines);
  eng->actorData->setParametersFromSaveLines(lines);
  eng->gameTime->setParametersFromSaveLines(lines);

  eng->gameTime->insertActorInLoop(eng->player);
  eng->dungeonClimb->travelDown();
}

bool SaveHandler::isSaveAvailable() {
  vector<string> lines;
  readFile(lines);

  bool isSaveExists = false;

  if(lines.size() > 0) {
    isSaveExists = true;
  }

  writeFile(lines);
  return isSaveExists;
}

void SaveHandler::readFile(vector<string>& lines) {
  lines.resize(0);

  string curLine;
  ifstream file("save/save");

  if(file.is_open()) {
    while(getline(file, curLine)) {
      lines.push_back(curLine);
    }
    file.close();

    vector<string> emptyLines;
    emptyLines.resize(0);
    writeFile(emptyLines);
  }
  else {
    tracer << "[WARNING] Could not open save file, in SaveHandler::readFile()" << endl;
  }
}

void SaveHandler::writeFile(const vector<string>& lines) const {
  ofstream file;
  file.open("save/save", ios::trunc);

  for(unsigned int i = 0; i < lines.size(); i++) {
    file << lines.at(i);
    if(i != lines.size() - 1) {
      file << endl;
    }
  }

  file.close();
}
