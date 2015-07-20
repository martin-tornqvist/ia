#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "feature.hpp"
#include "ability_values.hpp"
#include "art.hpp"
#include "feature_rigid.hpp"
#include "cmn_data.hpp"

class Trap_impl;

enum class Trap_id
{
    blinding,
    dart,
    gas_confusion,
    gas_fear,
    gas_paralyze,
    smoke,
    alarm,
    spear,
    web,
    teleport,
    summon,
    END,
    any
};

enum class Trap_placement_valid {no, yes};

class Trap: public Rigid
{
public:
    Trap(const Pos& feature_pos, const Rigid* const mimic_feature, Trap_id id);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Trap(const Pos& feature_pos) :
        Rigid                   (feature_pos),
        mimic_feature_          (nullptr),
        is_hidden_              (false),
        nr_turns_until_trigger_ (-1),
        trap_impl_              (nullptr) {}

    Trap() = delete;

    ~Trap();

    Feature_id id() const override
    {
        return Feature_id::trap;
    }

    bool valid()
    {
        //This trap is valid if we have succesfully created an implementation
        return trap_impl_;
    }

    void bump(Actor& actor_bumping) override;

    char glyph() const override;

    Tile_id tile() const override;

    std::string name(const Article article) const override;

    void disarm() override;

    void on_new_turn_hook() override;

    bool can_have_corpse() const override
    {
        return is_hidden_;
    }

    bool can_have_blood() const override
    {
        return is_hidden_;
    }

    bool can_have_gore() const override
    {
        return is_hidden_;
    }

    bool is_magical() const;

    void reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES);

    bool is_hidden() const
    {
        return is_hidden_;
    }

    Matl matl() const;

    Dir actor_try_leave(Actor& actor, const Dir dir);

    Trap_id trap_type() const;

    const Trap_impl* trap_impl() const
    {
        return trap_impl_;
    }

    void player_try_spot_hidden();

private:
    Trap_impl* mk_trap_impl_from_id(const Trap_id trap_id) const;

    Clr clr_default() const override;
    Clr clr_bg_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    Did_trigger_trap trigger_trap(Actor* const actor) override;

    void trigger_start(const Actor* actor);

    const Rigid* const mimic_feature_;
    bool is_hidden_;
    int nr_turns_until_trigger_;

    Trap_impl* trap_impl_;
};

class Trap_impl
{
protected:
    friend class Trap;
    Trap_impl(Pos pos, Trap_id trap_type, const Trap* const base_trap) :
        pos_        (pos),
        trap_type_  (trap_type),
        base_trap_  (base_trap) {}

    virtual ~Trap_impl() {}

    //Called by the trap feature after picking a random trap implementation.
    //This allows the specific implementation initialize and to modify the map.
    //The implementation may report that the placement is impossible
    //(e.g. no suitable wall to fire a dart from), in which case another
    //implementation will be picked at random.
    virtual Trap_placement_valid on_place()
    {
        return Trap_placement_valid::yes;
    }

    virtual Dir actor_try_leave(Actor& actor, const Dir dir)
    {
        (void)actor;
        return dir;
    }

    //NOTE: The trigger may happen several turns after the trap activates, so it's kinds pointless
    // to provide actor triggering as a parameter here.
    virtual void trigger() = 0;

    virtual Range nr_turns_range_to_trigger() const = 0;

    virtual std::string title() const
    {
        return "Trap";
    }

    virtual Clr clr() const
    {
        return clr_red_lgt;
    }

    virtual Tile_id tile() const
    {
        return Tile_id::trap_general;
    }

    virtual char glyph() const
    {
        return '^';
    }

    virtual bool is_magical() const = 0;
    virtual bool is_disarmable() const = 0;

    virtual std::string disarm_msg() const
    {
        return is_magical() ? "I dispel a magic trap." : "I disarm a trap.";
    }

    virtual std::string disarm_fail_msg() const
    {
        return is_magical() ? "I fail to dispel a magic trap." : "I fail to disarm a trap.";
    }

    Pos pos_;
    Trap_id trap_type_;

    Pos dart_origin_pos_;

    const Trap* const base_trap_;
};

class Trap_dart: public Trap_impl
{
private:
    friend class Trap;

