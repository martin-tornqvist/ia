#ifndef POPULATE_TRAPS_H
#define POPULATE_TRAPS_H

class Engine;

class PopulateTraps {
public:
  PopulateTraps(Engine* engine) : eng(engine) {
  }

private:
  Engine* const eng;
};

#endif
