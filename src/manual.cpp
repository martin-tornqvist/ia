#include "manual.hpp"

#include "init.hpp"

#include <fstream>
#include <vector>

#include "input.hpp"
#include "text_format.hpp"
#include "render.hpp"

namespace manual
{

namespace
{

std::vector<std::string> lines_;

void read_file()
{
    std::string cur_line;
    std::ifstream file("manual.txt");

    if (!file.is_open())
    {
        return;
    }

    std::vector<std::string> formatted;

    while (getline(file, cur_line))
    {
        if (cur_line.empty())
        {
            lines_.push_back(cur_line);
        }
        else //Current line not empty
        {
            //Do not format lines that start with two spaces
            bool should_format_line = true;

            if (cur_line.size() >= 2)
            {
                if (cur_line[0] == ' ' && cur_line[1] == ' ')
                {
                    should_format_line = false;
                }
            }

            if (should_format_line)
            {
                text_format::split(cur_line, MAP_W, formatted);

                for (const auto& line : formatted)
                {
                    lines_.push_back(line);
                }
            }
            else //Do not format line
            {
                lines_.push_back(cur_line);
            }
        }
    }

    file.close();
}

} //namespace

void init()
{
    read_file();
}

void run()
{
    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = lines_.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

    int top_nr = 0;
    int btm_nr = std::min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

    while (true)
    {
        render::clear_screen();

        render::draw_info_scr_interface("Browsing manual",
                                        Inf_screen_type::scrolling);

        int y_pos = 1;

        for (int i = top_nr; i <= btm_nr; ++i)
        {
            render::draw_text(lines_[i],
                              Panel::screen,
                              P(0, y_pos++),
                              clr_text);
        }

        render::update_screen();

        const Key_data& d = input::input();

        if (d.key == '2' || d.sdl_key == SDLK_DOWN || d.key == 'j')
        {
            top_nr += LINE_JUMP;

            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = std::min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }
        else if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - LINE_JUMP);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }

        btm_nr = std::min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
    }
}

} //manual
