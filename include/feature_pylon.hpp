#ifndef FEATURE_PYLON_HPP
#define FEATURE_PYLON_HPP

#include <memory>

#include "feature_rigid.hpp"

enum class PylonId
{
    burning,
    slow,
    terrify,
    invis,
    knockback,
    teleport,
    END,
    any
};

class PylonImpl;

// -----------------------------------------------------------------------------
// Pylon
// -----------------------------------------------------------------------------
class Pylon: public Rigid
{
public:
    Pylon(const P& p, PylonId id);

    Pylon() = delete;

    ~Pylon() {}

    FeatureId id() const override
    {
        return FeatureId::pylon;
    }

    std::string name(const Article article) const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void on_lever_pulled(Lever* const lever) override;

    void add_light_hook(bool light[map_w][map_h]) const override;

    int nr_turns_active() const
    {
        return nr_turns_active_;
    }

private:
    PylonImpl* make_pylon_impl_from_id(const PylonId id);

    virtual void on_new_turn_hook() override;

    Color color_default() const override;

    std::unique_ptr<PylonImpl> pylon_impl_;

    bool is_activated_;

    int nr_turns_active_;
};

// -----------------------------------------------------------------------------
// Pylon implementation
// -----------------------------------------------------------------------------
class PylonImpl
{
public:
    PylonImpl(P p, Pylon* pylon) :
        pos_(p),
        pylon_(pylon) {}

    virtual void on_new_turn_activated() = 0;

protected:
    // void emit_trigger_snd() const;

    std::vector<Actor*> living_actors_reached() const;

    Actor* rnd_reached_living_actor() const;

    P pos_;

    Pylon* const pylon_;
};

class PylonBurning: public PylonImpl
{
public:
    PylonBurning(P p, Pylon* pylon) :
        PylonImpl(p, pylon) {}

    void on_new_turn_activated() override;
};

class PylonTerrify: public PylonImpl
{
public:
    PylonTerrify(P p, Pylon* pylon) :
        PylonImpl(p, pylon) {}

    void on_new_turn_activated() override;
};

class PylonInvis: public PylonImpl
{
public:
    PylonInvis(P p, Pylon* pylon) :
        PylonImpl(p, pylon) {}

    void on_new_turn_activated() override;
};

class PylonSlow: public PylonImpl
{
public:
    PylonSlow(P p, Pylon* pylon) :
        PylonImpl(p, pylon) {}

    void on_new_turn_activated() override;
};

class PylonKnockback: public PylonImpl
{
public:
    PylonKnockback(P p, Pylon* pylon) :
        PylonImpl(p, pylon) {}

    void on_new_turn_activated() override;
};

class PylonTeleport: public PylonImpl
{
public:
    PylonTeleport(P p, Pylon* pylon) :
        PylonImpl(p, pylon) {}

    void on_new_turn_activated() override;
};

#endif // FEATURE_PYLON_HPP
