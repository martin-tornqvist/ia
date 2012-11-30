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

const int X_POS_NAME = 1;
const int X_POS_SCORE = 20;
const int X_POS_LVL = 31;
const int X_POS_DLVL = 36;
const int X_POS_INSANITY = 42;
const int X_POS_RANK = 52;


bool HighScore::isEntryHigher(const HighScoreEntry& current, const HighScoreEntry& other) {
  return other.getScore() < current.getScore();
}

void HighScore::gameOver(const bool IS_VICTORY) {
  //Make a vector of name and xp pairs.
  vector<HighScoreEntry> entries;

  //Read the highscore file and fill the vector with entries from it.
  readFile(entries);

  //Make a new entry for the current player, and add it to the vector.
  HighScoreEntry currentPlayer(
    eng->player->getNameA(),
    eng->dungeonMaster->getXp(),
    eng->dungeonMaster->getLevel(),
    eng->map->getDungeonLevel(),
    eng->player->insanityLong,
    IS_VICTORY);

  entries.push_back(currentPlayer);

  //Sort the entries according to xp.
  sortEntries(entries);

  //Write a new high score file.
  writeFile(entries);
}

void HighScore::renderHighScoreScreen(const vector<HighScoreEntry>& entries, const int TOP_ELEMENT) const {
  eng->renderer->clearWindow();

  if(entries.size() == 0) {
    const int X0 = 1;
    const int Y0 = 4;
    eng->renderer->drawText("No High Score entries found [Space/Esc] to exit.", renderArea_screen, X0, Y0, clrWhite);
  } else {
    const string decorationLine(MAP_X_CELLS - 2, '-');

    eng->renderer->drawText(decorationLine, renderArea_characterLines, 0, 1, clrWhite);
    eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);

    int yPos = 1;

    eng->renderer->drawText(decorationLine, renderArea_screen, 1, yPos, clrWhite);
    eng->renderer->drawText("Displaying High Score", renderArea_screen, 3, yPos, clrWhite);

    yPos++;

    eng->renderer->drawText("NAME", renderArea_screen, X_POS_NAME, yPos, clrGray);
    eng->renderer->drawText("SCORE", renderArea_screen, X_POS_SCORE, yPos, clrGray);
    eng->renderer->drawText("LVL", renderArea_screen, X_POS_LVL, yPos, clrGray);
    eng->renderer->drawText("DLVL", renderArea_screen, X_POS_DLVL, yPos, clrGray);
    eng->renderer->drawText("INSANITY", renderArea_screen, X_POS_INSANITY, yPos, clrGray);

    yPos++;

    for(unsigned int i = TOP_ELEMENT; i < entries.size() && (i - TOP_ELEMENT) < static_cast<unsigned int>(MAP_Y_CELLS); i++) {

      const string NAME = entries.at(i).getName();
      const string SCORE = intToString(entries.at(i).getScore());
      const string LVL = intToString(entries.at(i).getLvl());
      const string DLVL = intToString(entries.at(i).getDlvl());
      const string INSANITY = intToString(entries.at(i).getInsanity());

//			const unsigned int FILL_START = X_POS_LEFT + NAME.size();
//			const unsigned int FILL_LENGTH = X_POS_RIGHT - FILL_START;
//			for(unsigned int xx = 0; xx < FILL_LENGTH; xx++) {
//				eng->renderer->drawText(".", renderArea_screen, FILL_START + xx, y, clrGray);
//			}
      const sf::Color clr = clrRedLight;
      eng->renderer->drawText(NAME, renderArea_screen, X_POS_NAME, yPos, clr);
      eng->renderer->drawText(SCORE, renderArea_screen, X_POS_SCORE, yPos, clr);
      eng->renderer->drawText(LVL, renderArea_screen, X_POS_LVL, yPos, clr);
      eng->renderer->drawText(DLVL, renderArea_screen, X_POS_DLVL, yPos, clr);
      eng->renderer->drawText(INSANITY, renderArea_screen, X_POS_INSANITY, yPos, clr);
      yPos++;
    }
  }

  eng->renderer->updateWindow();
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

    if(d.key_ == '2' || d.sfmlKey_ == sf::Keyboard::Down) {
      topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(entries.size()) - static_cast<int>(MAP_Y_CELLS)));
      renderHighScoreScreen(entries, topElement);
    }
    if(d.key_ == '8' || d.sfmlKey_ == sf::Keyboard::Up) {
      topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(entries.size()) - static_cast<int>(MAP_Y_CELLS)));
      renderHighScoreScreen(entries, topElement);
    }
    if(d.sfmlKey_ == sf::Keyboard::Space || d.sfmlKey_ == sf::Keyboard::Escape) {
      done = true;
    }
  }
}

void HighScore::sortEntries(vector<HighScoreEntry>& entries) {
  sort(entries.begin(), entries.end(), HighScore::isEntryHigher);
}

void HighScore::writeFile(vector<HighScoreEntry>& entries) {
  ofstream file;
  file.open("highscores", ios::trunc);

  for(unsigned int i = 0; i < entries.size(); i++) {
    const HighScoreEntry& entry = entries.at(i);

    const string VICTORY_STR = entry.isVictoryGame() ? "V" : "D";
    file << VICTORY_STR << endl;
    file << entry.getName() << endl;
    file << entry.getXp() << endl;
    file << entry.getLvl() << endl;
    file << entry.getDlvl() << endl;
    file << entry.getInsanity() << endl;
  }
}

void HighScore::readFile(vector<HighScoreEntry>& entries) {
  ifstream file;
  file.open("highscores");

  if(file.is_open()) {
    string line;

    while(getline(file, line)) {
      bool isVictory = line.at(0) == 'V';
      getline(file, line);
      const string NAME = line;
      getline(file, line);
      const int XP = stringToInt(line);
      getline(file, line);
      const int LVL = stringToInt(line);
      getline(file, line);
      const int DLVL = stringToInt(line);
      getline(file, line);
      const int INSANITY = stringToInt(line);
      entries.push_back(HighScoreEntry(NAME, XP, LVL, DLVL, INSANITY, isVictory));
    }
    file.close();
  }
}
