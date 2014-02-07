#ifndef QUERY_H
#define QUERY_H

#include "CommonTypes.h"

class Engine;

enum class YesNoAnswer {
  yes, no, special
};

class Query {
public:
  Query(Engine& engine) : eng(engine) {}
  void waitForKeyPress() const;

  YesNoAnswer yesOrNo(char keyForSpecialEvent = -1) const;

  Pos dir() const;

  void waitForEscOrSpace() const;

  int number(const Pos& pos, const SDL_Color clr, const int MIN,
             const int MAX_NR_DIGITS, const int DEFAULT,
             const bool CANCEL_RETURNS_DEFAULT) const;

private:
//  int readKeys() const;

  Engine& eng;
};

#endif
