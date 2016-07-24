#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace config
{

void init();
void run_options_menu();
void toggle_fullscreen();

bool            is_tiles_mode();
std::string     font_name();
bool            is_fullscreen();
int             scr_px_w();
int             scr_px_h();
int             cell_px_w();
int             cell_px_h();
int             log_px_h();
int             map_px_h();
int             map_px_offset_h();
int             char_lines_px_offset_h();
int             char_lines_px_h();
bool            is_text_mode_wall_full_square();
bool            is_tiles_wall_full_square();
bool            is_audio_enabled();
bool            is_bot_playing();
void            toggle_bot_playing();
bool            is_ranged_wpn_meleee_prompt();
bool            is_ranged_wpn_auto_reload();
bool            is_intro_lvl_skipped();
bool            is_any_key_confirm_more();
int             delay_projectile_draw();
int             delay_shotgun();
int             delay_explosion();
bool            is_light_explosive_prompt();

} //config

#endif
