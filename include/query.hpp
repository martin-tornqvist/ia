#ifndef QUERY_H
#define QUERY_H

#include "cmn_types.hpp"

struct Key_data;

enum class Yes_no_answer {yes, no, special};

namespace query
{

void init();

void wait_for_key_press();

Yes_no_answer yes_or_no(char key_for_special_event = -1);

Dir dir();

void wait_forConfirm();

Key_data letter(const bool ACCEPT_ENTER);

int number(const P& pos, const Clr clr, const int MIN, const int MAX_NR_DIGITS,
           const int DEFAULT, const bool CANCEL_RETURNS_DEFAULT);

} //Query

#endif
