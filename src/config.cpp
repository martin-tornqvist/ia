#include "config.hpp"

#include "init.hpp"

#include <fstream>
#include <SDL_image.h>

#include "browser.hpp"
#include "query.hpp"
#include "io.hpp"
#include "sdl_base.hpp"
#include "audio.hpp"
#include "text_format.hpp"

// -----------------------------------------------------------------------------
// Config
// -----------------------------------------------------------------------------
namespace config
{

namespace
{

const std::vector<std::string> font_image_names
{
    "8x12_DOS.png",
    "11x19.png",
    "11x22.png",
    "12x24.png",
    "16x24_v1.png",
    "16x24_v2.png",
    "16x24_v3.png",
    "16x24_DOS.png",
    "16x24_typewriter_v1.png",
    "16x24_typewriter_v2.png",
};

const int opt_y0_ = 1;
const int opt_values_x_pos_ = 40;

std::string font_name_ = "";
bool use_light_fade_effect_ = false;
bool is_fullscr_ = false;
bool is_tiles_wall_full_square_ = false;
bool is_text_mode_wall_full_square_ = false;
bool is_light_explosive_prompt_ = false;
bool is_drink_malign_pot_prompt_ = false;
bool is_ranged_wpn_meleee_prompt_ = false;
bool is_ranged_wpn_auto_reload_ = false;
bool is_intro_lvl_skipped_ = false;
bool is_any_key_confirm_more_ = false;
int map_px_h_ = -1;
int log_px_h_ = -1;
int map_px_offset_h_ = -1;
int stat_lines_px_h_ = -1;
int stat_lines_px_offset_h_ = -1;
int scr_px_w_ = -1;
int scr_px_h_ = -1;
int delay_projectile_draw_ = -1;
int delay_shotgun_ = -1;
int delay_explosion_ = -1;
std::string default_player_name_ = "";
bool is_bot_playing_ = false;
bool is_audio_enabled_ = false;
bool is_amb_audio_enabled_ = false;
bool is_tiles_mode_ = false;
int cell_px_w_ = -1;
int cell_px_h_ = -1;

P parse_dims_from_font_name(std::string font_name)
{
    char ch = font_name.front();

    while (ch < '0' || ch > '9')
    {
        font_name.erase(begin(font_name));

        ch = font_name.front();
    }

    std::string w_str = "";

    while (ch != 'x')
    {
        font_name.erase(begin(font_name));

        w_str += ch;

        ch = font_name.front();
    }

    font_name.erase(begin(font_name));

    ch = font_name.front();

    std::string h_str = "";

    while (ch != '_' && ch != '.')
    {
        font_name.erase(begin(font_name));

        h_str += ch;

        ch = font_name.front();
    }

    TRACE << "Parsed font image name, found dims: "
          << w_str << "x" << h_str << std::endl;

    const int w = to_int(w_str);
    const int h = to_int(h_str);

    return P(w, h);
}

void update_render_dims()
{
    TRACE_FUNC_BEGIN;

    const P font_dims = parse_dims_from_font_name(font_name_);

    cell_px_w_ = font_dims.x;
    cell_px_h_ = font_dims.y;

    map_px_h_ = cell_px_h_ * map_h;

    map_px_offset_h_ = cell_px_h_ * map_offset_h;

    log_px_h_ = cell_px_h_ * log_h;

    stat_lines_px_h_ = cell_px_h_ * stat_lines_h;

    stat_lines_px_offset_h_ = cell_px_h_ * stat_lines_offset_h;

    scr_px_w_ = cell_px_w_ * screen_w;

    scr_px_h_ = cell_px_h_ * screen_h;

    TRACE_FUNC_END;
}

void set_default_variables()
{
    TRACE_FUNC_BEGIN;

    is_audio_enabled_ = true;
    is_amb_audio_enabled_ = true;
    is_tiles_mode_ = true;
    font_name_ = "16x24_v1.png";

    update_render_dims();

    use_light_fade_effect_ = false;
    is_fullscr_ = false;
    is_tiles_wall_full_square_ = false;
    is_text_mode_wall_full_square_ = true;
    is_intro_lvl_skipped_ = false;
    is_any_key_confirm_more_ = true;
    is_light_explosive_prompt_ = false;
    is_drink_malign_pot_prompt_ = false;
    is_ranged_wpn_meleee_prompt_ = true;
    is_ranged_wpn_auto_reload_ = false;
    delay_projectile_draw_ = 30;
    delay_shotgun_ = 75;
    delay_explosion_ = 225;
    default_player_name_ = "";

    TRACE_FUNC_END;
}

void player_sets_option(const MenuBrowser& browser)
{
    switch (browser.y())
    {
    case 0: // Audio
    {
        is_audio_enabled_ = !is_audio_enabled_;

        audio::init();
    }
    break;

    case 1: // Ambient audio
    {
        is_amb_audio_enabled_ = !is_amb_audio_enabled_;

        audio::init();
    }
    break;

    case 2: // Tiles mode
    {
        is_tiles_mode_ = !is_tiles_mode_;

        // If we do not have a font loaded with the same size as the tiles, use
        // the first font with matching size
        if (is_tiles_mode_ &&
            ((cell_px_w_ != tile_px_w) ||
             (cell_px_h_ != tile_px_h)))
        {
            for (const auto& font_name : font_image_names)
            {
                const P font_dims = parse_dims_from_font_name(font_name);

                if (font_dims == P(tile_px_w, tile_px_h))
                {
                    font_name_ = font_name;

                    break;
                }
            }
        }

        update_render_dims();

        sdl_base::init();

        io::init();
    }
    break;

    case 3: // Font
    {
        // Set next font
        for (size_t i = 0; i < font_image_names.size(); ++i)
        {
            if (font_name_ == font_image_names[i])
            {
                font_name_ =
                    (i == (font_image_names.size() - 1)) ?
                    font_image_names.front() :
                    font_image_names[i + 1];
                break;
            }
        }

        update_render_dims();

        // In tiles mode - skip fonts until we find one with matching size
        if (is_tiles_mode_)
        {
            while ((cell_px_w_ != tile_px_w) ||
                   (cell_px_h_ != tile_px_h))
            {
                for (size_t i = 0; i < font_image_names.size(); ++i)
                {
                    if (font_name_ == font_image_names[i])
                    {
                        font_name_ =
                            (i == font_image_names.size() - 1) ?
                            font_image_names.front() :
                            font_image_names[i + 1];
                        break;
                    }
                }

                update_render_dims();
            }
        }

        sdl_base::init();

        io::init();
    }
    break;

    case 4: // Light fade effect
    {
        use_light_fade_effect_ = !use_light_fade_effect_;
    }
    break;

    case 5: // Fullscreen
    {
        toggle_fullscreen();
    }
    break;

    case 6: // Tiles mode wall symbol
    {
        is_tiles_wall_full_square_ = !is_tiles_wall_full_square_;
    }
    break;

    case 7: // Text mode wall symbol
    {
        is_text_mode_wall_full_square_ = !is_text_mode_wall_full_square_;
    }
    break;

    case 8: // Skip intro level
    {
        is_intro_lvl_skipped_ = !is_intro_lvl_skipped_;
    }
    break;

    case 9: // Confirm "more" with any key
    {
        is_any_key_confirm_more_ = !is_any_key_confirm_more_;
    }
    break;

    case 10: // Print warning when lighting explovies
    {
        is_light_explosive_prompt_ = !is_light_explosive_prompt_;
    }
    break;

    case 11: // Print warning when drinking known malign potions
    {
        is_drink_malign_pot_prompt_ = !is_drink_malign_pot_prompt_;
    }
    break;

    case 12: // Print warning when melee attacking with ranged weapons
    {
        is_ranged_wpn_meleee_prompt_ = !is_ranged_wpn_meleee_prompt_;
    }
    break;

    case 13: // Ranged weapon auto reload
    {
        is_ranged_wpn_auto_reload_ = !is_ranged_wpn_auto_reload_;
    }
    break;

    case 14: // Projectile delay
    {
        const P p(opt_values_x_pos_, opt_y0_ + browser.y());

        const int nr = query::number(
            p,
            colors::menu_highlight(),
            1,
            3,
            delay_projectile_draw_,
            true);

        if (nr != -1)
        {
            delay_projectile_draw_ = nr;
        }
    }
    break;

    case 15: // Shotgun delay
    {
        const P p(opt_values_x_pos_, opt_y0_ + browser.y());

        const int nr = query::number(
            p,
            colors::menu_highlight(),
            1,
            3,
            delay_shotgun_,
            true);

        if (nr != -1)
        {
            delay_shotgun_ = nr;
        }
    }
    break;

    case 16: // Explosion delay
    {
        const P p(opt_values_x_pos_, opt_y0_ + browser.y());

        const int nr = query::number(
            p,
            colors::menu_highlight(),
            1,
            3,
            delay_explosion_,
            true);

        if (nr != -1)
        {
            delay_explosion_ = nr;
        }
    }
    break;

    case 17: // Reset to defaults
    {
        set_default_variables();

        update_render_dims();

        sdl_base::init();

        io::init();

        audio::init();
    }
    break;

    default:
        ASSERT(false);
        break;
    }
}

void read_file(std::vector<std::string>& lines)
{
    std::ifstream file;
    file.open("res/data/config");

    if (file.is_open())
    {
        std::string line;

        while (getline(file, line))
        {
            lines.push_back(line);
        }

        file.close();
    }
}

void set_variables_from_lines(std::vector<std::string>& lines)
{
    TRACE_FUNC_BEGIN;

    is_audio_enabled_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_amb_audio_enabled_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_tiles_mode_ = lines.front() == "1";
    lines.erase(begin(lines));

    font_name_ = lines.front();
    lines.erase(begin(lines));

    update_render_dims();

    use_light_fade_effect_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_fullscr_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_tiles_wall_full_square_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_text_mode_wall_full_square_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_intro_lvl_skipped_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_any_key_confirm_more_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_light_explosive_prompt_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_drink_malign_pot_prompt_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_ranged_wpn_meleee_prompt_ = lines.front() == "1";
    lines.erase(begin(lines));

    is_ranged_wpn_auto_reload_ = lines.front() == "1";
    lines.erase(begin(lines));

    delay_projectile_draw_ = to_int(lines.front());
    lines.erase(begin(lines));

    delay_shotgun_ = to_int(lines.front());
    lines.erase(begin(lines));

    delay_explosion_ = to_int(lines.front());
    lines.erase(begin(lines));

    default_player_name_ = "";

    if (lines.front() == "1")
    {
        lines.erase(begin(lines));

        default_player_name_ = lines.front();
    }

    lines.erase(begin(lines));

    ASSERT(lines.empty());

    TRACE_FUNC_END;
}

void write_lines_to_file(const std::vector<std::string>& lines)
{
    std::ofstream file;
    file.open("res/data/config", std::ios::trunc);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        file << lines[i];

        if (i != (lines.size() - 1))
        {
            file << std::endl;
        }
    }

