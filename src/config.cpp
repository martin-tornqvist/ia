#include "config.hpp"

#include "init.hpp"

#include <fstream>
#include <SDL_image.h>

#include "converters.hpp"
#include "menu_input_handling.hpp"
#include "menu_browser.hpp"
#include "query.hpp"
#include "render.hpp"
#include "input.hpp"
#include "audio.hpp"
#include "text_format.hpp"
#include "utils.hpp"


using namespace std;

namespace config
{

namespace
{

const int NR_OPTIONS  = 13;
const int OPT_Y0      = 1;

string  font_name_                      = "";
bool    is_fullscreen_                  = false;
bool    is_tiles_wall_full_square_      = false;
bool    is_ascii_wall_full_square_      = false;
bool    is_ranged_wpn_meleee_prompt_    = false;
bool    is_ranged_wpn_auto_reload_      = false;
bool    is_intro_lvl_skipped_           = false;
int     map_px_h_                       = -1;
int     log_px_h_                       = -1;
int     map_px_offset_h_                = -1;
int     char_lines_px_h_                = -1;
int     char_lines_px_offset_h_         = -1;
int     screen_px_w_                    = -1;
int     screen_px_h_                    = -1;
int     delay_projectile_draw_          = -1;
int     delay_shotgun_                  = -1;
int     delay_explosion_                = -1;
bool    is_bot_playing_                 = false;
bool    is_audio_enabled_               = false;
bool    is_tiles_mode_                  = false;
int     cell_w_                         = -1;
int     cell_h_                         = -1;

vector<string> font_image_names;

void set_cell_dims_from_font_name()
{
    TRACE_FUNC_BEGIN;
    string font_name = font_name_;

    char ch = 'a';

    while (ch < '0' || ch > '9')
    {
        font_name.erase(begin(font_name));
        ch = font_name[0];
    }

    string w_str = "";

    while (ch != 'x')
    {
        font_name.erase(begin(font_name));
        w_str += ch;
        ch = font_name[0];
    }

    font_name.erase(begin(font_name));
    ch = font_name[0];

    string h_str = "";

    while (ch != '_' && ch != '.')
    {
        font_name.erase(begin(font_name));
        h_str += ch;
        ch = font_name[0];
    }

    TRACE << "Parsed font image name, found dims: "
          << w_str << "x" << h_str << endl;

    cell_w_ = to_int(w_str);
    cell_h_ = to_int(h_str);
    TRACE_FUNC_END;
}

void set_default_variables()
{
    TRACE_FUNC_BEGIN;
    is_audio_enabled_               = true;
    is_tiles_mode_                  = true;
    font_name_                      = "images/16x24_v1.png";
    set_cell_dims_from_font_name();
    is_fullscreen_                  = false;
    is_tiles_wall_full_square_      = false;
    is_ascii_wall_full_square_      = true;
    is_intro_lvl_skipped_           = false;
    is_ranged_wpn_meleee_prompt_    = true;
    is_ranged_wpn_auto_reload_      = false;
    delay_projectile_draw_          = 50;
    delay_shotgun_                  = 120;
    delay_explosion_                = 300;
    TRACE_FUNC_END;
}

void player_sets_option(const Menu_browser* const browser, const int OPTION_VALUES_X_POS)
{
    switch (browser->get_pos().y)
    {
    case 0:
        is_audio_enabled_ = !is_audio_enabled_;
        audio::init();
        break;

    case 1:
        is_tiles_mode_ = !is_tiles_mode_;

        if (is_tiles_mode_ && (cell_w_ != 16 || cell_h_ != 24))
        {
            font_name_ = "images/16x24_v1.png";
        }

        set_cell_dims_from_font_name();
        set_cell_dim_dependent_variables();
        render::init();
        break;

    case 2:
        for (unsigned int i = 0; i < font_image_names.size(); ++i)
        {
            if (font_name_ == font_image_names[i])
            {
                font_name_ = i == font_image_names.size() - 1 ?
                             font_image_names.front() :
                             font_image_names[i + 1];
                break;
            }
        }

        set_cell_dims_from_font_name();

        if (is_tiles_mode_)
        {
            while (cell_w_ != 16 || cell_h_ != 24)
            {
                for (unsigned int i = 0; i < font_image_names.size(); ++i)
                {
                    if (font_name_ == font_image_names[i])
                    {
                        font_name_ = i == font_image_names.size() - 1 ?
                                     font_image_names.front() :
                                     font_image_names[i + 1];
                        break;
                    }
                }

                set_cell_dims_from_font_name();
            }
        }

        set_cell_dim_dependent_variables();
        render::init();
        break;

    case 3:
        is_fullscreen_ = !is_fullscreen_;
        render::init();
        break;

    case 4:
        is_tiles_wall_full_square_ = !is_tiles_wall_full_square_;
        break;

    case 5:
        is_ascii_wall_full_square_ = !is_ascii_wall_full_square_;
        break;

    case 6:
        is_intro_lvl_skipped_ = !is_intro_lvl_skipped_;
        break;

    case 7:
        is_ranged_wpn_meleee_prompt_ = !is_ranged_wpn_meleee_prompt_;
        break;

    case 8:
        is_ranged_wpn_auto_reload_ = !is_ranged_wpn_auto_reload_;
        break;

    case 9:
    {
        const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->get_pos().y);
        const int NR = query::number(p, clr_menu_highlight, 1, 3, delay_projectile_draw_, true);

        if (NR != -1)
        {
            delay_projectile_draw_ = NR;
        }
    } break;

    case 10:
    {
        const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->get_pos().y);
        const int NR =
            query::number(p, clr_menu_highlight, 1, 3, delay_shotgun_, true);

        if (NR != -1) {delay_shotgun_ = NR;}
    } break;

