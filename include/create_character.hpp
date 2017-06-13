#ifndef CREATE_CHARACTER_HPP
#define CREATE_CHARACTER_HPP

#include "state.hpp"
#include "player_bon.hpp"
#include "browser.hpp"

enum class TraitScreenMode
{
    pick_new,
    view_unavail
};

class NewGameState: public State
{
public:
    NewGameState();

    ~NewGameState();

    void on_pushed() override;

    void on_resume() override;

    states::StateId id() override; 
};

class GainLvlState: public State
{
public:
    GainLvlState();

    ~GainLvlState();

    void on_start() override;

    void update() override;

    states::StateId id() override;
};

class PickBgState: public State
{
public:
    PickBgState();

    ~PickBgState();

    void on_start() override;

    void update() override;

    void draw() override;

    states::StateId id() override;

private:
    MenuBrowser browser_;

    std::vector<Bg> bgs_;
};

class PickTraitState: public State
{
public:
    PickTraitState();

    ~PickTraitState();

    void on_start() override;

    void update() override;

    void draw() override;

    states::StateId id() override;

private:
    MenuBrowser browser_traits_avail_;
    MenuBrowser browser_traits_unavail_;

    std::vector<Trait> traits_avail_;
    std::vector<Trait> traits_unavail_;

    TraitScreenMode screen_mode_;
};

class EnterNameState: public State
{
public:
    EnterNameState();

    ~EnterNameState();

    void on_start() override;

    void update() override;

    void draw() override;

    states::StateId id() override;

private:
    std::string current_str_;
};

#endif // CREATE_CHARACTER_HPP
