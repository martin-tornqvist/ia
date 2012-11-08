#include "PlayerCreateCharacter.h"

#include "Engine.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "PlayerAllocBonus.h"

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
    SDL_Delay(1);
  }

  ActorDefinition& iDef = *(eng->player->getDef());
  iDef.name_a = iDef.name_the = name;

  yPos++;
}

void PlayerEnterName::draw(const string& currentString, const int RENDER_Y_POS) {
  eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, RENDER_Y_POS, MAP_X_CELLS, RENDER_Y_POS + 1);
  int x0 = MAP_X_CELLS_HALF; //1;
  const string LABEL = "Enter character name";
  eng->renderer->drawTextCentered(LABEL, renderArea_screen, x0, RENDER_Y_POS, clrWhite);
  const string NAME_STR = currentString.size() < PLAYER_NAME_MAX_LENGTH ? currentString + "_" : currentString;
  eng->renderer->drawTextCentered(NAME_STR, renderArea_screen, x0, RENDER_Y_POS + 1, clrRedLight);
  eng->renderer->flip();
}

void PlayerEnterName::readKeys(string& currentString, bool& done, const int RENDER_Y_POS) {
  SDL_Event event;

  while(SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_KEYDOWN:
    {
      int key = event.key.keysym.unicode;

      if(key == SDLK_RETURN) {
        done = true;
        return;
      }

      if(currentString.size() < PLAYER_NAME_MAX_LENGTH) {
        if(
          key == int(' ') ||
          (key >= int('a') && key <= int('z')) ||
          (key >= int('A') && key <= int('Z')) ||
          (key >= int('0') && key <= int('9')))
        {
          currentString.push_back(char(key));
          draw(currentString, RENDER_Y_POS);
          return;
        }
      }

      if(currentString.size() > 0) {
        if(key == SDLK_BACKSPACE) {
          currentString.erase(currentString.end() - 1);
          draw(currentString, RENDER_Y_POS);
        }
      }
    }
    break;
    }
  }
}