    file.close();
}

std::vector<std::string> lines_from_variables()
{
    TRACE_FUNC_BEGIN;

    std::vector<std::string> lines;

    lines.clear();
    lines.push_back(is_audio_enabled_ ? "1" : "0");
    lines.push_back(is_amb_audio_enabled_ ? "1" : "0");
    lines.push_back(is_tiles_mode_ ? "1" : "0");
    lines.push_back(font_name_);
    lines.push_back(use_light_fade_effect_ ? "1" : "0");
    lines.push_back(is_fullscr_ ? "1" : "0");
    lines.push_back(is_tiles_wall_full_square_ ? "1" : "0");
    lines.push_back(is_text_mode_wall_full_square_ ? "1" : "0");
    lines.push_back(is_intro_lvl_skipped_ ? "1" : "0");
    lines.push_back(is_any_key_confirm_more_ ? "1" : "0");
    lines.push_back(is_light_explosive_prompt_ ? "1" : "0");
    lines.push_back(is_drink_malign_pot_prompt_ ? "1" : "0");
    lines.push_back(is_ranged_wpn_meleee_prompt_ ? "1" : "0");
    lines.push_back(is_ranged_wpn_auto_reload_ ? "1" : "0");
    lines.push_back(std::to_string(delay_projectile_draw_));
    lines.push_back(std::to_string(delay_shotgun_));
    lines.push_back(std::to_string(delay_explosion_));

    if (default_player_name_.empty())
    {
        lines.push_back("0");
    }
    else // Default player name has been set
    {
        lines.push_back("1");

        lines.push_back(default_player_name_);
    }

    TRACE_FUNC_END;

    return lines;
}

} // namespace

