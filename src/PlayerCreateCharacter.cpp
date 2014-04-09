#include "PlayerCreateCharacter.h"

#include "Engine.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Input.h"
#include "MenuInputHandler.h"
#include "TextFormatting.h"
#include "Utils.h"

void PlayerCreateCharacter::createCharacter() const {
  pickBg();
  pickNewTrait(true);
  PlayerEnterName(eng).run();
}

void PlayerCreateCharacter::pickBg() const {
  if(Config::isBotPlaying()) {
    PlayerBon::pickBg(Bg(Rnd::range(0, int(Bg::endOfBgs) - 1)), eng);
  } else {
    vector<Bg> bgs;
    PlayerBon::getPickableBgs(bgs);

    MenuBrowser browser(bgs.size(), 0);
    drawPickBg(bgs, browser);

    while(true) {
      const MenuAction action = eng.menuInputHandler->getAction(browser);
      switch(action) {
        case MenuAction::browsed: {drawPickBg(bgs, browser);} break;

        case MenuAction::esc:
        case MenuAction::space: {} break;

        case MenuAction::selected: {
          PlayerBon::pickBg(bgs.at(browser.getPos().y), eng);
          return;
        } break;

        case MenuAction::selectedShift: {} break;
      }
    }
  }
}

void PlayerCreateCharacter::drawPickBg(const vector<Bg>& bgs,
                                       const MenuBrowser& browser) const {
  Renderer::clearScreen();
  Renderer::drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  Renderer::drawTextCentered("Choose your background", Panel::screen,
                             Pos(MAP_W_HALF, 0), clrWhite, clrBlack, true);

  const Pos& browserPos = browser.getPos();

  const SDL_Color& clrActive      = clrNosfTealLgt;
  const SDL_Color& clrInactive    = clrNosfTealDrk;
  const SDL_Color& clrActiveBg    = clrBlack;
  const SDL_Color& clrInactiveBg  = clrBlack;

  const int Y0_BGS = 2;

  int y = Y0_BGS;

  const Bg markedBg = bgs.at(browserPos.y);

  const int NR_BGS = bgs.size();

  //------------------------------------------------------------- BACKGROUNDS
  for(int i = 0; i < NR_BGS; i++) {
    const Bg bg = bgs.at(i);
    string name = "";
    PlayerBon::getBgTitle(bg, name);
    const bool IS_MARKED = bg == markedBg;
    const SDL_Color& drwClr   = IS_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg = IS_MARKED ? clrActiveBg : clrInactiveBg;
    Renderer::drawTextCentered(name, Panel::screen, Pos(MAP_W_HALF, y),
                               drwClr, drwClrBg);
    y++;
  }
  y++;

  const int BGS_BOX_W_HALF = 7;
  Rect boxRect(Pos(MAP_W_HALF - BGS_BOX_W_HALF, Y0_BGS - 1),
               Pos(MAP_W_HALF + BGS_BOX_W_HALF, Y0_BGS + NR_BGS));
  Renderer::drawPopupBox(boxRect);

  //------------------------------------------------------------- DESCRIPTION
  const int MARGIN_W_DESCR  = 12;
  const int X0_DESCR        = MARGIN_W_DESCR;
  const int MAX_W_DESCR     = MAP_W - (MARGIN_W_DESCR * 2);

  vector<string> rawDescrLines;
  PlayerBon::getBgDescr(markedBg, rawDescrLines);
  for(string & rawLine : rawDescrLines) {
    vector<string> formattedLines;
    TextFormatting::lineToLines(rawLine, MAX_W_DESCR, formattedLines);
    for(string & line : formattedLines) {
      Renderer::drawText(line, Panel::screen, Pos(X0_DESCR, y), clrWhite);
      y++;
    }
//    y++;
  }
  Renderer::updateScreen();
}

