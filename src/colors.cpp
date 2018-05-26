#include "colors.hpp"

#include <SDL_video.h>
#include <vector>

#include "rl_utils.hpp"
#include "xml.hpp"

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
// Defined in colors.xml
static SDL_Color black_;
static SDL_Color extra_dark_gray_;
static SDL_Color dark_gray_;
static SDL_Color gray_;
static SDL_Color white_;
static SDL_Color light_white_;
static SDL_Color red_;
static SDL_Color light_red_;
static SDL_Color dark_green_;
static SDL_Color green_;
static SDL_Color light_green_;
static SDL_Color dark_yellow_;
static SDL_Color yellow_;
static SDL_Color blue_;
static SDL_Color light_blue_;
static SDL_Color magenta_;
static SDL_Color light_magenta_;
static SDL_Color cyan_;
static SDL_Color light_cyan_;
static SDL_Color brown_;
static SDL_Color dark_brown_;
static SDL_Color gray_brown_;
static SDL_Color dark_gray_brown_;
static SDL_Color violet_;
static SDL_Color dark_violet_;
static SDL_Color orange_;
static SDL_Color sepia_;
static SDL_Color light_sepia_;
static SDL_Color dark_sepia_;
static SDL_Color teal_;
static SDL_Color light_teal_;
static SDL_Color dark_teal_;

static SDL_Color text_;
static SDL_Color menu_highlight_;
static SDL_Color menu_dark_;
static SDL_Color title_;
static SDL_Color msg_good_;
static SDL_Color msg_bad_;
static SDL_Color msg_note_;
static SDL_Color mon_unaware_bg_;
static SDL_Color mon_allied_bg_;
static SDL_Color mon_temp_property_bg_;

static std::vector< std::pair<std::string, Color> > str_color_pairs_;

static SDL_Color rgb_hex_str_to_sdl_color(const std::string str)
{
        if (str.size() != 6)
        {
                TRACE_ERROR_RELEASE << "Invalid rgb hex string: '"
                                    << str
                                    << "'"
                                    << std::endl;

                PANIC;
        }

        uint8_t rgb[3] = {};

        for (int i = 0; i < 3; ++i)
        {
                const std::string hex8_str = str.substr(2 * i, 2);

                rgb[i] =  (uint8_t)std::stoi(hex8_str, 0, 16);
        }

        const SDL_Color sdl_color = {rgb[0], rgb[1], rgb[2], 0};

        return sdl_color;
}

static void load_color(xml::Element* colors_e,
                       const std::string& name,
                       SDL_Color& target_color)
{
        for (auto e = xml::first_child(colors_e);
             e;
             e = xml::next_sibling(e))
        {
                const std::string current_name =
                        xml::get_attribute_str(e, "name");

                if (current_name != name)
                {
                        continue;
                }

                const std::string rgb_hex_str =
                        xml::get_attribute_str(e, "rgb_hex");

                const SDL_Color sdl_color =
                        rgb_hex_str_to_sdl_color(rgb_hex_str);

                TRACE << "Loaded color - "
                      << "name: \"" << name << "\""
                      << ", hexadecimal RGB string: \"" << rgb_hex_str << "\""
                      << ", decimal RGB values: "
                      << (int)sdl_color.r << ", "
                      << (int)sdl_color.g << ", "
                      << (int)sdl_color.b
                      << std::endl;

                target_color = sdl_color;

                str_color_pairs_.push_back({name, Color(sdl_color)});

                break;
        }
}

static void load_gui_color(xml::Element* gui_e,
                           const std::string type,
                           SDL_Color& target_color)
{
        for (auto e = xml::first_child(gui_e) ;
             e;
             e = xml::next_sibling(e))
        {
                const std::string current_type =
                        xml::get_attribute_str(e, "type");

                if (current_type != type)
                {
                        continue;
                }

                const std::string name =
                        xml::get_attribute_str(e, "color");

                TRACE << "Loaded gui color - "
                      << "type: \"" << type << "\", "
                      << "name: \"" << name << "\""
                      << std::endl;

                const auto color = colors::name_to_color(name);

                target_color = color.sdl_color();

                str_color_pairs_.push_back({name, color});

                break;
        }
}

