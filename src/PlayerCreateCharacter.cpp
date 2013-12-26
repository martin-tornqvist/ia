#include "PlayerCreateCharacter.h"

#include "Engine.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Input.h"
#include "MenuInputHandler.h"
#include "TextFormatting.h"

const int NR_TRAITS = 2;
const int NR_SKILLS = 2;

void PlayerCreateCharacter::createCharacter() const {
  //------------------------------------------------------ NAME
  pickNewTrait(true);
  PlayerEnterName playerEnterName(eng);
  playerEnterName.run(Pos(50, Y0_CREATE_CHARACTER + 2));
}

void PlayerCreateCharacter::pickNewTrait(
  const bool IS_CHARACTER_CREATION)  const {

  if(eng.config->isBotPlaying == false) {
    vector<Trait_t> pickableTraits;
    eng.playerBonHandler->getAllPickableTraits(pickableTraits);

    if(pickableTraits.empty() == false) {

      const int NR_TRAITS_TOT     = int(pickableTraits.size());
      const int NR_TRAITS_COL_TWO = NR_TRAITS_TOT / 2;
      const int NR_TRAITS_COL_ONE = NR_TRAITS_TOT - NR_TRAITS_COL_TWO;

      vector<Trait_t> traitsColOne;
      vector<Trait_t> traitsColTwo;
      traitsColOne.resize(0);
      traitsColTwo.resize(0);

      for(int i = 0; i < NR_TRAITS_TOT; i++) {
        const Trait_t trait = pickableTraits.at(i);
        if(i < NR_TRAITS_COL_ONE) {
          traitsColOne.push_back(trait);
        } else {
          traitsColTwo.push_back(trait);
        }
      }

      MenuBrowser browser(traitsColOne.size(), traitsColTwo.size());
      drawPickTrait(
        traitsColOne, traitsColTwo, browser, IS_CHARACTER_CREATION);

      while(true) {
        const MenuAction_t action = eng.menuInputHandler->getAction(browser);
        switch(action) {
          case menuAction_browsed: {
            drawPickTrait(
              traitsColOne, traitsColTwo, browser, IS_CHARACTER_CREATION);
          }
          break;

          case menuAction_canceled: {
          } break;

          case menuAction_selected: {
            const Pos browserPos = browser.getPos();
            if(browserPos.x == 0) {
              eng.playerBonHandler->pickTrait(
                traitsColOne.at(browser.getPos().y));
            } else {
              eng.playerBonHandler->pickTrait(
                traitsColTwo.at(browser.getPos().y));
            }
            eng.renderer->drawMapAndInterface();
            return;
          }
          break;

          case menuAction_selectedWithShift:
          {} break;

        }
      }
    }
  }
}

