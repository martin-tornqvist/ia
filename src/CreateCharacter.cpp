#include "CreateCharacter.h"

#include "Render.h"
#include "ActorPlayer.h"
#include "Input.h"
#include "MenuInputHandling.h"
#include "TextFormatting.h"
#include "Utils.h"
#include "Map.h"

using namespace std;

namespace CreateCharacter {

namespace {

namespace EnterName {

void draw(const string& curString) {
  Render::clearScreen();
  Render::drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  Render::drawTextCentered("What is your name?", Panel::screen,
                             Pos(MAP_W_HALF, 0), clrWhite);
  const int Y_NAME = 2;
  const string NAME_STR =
    curString.size() < PLAYER_NAME_MAX_LENGTH ? curString + "_" :
    curString;
  const size_t NAME_X0 = MAP_W_HALF - (PLAYER_NAME_MAX_LENGTH / 2);
  const size_t NAME_X1 = NAME_X0 + PLAYER_NAME_MAX_LENGTH - 1;
  Render::drawText(NAME_STR, Panel::screen, Pos(NAME_X0, Y_NAME),
                     clrMenuHighlight);
  Rect boxRect(Pos(NAME_X0 - 1, Y_NAME - 1), Pos(NAME_X1 + 1, Y_NAME + 1));
  Render::drawPopupBox(boxRect);
  Render::updateScreen();
}

void readKeys(string& curString, bool& isDone) {
  const KeyData& d = Input::readKeysUntilFound(false);

  if(d.sdlKey == SDLK_RETURN) {
    isDone = true;
    curString = curString.empty() ? "Player" : curString;
    return;
  }

  if(curString.size() < PLAYER_NAME_MAX_LENGTH) {
    if(
      d.sdlKey == SDLK_SPACE ||
      (d.key >= int('a') && d.key <= int('z')) ||
      (d.key >= int('A') && d.key <= int('Z')) ||
      (d.key >= int('0') && d.key <= int('9'))) {
      if(d.sdlKey == SDLK_SPACE) {
        curString.push_back(' ');
      } else {
        curString.push_back(char(d.key));
      }
      draw(curString);
      return;
    }
  }

  if(curString.size() > 0) {
    if(d.sdlKey == SDLK_BACKSPACE) {
      curString.erase(curString.end() - 1);
      draw(curString);
    }
  }
}

void run() {
  string name = "";
  draw(name);
  bool isDone = false;
  while(!isDone) {
    if(Config::isBotPlaying()) {
      name = "AZATHOTH";
      isDone = true;
    } else {
      readKeys(name, isDone);
    }
  }
  ActorDataT& def = Map::player->getData();
  def.nameA      = def.nameThe = name;
}

} //EnterName

void drawPickBg(const vector<Bg>& bgs, const MenuBrowser& browser) {
  Render::clearScreen();
  Render::drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  Render::drawTextCentered("Choose your background", Panel::screen,
                             Pos(MAP_W_HALF, 0), clrWhite, clrBlack, true);

  const Pos& browserPos = browser.getPos();

  const int Y0_BGS = 2;

  int y = Y0_BGS;

  const Bg markedBg = bgs.at(browserPos.y);

  const int NR_BGS = bgs.size();

  //------------------------------------------------------------- BACKGROUNDS
  for(int i = 0; i < NR_BGS; ++i) {
    const Bg bg = bgs.at(i);
    string name = "";
    PlayerBon::getBgTitle(bg, name);
    const bool IS_MARKED  = bg == markedBg;
    const Clr& drwClr     = IS_MARKED ? clrMenuHighlight : clrMenuDrk;
    Render::drawTextCentered(name, Panel::screen, Pos(MAP_W_HALF, y), drwClr);
    y++;
  }
  y++;

  const int BGS_BOX_W_HALF = 7;
  Rect boxRect(Pos(MAP_W_HALF - BGS_BOX_W_HALF, Y0_BGS - 1),
               Pos(MAP_W_HALF + BGS_BOX_W_HALF, Y0_BGS + NR_BGS));
  Render::drawPopupBox(boxRect);

  //------------------------------------------------------------- DESCRIPTION
  const int MARGIN_W_DESCR  = 12;
  const int X0_DESCR        = MARGIN_W_DESCR;
  const int MAX_W_DESCR     = MAP_W - (MARGIN_W_DESCR * 2);

  vector<string> rawDescrLines;
  PlayerBon::getBgDescr(markedBg, rawDescrLines);
  for(string& rawLine : rawDescrLines) {
    vector<string> formattedLines;
    TextFormatting::lineToLines(rawLine, MAX_W_DESCR, formattedLines);
    for(string& line : formattedLines) {
      Render::drawText(line, Panel::screen, Pos(X0_DESCR, y), clrWhite);
      y++;
    }
  }
  Render::updateScreen();
}

void pickBg() {
  if(Config::isBotPlaying()) {
    PlayerBon::pickBg(Bg(Rnd::range(0, int(Bg::END) - 1)));
  } else {
    vector<Bg> bgs;
    PlayerBon::getPickableBgs(bgs);

    MenuBrowser browser(bgs.size(), 0);
    drawPickBg(bgs, browser);

    while(true) {
      const MenuAction action = MenuInputHandling::getAction(browser);
      switch(action) {
        case MenuAction::browsed: {drawPickBg(bgs, browser);} break;

        case MenuAction::esc:
        case MenuAction::space: {} break;

        case MenuAction::selected: {
          PlayerBon::pickBg(bgs.at(browser.getPos().y));
          return;
        } break;

        case MenuAction::selectedShift: {} break;
      }
    }
  }
}

void drawPickTrait(
  const vector<Trait>& traits1, const vector<Trait>& traits2,
  const MenuBrowser& browser, const bool IS_CHARACTER_CREATION) {

  Render::clearScreen();
  Render::drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  const int NR_TRAITS_1 = traits1.size();
  const int NR_TRAITS_2 = traits2.size();

  int lenOfLongestInCol2 = -1;
  for(const Trait& id : traits2) {
    string title = "";
    PlayerBon::getTraitTitle(id, title);
    const int CUR_LEN = title.length();
    if(CUR_LEN > lenOfLongestInCol2) {lenOfLongestInCol2 = CUR_LEN;}
  }

  const int MARGIN_W        = 19;
  const int X_COL_ONE       = MARGIN_W;
  const int X_COL_TWO_RIGHT = MAP_W - MARGIN_W - 1;
  const int X_COL_TWO       = X_COL_TWO_RIGHT - lenOfLongestInCol2 + 1;

  string title = IS_CHARACTER_CREATION ?
                 "Which additional trait do you start with?" :
                 "You have reached a new level! Which trait do you gain?";

  Render::drawTextCentered(title, Panel::screen, Pos(MAP_W_HALF, 0),
                             clrWhite, clrBlack, true);

  const Pos& browserPos = browser.getPos();

  //------------------------------------------------------------- TRAITS
  const int Y0_TRAITS = 2;
  int y = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_1; ++i) {
    const Trait trait = traits1.at(i);
    string name = "";
    PlayerBon::getTraitTitle(trait, name);
    const bool IS_MARKED  = browserPos.x == 0 && browserPos.y == int(i);
    const Clr& drwClr     = IS_MARKED ? clrMenuHighlight : clrMenuDrk;
    Render::drawText(name, Panel::screen, Pos(X_COL_ONE, y), drwClr);
    y++;
  }
  y = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_2; ++i) {
    const Trait trait = traits2.at(i);
    string name = "";
    PlayerBon::getTraitTitle(trait, name);
    const bool IS_MARKED  = browserPos.x == 1 && browserPos.y == int(i);
    const Clr& drwClr     = IS_MARKED ? clrMenuHighlight : clrMenuDrk;
    Render::drawText(name, Panel::screen, Pos(X_COL_TWO, y), drwClr);
    y++;
  }

