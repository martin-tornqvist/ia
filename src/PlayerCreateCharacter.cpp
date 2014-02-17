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
  if(eng.config->isBotPlaying) {
    eng.playerBonHandler->pickBg(Bg(Rnd::range(0, endOfBgs - 1)));
  } else {
    vector<Bg> bgs;
    eng.playerBonHandler->getPickableBgs(bgs);

    MenuBrowser browser(bgs.size(), 0);
    drawPickBg(bgs, browser);

    while(true) {
      const MenuAction action = eng.menuInputHandler->getAction(browser);
      switch(action) {
        case menuAction_browsed: {drawPickBg(bgs, browser);} break;

        case menuAction_esc:
        case menuAction_space: {} break;

        case menuAction_selected: {
          eng.playerBonHandler->pickBg(bgs.at(browser.getPos().y));
          return;
        } break;

        case menuAction_selectedWithShift: {} break;
      }
    }
  }
}

void PlayerCreateCharacter::drawPickBg(const vector<Bg>& bgs,
                                       const MenuBrowser& browser) const {
  eng.renderer->clearScreen();
  eng.renderer->drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  eng.renderer->drawTextCentered("Choose your background", panel_screen,
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
    eng.playerBonHandler->getBgTitle(bg, name);
    const bool IS_MARKED = bg == markedBg;
    const SDL_Color& drwClr   = IS_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg = IS_MARKED ? clrActiveBg : clrInactiveBg;
    eng.renderer->drawTextCentered(name, panel_screen, Pos(MAP_W_HALF, y),
                                   drwClr, drwClrBg);
    y++;
  }
  y++;

  const int BGS_BOX_W_HALF = 6;
  Rect boxRect(Pos(MAP_W_HALF - BGS_BOX_W_HALF, Y0_BGS - 1),
               Pos(MAP_W_HALF + BGS_BOX_W_HALF, Y0_BGS + NR_BGS));
  eng.renderer->drawPopupBox(boxRect);

  //------------------------------------------------------------- DESCRIPTION
  const int MARGIN_W_DESCR  = 12;
  const int X0_DESCR        = MARGIN_W_DESCR;
  const int MAX_W_DESCR     = MAP_W - (MARGIN_W_DESCR * 2);

  vector<string> rawDescrLines;
  eng.playerBonHandler->getBgDescr(markedBg, rawDescrLines);
  for(string & rawLine : rawDescrLines) {
    vector<string> formattedLines;
    TextFormatting::lineToLines(rawLine, MAX_W_DESCR, formattedLines);
    for(string & line : formattedLines) {
      eng.renderer->drawText(line, panel_screen, Pos(X0_DESCR, y), clrWhite);
      y++;
    }
//    y++;
  }
  eng.renderer->updateScreen();
}

void PlayerCreateCharacter::pickNewTrait(
  const bool IS_CHARACTER_CREATION)  const {

  if(eng.config->isBotPlaying == false) {
    vector<TraitId> pickableTraits;
    eng.playerBonHandler->getPickableTraits(pickableTraits);

    if(pickableTraits.empty() == false) {

      const int NR_TRAITS_TOT = int(pickableTraits.size());
      const int NR_TRAITS_2   = NR_TRAITS_TOT / 2;
      const int NR_TRAITS_1   = NR_TRAITS_TOT - NR_TRAITS_2;

      vector<TraitId> traits1; traits1.resize(0);
      vector<TraitId> traits2; traits2.resize(0);

      for(int i = 0; i < NR_TRAITS_TOT; i++) {
        const TraitId trait = pickableTraits.at(i);
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
          case menuAction_browsed: {
            drawPickTrait(traits1, traits2, browser, IS_CHARACTER_CREATION);
          } break;

          case menuAction_esc:
          case menuAction_space: {} break;

          case menuAction_selected: {
            const Pos pos = browser.getPos();
            eng.playerBonHandler->pickTrait(
              pos.x == 0 ? traits1.at(pos.y) : traits2.at(pos.y));
            if(IS_CHARACTER_CREATION == false) {
              eng.renderer->drawMapAndInterface();
            }
            return;
          } break;

          case menuAction_selectedWithShift: {} break;
        }
      }
    }
  }
}

