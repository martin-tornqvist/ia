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
#include "PlayerBonuses.h"
#include "TextFormatting.h"

void Postmortem::run(bool* const quitGame) {
  makeInfoLines();

  makeMemorialFile();

  readKeysMenu(quitGame);
}

void Postmortem::makeInfoLines() {
  tracer << "Postmortem::makeInfoLines()..." << endl;

  const SDL_Color clrHeading  = clrWhiteHigh;
  const SDL_Color clrInfo     = clrWhite;

  postmortemLines.push_back(
    StringAndClr(" " + eng->player->getNameA(), clrHeading));

  postmortemLines.push_back(
    StringAndClr("   * Explored to the depth of dungeon level " +
                 intToString(eng->map->getDLVL()), clrInfo));
  postmortemLines.push_back(
    StringAndClr("   * Was " +
                 intToString(min(100, eng->player->getInsanity())) +
                 "% insane", clrInfo));

  //TODO Make some sort of insanity class or something where this info is stored,
  //this shit is ugly as hell
  if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of enclosed spaces", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_dog])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of dogs", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_rat])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of rats", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_undead])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of the dead", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_openPlace])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of open places", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_spider])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of spiders", clrInfo));
  if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
    postmortemLines.push_back(
      StringAndClr("   * Had a phobia of deep places", clrInfo));

  if(eng->player->insanityObsessions[insanityObsession_masochism])
    postmortemLines.push_back(
      StringAndClr("   * Had a masochistic obsession", clrInfo));
  if(eng->player->insanityObsessions[insanityObsession_sadism])
    postmortemLines.push_back(
      StringAndClr("   * Had a sadistic obsession", clrInfo));

  postmortemLines.push_back(StringAndClr(" ", clrInfo));


  tracer << "Postmortem: Listing abilities gained" << endl;
  postmortemLines.push_back(StringAndClr(" Abilities gained:", clrHeading));
  string abilitiesLine;
  eng->playerBonHandler->getAllPickedBonTitlesLine(abilitiesLine);
  if(abilitiesLine == "") {
    postmortemLines.push_back(StringAndClr("   * None", clrInfo));
  } else {
    vector<string> abilitiesLines =
      eng->textFormatting->lineToLines(abilitiesLine, 60);
    for(unsigned int i = 0; i < abilitiesLines.size(); i++) {
      postmortemLines.push_back(
        StringAndClr("   " + abilitiesLines.at(i), clrInfo));
    }
  }
  postmortemLines.push_back(StringAndClr(" ", clrInfo));

  tracer << "Postmortem: Listing monster kills" << endl;
  // TODO Add name_plural_a, and name_plural_the to actor defs?
  postmortemLines.push_back(StringAndClr(" Monsters killed:", clrHeading));
  vector< pair<string, int> > killList;
  int nrOfTotalKills = 0;
  for(unsigned int i = actor_player + 1; i < endOfActorIds; i++) {
    const ActorData& d = eng->actorDataHandler->dataList[i];
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
    postmortemLines.push_back(StringAndClr("   * None", clrInfo));
  } else {
    postmortemLines.back().str += " (" + intToString(nrOfTotalKills) + " total)";

    for(unsigned int i = 0; i < killList.size(); i++) {
      const string name = killList.at(i).first;
      const int nrOfKills = killList.at(i).second;
      const string nrOfKillsStr =
        nrOfKills == -1 ? "" : (": " + intToString(nrOfKills));
      postmortemLines.push_back(
        StringAndClr("   * " + name + nrOfKillsStr, clrInfo));
    }
  }
  postmortemLines.push_back(StringAndClr(" ", clrInfo));

  postmortemLines.push_back(StringAndClr(" The last messages:", clrHeading));
  int historyElement = static_cast<unsigned int>(
                         max(0, int(eng->log->history.size()) - 10));
  for(unsigned int i = historyElement; i < eng->log->history.size(); i++) {
    string row = "";
    for(unsigned int ii = 0; ii < eng->log->history.at(i).size(); ii++) {
      row += eng->log->history.at(i).at(ii).str + " ";
    }
    postmortemLines.push_back(StringAndClr("   " + row, clrInfo));
  }
  postmortemLines.push_back(StringAndClr(" ", clrInfo));

  tracer << "Postmortem: Drawing the final map" << endl;
  postmortemLines.push_back(StringAndClr(" The final moment:", clrHeading));
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
          if(eng->mapTests->isCellInsideMap(Pos(x + dx, y + dy))) {
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
    string currentRow = "";
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(Pos(x, y) == eng->player->pos) {
        currentRow.push_back('@');
      } else {
        if(
          eng->renderer->renderArrayAscii[x][y].glyph == ' ' &&
          (y == 0 || x == 0 || y == MAP_Y_CELLS - 1 || x == MAP_X_CELLS - 1)) {
          currentRow.push_back('*');
        } else {
          if(eng->renderer->renderArrayAscii[x][y].glyph ==
              eng->featureDataHandler->getData(feature_stoneWall)->glyph) {
            currentRow.push_back('#');
          } else {
            currentRow.push_back(eng->renderer->renderArrayAscii[x][y].glyph);
          }
        }
      }
    }
    postmortemLines.push_back(StringAndClr(currentRow, clrInfo));
    currentRow.clear();
  }

  tracer << "Postmortem::makeInfoLines() [DONE]" << endl;
}

