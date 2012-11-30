#include "PlayerCreateCharacter.h"

#include "Engine.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "PlayerAllocBonus.h"
#include "Input.h"

void PlayerCreateCharacter::run() {
  int currentRenderYpos = 8;
  PlayerEnterName playerEnterName(eng);
  playerEnterName.run(currentRenderYpos);
  eng->playerAllocBonus->run();
}

//-----------------------------------------------------PLAYER NAME ENTERING
void PlayerEnterName::run(int& yPos) {
  string name = "";
  draw(name, yPos);
  bool done = false;
  while(done == false) {
    if(eng->config->BOT_PLAYING == false) {
      readKeys(name, done, yPos);
    } else {
      name = "AZATHOTH";
      done = true;
    }
  }

  ActorDefinition& iDef = *(eng->player->getDef());
  iDef.name_a = iDef.name_the = name;

  yPos++;
}

void PlayerEnterName::draw(const string& currentString, const int RENDER_Y_POS) {
//  eng->renderer->coverArea(renderArea_screen, 0, RENDER_Y_POS, MAP_X_CELLS, RENDER_Y_POS + 1);
  eng->renderer->coverRenderArea(renderArea_screen);
  int x0 = MAP_X_CELLS_HALF; //1;
  const string LABEL = "Enter character name";
  eng->renderer->drawTextCentered(LABEL, renderArea_screen, x0, RENDER_Y_POS, clrWhite);
  const string NAME_STR = currentString.size() < PLAYER_NAME_MAX_LENGTH ? currentString + "_" : currentString;
  eng->renderer->drawTextCentered(NAME_STR, renderArea_screen, x0, RENDER_Y_POS + 1, clrRedLight);
  eng->renderer->updateWindow();
}

void PlayerEnterName::readKeys(string& currentString, bool& done, const int RENDER_Y_POS) {
  const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

  if(d.sfmlKey_ == sf::Keyboard::Return) {
    done = true;
    return;
  }

  if(currentString.size() < PLAYER_NAME_MAX_LENGTH) {
    if(
      d.sfmlKey_ == sf::Keyboard::Space ||
      (d.key_ >= int('a') && d.key_ <= int('z')) ||
      (d.key_ >= int('A') && d.key_ <= int('Z')) ||
      (d.key_ >= int('0') && d.key_ <= int('9'))) {
      if(d.sfmlKey_ == sf::Keyboard::Space) {
        currentString.push_back(' ');
      } else {
        currentString.push_back(char(d.key_));
      }
      draw(currentString, RENDER_Y_POS);
      return;
    }
  }

  if(currentString.size() > 0) {
    if(d.sfmlKey_ == sf::Keyboard::Back) {
      currentString.erase(currentString.end() - 1);
      draw(currentString, RENDER_Y_POS);
    }
  }
}
