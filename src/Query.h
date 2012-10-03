#ifndef QUERY_H
#define QUERY_H

#include "ConstTypes.h"

class Engine;

class Query
{
public:
  Query(Engine* engine) : eng(engine) {}
  void waitForKeyPress() const;

  bool yesOrNo() const;

  coord direction() const;

  void waitForEscOrSpace() const;

private:
  int readKeys() const;

  Engine* eng;
};

#endif