void PlayerCreateCharacter::pickNewTrait(
  const bool IS_CHARACTER_CREATION)  const {

  if(Config::isBotPlaying() == false) {
    vector<Trait> pickableTraits;
    PlayerBon::getPickableTraits(pickableTraits);

    if(pickableTraits.empty() == false) {

      const int NR_TRAITS_TOT = int(pickableTraits.size());
      const int NR_TRAITS_2   = NR_TRAITS_TOT / 2;
      const int NR_TRAITS_1   = NR_TRAITS_TOT - NR_TRAITS_2;

      vector<Trait> traits1; traits1.resize(0);
      vector<Trait> traits2; traits2.resize(0);

      for(int i = 0; i < NR_TRAITS_TOT; i++) {
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
        const MenuAction action = eng.menuInputHandler->getAction(browser);
        switch(action) {
          case MenuAction::browsed: {
            drawPickTrait(traits1, traits2, browser, IS_CHARACTER_CREATION);
          } break;

          case MenuAction::esc:
          case MenuAction::space: {} break;

          case MenuAction::selected: {
            const Pos pos = browser.getPos();
            PlayerBon::pickTrait(
              pos.x == 0 ? traits1.at(pos.y) : traits2.at(pos.y), eng);
            if(IS_CHARACTER_CREATION == false) {
              Renderer::drawMapAndInterface();
            }
            return;
          } break;

          case MenuAction::selectedShift: {} break;
        }
      }
    }
  }
}

void PlayerCreateCharacter::drawPickTrait(
  const vector<Trait>& traits1, const vector<Trait>& traits2,
  const MenuBrowser& browser, const bool IS_CHARACTER_CREATION) const {

  Renderer::clearScreen();
  Renderer::drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  const int NR_TRAITS_1 = traits1.size();
  const int NR_TRAITS_2 = traits2.size();

  int lenOfLongestInCol2 = -1;
  for(const Trait & id : traits2) {
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

  Renderer::drawTextCentered(title, Panel::screen, Pos(MAP_W_HALF, 0),
                             clrWhite, clrBlack, true);

  const Pos& browserPos = browser.getPos();

  const SDL_Color& clrActive      = clrNosfTealLgt;
  const SDL_Color& clrInactive    = clrNosfTealDrk;
  const SDL_Color& clrActiveBg    = clrBlack;
  const SDL_Color& clrInactiveBg  = clrBlack;

  //------------------------------------------------------------- TRAITS
  const int Y0_TRAITS = 2;
  int y = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_1; i++) {
    const Trait trait = traits1.at(i);
    string name = "";
    PlayerBon::getTraitTitle(trait, name);
    const bool IS_MARKED = browserPos.x == 0 && browserPos.y == int(i);
    const SDL_Color& drwClr   = IS_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg = IS_MARKED ? clrActiveBg : clrInactiveBg;
    Renderer::drawText(
      name, Panel::screen, Pos(X_COL_ONE, y), drwClr, drwClrBg);
    y++;
  }
  y = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_2; i++) {
    const Trait trait = traits2.at(i);
    string name = "";
    PlayerBon::getTraitTitle(trait, name);
    const bool IS_MARKED = browserPos.x == 1 && browserPos.y == int(i);
    const SDL_Color& drwClr   = IS_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg = IS_MARKED ? clrActiveBg : clrInactiveBg;
    Renderer::drawText(
      name, Panel::screen, Pos(X_COL_TWO, y), drwClr, drwClrBg);
    y++;
  }

  //Draw frame around traits
  Rect boxRect(
    Pos(MARGIN_W - 2, Y0_TRAITS - 1),
    Pos(X_COL_TWO_RIGHT + 2, Y0_TRAITS + traits1.size()));
  Renderer::drawPopupBox(boxRect);

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
  for(const string & str : descrLines) {
    Renderer::drawText(str, Panel::screen, Pos(X0_DESCR, y), clrWhite);
    y++;
  }

  //------------------------------------------------------------- PREREQUISITES
  const int Y0_PREREQS = 17;
  y = Y0_PREREQS;
  vector<Trait> traitPrereqs;
  Bg bgPrereq = Bg::endOfBgs;
  PlayerBon::getTraitPrereqs(markedTrait, traitPrereqs, bgPrereq);
  if(traitPrereqs.empty() == false || bgPrereq != Bg::endOfBgs) {
    Renderer::drawText("This trait had the following prerequisite(s):",
                       Panel::screen, Pos(X0_DESCR, y), clrWhite);
    y++;

    string prereqStr = "";

    if(bgPrereq != Bg::endOfBgs) {
      PlayerBon::getBgTitle(bgPrereq, prereqStr);
    }

    for(Trait prereqTrait : traitPrereqs) {
      string prereqTitle = "";
      PlayerBon::getTraitTitle(prereqTrait, prereqTitle);
      prereqStr += (prereqStr.empty() ? "" : ", ") + prereqTitle;
    }

    vector<string> prereqLines;
    TextFormatting::lineToLines(prereqStr, MAX_W_DESCR, prereqLines);
    for(const string & str : prereqLines) {
      Renderer::drawText(str, Panel::screen, Pos(X0_DESCR, y), clrWhite);
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
    for(const string & str : pickedLines) {
      Renderer::drawText(
        str, Panel::screen, Pos(1, y), clrWhite);
      y++;
    }
  }

  Renderer::updateScreen();
}

