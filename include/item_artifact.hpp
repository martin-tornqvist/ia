#ifndef ITEM_ARTIFACT_HPP
#define ITEM_ARTIFACT_HPP

#include "item.hpp"
#include "sound.hpp"

// -----------------------------------------------------------------------------
// Staff of the Pharaohs
// -----------------------------------------------------------------------------
class PharaohStaff: public Wpn
{
public:
    PharaohStaff(ItemDataT* const item_data);
};

// -----------------------------------------------------------------------------
// Talisman of Reflection
// -----------------------------------------------------------------------------
class ReflTalisman: public Item
{
public:
    ReflTalisman(ItemDataT* const item_data);

private:
    void on_pickup_hook() override;

    void on_removed_from_inv_hook() override;
};

// -----------------------------------------------------------------------------
// Talisman of Resurrection
// -----------------------------------------------------------------------------
class ResurrectTalisman: public Item
{
public:
    ResurrectTalisman(ItemDataT* const item_data);
};

// -----------------------------------------------------------------------------
// Talisman of Teleportation Control
// -----------------------------------------------------------------------------
class TeleCtrlTalisman: public Item
{
public:
    TeleCtrlTalisman(ItemDataT* const item_data);

private:
    void on_pickup_hook() override;

    void on_removed_from_inv_hook() override;
};

// -----------------------------------------------------------------------------
// Horn of Malice
// -----------------------------------------------------------------------------
class HornOfMaliceHeard: public SndHeardEffect
{
public:
    HornOfMaliceHeard() {}

    ~HornOfMaliceHeard() {}

    void run(Actor& actor) const override;
};

class HornOfMalice: public Item
{
public:
    HornOfMalice(ItemDataT* const item_data);

    std::string name_inf() const override;

    void save() override;

    void load() override;

    ConsumeItem activate(Actor* const actor) override;

private:
    int charges_;
};

// -----------------------------------------------------------------------------
// Horn of Deafening
// -----------------------------------------------------------------------------
class HornOfDeafeningHeard: public SndHeardEffect
{
public:
    HornOfDeafeningHeard() {}

    ~HornOfDeafeningHeard() {}

    void run(Actor& actor) const override;
};

class HornOfDeafening: public Item
{
public:
    HornOfDeafening(ItemDataT* const item_data);

    std::string name_inf() const override;

    void save() override;

    void load() override;

    ConsumeItem activate(Actor* const actor) override;

private:
    int charges_;
};

// -----------------------------------------------------------------------------
// Horn of Banishment
// -----------------------------------------------------------------------------
class HornOfBanishmentHeard: public SndHeardEffect
{
public:
    HornOfBanishmentHeard() {}

    ~HornOfBanishmentHeard() {}

    void run(Actor& actor) const override;
};

class HornOfBanishment: public Item
{
public:
    HornOfBanishment(ItemDataT* const item_data);

    std::string name_inf() const override;

    void save() override;

    void load() override;

    ConsumeItem activate(Actor* const actor) override;

private:
    int charges_;
};


// -----------------------------------------------------------------------------
// Arcane Clockwork
// -----------------------------------------------------------------------------
class Clockwork: public Item
{
public:
    Clockwork(ItemDataT* const item_data);

    ConsumeItem activate(Actor* const actor) override;

    std::string name_inf() const override;

    void save() override;

    void load() override;

private:
    void on_pickup_hook() override;

    void on_removed_from_inv_hook() override;

    int charges_;
};

// -----------------------------------------------------------------------------
// Spirit Dagger
// -----------------------------------------------------------------------------
class SpiritDagger: public Wpn
{
public:
    SpiritDagger(ItemDataT* const item_data);

    void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

// -----------------------------------------------------------------------------
// Orb of Life
// -----------------------------------------------------------------------------
class OrbOfLife: public Item
{
public:
    OrbOfLife(ItemDataT* const item_data);

    void on_std_turn_in_inv(const InvType inv_type) override;

private:
    void on_pickup_hook() override;

    void on_removed_from_inv_hook() override;
};

#endif // ITEM_ARTIFACT_HPP