  //Draw frame around traits
  Rect boxRect(
    Pos(MARGIN_W - 2, Y0_TRAITS - 1),
    Pos(X_COL_TWO_RIGHT + 2, Y0_TRAITS + traits1.size()));
  Render::drawPopupBox(boxRect);

  //------------------------------------------------------------- DESCRIPTION
  const int Y0_DESCR = Y0_TRAITS + NR_TRAITS_1 + 1;
  const int X0_DESCR = X_COL_ONE;
  y = Y0_DESCR;
  const Trait markedTrait =
    browserPos.x == 0 ? traits1.at(browserPos.y) :
    traits2.at(browserPos.y);
  string descr = "";
  PlayerBon::getTraitDescr(markedTrait, descr);
  const int MAX_W_DESCR = X_COL_TWO_RIGHT - X_COL_ONE + 1;
  vector<string> descrLines;
  TextFormatting::lineToLines(
    "Effect(s): " + descr, MAX_W_DESCR, descrLines);
  for(const string& str : descrLines) {
    Render::drawText(str, Panel::screen, Pos(X0_DESCR, y), clrWhite);
    y++;
  }

  //------------------------------------------------------------- PREREQUISITES
  const int Y0_PREREQS = 17;
  y = Y0_PREREQS;
  vector<Trait> traitPrereqs;
  Bg bgPrereq = Bg::END;
  PlayerBon::getTraitPrereqs(markedTrait, traitPrereqs, bgPrereq);
  if(!traitPrereqs.empty() || bgPrereq != Bg::END) {
    Render::drawText("This trait had the following prerequisite(s):",
                       Panel::screen, Pos(X0_DESCR, y), clrWhite);
    y++;

    string prereqStr = "";

    if(bgPrereq != Bg::END) {
      PlayerBon::getBgTitle(bgPrereq, prereqStr);
    }

    for(Trait prereqTrait : traitPrereqs) {
      string prereqTitle = "";
      PlayerBon::getTraitTitle(prereqTrait, prereqTitle);
      prereqStr += (prereqStr.empty() ? "" : ", ") + prereqTitle;
    }

    vector<string> prereqLines;
    TextFormatting::lineToLines(prereqStr, MAX_W_DESCR, prereqLines);
    for(const string& str : prereqLines) {
      Render::drawText(str, Panel::screen, Pos(X0_DESCR, y), clrWhite);
      y++;
    }
  }