static void load_colors()
{
        tinyxml2::XMLDocument doc;

        xml::load_file("res/data/colors.xml", doc);

        auto colors_e = xml::first_child(doc);

        load_color(colors_e, "black", black_);
        load_color(colors_e, "extra_dark_gray", extra_dark_gray_);
        load_color(colors_e, "dark_gray", dark_gray_);
        load_color(colors_e, "gray", gray_);
        load_color(colors_e, "white", white_);
        load_color(colors_e, "light_white", light_white_);
        load_color(colors_e, "red", red_);
        load_color(colors_e, "light_red", light_red_);
        load_color(colors_e, "dark_green", dark_green_);
        load_color(colors_e, "green", green_);
        load_color(colors_e, "light_green", light_green_);
        load_color(colors_e, "dark_yellow", dark_yellow_);
        load_color(colors_e, "yellow", yellow_);
        load_color(colors_e, "blue", blue_);
        load_color(colors_e, "light_blue", light_blue_);
        load_color(colors_e, "magenta", magenta_);
        load_color(colors_e, "light_magenta", light_magenta_);
        load_color(colors_e, "cyan", cyan_);
        load_color(colors_e, "light_cyan", light_cyan_);
        load_color(colors_e, "brown", brown_);
        load_color(colors_e, "dark_brown", dark_brown_);
        load_color(colors_e, "gray_brown", gray_brown_);
        load_color(colors_e, "dark_gray_brown", dark_gray_brown_);
        load_color(colors_e, "violet", violet_);
        load_color(colors_e, "dark_violet", dark_violet_);
        load_color(colors_e, "orange", orange_);
        load_color(colors_e, "sepia", sepia_);
        load_color(colors_e, "light_sepia", light_sepia_);
        load_color(colors_e, "dark_sepia", dark_sepia_);
        load_color(colors_e, "teal", teal_);
        load_color(colors_e, "light_teal", light_teal_);
        load_color(colors_e, "dark_teal", dark_teal_);
}

static void load_gui_colors()
{
        tinyxml2::XMLDocument doc;

        xml::load_file("res/data/colors_gui.xml", doc);

        auto gui_e = xml::first_child(doc);

        load_gui_color(gui_e, "text", text_);
        load_gui_color(gui_e, "menu_highlight", menu_highlight_);
        load_gui_color(gui_e, "menu_dark", menu_dark_);
        load_gui_color(gui_e, "title", title_);
        load_gui_color(gui_e, "message_good", msg_good_);
        load_gui_color(gui_e, "message_bad", msg_bad_);
        load_gui_color(gui_e, "message_note", msg_note_);
        load_gui_color(gui_e, "monster_unaware", mon_unaware_bg_);
        load_gui_color(gui_e, "monster_allied", mon_allied_bg_);
        load_gui_color(gui_e, "monster_temp_property", mon_temp_property_bg_);
}

//-----------------------------------------------------------------------------
// Color
//-----------------------------------------------------------------------------
Color::Color() :
        sdl_color_({0, 0, 0, 0}),
        is_defined_(false)
{

}

Color::Color(uint8_t r, uint8_t g, uint8_t b) :
        sdl_color_({r, g, b, 0}),
        is_defined_(true)
{

}

Color::Color(const SDL_Color& sdl_color) :
        sdl_color_(sdl_color),
        is_defined_(true)
{

}

Color::~Color()
{

}

Color& Color::operator=(const Color& other)
{
        sdl_color_.r = other.sdl_color_.r;
        sdl_color_.g = other.sdl_color_.g;
        sdl_color_.b = other.sdl_color_.b;

        return *this;
}

bool Color::operator==(const Color& other) const
{
        return
                sdl_color_.r == other.sdl_color_.r &&
                sdl_color_.g == other.sdl_color_.g &&
                sdl_color_.b == other.sdl_color_.b;
}

bool Color::operator!=(const Color& other) const
{
        return
                sdl_color_.r != other.sdl_color_.r ||
                sdl_color_.g != other.sdl_color_.g ||
                sdl_color_.b != other.sdl_color_.b;
}

Color Color::fraction(const double div)
{
        auto result =
                Color((uint8_t)((double)sdl_color_.r / div),
                      (uint8_t)((double)sdl_color_.g / div),
                      (uint8_t)((double)sdl_color_.b / div));

        return result;
}

bool Color::is_defined() const
{
        return is_defined_;
}

void Color::clear()
{
        sdl_color_.r = sdl_color_.g = sdl_color_.b = 0;

        is_defined_ = false;
}

SDL_Color Color::sdl_color() const
{
        return sdl_color_;
}

uint8_t Color::r() const
{
        return sdl_color_.r;
}