void init()
{
    font_name_ = "";
    is_bot_playing_ = false;

    set_default_variables();

    std::vector<std::string> lines;

    // Load config file, if it exists
    read_file(lines);

    if (!lines.empty())
    {
        // A config file exists, set values from parsed config lines
        set_variables_from_lines(lines);
    }

    update_render_dims();
}

bool is_tiles_mode()
{
    return is_tiles_mode_;
}

std::string font_name()
{
    return font_name_;
}

bool use_light_fade_effect()
{
    return use_light_fade_effect_;
}

bool is_fullscreen()
{
    return is_fullscr_;
}

int scr_px_w()
{
    return scr_px_w_;
}

int scr_px_h()
{
    return scr_px_h_;
}

int cell_px_w()
{
    return cell_px_w_;
}

int cell_px_h()
{
    return cell_px_h_;
}

int log_px_h()
{
    return log_px_h_;
}

int map_px_h()
{
    return map_px_h_;
}

int map_px_offset_h()
{
    return map_px_offset_h_;
}

int stat_lines_px_offset_h()
{
    return stat_lines_px_offset_h_;
}
int stat_lines_px_h()
{
    return stat_lines_px_h_;
}

bool is_text_mode_wall_full_square()
{
    return is_text_mode_wall_full_square_;
}

