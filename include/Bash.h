#ifndef BASH_H
#define BASH_H

#include "FeatureDoor.h"

class Engine;
class Feature;

class Bash {
public:
  Bash(Engine& engine) : eng(engine) {}
  ~Bash() {}

  void playerBash() const;

private:
  friend class Feature;
  void playerBashFeature(Feature* const feature) const;

  Engine& eng;
};

#endif
