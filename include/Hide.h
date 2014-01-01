#ifndef HIDE_H
#define HIDE_H

class Engine;

class Hide {
public:
  Hide(Engine& engine) : eng(engine) {}

  void playerTryHide();

private:
  Engine& eng;
};

#endif
