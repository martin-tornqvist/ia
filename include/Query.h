#ifndef QUERY_H
#define QUERY_H

#include "CmnTypes.h"

enum class YesNoAnswer {yes, no, special};

namespace Query {

void waitForKeyPress();

YesNoAnswer yesOrNo(char keyForSpecialEvent = -1);

Pos dir();

void waitForEscOrSpace();

int number(const Pos& pos, const Clr clr, const int MIN,
           const int MAX_NR_DIGITS, const int DEFAULT,
           const bool CANCEL_RETURNS_DEFAULT);

} //Query

#endif
