#ifndef CREATE_CHARACTER_HPP
#define CREATE_CHARACTER_HPP

#include "state.hpp"
#include "player_bon.hpp"
#include "browser.hpp"

class NewGameState: public State
{
public:
    NewGameState() {}

    ~NewGameState() {}

    void on_pushed() override;

    void on_resume() override;
};

class GainLvlState: public State
{
public:
    GainLvlState() {}

    ~GainLvlState() {}

    void on_start() override;

    void update() override;
};

class PickBgState: public State
{
public:
    PickBgState();

    ~PickBgState() {}

    void on_start() override;

    void update() override;

    void draw() override;

private:
    MenuBrowser browser_;

    std::vector<Bg> bgs_;
};

class PickTraitState: public State
{
public:
    PickTraitState();

    ~PickTraitState() {}

    void on_start() override;

    void update() override;

    void draw() override;

private:
    MenuBrowser browser_;

    std::vector<Trait> traits_;
};

class EnterNameState: public State
{
public:
    EnterNameState();

    ~EnterNameState() {}

    void update() override;

    void draw() override;

private:
    std::string current_str_;
};

#endif // CREATE_CHARACTER_HPP
