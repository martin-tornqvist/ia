#ifndef STATE_HPP
#define STATE_HPP

#include <vector>
#include <memory>

namespace states 
{

enum class StateId 
{
    DEFAULT_STATE,
    MENU_STATE,
    GAME_STATE,
    BACKGROUND_STATE,
    TRAIT_STATE,
    NAME_STATE,
    NEWGAME_STATE,
    NEWLEVEL_STATE,
    POSTMENU_STATE,
    POSTINFO_STATE,
    BROWSESPELLS_STATE,
    MESSAGE_STATE,
    MARKER_STATE,
    MANUAL_STATE,
    VIEWACTOR_STATE,
    INV_STATE,
    HIGHSCORE_STATE,
    CONFIG_STATE,
    DESCRIPT_STATE
};

} // states

class State
{
public:
    State() :
        has_started_(false) {}

    virtual ~State() {}

    // Executed immediately when the state is pushed.
    virtual void on_pushed() {}

    // Executed the first time that the state becomes the current state.
    // Sometimes multiple states may be pushed in a sequence, and one of the
    // later states may want to perform actions only when it actually becomes
    // the current state.
    virtual void on_start() {}

    // Executed immediately when the state is popped.
    // This should only be used for cleanup, do not push or pop other states
    // from this call (this is not supported).
    virtual void on_popped() {}

    virtual void draw() {}

    // If true, this state is drawn overlayed on the state(s) below. This can be
    // used for example to draw a marker state on top of the map.
    virtual bool draw_overlayed() const
    {
        return false;
    }

    // Read input, process game logic etc.
    virtual void update() {}

    // All states above have been popped
    virtual void on_resume() {}

    // Another state is pushed on top
    virtual void on_pause() {}

    bool has_started() const
    {
        return has_started_;
    }

    void set_started()
    {
        has_started_ = true;
    }

    bool has_started()
    {
        return has_started_;
    }

    virtual states::StateId id()
    {
        return states::StateId::DEFAULT_STATE;
    }

private:
    bool has_started_;
};

namespace states
{

void init();

void cleanup();

void start();

void draw();

void update();

void push(std::unique_ptr<State> state);

void pop();

void pop_all();

bool is_empty();

bool is_current_state(const State& state);

void pop_until(const states::StateId);

bool contains_state(const states::StateId);

} // states

#endif // STATE_HPP
