#ifndef COLORS_HPP
#define COLORS_HPP

#include <string>

#include <SDL_video.h>

//-----------------------------------------------------------------------------
// Color
//-----------------------------------------------------------------------------
class Color
{
public:
        Color();

        Color(uint8_t r, uint8_t g, uint8_t b);

        Color(const SDL_Color& sdl_color);

        ~Color();

        Color& operator=(const Color& other);

        bool operator==(const Color& other) const;

        bool operator!=(const Color& other) const;

        Color fraction(const double div);

        bool is_defined() const;

        void clear();

        SDL_Color sdl_color() const;

        uint8_t r() const;
        uint8_t g() const;
        uint8_t b() const;

        void set_r(const uint8_t value);
        void set_g(const uint8_t value);
        void set_b(const uint8_t value);

private:
        SDL_Color sdl_color_;

        bool is_defined_;
};

namespace colors
{

void init();

Color name_to_color(const std::string& name);

std::string color_to_name(const Color& color);

//-----------------------------------------------------------------------------
// Available colors
//-----------------------------------------------------------------------------
Color black();
Color extra_dark_gray();
Color dark_gray();
Color gray();
Color white();
Color light_white();
Color red();
Color light_red();
Color dark_green();
Color green();
Color light_green();
Color dark_yellow();
Color yellow();
Color blue();
Color light_blue();
Color magenta();
Color light_magenta();
Color cyan();
Color light_cyan();
Color brown();
Color dark_brown();
Color gray_brown();
Color dark_gray_brown();
Color violet();
Color dark_violet();
Color orange();
Color sepia();
Color light_sepia();
Color dark_sepia();
Color teal();
Color light_teal();
Color dark_teal();

//-----------------------------------------------------------------------------
// GUI colors (using the colors above)
//-----------------------------------------------------------------------------
Color text();
Color menu_highlight();
Color menu_dark();
Color title();
Color msg_good();
Color msg_bad();
Color msg_note();
Color mon_unaware_bg();
Color mon_allied_bg();
Color mon_temp_property_bg();

} // colors

#endif // COLORS_HPP
