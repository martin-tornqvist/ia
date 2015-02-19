#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "feature.hpp"
#include "ability_values.hpp"
#include "art.hpp"
#include "feature_rigid.hpp"
#include "cmn_data.hpp"

class Specific_trap_base;

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

class Trap: public Rigid
{
public:
    Trap(const Pos& pos, const Rigid* const mimic_feature, Trap_id type);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Trap(const Pos& pos) :
        Rigid         (pos),
        mimic_feature_ (nullptr),
        is_hidden_     (false),
        specific_trap_ (nullptr) {}

    Trap() = delete;

    ~Trap();

    Feature_id   get_id() const override {return Feature_id::trap;}

    void        bump(Actor& actor_bumping)             override;
    char        get_glyph()                      const override;
    Tile_id      get_tile()                       const override;
    std::string get_name(const Article article)  const override;
    void        disarm()                              override;
    bool        can_have_corpse()                 const override {return is_hidden_;}
    bool        can_have_blood()                  const override {return is_hidden_;}
    bool        can_have_gore()                   const override {return is_hidden_;}

    bool is_magical() const;

    void trigger_on_purpose(Actor& actor_triggering);

    void reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES);

    bool is_hidden() const {return is_hidden_;}

    Matl get_matl() const;

    Dir actor_try_leave(Actor& actor, const Dir dir);

    Trap_id get_trap_type() const;

    const Specific_trap_base* get_specific_trap() const {return specific_trap_;}

    void  player_try_spot_hidden();

private:
    Clr get_clr_()   const override;
    Clr get_clr_bg_() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    Did_trigger_trap trigger_trap(Actor* const actor) override;

    void set_specific_trap_from_id(const Trap_id id);

    const Rigid* const  mimic_feature_;
    bool                is_hidden_;
    Specific_trap_base*   specific_trap_;
};

class Specific_trap_base
{
protected:
    friend class Trap;
    Specific_trap_base(Pos pos, Trap_id trap_type) :
        pos_(pos), trap_type_(trap_type) {}

    virtual ~Specific_trap_base() {}

    virtual Dir actor_try_leave(Actor& actor, const Dir dir)
    {
        (void)actor;
        return dir;
    }

    virtual void trigger(
        Actor& actor, const ability_roll_result dodge_result) = 0;
    virtual std::string get_title()  const = 0;
    virtual Clr get_clr()      const = 0;
    virtual char get_glyph()         const = 0;
    virtual Tile_id get_tile()        const = 0;
    virtual bool is_magical()        const = 0;
    virtual bool is_disarmable()     const = 0;
    virtual std::string get_disarm_msg() const
    {
        return is_magical() ? "I dispel a magic trap." : "I disarm a trap.";
    }
    virtual std::string get_disarm_fail_msg() const
    {
        return is_magical() ?
               "I fail to dispel a magic trap." :
               "I fail to disarm a trap.";
    }

    Pos pos_;
    Trap_id trap_type_;
};

class Trap_dart: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_dart(Pos pos);
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_white_high;}
    std::string      get_title()      const override {return "Dart trap";}
    Tile_id      get_tile()       const override {return Tile_id::trap_general;}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    bool        is_disarmable()  const override {return true;}

    bool        is_poisoned;
};

class Trap_spear: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_spear(Pos pos);
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_white_high;}
    std::string      get_title()      const override {return "Spear trap";}
    Tile_id      get_tile()       const override {return Tile_id::trap_general;}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    bool        is_disarmable()  const override {return true;}

    bool is_poisoned;
};

class Trap_gas_confusion: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_gas_confusion(Pos pos) :
        Specific_trap_base(pos, Trap_id::gas_confusion) {}
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_magenta;}
    std::string      get_title()      const override {return "Gas trap";}
    Tile_id      get_tile()       const override {return Tile_id::trap_general;}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_gas_paralyzation: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_gas_paralyzation(Pos pos) :
        Specific_trap_base(pos, Trap_id::gas_paralyze)
    {
    }
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_magenta;}
    std::string      get_title()      const override {return "Gas trap";}
    Tile_id      get_tile()       const override {return Tile_id::trap_general;}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_gas_fear: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_gas_fear(Pos pos) :
        Specific_trap_base(pos, Trap_id::gas_fear) {}
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_magenta;}
    std::string      get_title()      const override {return "Gas trap";}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    Tile_id      get_tile()       const override {return Tile_id::trap_general;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_blinding_flash: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_blinding_flash(Pos pos) :
        Specific_trap_base(pos, Trap_id::blinding) {}
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_yellow;}
    std::string      get_title()      const override {return "Blinding trap";}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    Tile_id      get_tile()       const override {return Tile_id::trap_general;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_teleport: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_teleport(Pos pos) :
        Specific_trap_base(pos, Trap_id::teleport)
    {
    }
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_cyan;}
    std::string      get_title()      const override {return "Teleporter trap";}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return true;}
    Tile_id      get_tile()       const override {return Tile_id::elder_sign;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_summon_mon: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_summon_mon(Pos pos) :
        Specific_trap_base(pos, Trap_id::summon)
    {
    }
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_brown_drk;}
    std::string      get_title()      const override {return "Monster summoning trap";}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return true;}
    Tile_id      get_tile()       const override {return Tile_id::elder_sign;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_smoke: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_smoke(Pos pos) :
        Specific_trap_base(pos, Trap_id::smoke) {}
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_gray;}
    std::string      get_title()      const override {return "Smoke trap";}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    Tile_id      get_tile()       const override { return Tile_id::trap_general;}
    bool        is_disarmable()  const override {return true;}
};

class Trap_alarm: public Specific_trap_base
{
private:
    friend class Trap;
    Trap_alarm(Pos pos) :
        Specific_trap_base(pos, Trap_id::alarm) {}
    void trigger(Actor& actor, const ability_roll_result dodge_result);
    Clr   get_clr()        const override {return clr_brown;}
    std::string      get_title()      const override {return "Alarm trap";}
    char        get_glyph()      const override {return '^';}
    bool        is_magical()     const override {return false;}
    Tile_id      get_tile()       const override { return Tile_id::trap_general;}
    bool        is_disarmable()  const override {return true;}
};


class Trap_web: public Specific_trap_base
{
public:
    Dir actor_try_leave(Actor& actor, const Dir dir);

    bool is_holding() const {return is_holding_actor_;}

private:
    friend class Trap;
    Trap_web(Pos pos) : Specific_trap_base(pos, Trap_id::web), is_holding_actor_(false) {}

    void trigger(Actor& actor, const ability_roll_result dodge_result);

    Clr         get_clr()        const override {return clr_white_high;}
    std::string get_title()      const override {return "Spider web";}
    char        get_glyph()      const override {return '*';}
    bool        is_magical()     const override {return false;}
    Tile_id      get_tile()       const override {return Tile_id::web;}
    bool        is_disarmable()  const override {return true;}

    std::string get_disarm_msg() const override
    {
        return "I tear down a spider web.";
    }
    std::string get_disarm_fail_msg() const override
    {
        return "I fail to tear down a spider web.";
    }

    bool is_holding_actor_;
};

#endif
