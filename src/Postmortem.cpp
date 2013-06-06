#include "Postmortem.h"

#include <fstream>
#include <iostream>

#include "Engine.h"
#include "Input.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Highscore.h"

void Postmortem::run(bool* const quitGame) {
  makeInfoLines();

  makeMemorialFile();

  readKeysMenu(quitGame);
}

void Postmortem::makeInfoLines() {
  tracer << "Postmortem::makeInfoLines()..." << endl;

  const SDL_Color clrHeading  = clrNosferatuTealLgt;
  const SDL_Color clrInfo     = clrNosferatuTeal;

  postmortemLines.push_back(StringAndColor(" " + eng->player->getNameA(), clrHeading));

  postmortemLines.push_back(StringAndColor("   * Advanced to character level " + intToString(eng->dungeonMaster->getLevel()), clrInfo));
  postmortemLines.push_back(StringAndColor("   * Gained " + intToString(eng->dungeonMaster->getXp()) + " experience points", clrInfo));
  postmortemLines.push_back(StringAndColor("   * Explored to the depth of dungeon level " + intToString(eng->map->getDungeonLevel()), clrInfo));
  postmortemLines.push_back(StringAndColor("   * Was " + intToString(min(100, eng->player->getInsanity())) + "% insane", clrInfo));

  //TODO Make some sort of insanity class or something where this info is stored, this shit is ugly as hell
  if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of enclosed spaces", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_dog])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of dogs", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_rat])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of rats", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_undead])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of the dead", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_openPlace])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of open places", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_spider])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of spiders", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
    postmortemLines.push_back(StringAndColor("   * Had a phobia of deep places", clrInfo));

  if(eng->player->insanityObsessions[insanityObsession_masochism])
    postmortemLines.push_back(StringAndColor("   * Had a masochistic obsession", clrInfo));
  if(eng->player->insanityObsessions[insanityObsession_sadism])
    postmortemLines.push_back(StringAndColor("   * Had a sadistic obsession", clrInfo));

  postmortemLines.push_back(StringAndColor(" ", clrInfo));

  tracer << "Postmortem: Listing monster kills" << endl;
  // TODO Add name_plural_a, and name_plural_the to actor defs?
  postmortemLines.push_back(StringAndColor(" Monsters killed:", clrHeading));
  vector< pair<string, int> > killList;
  int nrOfTotalKills = 0;
  for(unsigned int i = actor_player + 1; i < endOfActorIds; i++) {
    const ActorDefinition& d = eng->actorData->actorDefinitions[i];
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
    postmortemLines.push_back(StringAndColor("   * None", clrInfo));
  } else {
    postmortemLines.back().str += " (" + intToString(nrOfTotalKills) + " total)";

    for(unsigned int i = 0; i < killList.size(); i++) {
      const string name = killList.at(i).first;
      const int nrOfKills = killList.at(i).second;
      const string nrOfKillsStr = nrOfKills == -1 ? "" : (": " + intToString(nrOfKills));
      postmortemLines.push_back(StringAndColor("   * " + name + nrOfKillsStr, clrInfo));
    }
  }
  postmortemLines.push_back(StringAndColor(" ", clrInfo));

  postmortemLines.push_back(StringAndColor(" The last messages:", clrHeading));
  int historyElement = static_cast<unsigned int>(max(0, static_cast<int>(eng->log->history.size()) - 10));
  for(unsigned int i = historyElement; i < eng->log->history.size(); i++) {
    string row = "";
    for(unsigned int ii = 0; ii < eng->log->history.at(i).size(); ii++) {
      row += eng->log->history.at(i).at(ii).str + " ";
    }
    postmortemLines.push_back(StringAndColor("   " + row, clrInfo));
  }
  postmortemLines.push_back(StringAndColor(" ", clrInfo));

  tracer << "Postmortem: Drawing the final map" << endl;
  postmortemLines.push_back(StringAndColor(" The final moment:", clrHeading));
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
          if(eng->mapTests->isCellInsideMap(coord(x + dx, y + dy))) {
            if(eng->map->featuresStatic[x + dx][y + dy]->isVisionPassable()) {
              eng->map->playerVision[x][y] = true;
            }
          }
        }
      }
    }
  }
  eng->renderer->drawAscii(); //To set the glyph array
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    string currentRow;
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(coord(x, y) == eng->player->pos) {
        currentRow.push_back('@');
      } else {
        if(eng->renderer->renderArrayAscii[x][y].glyph == ' ' &&
            (y == 0 || x == 0 || y == MAP_Y_CELLS - 1 || x == MAP_X_CELLS - 1)) {
          currentRow.push_back('*');
        } else {
          if(eng->renderer->renderArrayAscii[x][y].glyph ==
              eng->featureData->getFeatureDef(feature_stoneWall)->glyph) {
            currentRow.push_back('#');
          } else {
            currentRow.push_back(eng->renderer->renderArrayAscii[x][y].glyph);
          }
        }
      }
    }
    postmortemLines.push_back(StringAndColor(currentRow, clrInfo));
    currentRow.clear();
  }

  tracer << "Postmortem::makeInfoLines() [DONE]" << endl;
}