void PlayerCreateCharacter::drawPickTrait(
  const vector<TraitId>& traits1, const vector<TraitId>& traits2,
  const MenuBrowser& browser, const bool IS_CHARACTER_CREATION) const {

  eng.renderer->clearScreen();
  eng.renderer->drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  const int NR_TRAITS_1 = traits1.size();
  const int NR_TRAITS_2 = traits2.size();

  int lenOfLongestInCol2 = -1;
  for(const TraitId & id : traits2) {
    string title = "";
    eng.playerBonHandler->getTraitTitle(id, title);
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

  eng.renderer->drawTextCentered(title, panel_screen, Pos(MAP_W_HALF, 0),
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
    const TraitId trait = traits1.at(i);
    string name = "";
    eng.playerBonHandler->getTraitTitle(trait, name);
    const bool IS_MARKED = browserPos.x == 0 && browserPos.y == int(i);
    const SDL_Color& drwClr   = IS_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg = IS_MARKED ? clrActiveBg : clrInactiveBg;
    eng.renderer->drawText(
      name, panel_screen, Pos(X_COL_ONE, y), drwClr, drwClrBg);
    y++;
  }
  y = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_2; i++) {
    const TraitId trait = traits2.at(i);
    string name = "";
    eng.playerBonHandler->getTraitTitle(trait, name);
    const bool IS_MARKED = browserPos.x == 1 && browserPos.y == int(i);
    const SDL_Color& drwClr   = IS_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg = IS_MARKED ? clrActiveBg : clrInactiveBg;
    eng.renderer->drawText(
      name, panel_screen, Pos(X_COL_TWO, y), drwClr, drwClrBg);
    y++;
  }

  //Draw frame around traits
  Rect boxRect(
    Pos(MARGIN_W - 2, Y0_TRAITS - 1),
    Pos(X_COL_TWO_RIGHT + 2, Y0_TRAITS + traits1.size()));
  eng.renderer->drawPopupBox(boxRect);

  //------------------------------------------------------------- DESCRIPTION
  const int Y0_DESCR = Y0_TRAITS + NR_TRAITS_1 + 1;
  const int X0_DESCR = X_COL_ONE;
  y = Y0_DESCR;
  const TraitId markedTrait =
    browserPos.x == 0 ? traits1.at(browserPos.y) :
    traits2.at(browserPos.y);
  string descr = "";
  eng.playerBonHandler->getTraitDescr(markedTrait, descr);
  const int MAX_W_DESCR = X_COL_TWO_RIGHT - X_COL_ONE + 1;
  vector<string> descrLines;
  TextFormatting::lineToLines(
    "Effect(s): " + descr, MAX_W_DESCR, descrLines);
  for(const string & str : descrLines) {
    eng.renderer->drawText(str, panel_screen, Pos(X0_DESCR, y), clrWhite);
    y++;
  }

  //------------------------------------------------------------- PREREQUISITES
  const int Y0_PREREQS = 17;
  y = Y0_PREREQS;
  vector<TraitId> traitPrereqs;
  Bg bgPrereq = endOfBgs;
  eng.playerBonHandler->getTraitPrereqs(markedTrait, traitPrereqs, bgPrereq);
  if(traitPrereqs.empty() == false || bgPrereq != endOfBgs) {
    eng.renderer->drawText("This trait had the following prerequisite(s):",
                           panel_screen, Pos(X0_DESCR, y), clrWhite);
    y++;

    string prereqStr = "";

    if(bgPrereq != endOfBgs) {
      eng.playerBonHandler->getBgTitle(bgPrereq, prereqStr);
    }

    for(TraitId prereqTrait : traitPrereqs) {
      string prereqTitle = "";
      eng.playerBonHandler->getTraitTitle(prereqTrait, prereqTitle);
      prereqStr += (prereqStr.empty() ? "" : ", ") + prereqTitle;
    }

    vector<string> prereqLines;
    TextFormatting::lineToLines(prereqStr, MAX_W_DESCR, prereqLines);
    for(const string & str : prereqLines) {
      eng.renderer->drawText(str, panel_screen, Pos(X0_DESCR, y), clrWhite);
      y++;
    }
  }

  //------------------------------------------------------------- PREVIOUS
  y = Y0_PREREQS + 4;
  const int MAX_W_PREV_PICKS  = SCREEN_W - 2;
  string pickedStr = "";
  eng.playerBonHandler->getAllPickedTraitsTitlesLine(pickedStr);
  if(pickedStr != "") {
    pickedStr = "Trait(s) gained: " + pickedStr;
    vector<string> pickedLines;
    TextFormatting::lineToLines(pickedStr, MAX_W_PREV_PICKS, pickedLines);
    for(const string & str : pickedLines) {
      eng.renderer->drawText(
        str, panel_screen, Pos(1, y), clrWhite);
      y++;
    }
  }

  eng.renderer->updateScreen();
}

void PlayerEnterName::run() const {
  string name = "";
  draw(name);
  bool isDone = false;
  while(isDone == false) {
    if(eng.config->isBotPlaying) {
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
  eng.renderer->clearScreen();
  eng.renderer->drawPopupBox(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

  eng.renderer->drawTextCentered("What is your name?", panel_screen,
                                 Pos(MAP_W_HALF, 0), clrWhite);
  const int Y_NAME = 2;
  const string NAME_STR =
    currentString.size() < PLAYER_NAME_MAX_LENGTH ? currentString + "_" :
    currentString;
  const int NAME_X0 = MAP_W_HALF - (PLAYER_NAME_MAX_LENGTH / 2);
  const int NAME_X1 = NAME_X0 + PLAYER_NAME_MAX_LENGTH - 1;
  eng.renderer->drawText(NAME_STR, panel_screen, Pos(NAME_X0, Y_NAME),
                         clrNosfTealLgt);
  Rect boxRect(Pos(NAME_X0 - 1, Y_NAME - 1), Pos(NAME_X1 + 1, Y_NAME + 1));
  eng.renderer->drawPopupBox(boxRect);
  eng.renderer->updateScreen();
}

void PlayerEnterName::readKeys(string& currentString, bool& isDone) const {
  const KeyboardReadReturnData& d = eng.input->readKeysUntilFound();

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
