#ifndef MARKER_HPP
#define MARKER_HPP

#include <climits>

#include "rl_utils.hpp"
#include "global.hpp"
#include "state.hpp"
#include "game.hpp"

struct InputData;
class Wpn;
class Item;

// -----------------------------------------------------------------------------
// Abstract marker state base class
// -----------------------------------------------------------------------------
class MarkerState: public State
{
public:
    MarkerState() :
        State   (),
        pos_    () {}

    virtual ~MarkerState() {}

    void on_start() override;

    void draw() override;

    bool draw_overlayed() const override
    {
        return true;
    }

    void update() override;

protected:
    void draw_marker(const P& p,
                     const std::vector<P>& trail,
                     const int effective_range,
                     const int blocked_from_idx);

    // Fire etc
    virtual void handle_input(const InputData& input) = 0;

    // Print messages
    virtual void on_moved() = 0;

    // Used for overlays, etc - it should be pretty rare that this is needed
    virtual void on_draw() {}

    virtual bool use_player_tgt() const
    {
        return false;
    }

    virtual bool show_blocked() const
    {
        return false;
    }

    virtual int effective_range() const
    {
        return INT_MAX;
    }

    // Necessary e.g. for marker states drawing overlayed graphics
    CellRenderData marker_render_data_[map_w][map_h];

    P pos_;

private:
    void move(const Dir dir);

    bool try_go_to_tgt();

    void try_go_to_closest_enemy();
};

// -----------------------------------------------------------------------------
// View marker state
// -----------------------------------------------------------------------------
class Viewing: public MarkerState
{
public:
    Viewing() :
        MarkerState () {}

    ~Viewing() {}

protected:
    void on_moved() override;

    void handle_input(const InputData& input) override;

    bool use_player_tgt() const override
    {
        return true;
    }

    bool show_blocked() const override
    {
        return false;
    }
};

// -----------------------------------------------------------------------------
// Aim (and fire) marker state
// -----------------------------------------------------------------------------
class Aiming: public MarkerState
{
public:
    Aiming(Wpn& wpn) :
        MarkerState (),
        wpn_        (wpn) {}

    ~Aiming() {}

protected:
    void on_moved() override;

    void handle_input(const InputData& input) override;

    bool use_player_tgt() const override
    {
        return true;
    }

    bool show_blocked() const override
    {
        return true;
    }

    int effective_range() const override;

    Wpn& wpn_;
};

// -----------------------------------------------------------------------------
// Throw attack marker state
// -----------------------------------------------------------------------------
class Throwing: public MarkerState
{
public:
    Throwing(Item& item_to_throw) :
        MarkerState     (),
        item_to_throw_  (&item_to_throw) {}

    ~Throwing() {}

protected:
    void on_moved() override;

    void handle_input(const InputData& input) override;

    bool use_player_tgt() const override
    {
        return true;
    }

    bool show_blocked() const override
    {
        return true;
    }

    int effective_range() const override;

    Item* item_to_throw_;
};

// -----------------------------------------------------------------------------
// Throw explosive marker state
// -----------------------------------------------------------------------------
class ThrowingExplosive: public MarkerState
{
public:
    ThrowingExplosive(const Item& explosive) :
        MarkerState (),
        explosive_  (explosive) {}

    ~ThrowingExplosive() {}

protected:
    void on_draw() override;

    void on_moved() override;

    void handle_input(const InputData& input) override;

    bool use_player_tgt() const override
    {
        return false;
    }

    bool show_blocked() const override
    {
        return true;
    }

    const Item& explosive_;
};

#endif // MARKER_HPP
