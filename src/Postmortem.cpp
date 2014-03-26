#include "Postmortem.h"

#include <fstream>
#include <iostream>

#include "Engine.h"

#include "Renderer.h"
#include "Input.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Highscore.h"
#include "PlayerBonuses.h"
#include "TextFormatting.h"

void Postmortem::run(bool* const quitGame) {
  makeInfoLines();
  makeMemorialFile();
  readKeysMenu(quitGame);
}

void Postmortem::makeInfoLines() {
  trace << "Postmortem::makeInfoLines()..." << endl;

  const SDL_Color clrHeading  = clrWhiteHigh;
  const SDL_Color clrInfo     = clrWhite;

  postmortemLines.push_back(
    StrAndClr(" " + eng.player->getNameA(), clrHeading));

  postmortemLines.push_back(
    StrAndClr("   * Explored to the depth of dungeon level " +
              toString(eng.map->getDlvl()), clrInfo));
  postmortemLines.push_back(
    StrAndClr("   * Was " +
              toString(min(100, eng.player->getInsanity())) +
              "% insane", clrInfo));

  //TODO Make some sort of insanity class or something where this info is stored,
  //this is ugly as hell
  if(eng.player->phobias[int(Phobia::closedPlace)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of enclosed spaces", clrInfo));
  if(eng.player->phobias[int(Phobia::dog)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of dogs", clrInfo));
  if(eng.player->phobias[int(Phobia::rat)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of rats", clrInfo));
  if(eng.player->phobias[int(Phobia::undead)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of the dead", clrInfo));
  if(eng.player->phobias[int(Phobia::openPlace)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of open places", clrInfo));
  if(eng.player->phobias[int(Phobia::spider)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of spiders", clrInfo));
  if(eng.player->phobias[int(Phobia::deepPlaces)])
    postmortemLines.push_back(
      StrAndClr("   * Had a phobia of deep places", clrInfo));

  if(eng.player->obsessions[int(Obsession::masochism)])
    postmortemLines.push_back(
      StrAndClr("   * Had a masochistic obsession", clrInfo));
  if(eng.player->obsessions[int(Obsession::sadism)])
    postmortemLines.push_back(
      StrAndClr("   * Had a sadistic obsession", clrInfo));

  postmortemLines.push_back(StrAndClr(" ", clrInfo));


  trace << "Postmortem: Listing abilities gained" << endl;
  postmortemLines.push_back(StrAndClr(" Abilities gained:", clrHeading));
  string abilitiesLine;
  eng.playerBonHandler->getAllPickedTraitsTitlesLine(abilitiesLine);
  if(abilitiesLine.empty()) {
    postmortemLines.push_back(StrAndClr("   * None", clrInfo));
  } else {
    vector<string> abilitiesLines;
    TextFormatting::lineToLines(abilitiesLine, 60, abilitiesLines);
    for(unsigned int i = 0; i < abilitiesLines.size(); i++) {
      postmortemLines.push_back(
        StrAndClr("   " + abilitiesLines.at(i), clrInfo));
    }
  }
  postmortemLines.push_back(StrAndClr(" ", clrInfo));

  trace << "Postmortem: Listing monster kills" << endl;
  // TODO Add name_plural_a, and name_plural_the to actor defs?
  postmortemLines.push_back(StrAndClr(" Monsters killed:", clrHeading));
  vector< pair<string, int> > killList;
  int nrOfTotalKills = 0;
  for(unsigned int i = actor_player + 1; i < endOfActorIds; i++) {
    const ActorData& d = eng.actorDataHandler->dataList[i];
    if(d.nrOfKills > 0) {

      nrOfTotalKills += d.nrOfKills;

      bool isAlreadyAdded = false;
      for(unsigned int ii = 0; ii < killList.size(); ii++) {
        if(killList.at(ii).first == d.name_a) {
          killList.at(ii).second += d.nrOfKills;
          isAlreadyAdded = true;
        }
      }

      if(isAlreadyAdded == false) {
        if(d.isUnique) {
          killList.push_back(pair<string, int>(d.name_a, -1));
        } else {
          killList.push_back(pair<string, int>(d.name_a, d.nrOfKills));
        }
      }
    }
  }
  if(killList.empty()) {
    postmortemLines.push_back(StrAndClr("   * None", clrInfo));
  } else {
    postmortemLines.back().str += " (" + toString(nrOfTotalKills) + " total)";

    for(unsigned int i = 0; i < killList.size(); i++) {
      const string name = killList.at(i).first;
      const int nrOfKills = killList.at(i).second;
      const string nrOfKillsStr =
        nrOfKills == -1 ? "" : (": " + toString(nrOfKills));
      postmortemLines.push_back(
        StrAndClr("   * " + name + nrOfKillsStr, clrInfo));
    }
  }
  postmortemLines.push_back(StrAndClr(" ", clrInfo));

  postmortemLines.push_back(StrAndClr(" The last messages:", clrHeading));
  int historyElement = max(0, int(eng.log->history.size()) - 20);
  for(unsigned int i = historyElement; i < eng.log->history.size(); i++) {
    string row = "";
    for(unsigned int ii = 0; ii < eng.log->history.at(i).size(); ii++) {
      string msgStr = "";
      eng.log->history.at(i).at(ii).getStrWithRepeats(msgStr);
      row += msgStr + " ";
    }
    postmortemLines.push_back(StrAndClr("   " + row, clrInfo));
  }
  postmortemLines.push_back(StrAndClr(" ", clrInfo));

  trace << "Postmortem: Drawing the final map" << endl;
  postmortemLines.push_back(StrAndClr(" The final moment:", clrHeading));
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
          if(Utils::isPosInsideMap(Pos(x + dx, y + dy))) {
            const FeatureStatic* const f =
              eng.map->cells[x + dx][y + dy].featureStatic;
            if(f->isVisionPassable()) {
              eng.map->cells[x][y].isSeenByPlayer = true;
            }
          }
        }
      }
    }
  }
  Renderer::drawMap(); //To set the glyph array
  for(int y = 0; y < MAP_H; y++) {
    string currentRow = "";
    for(int x = 0; x < MAP_W; x++) {
      if(Pos(x, y) == eng.player->pos) {
        currentRow.push_back('@');
      } else {
        if(
          Renderer::renderArray_[x][y].glyph == ' ' &&
          (y == 0 || x == 0 || y == MAP_H - 1 || x == MAP_W - 1)) {
          currentRow.push_back('*');
        } else {
          if(Renderer::renderArray_[x][y].glyph ==
              eng.featureDataHandler->getData(feature_stoneWall)->glyph
              || Renderer::renderArray_[x][y].glyph ==
              eng.featureDataHandler->getData(feature_rubbleHigh)->glyph) {
            currentRow.push_back('#');
          } else if(Renderer::renderArray_[x][y].glyph ==
                    eng.featureDataHandler->getData(feature_statue)->glyph) {
            currentRow.push_back('M');
          } else {
            currentRow.push_back(Renderer::renderArray_[x][y].glyph);
          }
        }
      }
    }
    postmortemLines.push_back(StrAndClr(currentRow, clrInfo));
    currentRow.clear();
  }

  trace << "Postmortem::makeInfoLines() [DONE]" << endl;
}

void Postmortem::renderInfo(const int TOP_ELEMENT) {
  Renderer::clearScreen();

  const string decorationLine(MAP_W, '-');
  Renderer::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  const int X_LABEL = 3;

  Renderer::drawText(" Displaying postmortem information ", Panel::screen,
                     Pos(X_LABEL, 0), clrGray);

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

  Renderer::drawText(" 2/8, down/up to navigate | space/esc to exit  ",
                     Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);

  const int NR_LINES_TOT = int(postmortemLines.size());
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
  int yPos = 1;

  for(
    int i = TOP_ELEMENT;
    i < NR_LINES_TOT && (i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR;
    i++) {
    Renderer::drawText(
      postmortemLines.at(i).str, Panel::screen, Pos(0, yPos++),
      postmortemLines.at(i).clr);
  }

  Renderer::updateScreen();
}

void Postmortem::runInfo() {
  const int LINE_JUMP           = 3;
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
  const int NR_LINES_TOT        = postmortemLines.size();

  int topNr = 0;

  while(true) {
    renderInfo(topNr);

    const KeyboardReadRetData& d = Input::readKeysUntilFound(eng);

    if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2') {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR) {
        topNr = 0;
      } else {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    } else if(d.sdlKey_ == SDLK_UP || d.key_ == '8') {
      topNr = max(0, topNr - LINE_JUMP);
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      break;
    }
  }
}

void Postmortem::makeMemorialFile() {
  const string timeStamp =
    eng.dungeonMaster->getTimeStarted().getTimeStr(time_second, false);
  const string memorialFileName =
    eng.player->getNameA() + "_" + timeStamp + ".txt";
  const string memorialFilePath = "data/" + memorialFileName;

  // Add memorial file
  ofstream file;
  file.open(memorialFilePath.data(), ios::trunc);
  for(unsigned int i = 0; i < postmortemLines.size(); i++) {
    file << postmortemLines.at(i).str << endl;
  }
  file.close();

  // Add reference to memorial file in list
  const string memorialList = "data/memorialFileList";
  file.open(memorialList.data(), ios::app);
  file << memorialFileName << endl;
  file.close();
}

void Postmortem::readKeysMenu(bool* const quitGame) {
  MenuBrowser browser(5, 0);

  renderMenu(browser);

  bool done = false;
  while(done == false) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        renderMenu(browser);
      } break;

      case MenuAction::esc: {
        *quitGame = true;
        done      = true;
      } break;

      case MenuAction::space:
      case MenuAction::selectedShift: {} break;

      case MenuAction::selected: {
        if(browser.isPosAtElement(0)) {
          runInfo();
          renderMenu(browser);
        }
        if(browser.isPosAtElement(1)) {
          eng.highScore->runHighScoreScreen();
          renderMenu(browser);
        }
        if(browser.isPosAtElement(2)) {
          eng.log->displayHistory();
          renderMenu(browser);
        }
        if(browser.isPosAtElement(3)) {
          done = true;
        }
        if(browser.isPosAtElement(4)) {
          *quitGame = true;
          done      = true;
        }
      } break;
    }
  }
}

void Postmortem::renderMenu(const MenuBrowser& browser) {
  vector<string> art;

  string curLine;
  ifstream file("data/ascii_graveyard");

  if(file.is_open()) {
    while(getline(file, curLine)) {
      if(curLine.size() > 0) {
        art.push_back(curLine);
      }
    }
  } else {
    trace << "[WARNING] Could not open ascii graveyard file, ";
    trace << "in Postmortem::renderMenu()" << endl;
  }

  file.close();

  Renderer::coverPanel(Panel::screen);

  Pos pos(1, 1);

  for(unsigned int i = 0; i < art.size(); i++) {
    Renderer::drawText(art.at(i), Panel::screen, pos, clrWhiteHigh);
    pos.y += 1;
  }

  pos.set(45, 18);
  const string NAME_STR = eng.player->getData().name_a;
  Renderer::drawTextCentered(NAME_STR, Panel::screen, pos, clrWhiteHigh);

//  pos.y += 2;
//  const string LVL_STR = "LVL " + toString(eng.dungeonMaster->getLevel());
//  Renderer::drawTextCentered(LVL_STR, Panel::screen, pos, clrWhiteHigh);

  //Draw command labels
  pos.set(55, 14);
  Renderer::drawText(
    "a) Information", Panel::screen, pos,
    browser.isPosAtElement(0) ? clrWhite : clrRedLgt);
  pos.y += 1;

  Renderer::drawText(
    "b) View the High Score", Panel::screen, pos,
    browser.isPosAtElement(1) ? clrWhite : clrRedLgt);
  pos.y += 1;

  Renderer::drawText(
    "c) View messages", Panel::screen, pos,
    browser.isPosAtElement(2) ? clrWhite : clrRedLgt);
  pos.y += 1;

  Renderer::drawText(
    "d) Return to main menu", Panel::screen, pos,
    browser.isPosAtElement(3) ? clrWhite : clrRedLgt);
  pos.y += 1;

  Renderer::drawText(
    "e) Quit the game", Panel::screen, pos,
    browser.isPosAtElement(4) ? clrWhite : clrRedLgt);
  pos.y += 1;

  Renderer::updateScreen();
}
