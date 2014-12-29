#ifndef QUERY_H
#define QUERY_H

#include "CmnTypes.h"

struct KeyData;

enum class YesNoAnswer {yes, no, special};

namespace Query
{

void init();

void waitForKeyPress();

YesNoAnswer yesOrNo(char keyForSpecialEvent = -1);

Dir dir();

void waitForConfirm();

KeyData letter(const bool ACCEPT_ENTER);

int number(const Pos& pos, const Clr clr, const int MIN, const int MAX_NR_DIGITS,
           const int DEFAULT, const bool CANCEL_RETURNS_DEFAULT);

} //Query

#endif
