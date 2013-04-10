#include "Query.h"

#include <iostream>

#include "Engine.h"
#include "Input.h"
#include "Render.h"

using namespace std;


void Query::waitForKeyPress() const {
  if(eng->config->BOT_PLAYING == false) {
    eng->input->readKeysUntilFound();
  }
}

//int Query::readKeys() const {
//  while(true) {
//    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();
//
//        if(
//          d.sfmlKey_ == SDLK_RIGHT ||
//          d.sfmlKey_ == SDLK_UP ||
//          d.sfmlKey_ == SDLK_LEFT ||
//          d.sfmlKey_ == SDLK_DOWN ||
//          d.sfmlKey_ == SDLK_SPACE ||
//          d.sfmlKey_ == SDLK_ESCAPE ||
//          d.sfmlKey_ == SDLK_PAGEUP ||
//          d.sfmlKey_ == SDLK_HOME ||
//          d.sfmlKey_ == SDLK_END ||
//          d.sfmlKey_ == SDLK_PAGEDOWN) {
//          return key;
//        } else {
//          return event.key.keysym.unicode;
//        }
//      }
//      break;
//      default: {
//      }
//      break;
//      }
//    }
//    eng->sleep(1);
//  }
//  return 0;
//}

bool Query::yesOrNo() const {
  KeyboardReadReturnData d = eng->input->readKeysUntilFound();
  while(d.key_ != 'y' && d.key_ != 'n' && d.sfmlKey_ != sf::Keyboard::Escape && d.sfmlKey_ != sf::Keyboard::Space) {
    d = eng->input->readKeysUntilFound();
  }
  if(d.key_ == 'y') {
    return true;
  }
  return false;
}

int Query::number(const coord& cellToRenderAt, const sf::Color clr, const int MIN,
                  const int MAX_NR_DIGITS, const int DEFAULT, const bool CANCEL_RETURNS_DEFAULT) const {
  // Copy the current screen texture to use as background for number query
  sf::Texture screenTexture(eng->renderer->getScreenTextureCopy());
  sf::Sprite screenSprite(screenTexture);

  eng->renderer->clearWindow();

  eng->renderer->drawSprite(0, 0, screenSprite);

  int retNum = max(MIN, DEFAULT);
  eng->renderer->coverArea(renderArea_screen, cellToRenderAt.x, cellToRenderAt.y, MAX_NR_DIGITS + 1, 1);
  eng->renderer->drawText((retNum == 0 ? "" : intToString(retNum)) + "_", renderArea_screen, cellToRenderAt.x, cellToRenderAt.y, clr);
  eng->renderer->updateWindow();

  while(true) {
    KeyboardReadReturnData d;
    while((d.key_ < '0' || d.key_ > '9') && d.sfmlKey_ != sf::Keyboard::Return &&
          d.sfmlKey_ != sf::Keyboard::Space && d.sfmlKey_ != sf::Keyboard::Escape &&
          d.sfmlKey_ != sf::Keyboard::Back) {
      d = eng->input->readKeysUntilFound();
    }

    if(d.sfmlKey_ == sf::Keyboard::Return) {
      return max(MIN, retNum);
    }

    if(d.sfmlKey_ == sf::Keyboard::Space || d.sfmlKey_ == sf::Keyboard::Escape) {
      return CANCEL_RETURNS_DEFAULT ? DEFAULT : -1;
    }

    const string retNumStr = intToString(retNum);
    const int CUR_NUM_DIGITS = retNumStr.size();

    if(d.sfmlKey_ == sf::Keyboard::Back) {
      retNum = retNum / 10;
      eng->renderer->clearWindow();
      eng->renderer->drawSprite(0, 0, screenSprite);
      eng->renderer->coverArea(renderArea_screen, cellToRenderAt.x, cellToRenderAt.y, MAX_NR_DIGITS + 1, 1);
      eng->renderer->drawText((retNum == 0 ? "" : intToString(retNum)) + "_", renderArea_screen, cellToRenderAt.x, cellToRenderAt.y, clr);
      eng->renderer->updateWindow();
      continue;
    }

    if(CUR_NUM_DIGITS < MAX_NR_DIGITS) {
      int curDigit = d.key_ - '0';
      retNum = max(MIN, retNum * 10 + curDigit);
      eng->renderer->clearWindow();
      eng->renderer->drawSprite(0, 0, screenSprite);
      eng->renderer->coverArea(renderArea_screen, cellToRenderAt.x, cellToRenderAt.y, MAX_NR_DIGITS + 1, 1);
      eng->renderer->drawText((retNum == 0 ? "" : intToString(retNum)) + "_", renderArea_screen, cellToRenderAt.x, cellToRenderAt.y, clr);
      eng->renderer->updateWindow();
    }
  }
  return -1;
}

void Query::waitForEscOrSpace() const {
  if(eng->config->BOT_PLAYING == false) {
    KeyboardReadReturnData d = eng->input->readKeysUntilFound();
    while(d.sfmlKey_ != sf::Keyboard::Space && d.sfmlKey_ != sf::Keyboard::Escape) {
      d = eng->input->readKeysUntilFound();
    }
  }
}

coord Query::direction() const {
  KeyboardReadReturnData d = eng->input->readKeysUntilFound();

  while(d.sfmlKey_ != sf::Keyboard::Right && d.sfmlKey_ != sf::Keyboard::Up &&
        d.sfmlKey_ != sf::Keyboard::Left && d.sfmlKey_ != sf::Keyboard::Down &&
        d.sfmlKey_ != sf::Keyboard::Escape && d.sfmlKey_ != sf::Keyboard::Space &&
        d.sfmlKey_ != sf::Keyboard::PageUp && d.sfmlKey_ != sf::Keyboard::Home &&
        d.sfmlKey_ != sf::Keyboard::End && d.sfmlKey_ != sf::Keyboard::PageDown &&
        (d.key_ < '1' || d.key_ > '9' || d.key_ == '5')) {
    d = eng->input->readKeysUntilFound();
  }

  if(d.sfmlKey_ == sf::Keyboard::Space || d.sfmlKey_ == sf::Keyboard::Escape) {
    return coord(0, 0);
  }
  if(d.sfmlKey_ == sf::Keyboard::Right || d.key_ == '6') {
    if(d.isShiftHeld_) {
      return coord(1, -1);
    } else if(d.isCtrlHeld_) {
      return coord(1, 1);
    } else {
      return coord(1, 0);
    }
  }
  if(d.sfmlKey_ == sf::Keyboard::PageUp || d.key_ == '9') {
    return coord(1, -1);
  }
  if(d.sfmlKey_ == sf::Keyboard::Up || d.key_ == '8') {
    return coord(0, -1);
  }
  if(d.sfmlKey_ == sf::Keyboard::Home || d.key_ == '7') {
    return coord(-1, -1);
  }
  if(d.sfmlKey_ == sf::Keyboard::Left || d.key_ == '4') {
    if(d.isShiftHeld_) {
      return coord(-1, -1);
    } else if(d.isCtrlHeld_) {
      return coord(-1, 1);
    } else {
      return coord(-1, 0);
    }
  }
  if(d.sfmlKey_ == sf::Keyboard::End || d.key_ == '1') {
    return coord(-1, 1);
  }
  if(d.sfmlKey_ == sf::Keyboard::Down || d.key_ == '2') {
    return coord(0, 1);
  }
  if(d.sfmlKey_ == sf::Keyboard::PageDown || d.key_ == '3') {
    return coord(1, 1);
  }

  return coord(0, 0);
}
