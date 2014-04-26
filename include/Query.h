#ifndef QUERY_H
#define QUERY_H

#include "CmnTypes.h"

enum class YesNoAnswer {yes, no, special};

class Query {
public:
  Query() {}
  void waitForKeyPress() const;

  YesNoAnswer yesOrNo(char keyForSpecialEvent = -1) const;

  Pos dir() const;

  void waitForEscOrSpace() const;

  int number(const Pos& pos, const SDL_Color clr, const int MIN,
             const int MAX_NR_DIGITS, const int DEFAULT,
             const bool CANCEL_RETURNS_DEFAULT) const;
};

#endif