    case 11:
    {
        const Pos p(OPTION_VALUES_X_POS, OPT_Y0 + browser->get_pos().y);
        const int NR = query::number(p, clr_menu_highlight, 1, 3, delay_explosion_, true);

        if (NR != -1)
        {
            delay_explosion_ = NR;
        }
    } break;

    case 12:
        set_default_variables();
        set_cell_dims_from_font_name();
        set_cell_dim_dependent_variables();
        render::init();
        audio::init();
        break;

    default: {assert(false && "Illegal option number");} break;
    }
}

void draw(const Menu_browser* const browser, const int OPTION_VALUES_X_POS)
{
    render::clear_screen();

    int opt_nr = 0;

    const int X0 = 0;
    const int X1 = OPTION_VALUES_X_POS;

    string str = "";

    render::draw_text("-Options-", Panel::screen, Pos(X0, 0), clr_white);

    render::draw_text("Play audio", Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_audio_enabled_ ? "Yes" : "No";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    render::draw_text("Use tile set", Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_tiles_mode_ ? "Yes" : "No";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    render::draw_text("Font", Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    string font_disp_name;
    text_format::replace_all(font_name_,      "images/",  "",   font_disp_name);
    text_format::replace_all(font_disp_name,  "_",        " ",  font_disp_name);
    text_format::replace_all(font_disp_name,  ".png",     "",   font_disp_name);
    render::draw_text(font_disp_name, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    render::draw_text("Fullscreen (experimental)", Panel::screen,
                      Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(is_fullscreen_ ? "Yes" : "No",
                      Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Tiles mode wall symbol";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_tiles_wall_full_square_ ? "Full square" : "Pseudo-3D";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Ascii mode wall symbol";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_ascii_wall_full_square_ ? "Full square" : "Hash sign";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    render::draw_text("Skip intro level", Panel::screen,
                      Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_intro_lvl_skipped_ ? "Yes" : "No";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Ranged weapon melee attack warning";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_ranged_wpn_meleee_prompt_ ? "Yes" : "No";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Ranged weapon auto reload";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    str = is_ranged_wpn_auto_reload_ ? "Yes" : "No";
    render::draw_text(str, Panel::screen, Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Projectile delay (ms)";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(to_str(delay_projectile_draw_), Panel::screen,
                      Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Shotgun delay (ms)";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(to_str(delay_shotgun_), Panel::screen,
                      Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Explosion delay (ms)";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(":", Panel::screen, Pos(X1 - 2, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    render::draw_text(to_str(delay_explosion_), Panel::screen,
                      Pos(X1, OPT_Y0 + opt_nr),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);
    opt_nr++;

    str = "Reset to defaults";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr + 1),
                      browser->get_pos().y == opt_nr ?
                      clr_menu_highlight : clr_menu_drk);

    str = "[space/esc] to confirm changes";
    render::draw_text(str, Panel::screen, Pos(X0, OPT_Y0 + opt_nr + 4),
                      clr_white);

    str  = "Tile set requires resolution 1280x720 or higher. ";
    str += "Using ASCII mode for smaller";
    render::draw_text(str, Panel::screen, Pos(X0, SCREEN_H - 2), clr_gray);
    str = "resolutions is recommended (fonts of different sizes are available).";
    render::draw_text(str, Panel::screen, Pos(X0, SCREEN_H - 1), clr_gray);

    render::update_screen();
}

void read_file(vector<string>& lines)
{
    ifstream file;
    file.open("data/config");

    if (file.is_open())
    {
        string line;

        while (getline(file, line))
        {
            lines.push_back(line);
        }

        file.close();
    }
}

void set_all_variables_from_lines(vector<string>& lines)
{
    TRACE_FUNC_BEGIN;

    string cur_line = lines.front();
    is_audio_enabled_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();

    if (cur_line == "0")
    {
        is_tiles_mode_ = false;
    }
    else
    {
        is_tiles_mode_ = true;

        if (cell_w_ != 16 || cell_h_ != 24)
        {
            font_name_ = "images/16x24_v1.png";
            set_cell_dims_from_font_name();
        }
    }

    lines.erase(begin(lines));

    cur_line = lines.front();
    font_name_ = cur_line;
    set_cell_dims_from_font_name();
    lines.erase(begin(lines));

    cur_line = lines.front();
    is_fullscreen_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();
    is_tiles_wall_full_square_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();
    is_ascii_wall_full_square_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();
    is_intro_lvl_skipped_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();
    is_ranged_wpn_meleee_prompt_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();
    is_ranged_wpn_auto_reload_ = cur_line == "1";
    lines.erase(begin(lines));

    cur_line = lines.front();
    delay_projectile_draw_ = to_int(cur_line);
    lines.erase(begin(lines));

    cur_line = lines.front();
    delay_shotgun_ = to_int(cur_line);
    lines.erase(begin(lines));

    cur_line = lines.front();
    delay_explosion_ = to_int(cur_line);
    lines.erase(begin(lines));

    TRACE_FUNC_END;
}

void write_lines_to_file(vector<string>& lines)
{
    ofstream file;
    file.open("data/config", ios::trunc);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        file << lines[i];

        if (i != lines.size() - 1) {file << endl;}
    }

    file.close();
}

void collect_lines_from_variables(vector<string>& lines)
{
    TRACE_FUNC_BEGIN;
    lines.clear();
    lines.push_back(is_audio_enabled_               ? "1" : "0");
    lines.push_back(is_tiles_mode_                  ? "1" : "0");
    lines.push_back(font_name_);
    lines.push_back(is_fullscreen_                  ? "1" : "0");
    lines.push_back(is_tiles_wall_full_square_      ? "1" : "0");
    lines.push_back(is_ascii_wall_full_square_      ? "1" : "0");
    lines.push_back(is_intro_lvl_skipped_           ? "1" : "0");
    lines.push_back(is_ranged_wpn_meleee_prompt_    ? "1" : "0");
    lines.push_back(is_ranged_wpn_auto_reload_      ? "1" : "0");
    lines.push_back(to_str(delay_projectile_draw_));
    lines.push_back(to_str(delay_shotgun_));
    lines.push_back(to_str(delay_explosion_));
    TRACE_FUNC_END;
}

} //Namespace

void init()
{
    font_name_     = "";
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
    font_image_names.push_back("images/16x24_typewriter.png");

    set_default_variables();

    vector<string> lines;
    read_file(lines);

    if (lines.empty())
    {
        collect_lines_from_variables(lines);
    }
    else
    {
        set_all_variables_from_lines(lines);
    }

    set_cell_dim_dependent_variables();
}

bool    is_tiles_mode()                 {return is_tiles_mode_;}
string  get_font_name()                 {return font_name_;}
bool    is_fullscreen()                 {return is_fullscreen_;}
int     get_screen_px_w()               {return screen_px_w_;}
int     get_screen_px_h()               {return screen_px_h_;}
int     get_cell_w()                    {return cell_w_;}
int     get_cell_h()                    {return cell_h_;}
int     get_log_px_h()                  {return log_px_h_;}
int     get_map_px_h()                  {return map_px_h_;}
int     get_map_px_offset_h()           {return map_px_offset_h_;}
int     get_char_lines_px_offset_h()    {return char_lines_px_offset_h_;}
int     get_char_lines_px_h()           {return char_lines_px_h_;}
bool    is_ascii_wall_full_square()     {return is_ascii_wall_full_square_;}
bool    is_tiles_wall_full_square()     {return is_tiles_wall_full_square_;}
bool    is_audio_enabled()              {return is_audio_enabled_;}
bool    is_bot_playing()                {return is_bot_playing_;}
void    toggle_bot_playing()            {is_bot_playing_ = !is_bot_playing_;}
bool    is_ranged_wpn_meleee_prompt()   {return is_ranged_wpn_meleee_prompt_;}
bool    is_ranged_wpn_auto_reload()     {return is_ranged_wpn_auto_reload_;}
bool    is_intro_lvl_skipped()          {return is_intro_lvl_skipped_;}
int     get_delay_projectile_draw()     {return delay_projectile_draw_;}
int     get_delay_shotgun()             {return delay_shotgun_;}
int     get_delay_explosion()           {return delay_explosion_;}

void run_options_menu()
{
    Menu_browser browser(NR_OPTIONS, 0);
    vector<string> lines;

    const int OPTION_VALUES_X_POS = 40;

    draw(&browser, OPTION_VALUES_X_POS);

    while (true)
    {
        const Menu_action action = menu_input_handling::get_action(browser);

        switch (action)
        {
        case Menu_action::browsed:
            draw(&browser, OPTION_VALUES_X_POS);
            break;

        case Menu_action::esc:
        case Menu_action::space:
            //Since ASCII mode wall symbol may have changed,
            //we need to redefine the feature data list
            feature_data::init();
            return;

        case Menu_action::selected:
            draw(&browser, OPTION_VALUES_X_POS);
            player_sets_option(&browser, OPTION_VALUES_X_POS);
            collect_lines_from_variables(lines);
            write_lines_to_file(lines);
            draw(&browser, OPTION_VALUES_X_POS);
            break;

        default: {} break;
        }
    }
}

void set_cell_dim_dependent_variables()
{
    map_px_h_                = cell_h_ * MAP_H;
    map_px_offset_h_         = cell_h_ * MAP_OFFSET_H;
    log_px_h_                = cell_h_ * LOG_H;
    char_lines_px_h_         = cell_h_ * CHAR_LINES_H;
    char_lines_px_offset_h_  = cell_h_ * CHAR_LINES_OFFSET_H;
    screen_px_w_             = cell_w_ * SCREEN_W;
    screen_px_h_             = cell_h_ * SCREEN_H;
}

void toggle_fullscreen()
{
    is_fullscreen_ = !is_fullscreen_;
    set_cell_dims_from_font_name();
    set_cell_dim_dependent_variables();

    render::on_toggle_fullscreen();

    vector<string> lines;
    collect_lines_from_variables(lines);
    write_lines_to_file(lines);
}

} //Config



