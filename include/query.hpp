#ifndef QUERY_HPP
#define QUERY_HPP

#include "rl_utils.hpp"
#include "colors.hpp"

struct Key_data;

enum class Yes_no_answer
{
    no,
    yes,
    special
};

enum class Allow_center
{
    no,
    yes
};

namespace query
{

void init();

void wait_for_key_press();

Yes_no_answer yes_or_no(char key_for_special_event = -1);

Dir dir(const Allow_center allow_center);

void wait_for_msg_more();

void wait_for_confirm();

Key_data letter(const bool accept_enter);

int number(const P& pos,
           const Clr clr,
           const int min,
           const int max_nr_digits,
           const int default_value,
           const bool cancel_returns_default);

} //query

#endif
