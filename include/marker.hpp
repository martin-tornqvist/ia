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
    MarkerState(const P& origin) :
        State   (),
        origin_ (origin),
        pos_    () {}

    virtual ~MarkerState() {}

    void on_start() override;

    void draw() override;

    bool draw_overlayed() const override
    {
        return true;
    }

    void update() override;

    states::StateId id() override;

protected:
    virtual void on_start_hook() {}

    void draw_marker(const std::vector<P>& trail,
                     const int orange_from_king_dist,
                     const int red_from_king_dist,
                     const int red_from_idx);

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

    virtual int orange_from_king_dist() const
    {
        return -1;
    }

    virtual int red_from_king_dist() const
    {
        return -1;
    }

    // Necessary e.g. for marker states drawing overlayed graphics
    CellRenderData marker_render_data_[map_w][map_h];

    const P origin_;

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
    Viewing(const P& origin) :
        MarkerState(origin) {}

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
    Aiming(const P& origin, Wpn& wpn) :
        MarkerState (origin),
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

    int orange_from_king_dist() const override;

    int red_from_king_dist() const override;

    Wpn& wpn_;
};

// -----------------------------------------------------------------------------
// Throw attack marker state
// -----------------------------------------------------------------------------
class Throwing: public MarkerState
{
public:
    Throwing(const P& origin, Item& inv_item) :
        MarkerState (origin),
        inv_item_   (&inv_item) {}

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

    int orange_from_king_dist() const override;

    int red_from_king_dist() const override;

    Item* inv_item_;
};

// -----------------------------------------------------------------------------
// Throw explosive marker state
// -----------------------------------------------------------------------------
class ThrowingExplosive: public MarkerState
{
public:
    ThrowingExplosive(const P& origin, const Item& explosive) :
        MarkerState (origin),
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

    int red_from_king_dist() const override;

    const Item& explosive_;
};

// -----------------------------------------------------------------------------
// Teleport control marker state
// -----------------------------------------------------------------------------
class CtrlTele: public MarkerState
{
public:
    CtrlTele(const P& origin, const bool blocked[map_w][map_h]);

    ~CtrlTele() {}

protected:
    void on_start_hook() override;

    void on_moved() override;

    void handle_input(const InputData& input) override;

private:
    bool blocked_[map_w][map_h];
};

#endif // MARKER_HPP