  //------------------------------------------------------------- PREVIOUS
  y = Y0_PREREQS + 4;
  const int MAX_W_PREV_PICKS  = SCREEN_W - 2;
  string pickedStr = "";
  PlayerBon::getAllPickedTraitsTitlesLine(pickedStr);
  if(pickedStr != "") {
    pickedStr = "Trait(s) gained: " + pickedStr;
    vector<string> pickedLines;
    TextFormatting::lineToLines(pickedStr, MAX_W_PREV_PICKS, pickedLines);
    for(const string& str : pickedLines) {
      Render::drawText(
        str, Panel::screen, Pos(1, y), clrWhite);
      y++;
    }
  }

  Render::updateScreen();
}

} //namespace

void createCharacter() {
  pickBg();
  pickNewTrait(true);
  EnterName::run();
}

void pickNewTrait(const bool IS_CHARACTER_CREATION) {
  if(!Config::isBotPlaying()) {
    vector<Trait> pickableTraits;
    PlayerBon::getPickableTraits(pickableTraits);

    if(!pickableTraits.empty()) {

      const int NR_TRAITS_TOT = int(pickableTraits.size());
      const int NR_TRAITS_2   = NR_TRAITS_TOT / 2;
      const int NR_TRAITS_1   = NR_TRAITS_TOT - NR_TRAITS_2;

      vector<Trait> traits1; traits1.clear();
      vector<Trait> traits2; traits2.clear();

      for(int i = 0; i < NR_TRAITS_TOT; ++i) {
        const Trait trait = pickableTraits.at(i);
        if(i < NR_TRAITS_1) {
          traits1.push_back(trait);
        } else {
          traits2.push_back(trait);
        }
      }

      MenuBrowser browser(traits1.size(), traits2.size());
      drawPickTrait(traits1, traits2, browser, IS_CHARACTER_CREATION);

      while(true) {
        const MenuAction action = MenuInputHandling::getAction(browser);
        switch(action) {
          case MenuAction::browsed: {
            drawPickTrait(traits1, traits2, browser, IS_CHARACTER_CREATION);
          } break;

          case MenuAction::esc:
          case MenuAction::space: {} break;

          case MenuAction::selected: {
            const Pos pos = browser.getPos();
            PlayerBon::pickTrait(
              pos.x == 0 ? traits1.at(pos.y) : traits2.at(pos.y));
            if(!IS_CHARACTER_CREATION) {Render::drawMapAndInterface();}
            return;
          } break;

          case MenuAction::selectedShift: {} break;
        }
      }
    }
  }
}

} //CreateCharacter
