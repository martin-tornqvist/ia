#include "Postmortem.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "Init.h"
#include "Renderer.h"
#include "Input.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Log.h"
#include "MenuInputHandling.h"
#include "Highscore.h"
#include "PlayerBon.h"
#include "TextFormatting.h"
#include "MenuBrowser.h"

using namespace std;

namespace Postmortem {

namespace {

struct StrAndClr {
  StrAndClr(const string str_, const SDL_Color clr_) : str(str_), clr(clr_) {}
  StrAndClr() {}
  std::string str;
  SDL_Color   clr;
};

void mkInfoLines(vector<StrAndClr>& linesRef) {
  TRACE_FUNC_BEGIN;

  const SDL_Color clrHeading  = clrWhiteHigh;
  const SDL_Color clrInfo     = clrWhite;

  TRACE << "Postmortem: Finding number of killed monsters" << endl;
  vector<string> uniqueKilledNames;
  int nrKillsTotAllMonsters = 0;
  for(int i = actor_player + 1; i < endOfActorIds; i++) {
    const ActorDataT& d = ActorData::data[i];
    if(d.nrKills > 0) {
      nrKillsTotAllMonsters += d.nrKills;
      if(d.isUnique) {
        uniqueKilledNames.push_back(d.name_a);
      }
    }
  }

  linesRef.push_back(StrAndClr(" " + Map::player->getNameA(), clrHeading));

  linesRef.push_back(StrAndClr("   * Explored to the depth of dungeon level " +
                               toStr(Map::dlvl), clrInfo));
  linesRef.push_back(StrAndClr("   * Was " +
                               toStr(min(100, Map::player->getInsanity())) + "% "
                               "insane", clrInfo));
  linesRef.push_back(StrAndClr("   * Killed " + toStr(nrKillsTotAllMonsters) +
                               " monsters ", clrInfo));

  //TODO This is ugly as hell
  if(Map::player->phobias[int(Phobia::closedPlace)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of enclosed spaces", clrInfo));
  if(Map::player->phobias[int(Phobia::dog)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of dogs", clrInfo));
  if(Map::player->phobias[int(Phobia::rat)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of rats", clrInfo));
  if(Map::player->phobias[int(Phobia::undead)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of the dead", clrInfo));
  if(Map::player->phobias[int(Phobia::openPlace)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of open places", clrInfo));
  if(Map::player->phobias[int(Phobia::spider)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of spiders", clrInfo));
  if(Map::player->phobias[int(Phobia::deepPlaces)])
    linesRef.push_back(
      StrAndClr("   * Had a phobia of deep places", clrInfo));

  if(Map::player->obsessions[int(Obsession::masochism)])
    linesRef.push_back(StrAndClr("   * Had a masochistic obsession", clrInfo));
  if(Map::player->obsessions[int(Obsession::sadism)])
    linesRef.push_back(StrAndClr("   * Had a sadistic obsession", clrInfo));

  linesRef.push_back(StrAndClr(" ", clrInfo));

  TRACE << "Postmortem: Finding traits gained" << endl;
  linesRef.push_back(StrAndClr(" Traits gained:", clrHeading));
  string traitsLine;
  PlayerBon::getAllPickedTraitsTitlesLine(traitsLine);
  if(traitsLine.empty()) {
    linesRef.push_back(StrAndClr("   * None", clrInfo));
  } else {
    vector<string> abilitiesLines;
    TextFormatting::lineToLines(traitsLine, 60, abilitiesLines);
    for(string& str : abilitiesLines) {
      linesRef.push_back(StrAndClr("   " + str, clrInfo));
    }
  }
  linesRef.push_back(StrAndClr(" ", clrInfo));

  linesRef.push_back(StrAndClr(" Unique monsters killed:", clrHeading));
  if(uniqueKilledNames.empty()) {
    linesRef.push_back(StrAndClr("   * None", clrInfo));
  } else {
    for(string& monsterName : uniqueKilledNames) {
      linesRef.push_back(StrAndClr("   * " + monsterName, clrInfo));
    }
  }
  linesRef.push_back(StrAndClr(" ", clrInfo));

  linesRef.push_back(StrAndClr(" The last messages:", clrHeading));
  const vector< vector<Msg> >& history = Log::getHistory();
  int historyElement = max(0, int(history.size()) - 20);
  for(unsigned int i = historyElement; i < history.size(); i++) {
    string row = "";
    for(unsigned int ii = 0; ii < history.at(i).size(); ii++) {
      string msgStr = "";
      history.at(i).at(ii).getStrWithRepeats(msgStr);
      row += msgStr + " ";
    }
    linesRef.push_back(StrAndClr("   " + row, clrInfo));
  }
  linesRef.push_back(StrAndClr(" ", clrInfo));

  TRACE << "Postmortem: Drawing the final map" << endl;
  linesRef.push_back(StrAndClr(" The final moment:", clrHeading));
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
          if(Utils::isPosInsideMap(Pos(x + dx, y + dy))) {
            const auto* const f = Map::cells[x + dx][y + dy].featureStatic;
            if(f->isVisionPassable()) {Map::cells[x][y].isSeenByPlayer = true;}
          }
        }
      }
    }
  }
  Renderer::drawMap(); //To set the glyph array
  for(int y = 0; y < MAP_H; y++) {
    string curRow = "";
    for(int x = 0; x < MAP_W; x++) {
      if(Pos(x, y) == Map::player->pos) {
        curRow.push_back('@');
      } else {
        if(
          Renderer::renderArray[x][y].glyph == ' ' &&
          (y == 0 || x == 0 || y == MAP_H - 1 || x == MAP_W - 1)) {
          curRow.push_back('*');
        } else {
          if(Renderer::renderArray[x][y].glyph ==
              FeatureData::getData(FeatureId::wall)->glyph
              || Renderer::renderArray[x][y].glyph ==
              FeatureData::getData(FeatureId::rubbleHigh)->glyph) {
            curRow.push_back('#');
          } else if(Renderer::renderArray[x][y].glyph ==
                    FeatureData::getData(FeatureId::statue)->glyph) {
            curRow.push_back('M');
          } else {
            curRow.push_back(Renderer::renderArray[x][y].glyph);
          }
        }
      }
    }
    linesRef.push_back(StrAndClr(curRow, clrInfo));
    curRow.clear();
  }

  TRACE_FUNC_END;
  TRACE_FUNC_END;
}

void render(const vector<StrAndClr>& linesAndClr,
            const int TOP_ELEMENT) {
  Renderer::clearScreen();

  const string decorationLine(MAP_W, '-');
  Renderer::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  const int X_LABEL = 3;

  Renderer::drawText(" Displaying postmortem information ", Panel::screen,
                     Pos(X_LABEL, 0), clrGray);

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

  Renderer::drawText(" 2/8, down/up, j/k to navigate | space/esc to exit  ",
                     Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);

  const int NR_LINES_TOT = int(linesAndClr.size());
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
  int yPos = 1;

  for(
    int i = TOP_ELEMENT;
    i < NR_LINES_TOT && (i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR;
    i++) {
    Renderer::drawText(linesAndClr.at(i).str, Panel::screen, Pos(0, yPos++),
                       linesAndClr.at(i).clr);
  }

  Renderer::updateScreen();
}

void runInfo(const vector<StrAndClr>& lines) {
  const int LINE_JUMP           = 3;
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
  const int NR_LINES_TOT        = lines.size();

  int topNr = 0;

  while(true) {
    render(lines, topNr);

    const KeyboardReadRetData& d = Input::readKeysUntilFound();

    if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2' || d.key_ == 'j') {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR) {
        topNr = 0;
      } else {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    } else if(d.sdlKey_ == SDLK_UP || d.key_ == '8' || d.key_ == 'k') {
      topNr = max(0, topNr - LINE_JUMP);
    } else if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      break;
    }
  }
}

void mkMemorialFile(const vector<StrAndClr>& lines) {
  const string timeStamp =
    DungeonMaster::getTimeStarted().getTimeStr(time_second, false);
  const string memorialFileName =
    Map::player->getNameA() + "_" + timeStamp + ".txt";
  const string memorialFilePath = "data/" + memorialFileName;

  // Add memorial file
  ofstream file;
  file.open(memorialFilePath.data(), ios::trunc);
  for(const StrAndClr& line : lines) {file << line.str << endl;}
  file.close();

  // Add reference to memorial file in list
  const string memorialList = "data/memorialFileList";
  file.open(memorialList.data(), ios::app);
  file << memorialFileName << endl;
  file.close();
}

void renderMenu(const MenuBrowser& browser) {
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
    TRACE << "[WARNING] Could not open ascii graveyard file, "
          "in Postmortem::renderMenu()" << endl;
  }

  file.close();

  Renderer::coverPanel(Panel::screen);

  Pos pos(1, 1);

  for(unsigned int i = 0; i < art.size(); i++) {
    Renderer::drawText(art.at(i), Panel::screen, pos, clrWhiteHigh);
    pos.y++;
  }

  pos.set(45, 18);
  const string NAME_STR = Map::player->getData().name_a;
  Renderer::drawTextCentered(NAME_STR, Panel::screen, pos, clrWhiteHigh);

//  pos.y += 2;
//  const string LVL_STR = "LVL " + toStr(DungeonMaster::getLvl());
//  Renderer::drawTextCentered(LVL_STR, Panel::screen, pos, clrWhiteHigh);

  //Draw command labels
  pos.set(55, 14);
  Renderer::drawText("Information", Panel::screen, pos,
                     browser.isPosAtElement(0) ? clrWhite : clrRedLgt);
  pos.y++;

  Renderer::drawText("View the High Score", Panel::screen, pos,
                     browser.isPosAtElement(1) ? clrWhite : clrRedLgt);
  pos.y++;

  Renderer::drawText("View messages", Panel::screen, pos,
                     browser.isPosAtElement(2) ? clrWhite : clrRedLgt);
  pos.y++;

  Renderer::drawText("Return to main menu", Panel::screen, pos,
                     browser.isPosAtElement(3) ? clrWhite : clrRedLgt);
  pos.y++;

  Renderer::drawText("Quit the game", Panel::screen, pos,
                     browser.isPosAtElement(4) ? clrWhite : clrRedLgt);
  pos.y++;

  Renderer::updateScreen();
}

void readKeysMenu(const vector<StrAndClr>& linesAndClr, bool* const quitGame) {
  MenuBrowser browser(5, 0);

  renderMenu(browser);

  bool done = false;
  while(!done) {
    const MenuAction action = MenuInputHandling::getAction(browser);
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
          runInfo(linesAndClr);
          renderMenu(browser);
        }
        if(browser.isPosAtElement(1)) {
          HighScore::runHighScoreScreen();
          renderMenu(browser);
        }
        if(browser.isPosAtElement(2)) {
          Log::displayHistory();
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

} //namespace

void run(bool* const quitGame) {
  vector<StrAndClr> lines;
  mkInfoLines(lines);
  mkMemorialFile(lines);
  readKeysMenu(lines, quitGame);
}

} //Postmortem
