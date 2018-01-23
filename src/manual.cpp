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

// -----------------------------------------------------------------------------
// Browse manual
// -----------------------------------------------------------------------------
StateId BrowseManual::id()
{
    return StateId::manual;
}

void BrowseManual::on_start()
{
    read_file();
}

void BrowseManual::read_file()
{
    pages_.clear();

    std::ifstream file("res/manual.txt");

    if (!file.is_open())
    {
        TRACE << "Failed to open manual file" << std::endl;

        ASSERT(false);

        return;
    }

    std::string current_line;

    std::vector<std::string> formatted;

    std::vector<std::string> global_lines;

    while (getline(file, current_line))
    {
        if (current_line.empty())
        {
            global_lines.push_back(current_line);
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
                    global_lines.push_back(line);
                }
            }
            else // Do not format line
            {
                global_lines.push_back(current_line);
            }
        }
    }

    file.close();

    // Sort the parsed lines into section
    ManualPage current_page;

    const std::string delim(80, '-');

    // Sort the parsed lines into different pages
    for (size_t line_idx = 0; line_idx < global_lines.size(); ++line_idx)
    {
        if (global_lines[line_idx] == delim)
        {
            if (!current_page.lines.empty())
            {
                pages_.push_back(current_page);

                current_page.lines.clear();
            }

            // Skip first delimiter
            ++line_idx;

            // The title is printed on this line
            current_page.title = global_lines[line_idx];

            // Skip second delimiter
            line_idx += 2;
        }

        current_page.lines.push_back(global_lines[line_idx]);
    }

    browser_.reset(pages_.size());
}

void BrowseManual::draw()
{
    io::draw_text_center("Browsing manual",
                         Panel::screen,
                         P(map_w_half, 0),
                         colors::title(),
                         colors::black(),
                         true);

    const int nr_pages = pages_.size();

    std::string key_str = "a) ";

    const int labels_y0 = 1;

    for (int idx = 0; idx < (int)nr_pages; ++idx)
    {
        const bool is_marked = browser_.y() == idx;

        const Color& draw_color =
            is_marked ?
            colors::menu_highlight() :
            colors::menu_dark();

        const auto& page = pages_[idx];

        const int y = labels_y0 + idx;

        io::draw_text(key_str + page.title,
                      Panel::screen,
                      P(0, y),
                      draw_color);

        ++key_str[0];
    }
}

void BrowseManual::update()
{
    const auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    case MenuAction::selected_shift:
    {
        const auto& page = pages_[browser_.y()];

        std::unique_ptr<State> browse_page(
            new BrowseManualPage(page));

        states::push(std::move(browse_page));
    }
    break;

    case MenuAction::esc:
    case MenuAction::space:
    {
        states::pop();
    }
    break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Manual page
// -----------------------------------------------------------------------------
StateId BrowseManualPage::id()
{
    return StateId::manual;
}

void BrowseManualPage::draw()
{
    io::draw_info_scr_interface(page_.title,
                                InfScreenType::scrolling);

    const int nr_lines_tot = page_.lines.size();

    const int btm_nr =
        std::min(top_idx_ + max_nr_lines_on_scr_ - 1,
                 nr_lines_tot - 1);

    int screen_y = 1;

    for (int i = top_idx_; i <= btm_nr; ++i)
    {
        io::draw_text(page_.lines[i],
                      Panel::screen,
                      P(0, screen_y),
                      colors::text());

        ++screen_y;
    }
}

void BrowseManualPage::update()
{
    const int line_jump = 3;

    const int nr_lines_tot = page_.lines.size();

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
