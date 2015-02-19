#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include <vector>

#include "Colors.h"
#include "Converters.h"
#include "Cmn_types.h"

class Msg
{
public:
    Msg(const std::string& text, const Clr& clr, const int X_POS) :
        clr_        (clr),
        x_pos_       (X_POS),
        str_        (text),
        repeats_str_ (""),
        nr_         (1) {}

    Msg() :
        clr_        (clr_white),
        x_pos_       (0),
        str_        (""),
        repeats_str_ (""),
        nr_         (1) {}

    void get_str_with_repeats(std::string& str_ref) const
    {
        str_ref = str_ + (nr_ > 1 ? repeats_str_ : "");
    }

    void get_str_raw(std::string& str_ref) const {str_ref = str_;}

    void incr_repeat()
    {
        nr_++;
        repeats_str_ = "(x" + to_str(nr_) + ")";
    }

    Clr clr_;
    int x_pos_;

private:
    std::string str_;
    std::string repeats_str_;
    int nr_;
};

namespace Log
{

void init();

void add_msg(const std::string& str, const Clr& clr = clr_white,
            const bool INTERRUPT_PLAYER_ACTIONS = false,
            const bool ADD_MORE_PROMPT_AFTER_MSG = false);

//NOTE: This function can safely be called at any time. If there is content in the log,
//a "more" prompt will be done, and the log is cleared. If the log happens to be empty,
//nothing is done.
void more_prompt();

void draw_log(const bool SHOULD_UPDATE_SCREEN);

void display_history();

void clear_log();

void add_line_to_history(const std::string& line_to_add);

const std::vector< std::vector<Msg> >& get_history();

} //Log

#endif