void PlayerCreateCharacter::drawPickTrait(
  const vector<Trait_t>& traitsColOne, const vector<Trait_t>& traitsColTwo,
  const MenuBrowser& browser, const bool IS_CHARACTER_CREATION) const {

  eng.renderer->coverPanel(panel_screen);

  const int NR_TRAITS_COL_ONE = traitsColOne.size();
  const int NR_TRAITS_COL_TWO = traitsColTwo.size();

  const int MARGIN_W = 17;
  int lenOfLongestInCol2 = -1;
  for(const Trait_t & id : traitsColTwo) {
    string title = "";
    eng.playerBonHandler->getTraitTitle(id, title);
    const int CUR_LEN = title.length();
    if(CUR_LEN > lenOfLongestInCol2) {lenOfLongestInCol2 = CUR_LEN;}
  }

  const int X_COL_ONE       = MARGIN_W;
  const int X_COL_TWO_RIGHT = MAP_X_CELLS - MARGIN_W - 1;
  const int X_COL_TWO       = X_COL_TWO_RIGHT - lenOfLongestInCol2 + 1;

  const int Y0_TITLE = Y0_CREATE_CHARACTER;

  string label = IS_CHARACTER_CREATION ?
                 "Which trait do you start with?" :
                 "You have reached a new level! Which trait do you gain?";

  eng.renderer->drawTextCentered(
    label, panel_screen, Pos(MAP_X_CELLS_HALF, Y0_TITLE),
    clrWhite, clrBlack, true);

  const Pos& browserPos = browser.getPos();

  SDL_Color clrActive     = clrNosferatuSepiaLgt;
  SDL_Color clrInactive   = clrNosferatuSepiaDrk;
  SDL_Color clrActiveBg   = clrBlack;
  SDL_Color clrInactiveBg = clrBlack;

  //Draw traits
  const int Y0_TRAITS = Y0_TITLE + 2;
  int yPos = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_COL_ONE; i++) {
    const Trait_t trait = traitsColOne.at(i);
    string name = "";
    eng.playerBonHandler->getTraitTitle(trait, name);
    const bool IS_TRAIT_MARKED = browserPos.x == 0 && browserPos.y == int(i);
    const SDL_Color& drwClr =
      IS_TRAIT_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg =
      IS_TRAIT_MARKED ? clrActiveBg : clrInactiveBg;
    eng.renderer->drawText(
      name, panel_screen, Pos(X_COL_ONE, yPos), drwClr, drwClrBg);
    yPos++;
  }
  yPos = Y0_TRAITS;
  for(int i = 0; i < NR_TRAITS_COL_TWO; i++) {
    const Trait_t trait = traitsColTwo.at(i);
    string name = "";
    eng.playerBonHandler->getTraitTitle(trait, name);
    const bool IS_TRAIT_MARKED = browserPos.x == 1 && browserPos.y == int(i);
    const SDL_Color& drwClr =
      IS_TRAIT_MARKED ? clrActive : clrInactive;
    const SDL_Color& drwClrBg =
      IS_TRAIT_MARKED ? clrActiveBg : clrInactiveBg;
    eng.renderer->drawText(
      name, panel_screen, Pos(X_COL_TWO, yPos), drwClr, drwClrBg);
    yPos++;
  }

  //Draw frame around traits
  Rect boxRect(
    Pos(MARGIN_W - 2, Y0_TRAITS - 1),
    Pos(X_COL_TWO_RIGHT + 2, Y0_TRAITS + traitsColOne.size()));
  eng.renderer->drawPopupBox(boxRect, panel_screen);

  //Draw description
  const int Y0_DESCR = Y0_TRAITS + NR_TRAITS_COL_ONE + 2;
  yPos = Y0_DESCR;
  const Trait_t markedTrait =
    browserPos.x == 0 ? traitsColOne.at(browserPos.y) :
    traitsColTwo.at(browserPos.y);
  string descr;
  eng.playerBonHandler->getTraitDescr(markedTrait, descr);
  const int MAX_W_DESCR = X_COL_TWO_RIGHT - X_COL_ONE + 1;
  vector<string> descrLines;
  eng.textFormatting->lineToLines(
    "Effect(s): " + descr, MAX_W_DESCR, descrLines);
  for(const string & str : descrLines) {
    eng.renderer->drawText(str, panel_screen, Pos(X_COL_ONE, yPos), clrWhite);
    yPos++;
  }
  yPos++;

  yPos = max(Y0_DESCR + 3, yPos);

  vector<Trait_t> prereqsForCurTrait;
  eng.playerBonHandler->getTraitPrereqs(markedTrait, prereqsForCurTrait);
  const int NR_PREREQS = prereqsForCurTrait.size();
  if(NR_PREREQS > 0) {
    string prereqStr = "This trait had the following prerequisite(s): ";
    for(int i = 0; i < NR_PREREQS; i++) {
      const Trait_t prereqTrait = prereqsForCurTrait.at(i);
      string prereqTitle;
      eng.playerBonHandler->getTraitTitle(prereqTrait, prereqTitle);
      if(i == 0) {
        prereqStr += "\"" + prereqTitle + "\"";
      } else {
        prereqStr += ", \"" + prereqTitle + "\"";
      }
    }
    vector<string> prereqLines;
    eng.textFormatting->lineToLines(prereqStr, MAX_W_DESCR, prereqLines);
    for(const string & str : prereqLines) {
      eng.renderer->drawText(str, panel_screen, Pos(X_COL_ONE, yPos), clrWhite);
      yPos++;
    }
  }

  eng.renderer->updateScreen();
}

void PlayerEnterName::run(const Pos& pos) const {
  string name = "";
  draw(name, pos);
  bool done = false;
  while(done == false) {
    if(eng.config->isBotPlaying == false) {
      readKeys(name, done, pos);
    } else {
      name = "AZATHOTH";
      done = true;
    }
  }

  ActorData& def = eng.player->getData();
  def.name_a = def.name_the = name;
}

void PlayerEnterName::draw(const string& currentString, const Pos& pos) const {
  eng.renderer->coverArea(panel_screen, pos, Pos(16, 2));
  const string LABEL = "What is your name?";
  eng.renderer->drawText(LABEL, panel_screen, pos, clrWhite);
  const string NAME_STR =
    currentString.size() < PLAYER_NAME_MAX_LENGTH ? currentString + "_" :
    currentString;
  eng.renderer->drawText(
    NAME_STR, panel_screen, pos + Pos(0, 1), clrNosferatuSepiaLgt);
  eng.renderer->updateScreen();
}

void PlayerEnterName::readKeys(string& currentString, bool& done,
                               const Pos& pos) const {
  const KeyboardReadReturnData& d = eng.input->readKeysUntilFound();

  if(d.sdlKey_ == SDLK_RETURN) {
    done = true;
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
      draw(currentString, pos);
      return;
    }
  }

  if(currentString.size() > 0) {
    if(d.sdlKey_ == SDLK_BACKSPACE) {
      currentString.erase(currentString.end() - 1);
      draw(currentString, pos);
    }
  }
}