bool is_tiles_wall_full_square()
{
    return is_tiles_wall_full_square_;
}

bool is_audio_enabled()
{
    return is_audio_enabled_;
}

bool is_amb_audio_enabled()
{
    return is_amb_audio_enabled_;
}

bool is_bot_playing()
{
    return is_bot_playing_;
}

void toggle_bot_playing()
{
    is_bot_playing_ = !is_bot_playing_;
}

bool is_light_explosive_prompt()
{
    return is_light_explosive_prompt_;
}

bool is_drink_malign_pot_prompt()
{
    return is_drink_malign_pot_prompt_;
}

bool is_ranged_wpn_meleee_prompt()
{
    return is_ranged_wpn_meleee_prompt_;
}

bool is_ranged_wpn_auto_reload()
{
    return is_ranged_wpn_auto_reload_;
}

bool is_intro_lvl_skipped()
{
    return is_intro_lvl_skipped_;
}

bool is_any_key_confirm_more()
{
    return is_any_key_confirm_more_;
}

int delay_projectile_draw()
{
    return delay_projectile_draw_;
}
int delay_shotgun()
{
    return delay_shotgun_;
}

int delay_explosion()
{
    return delay_explosion_;
}

void set_default_player_name(const std::string& name)
{
    default_player_name_ = name;

    const auto lines = lines_from_variables();

    write_lines_to_file(lines);
}

std::string default_player_name()
{
    return default_player_name_;
}

void toggle_fullscreen()
{
    is_fullscr_ = !is_fullscr_;

    update_render_dims();

    io::clear_screen();

    io::update_screen();

    sdl_base::init();

    io::init();

    states::draw();

    io::update_screen();

    const auto lines = lines_from_variables();

    write_lines_to_file(lines);
}

} // config

// -----------------------------------------------------------------------------
// Config state
// -----------------------------------------------------------------------------
ConfigState::ConfigState() :
    State       (),
    browser_    (18)
{

}

StateId ConfigState::id()
{
    return StateId::config;
}

