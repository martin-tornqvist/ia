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

  int number(const coord& pos, const SDL_Color clr, const int MIN,
             const int MAX_NR_DIGITS, const int DEFAULT,
             const bool CANCEL_RETURNS_DEFAULT) const;

private:
//  int readKeys() const;

  Engine* eng;
};

#endif
