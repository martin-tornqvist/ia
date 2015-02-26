#include "menu_input_handling.hpp"

#include "input.hpp"
#include "config.hpp"

namespace menu_input_handling
{

Menu_action get_action(Menu_browser& browser)
{
    while (true)
    {
        Key_data d = input::get_input();

        if (d.sdl_key == SDLK_RIGHT || d.key == '6' || d.key == 'l')
        {
            browser.navigate(Dir::right);
            return Menu_action::browsed;
        }
        else if (d.sdl_key == SDLK_LEFT || d.key == '4' || d.key == 'h')
        {
            browser.navigate(Dir::left);
            return Menu_action::browsed;
        }
        else if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            browser.navigate(Dir::up);
            return Menu_action::browsed;
        }
        else if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            browser.navigate(Dir::down);
            return Menu_action::browsed;
        }
        else if (d.sdl_key == SDLK_RETURN)
        {
            return d.is_shift_held ? Menu_action::selected_shift : Menu_action::selected;
        }
        else if (d.sdl_key == SDLK_SPACE)
        {
            return Menu_action::space;
        }
        else if (d.sdl_key == SDLK_ESCAPE)
        {
            return Menu_action::esc;
        }
    }

    return Menu_action::esc;
}

} //Menu_input_handling
