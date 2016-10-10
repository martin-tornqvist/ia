#include "config.hpp"

#include "init.hpp"

#include <fstream>
#include <SDL_image.h>

#include "browser.hpp"
#include "query.hpp"
#include "io.hpp"
#include "audio.hpp"
#include "text_format.hpp"

// -----------------------------------------------------------------------------
// Config
// -----------------------------------------------------------------------------
namespace config
{

namespace
{

const int opt_y0_           = 1;
const int opt_values_x_pos_ = 40;

std::string  font_name_                 = "";
bool    use_light_fade_effect_          = false;
bool    is_fullscr_                     = false;
bool    is_tiles_wall_full_square_      = false;
bool    is_text_mode_wall_full_square_  = false;
bool    is_light_explosive_prompt_      = false;
bool    is_ranged_wpn_meleee_prompt_    = false;
bool    is_ranged_wpn_auto_reload_      = false;
bool    is_intro_lvl_skipped_           = false;
bool    is_any_key_confirm_more_        = false;
int     map_px_h_                       = -1;
int     log_px_h_                       = -1;
int     map_px_offset_h_                = -1;
int     stat_lines_px_h_                = -1;
int     stat_lines_px_offset_h_         = -1;
int     scr_px_w_                       = -1;
int     scr_px_h_                       = -1;
int     delay_projectile_draw_          = -1;
int     delay_shotgun_                  = -1;
int     delay_explosion_                = -1;
bool    is_bot_playing_                 = false;
bool    is_audio_enabled_               = false;
bool    is_tiles_mode_                  = false;
int     cell_px_w_                      = -1;
int     cell_px_h_                      = -1;

std::vector<std::string> font_image_names;

void set_cell_px_dim_dependent_variables()
{
    map_px_h_                   = cell_px_h_ * map_h;
    map_px_offset_h_            = cell_px_h_ * map_offset_h;
    log_px_h_                   = cell_px_h_ * log_h;
    stat_lines_px_h_            = cell_px_h_ * stat_lines_h;
    stat_lines_px_offset_h_     = cell_px_h_ * stat_lines_offset_h;
    scr_px_w_                   = cell_px_w_ * screen_w;
    scr_px_h_                   = cell_px_h_ * screen_h;
}

void set_cell_px_dims_from_font_name()
{
    TRACE_FUNC_BEGIN;
    std::string font_name = font_name_;

    char ch = 'a';

    while (ch < '0' || ch > '9')
    {
        font_name.erase(begin(font_name));
        ch = font_name[0];
    }

    std::string w_str = "";

    while (ch != 'x')
    {
        font_name.erase(begin(font_name));
        w_str += ch;
        ch = font_name[0];
    }

    font_name.erase(begin(font_name));
    ch = font_name[0];

    std::string h_str = "";

    while (ch != '_' && ch != '.')
    {
        font_name.erase(begin(font_name));
        h_str += ch;
        ch = font_name[0];
    }

    TRACE << "Parsed font image name, found dims: "
          << w_str << "x" << h_str << std::endl;

    cell_px_w_ = to_int(w_str);
    cell_px_h_ = to_int(h_str);
    TRACE_FUNC_END;
}

void set_default_variables()
{
    TRACE_FUNC_BEGIN;
    is_audio_enabled_               = true;
    is_tiles_mode_                  = true;
    font_name_                      = "images/16x24_v1.png";

    set_cell_px_dims_from_font_name();

    use_light_fade_effect_          = false;
    is_fullscr_                     = false;
    is_tiles_wall_full_square_      = false;
    is_text_mode_wall_full_square_  = true;
    is_intro_lvl_skipped_           = false;
    is_any_key_confirm_more_        = false;
    is_light_explosive_prompt_      = false;
    is_ranged_wpn_meleee_prompt_    = true;
    is_ranged_wpn_auto_reload_      = false;
    delay_projectile_draw_          = 25;
    delay_shotgun_                  = 75;
    delay_explosion_                = 225;
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

    case 1: // Tiles mode
    {
        is_tiles_mode_ = !is_tiles_mode_;

        if (is_tiles_mode_ && (cell_px_w_ != 16 || cell_px_h_ != 24))
        {
            font_name_ = "images/16x24_v1.png";
        }

        set_cell_px_dims_from_font_name();
        set_cell_px_dim_dependent_variables();
        io::init();
    }
    break;

    case 2: // Font
    {
        for (size_t i = 0; i < font_image_names.size(); ++i)
        {
            if (font_name_ == font_image_names[i])
            {
                font_name_ = i == font_image_names.size() - 1 ?
                             font_image_names.front() :
                             font_image_names[i + 1];
                break;
            }
        }

        set_cell_px_dims_from_font_name();

        if (is_tiles_mode_)
        {
            while (cell_px_w_ != 16 || cell_px_h_ != 24)
            {
                for (size_t i = 0; i < font_image_names.size(); ++i)
                {
                    if (font_name_ == font_image_names[i])
                    {
                        font_name_ = i == font_image_names.size() - 1 ?
                                     font_image_names.front() :
                                     font_image_names[i + 1];
                        break;
                    }
                }

                set_cell_px_dims_from_font_name();
            }
        }

        set_cell_px_dim_dependent_variables();
        io::init();
    }
    break;

    case 3: // Light fade effect
    {
        use_light_fade_effect_ = !use_light_fade_effect_;
    }
    break;

    case 4: // Fullscreen
    {
        is_fullscr_ = !is_fullscr_;
        io::init();
    }
    break;

    case 5: // Tiles mode wall symbol
    {
        is_tiles_wall_full_square_ = !is_tiles_wall_full_square_;
    }
    break;

    case 6: // Text mode wall symbol
    {
        is_text_mode_wall_full_square_ = !is_text_mode_wall_full_square_;
    }
    break;

    case 7: // Skip intro level
    {
        is_intro_lvl_skipped_ = !is_intro_lvl_skipped_;
    }
    break;

    case 8: // Confirm "more" with any key
    {
        is_any_key_confirm_more_ = !is_any_key_confirm_more_;
    }
    break;

    case 9: // Print warning when lighting explovies
    {
        is_light_explosive_prompt_ = !is_light_explosive_prompt_;
    }
    break;

    case 10: // Print warning when melee attacking with ranged weapons
    {
        is_ranged_wpn_meleee_prompt_ = !is_ranged_wpn_meleee_prompt_;
    }
    break;

    case 11: // Ranged weapon auto reload
    {
        is_ranged_wpn_auto_reload_ = !is_ranged_wpn_auto_reload_;
    }
    break;

    case 12: // Projectile delay
    {
        const P p(opt_values_x_pos_, opt_y0_ + browser.y());

        const int nr = query::number(p,
                                     clr_menu_highlight,
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

    case 13: // Shotgun delay
    {
        const P p(opt_values_x_pos_, opt_y0_ + browser.y());

        const int nr = query::number(p,
                                     clr_menu_highlight,
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

    case 14: // Explosion delay
    {
        const P p(opt_values_x_pos_, opt_y0_ + browser.y());

        const int nr = query::number(p,
                                     clr_menu_highlight,
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

    case 15: // Reset to defaults
    {
        set_default_variables();
        set_cell_px_dims_from_font_name();
        set_cell_px_dim_dependent_variables();
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
    file.open("data/config");

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

    std::string current_line = lines.front();
    is_audio_enabled_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();

    if (current_line == "0")
    {
        is_tiles_mode_ = false;
    }
    else
    {
        is_tiles_mode_ = true;

        if (cell_px_w_ != 16 || cell_px_h_ != 24)
        {
            font_name_ = "images/16x24_v1.png";
            set_cell_px_dims_from_font_name();
        }
    }

    lines.erase(begin(lines));

    current_line = lines.front();
    font_name_ = current_line;
    set_cell_px_dims_from_font_name();
    lines.erase(begin(lines));

    current_line = lines.front();
    use_light_fade_effect_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_fullscr_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_tiles_wall_full_square_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_text_mode_wall_full_square_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_intro_lvl_skipped_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_any_key_confirm_more_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_light_explosive_prompt_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_ranged_wpn_meleee_prompt_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    is_ranged_wpn_auto_reload_ = current_line == "1";
    lines.erase(begin(lines));

    current_line = lines.front();
    delay_projectile_draw_ = to_int(current_line);
    lines.erase(begin(lines));

    current_line = lines.front();
    delay_shotgun_ = to_int(current_line);
    lines.erase(begin(lines));

    current_line = lines.front();
    delay_explosion_ = to_int(current_line);
    lines.erase(begin(lines));

    TRACE_FUNC_END;
}

void write_lines_to_file(std::vector<std::string>& lines)
{
    std::ofstream file;
    file.open("data/config", std::ios::trunc);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        file << lines[i];

        if (i != lines.size() - 1)
        {
            file << std::endl;
        }
    }

    file.close();
}

void set_lines_from_variables(std::vector<std::string>& lines)
{
    TRACE_FUNC_BEGIN;
    lines.clear();
    lines.push_back(is_audio_enabled_               ? "1" : "0");
    lines.push_back(is_tiles_mode_                  ? "1" : "0");
    lines.push_back(font_name_);
    lines.push_back(use_light_fade_effect_          ? "1" : "0");
    lines.push_back(is_fullscr_                     ? "1" : "0");
    lines.push_back(is_tiles_wall_full_square_      ? "1" : "0");
    lines.push_back(is_text_mode_wall_full_square_  ? "1" : "0");
    lines.push_back(is_intro_lvl_skipped_           ? "1" : "0");
    lines.push_back(is_any_key_confirm_more_        ? "1" : "0");
    lines.push_back(is_light_explosive_prompt_      ? "1" : "0");
    lines.push_back(is_ranged_wpn_meleee_prompt_    ? "1" : "0");
    lines.push_back(is_ranged_wpn_auto_reload_      ? "1" : "0");
    lines.push_back(to_str(delay_projectile_draw_));
    lines.push_back(to_str(delay_shotgun_));
    lines.push_back(to_str(delay_explosion_));
    TRACE_FUNC_END;
}

} //namespace

void init()
{
    font_name_      = "";
    is_bot_playing_ = false;

    font_image_names.clear();
    font_image_names.push_back("images/8x12_DOS.png");
    font_image_names.push_back("images/11x19.png");
    font_image_names.push_back("images/11x22.png");
    font_image_names.push_back("images/12x24.png");
    font_image_names.push_back("images/16x24_v1.png");
    font_image_names.push_back("images/16x24_v2.png");
    font_image_names.push_back("images/16x24_v3.png");
    font_image_names.push_back("images/16x24_DOS.png");
    font_image_names.push_back("images/16x24_typewriter_v1.png");
    font_image_names.push_back("images/16x24_typewriter_v2.png");

    set_default_variables();

    std::vector<std::string> lines;

    // Load config file, if it exists
    read_file(lines);

    if (lines.empty())
    {
        // No config file, just keep the default values

        // Store the default values to strings, to create a new config file
        set_lines_from_variables(lines);
    }
    else // A configuration did exist
    {
        // Set values from parsed config lines
        set_variables_from_lines(lines);
    }

    set_cell_px_dim_dependent_variables();
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

void toggle_fullscreen()
{
    is_fullscr_ = !is_fullscr_;
    set_cell_px_dims_from_font_name();
    set_cell_px_dim_dependent_variables();

    io::on_toggle_fullscreen();

    std::vector<std::string> lines;
    set_lines_from_variables(lines);
    write_lines_to_file(lines);
}

} // config

// -----------------------------------------------------------------------------
// Config state
// -----------------------------------------------------------------------------
ConfigState::ConfigState() :
    State       (),
    browser_    (16)
{

}

void ConfigState::update()
{
    const auto input = io::get();

    const MenuAction action = browser_.read(input,
                                            MenuInputMode::scrolling);

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

        std::vector<std::string> lines;

        config::set_lines_from_variables(lines);

        config::write_lines_to_file(lines);
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
                      clr_white);

    std::string font_disp_name;

    text_format::replace_all(config::font_name_,
                             "images/",
                             "",
                             font_disp_name);

    text_format::replace_all(font_disp_name,
                             "_",
                             " ",
                             font_disp_name);

    text_format::replace_all(font_disp_name,
                             ".png",
                             "",
                             font_disp_name);

    std::vector< std::pair< std::string, std::string > > labels =
    {
        {
            "Play audio",
            config::is_audio_enabled_ ? "Yes" : "No"
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
            "Warning when lighting explosive",
            config::is_light_explosive_prompt_ ? "Yes" : "No"
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
            to_str(config::delay_projectile_draw_)
        },

        {
            "Shotgun delay (ms)",
            to_str(config::delay_shotgun_)
        },

        {
            "Explosion delay (ms)",
            to_str(config::delay_explosion_)
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

        const Clr& clr =
            browser_.y() == (int)i ?
            clr_menu_highlight :
            clr_menu_drk;


        const int y = config::opt_y0_ + i;

        io::draw_text(str_l,
                          Panel::screen,
                          P(0, y),
                          clr);

        if (str_r != "")
        {
            io::draw_text(":",
                              Panel::screen,
                              P(x1 - 2, y),
                              clr);

            io::draw_text(str_r,
                              Panel::screen,
                              P(x1, y),
                              clr);
        }
    } // for each label

    io::draw_text("[enter] to set option [space/esc] to exit",
                      Panel::screen,
                      P(0, 18),
                      clr_white);

    str =
        "NOTE: Tile set requires a resolution 1280x720 or higher. Using "
        "Text mode for smaller resolutions is recommended (fonts of different "
        "sizes are available)";

    std::vector<std::string> lines;

    text_format::split(str, screen_w, lines);

    int y = screen_h - lines.size();

    for (const std::string& line : lines)
    {
        io::draw_text(line,
                          Panel::screen,
                          P(0, y),
                          clr_gray);

        ++y;
    }
}
