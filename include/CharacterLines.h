#ifndef CHAR_LINES_HANDLER_H
#define CHAR_LINES_HANDLER_H

class Engine;

class CharacterLines {
public:
  CharacterLines(Engine& engine) : eng(engine) {}

private:
  friend class Renderer;
  void drawInfoLines();

  void drawLocationInfo();

  Engine& eng;
};

#endif

