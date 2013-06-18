#include "Highscore.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include "Engine.h"
#include "Highscore.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Popup.h"
#include "Input.h"

const int X_POS_DATE = 1;
const int X_POS_NAME = X_POS_DATE + 19;
const int X_POS_SCORE = X_POS_NAME + 14;
const int X_POS_LVL = X_POS_SCORE + 12;
const int X_POS_DLVL = X_POS_LVL + 8;
const int X_POS_INSANITY = X_POS_DLVL + 8;
const int X_POS_RANK = X_POS_INSANITY + 10;

bool HighScore::isEntryHigher(const HighScoreEntry& current,
                              const HighScoreEntry& other) {
  return other.getScore() < current.getScore();
}

vector<HighScoreEntry> HighScore::getEntriesSorted() {
  vector<HighScoreEntry> entries;
  readFile(entries);
  if(entries.empty() == false) {
    sortEntries(entries);
  }
  return entries;
}

void HighScore::gameOver(const bool IS_VICTORY) {
  vector<HighScoreEntry> entries = getEntriesSorted();

  HighScoreEntry currentPlayer(
    eng->basicUtils->getCurrentTime().getTimeStr(time_minute, true),
    eng->player->getNameA(),
    eng->dungeonMaster->getXp(),
    eng->dungeonMaster->getLevel(),
    eng->map->getDungeonLevel(),
    eng->player->getInsanity(),
    IS_VICTORY);

  entries.push_back(currentPlayer);

  sortEntries(entries);

  writeFile(entries);
}

void HighScore::renderHighScoreScreen(const vector<HighScoreEntry>& entries,
                                      const int TOP_ELEMENT) const {
  eng->renderer->clearScreen();

  if(entries.size() == 0) {
    const int X0 = 1;
    const int Y0 = 4;
    eng->renderer->drawText(
      "No High Score entries found | space/esc to exit ",
      renderArea_screen, X0, Y0, clrWhite);
  } else {
    const string decorationLine(MAP_X_CELLS - 2, '-');

    eng->renderer->drawText(
      decorationLine, renderArea_characterLines, 0, 1, clrWhite);
    eng->renderer->drawText(
      " 2/8, down/up to navigate | space/esc to exit ",
      renderArea_characterLines, 3, 1, clrWhite);

    int yPos = 1;

    eng->renderer->drawText(
      decorationLine, renderArea_screen, 1, yPos, clrWhite);
    eng->renderer->drawText(
      " Displaying High Scores ", renderArea_screen, 3, yPos, clrWhite);

    yPos++;

    eng->renderer->drawText(
      "Ended", renderArea_screen, X_POS_DATE, yPos, clrGray);
    eng->renderer->drawText(
      "Name", renderArea_screen, X_POS_NAME, yPos, clrGray);
    eng->renderer->drawText(
      "Score", renderArea_screen, X_POS_SCORE, yPos, clrGray);
    eng->renderer->drawText(
      "Level", renderArea_screen, X_POS_LVL, yPos, clrGray);
    eng->renderer->drawText(
      "Depth", renderArea_screen, X_POS_DLVL, yPos, clrGray);
    eng->renderer->drawText(
      "Insanity", renderArea_screen, X_POS_INSANITY, yPos, clrGray);

    yPos++;

    for(
      int i = TOP_ELEMENT;
      i < int(entries.size()) && (i - TOP_ELEMENT) < MAP_Y_CELLS;
      i++) {
      const string dateAndTime = entries.at(i).getDateAndTime();
      const string name = entries.at(i).getName();
      const string SCORE = intToString(entries.at(i).getScore());
      const string LVL = intToString(entries.at(i).getLvl());
      const string DLVL = intToString(entries.at(i).getDlvl());
      const string INSANITY = intToString(entries.at(i).getInsanity());

      const SDL_Color clr = clrNosferatuTeal;
      eng->renderer->drawText(
        dateAndTime, renderArea_screen, X_POS_DATE, yPos, clr);
      eng->renderer->drawText(
        name, renderArea_screen, X_POS_NAME, yPos, clr);
      eng->renderer->drawText(
        SCORE, renderArea_screen, X_POS_SCORE, yPos, clr);
      eng->renderer->drawText(
        LVL, renderArea_screen, X_POS_LVL, yPos, clr);
      eng->renderer->drawText(
        DLVL, renderArea_screen, X_POS_DLVL, yPos, clr);
      eng->renderer->drawText(
        INSANITY + "%", renderArea_screen, X_POS_INSANITY, yPos, clr);
      yPos++;
    }
  }

  eng->renderer->updateScreen();
}

void HighScore::runHighScoreScreen() {
  vector<HighScoreEntry> entries;
  readFile(entries);

  if(entries.empty()) {
    eng->popup->showMessage("No High Score entries found.", false);
    return;
  }

  sortEntries(entries);

  int topElement = 0;
  renderHighScoreScreen(entries, topElement);

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement =
        min(topElement + int(MAP_Y_CELLS / 5),
            int(entries.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      renderHighScoreScreen(entries, topElement);
    }
    if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement =
        min(topElement - int(MAP_Y_CELLS / 5),
            int(entries.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      renderHighScoreScreen(entries, topElement);
    }
    if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }
  }
}

void HighScore::sortEntries(vector<HighScoreEntry>& entries) {
  sort(entries.begin(), entries.end(), HighScore::isEntryHigher);
}

void HighScore::writeFile(vector<HighScoreEntry>& entries) {
  ofstream file;
  file.open("data/highscores", ios::trunc);

  for(unsigned int i = 0; i < entries.size(); i++) {
    const HighScoreEntry& entry = entries.at(i);

    const string VICTORY_STR = entry.isVictoryGame() ? "V" : "D";
    file << VICTORY_STR << endl;
    file << entry.getDateAndTime() << endl;
    file << entry.getName() << endl;
    file << entry.getXp() << endl;
    file << entry.getLvl() << endl;
    file << entry.getDlvl() << endl;
    file << entry.getInsanity() << endl;
  }
}

void HighScore::readFile(vector<HighScoreEntry>& entries) {
  ifstream file;
  file.open("data/highscores");

  if(file.is_open()) {
    string line;

    while(getline(file, line)) {
      bool isVictory = line.at(0) == 'V';
      getline(file, line);
      const string dateAndTime = line;
      getline(file, line);
      const string name = line;
      getline(file, line);
      const int XP = stringToInt(line);
      getline(file, line);
      const int LVL = stringToInt(line);
      getline(file, line);
      const int DLVL = stringToInt(line);
      getline(file, line);
      const int INSANITY = stringToInt(line);
      entries.push_back(
        HighScoreEntry(dateAndTime, name, XP, LVL, DLVL, INSANITY, isVictory));
    }
    file.close();
  }
}
