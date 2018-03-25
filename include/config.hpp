#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

#include "state.hpp"
#include "browser.hpp"

namespace config
{

void init();

void set_fullscreen(const bool value);

bool is_tiles_mode();
std::string font_name();
bool is_fullscreen();
int gui_cell_px_w();
int gui_cell_px_h();
int map_cell_px_w();
int map_cell_px_h();
bool is_text_mode_wall_full_square();
bool is_tiles_wall_full_square();
bool is_audio_enabled();
bool is_amb_audio_enabled();
bool is_bot_playing();
void toggle_bot_playing();
bool is_light_explosive_prompt();
bool is_drink_malign_pot_prompt();
bool is_ranged_wpn_meleee_prompt();
bool is_ranged_wpn_auto_reload();
bool is_intro_lvl_skipped();
bool is_any_key_confirm_more();
int delay_projectile_draw();
int delay_shotgun();
int delay_explosion();
void set_default_player_name(const std::string& name);
std::string default_player_name();

} // config

class ConfigState: public State
{
public:
    ConfigState();

    void update() override;

    void draw() override;

    StateId id() override;

private:
    MenuBrowser browser_;
};

#endif // CONFIG_HPP
