#include "manual.hpp"

#include "init.hpp"

#include <fstream>
#include <vector>

#include "text_format.hpp"
#include "io.hpp"

namespace
{

const int max_nr_lines_on_scr_ = screen_h - 2;

} // namespace

StateId BrowseManual::id()
{
    return StateId::manual;
}

void BrowseManual::on_start()
{
    read_file();
}

void BrowseManual::draw()
{
    io::draw_info_scr_interface("Browsing manual",
                                InfScreenType::scrolling);

    const int nr_lines_tot = lines_.size();

    const int btm_nr =
        std::min(top_idx_ + max_nr_lines_on_scr_ - 1,
                 nr_lines_tot - 1);

    int screen_y = 1;

    for (int i = top_idx_; i <= btm_nr; ++i)
    {
        io::draw_text(lines_[i],
                      Panel::screen,
                      P(0, screen_y),
                      clr_text);

        ++screen_y;
    }
}

void BrowseManual::update()
{
    const int line_jump = 3;

    const int nr_lines_tot = lines_.size();

    const auto input = io::get(false);

    switch (input.key)
    {
    case '2':
    case SDLK_DOWN:
    case 'j':
        top_idx_ += line_jump;

        if (nr_lines_tot <= max_nr_lines_on_scr_)
        {
            top_idx_ = 0;
        }
        else
        {
            top_idx_ = std::min(nr_lines_tot - max_nr_lines_on_scr_, top_idx_);
        }
        break;

    case '8':
    case SDLK_UP:
    case 'k':
        top_idx_ = std::max(0, top_idx_ - line_jump);
        break;

    case SDLK_SPACE:
    case SDLK_ESCAPE:
        //
        // Exit screen
        //
        states::pop();
        break;

    default:
        break;
    }
}

void BrowseManual::read_file()
{
    lines_.clear();

    std::ifstream file("res/manual.txt");

    if (!file.is_open())
    {
        TRACE << "Failed to open manual file" << std::endl;

        ASSERT(false);

        return;
    }

    std::string current_line;

    std::vector<std::string> formatted;

    while (getline(file, current_line))
    {
        if (current_line.empty())
        {
            lines_.push_back(current_line);
        }
        else // Current line not empty
        {
            // Do not format lines that start with two spaces
            bool should_format_line = true;

            if (current_line.size() >= 2)
            {
                if (current_line[0] == ' ' && current_line[1] == ' ')
                {
                    should_format_line = false;
                }
            }

            if (should_format_line)
            {
                formatted = text_format::split(current_line, map_w);

                for (const auto& line : formatted)
                {
                    lines_.push_back(line);
                }
            }
            else // Do not format line
            {
                lines_.push_back(current_line);
            }
        }
    }

    file.close();
}
