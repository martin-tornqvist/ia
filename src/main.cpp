#include "init.hpp"

#include "rl_utils.hpp"
#include "state.hpp"
#include "io.hpp"
#include "init.hpp"
#include "main_menu.hpp"

#ifdef _WIN32
#undef main
#endif
int main(int argc, char* argv[])
{
    TRACE_FUNC_BEGIN;

    (void)argc;
    (void)argv;

    init::init_io();
    init::init_game();

    std::unique_ptr<State> main_menu_state(new MainMenuState);

    states::push(std::move(main_menu_state));

    // Loop while there is at least one state
    while (!states::is_empty())
    {
        states::start();

        if (states::is_empty())
        {
            break;
        }

        io::clear_screen();

        states::draw();

        io::update_screen();

        states::update();
    }

    init::cleanup_game();
    init::cleanup_io();

    return 0;
}
