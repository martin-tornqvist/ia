#include "manual.hpp"

#include "init.hpp"

#include <fstream>
#include <vector>

#include "text_format.hpp"
#include "io.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static const int manual_text_x0 = 1;
static const int manual_text_x1 = 79; // TODO: Set from standard value
static const int manual_text_w = manual_text_x1 - manual_text_x0 + 1;

static std::vector<std::string> read_manual_file()
{
        std::vector<std::string> lines;

        std::ifstream file("res/manual.txt");

        if (!file.is_open())
        {
                TRACE_ERROR_RELEASE << "Could not open manual file"
                                    << std::endl;

                PANIC;
        }

        std::string current_line;

        while (getline(file, current_line))
        {
                lines.push_back(current_line);
        }

        file.close();

        return lines;
}

static std::vector<std::string> format_lines(
        std::vector<std::string>& raw_lines)
{
        std::vector<std::string> formatted_lines;

        for (auto& raw_line : raw_lines)
        {
                // Format the line if it does not start with a space
                const bool should_format_line =
                        raw_line.size() > 0 &&
                        raw_line[0] != ' ';

                if (should_format_line)
                {
                        const auto split_line = text_format::split(
                                raw_line,
                                manual_text_w
                                /* panels::get_w(Panel::screen) */);

                        for (const auto& line : split_line)
                        {
                                formatted_lines.push_back(line);
                        }
                }
                else // Do not format line
                {
                        formatted_lines.push_back(raw_line);
                }
        }

        return formatted_lines;
}

static std::vector<ManualPage> init_pages(
        const std::vector<std::string>& formatted_lines)
{
        std::vector<ManualPage> pages;

        ManualPage current_page;

        const std::string delim(80, '-');

        // Sort the parsed lines into different pages
        for (size_t line_idx = 0; line_idx < formatted_lines.size(); ++line_idx)
        {
                if (formatted_lines[line_idx] == delim)
                {
                        if (!current_page.lines.empty())
                        {
                                pages.push_back(current_page);

                                current_page.lines.clear();
                        }

                        // Skip first delimiter
                        ++line_idx;

                        // The title is printed on this line
                        current_page.title = formatted_lines[line_idx];

                        // Skip second delimiter
                        line_idx += 2;
                }

                current_page.lines.push_back(formatted_lines[line_idx]);
        }

        return pages;
}

// -----------------------------------------------------------------------------
// Browse manual
// -----------------------------------------------------------------------------
StateId BrowseManual::id()
{
        return StateId::manual;
}

void BrowseManual::on_start()
{
        raw_lines_ = read_manual_file();

        const auto formatted_lines = format_lines(raw_lines_);

        pages_ = init_pages(formatted_lines);

        browser_.reset(pages_.size());
}

void BrowseManual::draw()
{
        io::draw_text_center(
                "Browsing manual",
                Panel::screen,
                P(panels::get_center_x(Panel::screen), 0),
                colors::title(),
                true, // Draw background color
                colors::black(),
                true); // Allow pixel-level adjustment

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
                              P(manual_text_x0, y),
                              draw_color);

                ++key_str[0];
        }
}

void BrowseManual::on_window_resized()
{
        // const auto formatted_lines = format_lines(raw_lines_);

        // pages_ = init_pages(formatted_lines);
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
        return StateId::manual_page;
}

void BrowseManualPage::draw()
{
        draw_interface();

        const int nr_lines_tot = page_.lines.size();

        const int btm_nr =
                std::min(top_idx_ + max_nr_lines_on_screen() - 1,
                         nr_lines_tot - 1);

        int screen_y = 1;

        for (int i = top_idx_; i <= btm_nr; ++i)
        {
                io::draw_text(
                        page_.lines[i],
                        Panel::screen,
                        P(manual_text_x0, screen_y),
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

                if (nr_lines_tot <= max_nr_lines_on_screen())
                {
                        top_idx_ = 0;
                }
                else
                {
                        top_idx_ = std::min(
                                nr_lines_tot - max_nr_lines_on_screen(),
                                top_idx_);
                }
                break;

        case '8':
        case SDLK_UP:
        case 'k':
                top_idx_ = std::max(0, top_idx_ - line_jump);
                break;

        case SDLK_SPACE:
        case SDLK_ESCAPE:
                // Exit screen
                states::pop();
                break;

        default:
                break;
        }
}

std::string BrowseManualPage::title() const
{
        return page_.title;
}
