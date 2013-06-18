#ifndef CHARACTER_LINES_H
#define CHARACTER_LINES_H

class Engine;

class CharacterLines {
public:
  CharacterLines(Engine* engine) : eng(engine) {}
private:
  friend class Renderer;
  void drawInfoLines();
  void drawLocationInfo();
  Engine* eng;
};

#endif