void ConfigState::update()
{
    const auto input = io::get(true);

    const MenuAction action =
        browser_.read(input, MenuInputMode::scrolling);

    switch (action)
    {
    case MenuAction::esc:
    case MenuAction::space:
    {
        // Since text mode wall symbol may have changed, we need to
        // redefine the feature data list
        feature_data::init();

        states::pop();

        return;
    }
    break;

    case MenuAction::selected:
    {
        config::player_sets_option(browser_);

        const auto lines = config::lines_from_variables();

        config::write_lines_to_file(lines);

        io::flush_input();
    }
    break;

    default:
        break;
    }
}

void ConfigState::draw()
{
    const int x1 = config::opt_values_x_pos_;

    std::string str = "";

    io::draw_text("-Options-",
                  Panel::screen,
                  P(0, 0),
                  colors::white());

    std::string font_disp_name = config::font_name_;

    std::vector< std::pair< std::string, std::string > > labels =
    {
        {
            "Enable audio",
            config::is_audio_enabled_ ? "Yes" : "No"
        },

        {
            "Play ambient sounds",
            config::is_amb_audio_enabled_ ? "Yes" : "No"
        },

        {
            "Use tile set",
            config::is_tiles_mode_ ? "Yes" : "No"
        },

        {
            "Font", font_disp_name
        },

        {
            "Use light fade effect",
            config::use_light_fade_effect_ ? "Yes" : "No"
        },

        {
            "Fullscreen",
            config::is_fullscr_ ? "Yes" : "No"
        },

        {
            "Tiles mode wall symbol",
            config::is_tiles_wall_full_square_ ? "Full square" : "Pseudo-3D"
        },

        {
            "Text mode wall symbol",
            config::is_text_mode_wall_full_square_ ? "Full square" : "Hash sign"
        },

        {
            "Skip intro level",
            config::is_intro_lvl_skipped_ ? "Yes" : "No"
        },

        {
            "Any key confirms \"-More-\" prompts",
            config::is_any_key_confirm_more_ ? "Yes" : "No"
        },

        {
            "Warn when lighting explosives",
            config::is_light_explosive_prompt_ ? "Yes" : "No"
        },

        {
            "Warn when drinking malign potions",
            config::is_drink_malign_pot_prompt_ ? "Yes" : "No"
        },

        {
            "Ranged weapon melee attack warning",
            config::is_ranged_wpn_meleee_prompt_ ? "Yes" : "No"
        },

        {
            "Ranged weapon auto reload",
            config::is_ranged_wpn_auto_reload_ ? "Yes" : "No"
        },

        {
            "Projectile delay (ms)",
            std::to_string(config::delay_projectile_draw_)
        },

        {
            "Shotgun delay (ms)",
            std::to_string(config::delay_shotgun_)
        },

        {
            "Explosion delay (ms)",
            std::to_string(config::delay_explosion_)
        },

        {
            "Reset to defaults",
            ""
        }
    };

    for (size_t i = 0; i < labels.size(); ++i)
    {
        const auto& label = labels[i];

        const std::string& str_l = label.first;
        const std::string& str_r = label.second;

        const auto& color =
            browser_.y() == (int)i ?
            colors::menu_highlight() :
            colors::menu_dark();


        const int y = config::opt_y0_ + i;

        io::draw_text(str_l,
                          Panel::screen,
                          P(0, y),
                          color);

        if (str_r != "")
        {
            io::draw_text(":",
                              Panel::screen,
                              P(x1 - 2, y),
                              color);

            io::draw_text(str_r,
                              Panel::screen,
                              P(x1, y),
                              color);
        }
    } // for each label

    io::draw_text("[enter] to set option [space/esc] to exit",
                  Panel::screen,
                  P(0, 20),
                  colors::white());

    str =
        "NOTE: Tile set requires a resolution 1280x720 or higher. Using "
        "Text mode for smaller resolutions is recommended (fonts of different "
        "sizes are available).";

    const auto lines = text_format::split(str, screen_w);

    int y = screen_h - lines.size();

    for (const std::string& line : lines)
    {
        io::draw_text(line,
                      Panel::screen,
                      P(0, y),
                      colors::gray());

        ++y;
    }
}