void PlayerEnterName::run() const {
  string name = "";
  draw(name);
  bool isDone = false;
  while(isDone == false) {
    if(Config::isBotPlaying()) {
      name = "AZATHOTH";
      isDone = true;
    } else {
      readKeys(name, isDone);
    }
  }
  ActorData& def  = eng.player->getData();
  def.name_a      = def.name_the = name;
}

void PlayerEnterName::draw(const string& currentString) const {
  Renderer::clearScreen();
  Renderer::drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  Renderer::drawTextCentered("What is your name?", Panel::screen,
                             Pos(MAP_W_HALF, 0), clrWhite);
  const int Y_NAME = 2;
  const string NAME_STR =
    currentString.size() < PLAYER_NAME_MAX_LENGTH ? currentString + "_" :
    currentString;
  const int NAME_X0 = MAP_W_HALF - (PLAYER_NAME_MAX_LENGTH / 2);
  const int NAME_X1 = NAME_X0 + PLAYER_NAME_MAX_LENGTH - 1;
  Renderer::drawText(NAME_STR, Panel::screen, Pos(NAME_X0, Y_NAME),
                     clrNosfTealLgt);
  Rect boxRect(Pos(NAME_X0 - 1, Y_NAME - 1), Pos(NAME_X1 + 1, Y_NAME + 1));
  Renderer::drawPopupBox(boxRect);
  Renderer::updateScreen();
}

void PlayerEnterName::readKeys(string& currentString, bool& isDone) const {

  const KeyboardReadRetData& d = Input::readKeysUntilFound(eng, false);

  if(d.sdlKey_ == SDLK_RETURN) {
    isDone = true;
    currentString = currentString.empty() ? "Rogue" : currentString;
    return;
  }

  if(currentString.size() < PLAYER_NAME_MAX_LENGTH) {
    if(
      d.sdlKey_ == SDLK_SPACE ||
      (d.key_ >= int('a') && d.key_ <= int('z')) ||
      (d.key_ >= int('A') && d.key_ <= int('Z')) ||
      (d.key_ >= int('0') && d.key_ <= int('9'))) {
      if(d.sdlKey_ == SDLK_SPACE) {
        currentString.push_back(' ');
      } else {
        currentString.push_back(char(d.key_));
      }
      draw(currentString);
      return;
    }
  }

  if(currentString.size() > 0) {
    if(d.sdlKey_ == SDLK_BACKSPACE) {
      currentString.erase(currentString.end() - 1);
      draw(currentString);
    }
  }
}
