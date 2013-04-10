#ifndef DISARM_H
#define DISARM_H

class Engine;

class Disarm {
public:
  Disarm(Engine* const engine) : eng(engine) {

  }

  void playerDisarm() const;

private:
  Engine* const eng;
};

#endif
