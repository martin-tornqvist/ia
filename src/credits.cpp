#include "credits.hpp"

#include <fstream>
#include <string>
#include <vector>

#include "input.hpp"
#include "text_format.hpp"
#include "render.hpp"

using namespace std;

namespace credits
{

namespace
{

vector<string> lines_;

void read_file()
{
    lines_.clear();

    string cur_line;
    ifstream file("credits.txt");

    vector<string> formatted_lines;

    if (file.is_open())
    {
        while (getline(file, cur_line))
        {
            if (cur_line.empty())
            {
                lines_.push_back(cur_line);
            }
            else
            {
                text_format::line_to_lines(cur_line, MAP_W - 2, formatted_lines);

                for (string& line : formatted_lines) {lines_.push_back(line);}
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
    render::clear_screen();

    const string decoration_line(MAP_W, '-');

    const int X_LABEL = 3;

    render::draw_text(decoration_line, Panel::screen, Pos(0, 0), clr_gray);

    render::draw_text(" Displaying credits.txt ", Panel::screen,
                      Pos(X_LABEL, 0), clr_gray);

    render::draw_text(decoration_line, Panel::screen, Pos(0, SCREEN_H - 1),
                      clr_gray);

    render::draw_text(" [space/esc] to exit ", Panel::screen,
                      Pos(X_LABEL, SCREEN_H - 1), clr_gray);

    int y_pos = 1;

    for (string& line : lines_)
    {
        render::draw_text(line, Panel::screen, Pos(0, y_pos++), clr_white);
    }

    render::update_screen();

    //Read keys
    while (true)
    {
        const Key_data& d = input::get_input();

        if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
    }
}

} //Credits