void Postmortem::renderInfo(const int TOP_ELEMENT) {
  eng->renderer->coverRenderArea(renderArea_screen);
  const string decorationLine(MAP_X_CELLS - 2, '-');
  eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);
  eng->renderer->drawText(" Displaying postmortem information ", renderArea_screen, 3, 1, clrWhite);
  eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);
  eng->renderer->drawText(" 2/8, down/up to navigate | space/esc to exit  ", renderArea_characterLines, 3, 1, clrWhite);
  int x = 0;
  int y = 0;
  for(unsigned int i = TOP_ELEMENT; i < postmortemLines.size() && (i - TOP_ELEMENT) <= static_cast<unsigned int>(MAP_Y_CELLS); i++) {
    eng->renderer->drawText(postmortemLines.at(i).str, renderArea_mainScreen, x, y, postmortemLines.at(i).clr);
    y++;
  }

  eng->renderer->updateScreen();
}

void Postmortem::runInfo() {
  int topElement = 0;
  renderInfo(topElement);

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2') {
      topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(postmortemLines.size()) - static_cast<int>(MAP_Y_CELLS)));
      renderInfo(topElement);
    } else if(d.sdlKey_ == SDLK_UP || d.key_ == '8') {
      topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(postmortemLines.size()) - static_cast<int>(MAP_Y_CELLS)));
      renderInfo(topElement);
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }
  }
}

void Postmortem::makeMemorialFile() {
  const string timeStamp = eng->dungeonMaster->getTimeStarted().getTimeStr(time_second, false);
  const string memorialFileName = eng->player->getNameA() + "_" + timeStamp + ".txt";
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
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        renderMenu(browser);
      }
      break;
      case menuAction_canceled: {
      }
      break;
      case menuAction_selected: {
        if(browser.isPosAtKey('a')) {
          runInfo();
          renderMenu(browser);
        }
        if(browser.isPosAtKey('b')) {
          eng->highScore->runHighScoreScreen();
          renderMenu(browser);
        }
        if(browser.isPosAtKey('c')) {
          eng->log->displayHistory();
          renderMenu(browser);
        }
        if(browser.isPosAtKey('d')) {
          done = true;
        }
        if(browser.isPosAtKey('e')) {
          *quitGame = true;
          done = true;
        }
      }
      break;
      case menuAction_selectedWithShift:
      {} break;
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
    tracer << "[WARNING] Could not open ascii graveyard file, in Postmortem::renderMenu()" << endl;
  }

  file.close();

  eng->renderer->coverRenderArea(renderArea_screen);

  int x = 1;
  int y = 1;

  for(unsigned int i = 0; i < art.size(); i++) {
    eng->renderer->drawText(art.at(i), renderArea_screen, x, y, clrWhiteHigh);
    y += 1;
  }

  x = 45;
  y = 18;
  const string NAME_STR = eng->player->getDef()->name_a;
  eng->renderer->drawTextCentered(NAME_STR, renderArea_screen, x, y, clrWhiteHigh);

  y += 2;
  const string LVL_STR = "LVL " + intToString(eng->dungeonMaster->getLevel());
  eng->renderer->drawTextCentered(LVL_STR, renderArea_screen, x, y, clrWhiteHigh);

  //Draw command labels
  x = 55;
  y = 14;
  eng->renderer->drawText("a) Information", renderArea_screen, x, y, browser.isPosAtKey('a') ? clrWhite : clrRedLgt);
  y += 1;

  eng->renderer->drawText("b) View the High Score", renderArea_screen, x, y, browser.isPosAtKey('b') ? clrWhite : clrRedLgt);
  y += 1;

  eng->renderer->drawText("c) View messages", renderArea_screen, x, y, browser.isPosAtKey('c') ? clrWhite : clrRedLgt);
  y += 1;

  eng->renderer->drawText("d) Return to main menu", renderArea_screen, x, y, browser.isPosAtKey('d') ? clrWhite : clrRedLgt);
  y += 1;

  eng->renderer->drawText("e) Quit the game", renderArea_screen, x, y, browser.isPosAtKey('e') ? clrWhite : clrRedLgt);
  y += 1;

  eng->renderer->updateScreen();
}
