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
  PlayerEnterName playerEnterName(eng);
  playerEnterName.run(Pos(50, Y0_CREATE_CHARACTER + 2));
}

void PlayerCreateCharacter::pickTrait() const {
  //------------------------------------------------------ TRAITS AND SKILLS
  if(eng.config->isBotPlaying == false) {
    vector<Trait_t> pickableTraits;
    eng.playerBonHandler->getAllPickableTraits(pickableTraits);

    if(pickableTraits.empty() == false) {

      const int NR_TRAITS_TOT = int(pickableTraits.size());
      const int NR_TRAITS_COL_TWO = NR_TRAITS_TOT / 2;
      const int NR_TRAITS_COL_ONE = NR_TRAITS_TOT - NR_TRAITS_COL_TWO;

      vector<Trait_t> traitsColOne;
      vector<Trait_t> traitsColTwo;
      traitsColOne.resize(0);
      traitsColTwo.resize(0);

      for(int i = 0; i < NR_TRAITS_TOT; i++) {
        const Trait_t trait = pickableTraits.at(i);
        if(i < NR_TRAITS_COL_ONE) {
          traitsColOne.push_back(Trait_t(i));
        } else {
          traitsColTwo.push_back(Trait_t(i));
        }
      }

      MenuBrowser browser(traitsColOne.size(), traitsColTwo.size());
      drawPickTrait(traitsColOne, traitsColTwo, browser);

      while(true) {
        const MenuAction_t action = eng.menuInputHandler->getAction(browser);
        switch(action) {
          case menuAction_browsed: {
            drawPickTrait(traitsColOne, traitsColTwo, browser);
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
  const MenuBrowser& browser) const {

  eng.renderer->coverPanel(panel_screen);

  const int NR_TRAITS_COL_ONE = traitsColOne.size();
  const int NR_TRAITS_COL_TWO = traitsColTwo.size();

  const int X_COLUMN_ONE = 14;
  const int X_COLUMN_TWO = MAP_X_CELLS_HALF + 6;

  const int Y0_TITLE = Y0_CREATE_CHARACTER;

  eng.renderer->drawTextCentered(
    "Choose new ability", panel_screen, Pos(MAP_X_CELLS_HALF, Y0_TITLE),
    clrWhite, clrBlack, true);

  const Pos& browserPos = browser.getPos();

  //Draw traits
  const int Y0_BONUSES = Y0_TITLE + 2;
  int yPos = Y0_BONUSES;
  for(int i = 0; i < NR_TRAITS_COL_ONE; i++) {
    const Trait_t trait = traitsColOne.at(i);
    string name = "";
    eng.playerBonHandler->getTraitTitle(trait, name);
    const bool IS_TRAIT_MARKED = browserPos.x == 0 && browserPos.y == int(i);
    const SDL_Color drwClr = IS_TRAIT_MARKED ? clrWhite : clrRedLgt;
    eng.renderer->drawText(
      name, panel_screen, Pos(X_COLUMN_ONE, yPos), drwClr);
    yPos++;
  }
  yPos = Y0_BONUSES;
  for(int i = 0; i < NR_TRAITS_COL_TWO; i++) {
    const Trait_t trait = traitsColTwo.at(i);
    string name = "";
    eng.playerBonHandler->getTraitTitle(trait, name);
    const bool IS_TRAIT_MARKED = browserPos.x == 1 && browserPos.y == int(i);
    const SDL_Color drwClr = IS_TRAIT_MARKED ? clrWhite : clrRedLgt;
    eng.renderer->drawText(
      name, panel_screen, Pos(X_COLUMN_TWO, yPos), drwClr);
    yPos++;
  }

  //Draw description
  const int Y0_DESCR = Y0_BONUSES + NR_TRAITS_COL_ONE + 2;
  yPos = Y0_DESCR;
  const Trait_t markedTrait =
    browserPos.x == 0 ? traitsColOne.at(browserPos.y) :
    traitsColTwo.at(browserPos.y);
  string descr;
  eng.playerBonHandler->getTraitDescr(markedTrait, descr);
  const int MAX_WIDTH_DESCR = 50;
  vector<string> descrLines;
  eng.textFormatting->lineToLines(
    "Effect(s): " + descr, MAX_WIDTH_DESCR, descrLines);
  const int NR_DESCR_LINES = descrLines.size();
  for(int i = 0; i < NR_DESCR_LINES; i++) {
    eng.renderer->drawText(
      descrLines.at(i), panel_screen, Pos(X_COLUMN_ONE, yPos), clrRed);
    yPos++;
  }
  yPos++;

  yPos = max(Y0_DESCR + 3, yPos);

  vector<Trait_t> prereqsForCurTrait;
  eng.playerBonHandler->getTraitPrereqs(markedTrait, prereqsForCurTrait);
  const int NR_PREREQS = prereqsForCurTrait.size();
  if(NR_PREREQS > 0) {
    string prereqStr = "This ability had the following prerequisite(s): ";
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
    eng.textFormatting->lineToLines(prereqStr, MAX_WIDTH_DESCR, prereqLines);
    const int NR_PREREQ_LINES = prereqLines.size();
    for(int i = 0; i < NR_PREREQ_LINES; i++) {
      eng.renderer->drawText(
        prereqLines.at(i), panel_screen, Pos(X_COLUMN_ONE, yPos), clrRed);
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

  ActorData& def = *(eng.player->getData());
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