void Postmortem::renderInfo(const int TOP_ELEMENT) {
  eng->renderer->coverPanel(panel_screen);
  const string decorationLine(MAP_X_CELLS - 2, '-');
  eng->renderer->drawText(decorationLine, panel_screen, Pos(1, 1), clrWhite);
  eng->renderer->drawText(
    " Displaying postmortem information ", panel_screen, Pos(3, 1), clrWhite);
  eng->renderer->drawText(
    decorationLine, panel_character, Pos(1, 1), clrWhite);
  eng->renderer->drawText(
    " 2/8, down/up to navigate | space/esc to exit  ",
    panel_character, Pos(3, 1), clrWhite);
  int x = 0;
  int y = 0;
  const int NR = int(postmortemLines.size());

  for(int i = TOP_ELEMENT; i < NR && (i - TOP_ELEMENT) <= MAP_Y_CELLS; i++) {
    eng->renderer->drawText(
      postmortemLines.at(i).str, panel_map, Pos(x, y),
      postmortemLines.at(i).clr);
    y++;
  }

  eng->renderer->updateScreen();
}

void Postmortem::runInfo() {
  int topElement = 0;
  renderInfo(topElement);

  const int SCROLL_LINES = 3;

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2') {
      topElement = min(
                     topElement + SCROLL_LINES,
                     int(postmortemLines.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      renderInfo(topElement);
    } else if(d.sdlKey_ == SDLK_UP || d.key_ == '8') {
      topElement = min(
                     topElement - SCROLL_LINES,
                     int(postmortemLines.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      renderInfo(topElement);
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
    }
  }
}

void Postmortem::makeMemorialFile() {
  const string timeStamp =
    eng->dungeonMaster->getTimeStarted().getTimeStr(time_second, false);
  const string memorialFileName =
    eng->player->getNameA() + "_" + timeStamp + ".txt";
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
    tracer << "[WARNING] Could not open ascii graveyard file, ";
    tracer << "in Postmortem::renderMenu()" << endl;
  }

  file.close();

  eng->renderer->coverPanel(panel_screen);

  Pos pos(1, 1);

  for(unsigned int i = 0; i < art.size(); i++) {
    eng->renderer->drawText(art.at(i), panel_screen, pos, clrWhiteHigh);
    pos.y += 1;
  }

  pos.set(45, 18);
  const string NAME_STR = eng->player->getData()->name_a;
  eng->renderer->drawTextCentered(NAME_STR, panel_screen, pos, clrWhiteHigh);

//  pos.y += 2;
//  const string LVL_STR = "LVL " + intToString(eng->dungeonMaster->getLevel());
//  eng->renderer->drawTextCentered(LVL_STR, panel_screen, pos, clrWhiteHigh);

  //Draw command labels
  pos.set(55, 14);
  eng->renderer->drawText(
    "a) Information", panel_screen, pos,
    browser.isPosAtKey('a') ? clrWhite : clrRedLgt);
  pos.y += 1;

  eng->renderer->drawText(
    "b) View the High Score", panel_screen, pos,
    browser.isPosAtKey('b') ? clrWhite : clrRedLgt);
  pos.y += 1;

  eng->renderer->drawText(
    "c) View messages", panel_screen, pos,
    browser.isPosAtKey('c') ? clrWhite : clrRedLgt);
  pos.y += 1;

  eng->renderer->drawText(
    "d) Return to main menu", panel_screen, pos,
    browser.isPosAtKey('d') ? clrWhite : clrRedLgt);
  pos.y += 1;

  eng->renderer->drawText(
    "e) Quit the game", panel_screen, pos,
    browser.isPosAtKey('e') ? clrWhite : clrRedLgt);
  pos.y += 1;

  eng->renderer->updateScreen();
}
