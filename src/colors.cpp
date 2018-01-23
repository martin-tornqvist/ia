#include "colors.hpp"

#include <SDL_video.h>
#include <vector>

#include "tinyxml2.h"

#include "rl_utils.hpp"

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

namespace
{

// Defined in colors.xml
SDL_Color black_;
SDL_Color extra_dark_gray_;
SDL_Color dark_gray_;
SDL_Color gray_;
SDL_Color white_;
SDL_Color light_white_;
SDL_Color red_;
SDL_Color light_red_;
SDL_Color dark_green_;
SDL_Color green_;
SDL_Color light_green_;
SDL_Color dark_yellow_;
SDL_Color yellow_;
SDL_Color blue_;
SDL_Color light_blue_;
SDL_Color magenta_;
SDL_Color light_magenta_;
SDL_Color cyan_;
SDL_Color light_cyan_;
SDL_Color brown_;
SDL_Color dark_brown_;
SDL_Color gray_brown_;
SDL_Color dark_gray_brown_brown_;
SDL_Color violet_;
SDL_Color dark_violet_;
SDL_Color orange_;
SDL_Color sepia_;
SDL_Color light_sepia_;
SDL_Color dark_sepia_;
SDL_Color teal_;
SDL_Color light_teal_;
SDL_Color dark_teal_;

SDL_Color text_;
SDL_Color menu_highlight_;
SDL_Color menu_dark_;
SDL_Color title_;
SDL_Color msg_good_;
SDL_Color msg_bad_;
SDL_Color msg_note_;
SDL_Color allied_mon_;

std::vector< std::pair<std::string, Color> > str_color_pairs_;

void load_colors()
{
    tinyxml2::XMLDocument doc;

    const std::string xml_path = "res/data/colors.xml";

    TRACE << "Loading color definitions file: " << xml_path << std::endl;

    const auto load_result = doc.LoadFile(xml_path.c_str());

    if (load_result != tinyxml2::XML_SUCCESS)
    {
        TRACE << "Could not load color definitions file at: "
              << xml_path
              << std::endl;

        ASSERT(false);

        return;
    }

    TRACE << "Reading color definitions" << std::endl;

    auto parse_and_store = [&doc](const std::string name,
                                  SDL_Color& target_color)
    {
        auto e = doc.FirstChildElement()->FirstChildElement();

        for ( ; e; e = e->NextSiblingElement())
        {
            const std::string current_name = e->Attribute("name");

            if (current_name != name)
            {
                continue;
            }

            const std::string rgb_hex_str = e->Attribute("rgb_hex");

            if (rgb_hex_str.size() != 6)
            {
                TRACE << "Invalid rgb hex string: '"
                      << rgb_hex_str
                      << "'"
                      << std::endl;

                ASSERT(false);

                continue;
            }

            uint8_t rgb[3] = {};

            for (int i = 0; i < 3; ++i)
            {
                const std::string hex8_str = rgb_hex_str.substr(2 * i, 2);

                rgb[i] =  (uint8_t)std::stoi(hex8_str, 0, 16);
            }

            TRACE << "Found color definition --- "
                  << "name: \"" << name << "\""
                  << ", rgb hex string: \"" << rgb_hex_str << "\""
                  << ", rgb decimal: "
                  << (int)rgb[0] << ", "
                  << (int)rgb[1] << ", "
                  << (int)rgb[2]
                  << std::endl;

            const SDL_Color sdl_color = {rgb[0], rgb[1], rgb[2], 0};

            target_color = sdl_color;

            str_color_pairs_.push_back({name, Color(sdl_color)});

            break;
        }
    };

    parse_and_store("black", black_);
    parse_and_store("extra_dark_gray", extra_dark_gray_);
    parse_and_store("dark_gray", dark_gray_);
    parse_and_store("gray", gray_);
    parse_and_store("white", white_);
    parse_and_store("light_white", light_white_);
    parse_and_store("red", red_);
    parse_and_store("light_red", light_red_);
    parse_and_store("dark_green", dark_green_);
    parse_and_store("green", green_);
    parse_and_store("light_green", light_green_);
    parse_and_store("dark_yellow", dark_yellow_);
    parse_and_store("yellow", yellow_);
    parse_and_store("blue", blue_);
    parse_and_store("light_blue", light_blue_);
    parse_and_store("magenta", magenta_);
    parse_and_store("light_magenta", light_magenta_);
    parse_and_store("cyan", cyan_);
    parse_and_store("light_cyan", light_cyan_);
    parse_and_store("brown", brown_);
    parse_and_store("dark_brown", dark_brown_);
    parse_and_store("gray_brown", gray_brown_);
    parse_and_store("dark_gray_brown_brown", dark_gray_brown_brown_);
    parse_and_store("violet", violet_);
    parse_and_store("dark_violet", dark_violet_);
    parse_and_store("orange", orange_);
    parse_and_store("sepia", sepia_);
    parse_and_store("light_sepia", light_sepia_);
    parse_and_store("dark_sepia", dark_sepia_);
    parse_and_store("teal", teal_);
    parse_and_store("light_teal", light_teal_);
    parse_and_store("dark_teal", dark_teal_);
}

void load_gui_colors()
{
    tinyxml2::XMLDocument doc;

    const std::string xml_path = "res/data/colors_gui.xml";

    TRACE << "Loading gui color definitions file: " << xml_path << std::endl;

    const auto load_result = doc.LoadFile(xml_path.c_str());

    if (load_result != tinyxml2::XML_SUCCESS)
    {
        TRACE << "Could not load gui color definitions file at: "
              << xml_path
              << std::endl;

        ASSERT(false);

        return;
    }

    TRACE << "Reading gui color definitions" << std::endl;

    auto parse_and_store = [&doc](const std::string type,
                                  SDL_Color& target_color)
    {
        auto e = doc.FirstChildElement()->FirstChildElement();

        for ( ; e; e = e->NextSiblingElement())
        {
            const std::string current_type = e->Attribute("type");

            if (current_type != type)
            {
                continue;
            }

            const std::string name = e->Attribute("color");

            TRACE << "Found gui color definition --- "
                  << "type: \"" << type << "\", "
                  << "name: \"" << name << "\""
                  << std::endl;

            const auto color = name_to_color(name);

            target_color = color.sdl_color();

            str_color_pairs_.push_back({name, color});

            break;
        }
    };

    parse_and_store("text", text_);
    parse_and_store("menu_highlight", menu_highlight_);
    parse_and_store("menu_dark", menu_dark_);
    parse_and_store("title", title_);
    parse_and_store("message_good", msg_good_);
    parse_and_store("message_bad", msg_bad_);
    parse_and_store("message_note", msg_note_);
}

} // namespace

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
    auto search = std::find_if(begin(str_color_pairs_),
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

Color dark_gray_brown_brown()
{
    return Color(dark_gray_brown_brown_);
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

Color allied_mon()
{
    return Color(allied_mon_);
}

} // colors
