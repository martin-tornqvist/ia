#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace config
{

void init();
void run_options_menu();
void set_cell_dim_dependent_variables();
void toggle_fullscreen();

bool            is_tiles_mode();
std::string     get_font_name();
bool            is_fullscreen();
int             get_screen_pixel_w();
int             get_screen_pixel_h();
int             get_cell_w();
int             get_cell_h();
int             get_log_pixel_h();
int             get_map_pixel_h();
int             get_map_pixel_offset_h();
int             get_char_lines_pixel_offset_h();
int             get_char_lines_pixel_h();
bool            is_ascii_wall_full_square();
bool            is_tiles_wall_full_square();
bool            is_audio_enabled();
bool            is_bot_playing();
void            toggle_bot_playing();
bool            is_ranged_wpn_meleee_prompt();
bool            is_ranged_wpn_auto_reload();
bool            is_intro_lvl_skipped();
int             get_delay_projectile_draw();
int             get_delay_shotgun();
int             get_delay_explosion();
int             get_key_repeat_delay();
int             get_key_repeat_interval();

} //Config

#endif