uint8_t Color::g() const
{
        return sdl_color_.g;
}

uint8_t Color::b() const
{
        return sdl_color_.b;
}


void Color::set_r(const uint8_t value)
{
        sdl_color_.r = value;
}

void Color::set_g(const uint8_t value)
{
        sdl_color_.g = value;
}

void Color::set_b(const uint8_t value)
{
        sdl_color_.b = value;
}


// -----------------------------------------------------------------------------
// Color handling
// -----------------------------------------------------------------------------
namespace colors
{

void init()
{
        TRACE_FUNC_BEGIN;

        str_color_pairs_.clear();

        load_colors();

        load_gui_colors();

        TRACE_FUNC_END;
}

Color name_to_color(const std::string& name)
{
        auto search = std::find_if(begin(str_color_pairs_),
                                   end(str_color_pairs_),
                                   [name](const auto& str_color)
                                   {
                                           return str_color.first == name;
                                   });

        if (search == end(str_color_pairs_))
        {
                TRACE << "No color definition stored for color with name: "
                      << name << std::endl;

                ASSERT(false);

                return Color();
        }

        return search->second;
}

std::string color_to_name(const Color& color)
{
        auto search = std::find_if(
                begin(str_color_pairs_),
                end(str_color_pairs_),
                [color](const auto& str_color)
                {
                        return str_color.second == color;
                });

        if (search == end(str_color_pairs_))
        {
                const auto sdl_color = color.sdl_color();

                TRACE << "No color name stored for color with RGB: "
                      << sdl_color.r << ", "
                      << sdl_color.g << ", "
                      << sdl_color.b << std::endl;

                ASSERT(false);

                return "";
        }

        return search->first;
}

//-----------------------------------------------------------------------------
// Available colors
//-----------------------------------------------------------------------------
Color black()
{
        return Color(black_);
}

Color extra_dark_gray()
{
        return Color(extra_dark_gray_);
}

Color dark_gray()
{
        return Color(dark_gray_);
}

Color gray()
{
        return Color(gray_);
}

Color white()
{
        return Color(white_);
}

Color light_white()
{
        return Color(light_white_);
}

Color red()
{
        return Color(red_);
}

Color light_red()
{
        return Color(light_red_);
}

Color dark_green()
{
        return Color(dark_green_);
}

Color green()
{
        return Color(green_);
}

Color light_green()
{
        return Color(light_green_);
}

Color dark_yellow()
{
        return Color(dark_yellow_);
}

Color yellow()
{
        return Color(yellow_);
}

Color blue()
{
        return Color(blue_);
}

Color light_blue()
{
        return Color(light_blue_);
}

Color magenta()
{
        return Color(magenta_);
}

Color light_magenta()
{
        return Color(light_magenta_);
}

Color cyan()
{
        return Color(cyan_);
}

Color light_cyan()
{
        return Color(light_cyan_);
}

Color brown()
{
        return Color(brown_);
}

Color dark_brown()
{
        return Color(dark_brown_);
}

Color gray_brown()
{
        return Color(gray_brown_);
}

Color dark_gray_brown()
{
        return Color(dark_gray_brown_);
}

Color violet()
{
        return Color(violet_);
}

Color dark_violet()
{
        return Color(dark_violet_);
}

Color orange()
{
        return Color(orange_);
}

Color sepia()
{
        return Color(sepia_);
}

Color light_sepia()
{
        return Color(light_sepia_);
}

Color dark_sepia()
{
        return Color(dark_sepia_);
}

Color teal()
{
        return Color(teal_);
}

Color light_teal()
{
        return Color(light_teal_);
}

Color dark_teal()
{
        return Color(dark_teal_);
}


//-----------------------------------------------------------------------------
// GUI colors
//-----------------------------------------------------------------------------
Color text()
{
        return Color(text_);
}

Color menu_highlight()
{
        return Color(menu_highlight_);
}

Color menu_dark()
{
        return Color(menu_dark_);
}

Color title()
{
        return Color(title_);
}


Color msg_good()
{
        return Color(msg_good_);
}

Color msg_bad()
{
        return Color(msg_bad_);
}

Color msg_note()
{
        return Color(msg_note_);
}

Color mon_unaware_bg()
{
        return Color(mon_unaware_bg_);
}

Color mon_allied_bg()
{
        return Color(mon_allied_bg_);
}

Color mon_temp_property_bg()
{
        return Color(mon_temp_property_bg_);
}

} // colors
