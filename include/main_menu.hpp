#ifndef MAIN_MENU_HPP
#define MAIN_MENU_HPP

#include "state.hpp"
#include "browser.hpp"

class MainMenuState: public State
{
public:
    MainMenuState();

    ~MainMenuState();

    void draw() override;

    void update() override;

    void on_start() override;

    void on_resume() override;

private:
    MenuBrowser browser_;
};

#endif // MAIN_MENU_HPP