    Trap_dart(Pos pos, const Trap* const base_trap);

    void trigger();

    Trap_placement_valid on_place() override;

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 3};
    }

    Tile_id tile() const override
    {
        return Tile_id::trap_general;
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }

    bool is_poisoned_;

    Pos dart_origin_;

    bool is_dart_origin_destroyed_;
};

class Trap_spear: public Trap_impl
{
private:
    friend class Trap;

    Trap_spear(Pos pos, const Trap* const base_trap);

    void trigger();

    Trap_placement_valid on_place() override;

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 2};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }

    bool is_poisoned_;

    Pos spear_origin_;

    bool is_spear_origin_destroyed_;
};

class Trap_gas_confusion: public Trap_impl
{
private:
    friend class Trap;
    Trap_gas_confusion(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::gas_confusion, base_trap) {}

    void trigger();

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 4};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_gas_paralyzation: public Trap_impl
{
private:
    friend class Trap;
    Trap_gas_paralyzation(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::gas_paralyze, base_trap) {}

    void trigger();

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 4};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_gas_fear: public Trap_impl
{
private:
    friend class Trap;
    Trap_gas_fear(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::gas_fear, base_trap) {}

    void trigger();

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 4};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_blinding_flash: public Trap_impl
{
private:
    friend class Trap;
    Trap_blinding_flash(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::blinding, base_trap) {}

    void trigger();

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 3};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_smoke: public Trap_impl
{
private:
    friend class Trap;
    Trap_smoke(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::smoke, base_trap) {}

    void trigger();

    Range nr_turns_range_to_trigger() const override
    {
        return {1, 3};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_alarm: public Trap_impl
{
private:
    friend class Trap;
    Trap_alarm(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::alarm, base_trap) {}

    void trigger();

    Range nr_turns_range_to_trigger() const override
    {
        return {0, 2};
    }

    bool is_magical() const override
    {
        return false;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_teleport: public Trap_impl
{
private:
    friend class Trap;
    Trap_teleport(Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::teleport, base_trap) {}

    void trigger();

    Clr clr() const override
    {
        return clr_cyan;
    }

    std::string title() const override
    {
        return "Teleporter trap";
    }

    Range nr_turns_range_to_trigger() const override
    {
        return {0, 0};
    }

    bool is_magical() const override
    {
        return true;
    }

    Tile_id tile() const override
    {
        return Tile_id::elder_sign;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_summon_mon: public Trap_impl
{
private:
    friend class Trap;

    Trap_summon_mon (Pos pos, const Trap* const base_trap) :
        Trap_impl(pos, Trap_id::summon, base_trap) {}

    void trigger();

    Clr clr() const override
    {
        return clr_brown_drk;
    }

    std::string title() const override
    {
        return "Monster summoning trap";
    }

    bool is_magical() const override
    {
        return true;
    }

    Range nr_turns_range_to_trigger() const override
    {
        return {0, 0};
    }

    Tile_id tile() const override
    {
        return Tile_id::elder_sign;
    }

    bool is_disarmable() const override
    {
        return true;
    }
};

class Trap_web: public Trap_impl
{
public:
    Dir actor_try_leave(Actor& actor, const Dir dir);

    bool is_holding() const
    {
        return is_holding_actor_;
    }

private:
    friend class Trap;

    Trap_web(Pos pos, const Trap* const base_trap) :
        Trap_impl           (pos, Trap_id::web, base_trap),
        is_holding_actor_   (false) {}

    void trigger();

    Clr clr() const override
    {
        return clr_white_high;
    }

    std::string title() const override
    {
        return "Spider web";
    }

    char glyph() const override
    {
        return '*';
    }

    Range nr_turns_range_to_trigger() const override
    {
        return {0, 0};
    }

    bool is_magical() const override
    {
        return false;
    }

    Tile_id tile() const override
    {
        return Tile_id::web;
    }

    bool is_disarmable() const override
    {
        return true;
    }

    std::string disarm_msg() const override
    {
        return "I tear down a spider web.";
    }

    std::string disarm_fail_msg() const override
    {
        return "I fail to tear down a spider web.";
    }

    bool is_holding_actor_;
};

#endif
