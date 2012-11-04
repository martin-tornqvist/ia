#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "SDL.h"

using namespace std;

class Engine;

class PlayerCreateCharacter {
public:
  PlayerCreateCharacter(Engine* engine) : eng(engine) {}

  void run();

private:
  SDL_Event m_event;
  Uint8* m_keystates;
  Engine* eng;
};

class PlayerEnterName {
public:
private:
  friend class PlayerCreateCharacter;
  PlayerEnterName(Engine* engine) : eng(engine) {}
  void run(int& yPos);
  void draw(const string& currentString, const int RENDER_Y_POS);
  void readKeys(string& currentString, bool& done, const int RENDER_Y_POS);
  Engine* eng;
};

#endif
