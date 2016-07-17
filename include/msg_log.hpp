#ifndef LOG_HANDLER_HPP
#define LOG_HANDLER_HPP

#include <string>
#include <vector>

#include "colors.hpp"
#include "rl_utils.hpp"
#include "global.hpp"

class Msg
{
public:
    Msg(const std::string& text, const Clr& clr, const int x_pos) :
        clr_            (clr),
        x_pos_          (x_pos),
        str_            (text),
        repeats_str_    (""),
        nr_             (1) {}

    Msg() :
        clr_            (clr_white),
        x_pos_          (0),
        str_            (""),
        repeats_str_    (""),
        nr_             (1) {}

    void str_with_repeats(std::string& str_ref) const
    {
        str_ref = str_ + (nr_ > 1 ? repeats_str_ : "");
    }

    void str_raw(std::string& str_ref) const {str_ref = str_;}

    void incr_repeat()
    {
        ++nr_;
        repeats_str_ = "(x" + to_str(nr_) + ")";
    }

    Clr clr_;
    int x_pos_;

private:
    std::string str_;
    std::string repeats_str_;
    int nr_;
};

namespace msg_log
{

void init();

void add(const std::string& str,
         const Clr& clr = clr_text,
         const bool interrupt_all_player_actions = false,
         const MorePromptOnMsg add_more_prompt_on_msg = MorePromptOnMsg::no);

//NOTE: This function can safely be called at any time. If there is content in
//the log, a "more" prompt will be done, and the log is cleared. If the log
//happens to be empty, nothing is done.
void more_prompt();

void draw(const UpdateScreen update);

void display_history();

void clear();

void add_line_to_history(const std::string& line_to_add);

const std::vector< std::vector<Msg> >& history();

} //log

#endif
