#ifndef FEATURE_RIGID_HPP
#define FEATURE_RIGID_HPP

#include <memory>

#include "feature.hpp"

class Item;
class Lever;

enum class BurnState
{
    not_burned,
    burning,
    has_burned
};

enum class WasDestroyed
{
    no,
    yes
};

enum class DidTriggerTrap
{
    no,
    yes
};

enum class DidOpen
{
    no,
    yes
};

enum class DidClose
{
    no,
    yes
};

class ItemContainer
{
public:
    ItemContainer();

    ~ItemContainer();

    void init(const FeatureId feature_id, const int nr_items_to_attempt);

    void open(const P& feature_pos, Actor* const actor_opening);

    void destroy_single_fragile();

    std::vector<Item*> items_;
};

class Rigid: public Feature
{
public:
    Rigid(const P& p);

    Rigid() = delete;

    virtual ~Rigid() {}

    virtual FeatureId id() const override = 0;

    virtual std::string name(const Article article) const override = 0;

    virtual void on_new_turn() override final;

    Clr clr() const override final;

    virtual Clr clr_bg() const override final;

    virtual void hit(const int dmg,
                     const DmgType dmg_type,
                     const DmgMethod dmg_method,
                     Actor* const actor = nullptr) override;

    int shock_when_adj() const;

    void try_put_gore();

    TileId gore_tile() const
    {
        return gore_tile_;
    }

    char gore_glyph() const
    {
        return gore_glyph_;
    }

    void clear_gore();

    virtual DidOpen open(Actor* const actor_opening);

    virtual DidClose close(Actor* const actor_closing);

    virtual void disarm();

    virtual void on_lever_pulled(Lever* const lever)
    {
        (void)lever;
    }

    void add_light(bool light[map_w][map_h]) const override final;

    void mk_bloody()
    {
        is_bloody_ = true;
    }

    void set_has_burned()
    {
        burn_state_ = BurnState::has_burned;
    }

    BurnState burn_state() const
    {
        return burn_state_;
    }

    void corrupt_color();

    ItemContainer item_container_;

protected:
    virtual void on_new_turn_hook() {}

    virtual void on_hit(const int dmg,
                        const DmgType dmg_type,
                        const DmgMethod dmg_method,
                        Actor* const actor) = 0;

    virtual Clr clr_default() const = 0;

    virtual Clr clr_bg_default() const
    {
        return clr_black;
    }

    void try_start_burning(const bool is_msg_allowed);

    virtual WasDestroyed on_finished_burning();

    virtual DidTriggerTrap trigger_trap(Actor* const actor);

    virtual void add_light_hook(bool light[map_w][map_h]) const
    {
        (void)light;
    }

    virtual int base_shock_when_adj() const;

    TileId gore_tile_;
    char gore_glyph_;

private:
    bool is_bloody_;
    BurnState burn_state_;

    // Corrupted by a Strange Color monster
    int nr_turns_color_corrupted_;
};

enum class FloorType
{
    common,
    cave,
    stone_path
};

class Floor: public Rigid
{
public:
    Floor(const P& p);

    Floor() = delete;

    ~Floor() {}

    FeatureId id() const override
    {
        return FeatureId::floor;
    }

    TileId tile() const override;

    std::string name(const Article article) const override;

    FloorType type_;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Carpet: public Rigid
{
public:
    Carpet(const P& p);

    Carpet() = delete;

    ~Carpet() {}

    FeatureId id() const override
    {
        return FeatureId::carpet;
    }

    std::string name(const Article article) const override;

    WasDestroyed on_finished_burning() override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

enum class GrassType
{
    common,
    withered
};

class Grass: public Rigid
{
public:
    Grass(const P& p);

    Grass() = delete;

    ~Grass() {}

    FeatureId id() const override
    {
        return FeatureId::grass;
    }

    TileId tile() const override;
    std::string name(const Article article) const override;

    GrassType type_;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Bush: public Rigid
{
public:
    Bush(const P& p);

    Bush() = delete;

    ~Bush() {}

    FeatureId id() const override
    {
        return FeatureId::bush;
    }

    std::string name(const Article article) const override;
    WasDestroyed on_finished_burning() override;

    GrassType type_;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Vines: public Rigid
{
public:
    Vines(const P& p);

    Vines() = delete;

    ~Vines() {}

    FeatureId id() const override
    {
        return FeatureId::vines;
    }

    std::string name(const Article article) const override;
    WasDestroyed on_finished_burning() override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Chains: public Rigid
{
public:
    Chains(const P& p);

    Chains() = delete;

    ~Chains() {}

    FeatureId id() const override
    {
        return FeatureId::chains;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    Clr clr_bg_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Grating: public Rigid
{
public:
    Grating(const P& p);

    Grating() = delete;

    ~Grating() {}

    FeatureId id() const override
    {
        return FeatureId::grating;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Brazier: public Rigid
{
public:
    Brazier(const P& p) : Rigid(p) {}

    Brazier() = delete;

    ~Brazier() {}

    FeatureId id() const override
    {
        return FeatureId::brazier;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void add_light_hook(bool light[map_w][map_h]) const override;
};

enum class WallType
{
    common,
    common_alt,
    cave,
    egypt,
    cliff,
    leng_monestary
};

class Wall: public Rigid
{
public:
    Wall(const P& p);

    Wall() = delete;

    ~Wall() {}

    FeatureId id() const override
    {
        return FeatureId::wall;
    }

    std::string name(const Article article) const override;
    char glyph() const override;
    TileId front_wall_tile() const;
    TileId top_wall_tile() const;

    void set_rnd_common_wall();
    void set_random_is_moss_grown();

    WallType type_;
    bool is_mossy_;

    static bool is_tile_any_wall_front(const TileId tile);
    static bool is_tile_any_wall_top(const TileId tile);

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class RubbleLow: public Rigid
{
public:
    RubbleLow(const P& p);

    RubbleLow() = delete;

    ~RubbleLow() {}

    FeatureId id() const override
    {
        return FeatureId::rubble_low;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Bones: public Rigid
{
public:
    Bones(const P& p);

    Bones() = delete;

    ~Bones() {}

    FeatureId id() const override
    {
        return FeatureId::bones;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class RubbleHigh: public Rigid
{
public:
    RubbleHigh(const P& p);

    RubbleHigh() = delete;

    ~RubbleHigh() {}

    FeatureId id() const override
    {
        return FeatureId::rubble_high;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class GraveStone: public Rigid
{
public:
    GraveStone(const P& p);

    GraveStone() = delete;

    ~GraveStone() {}

    FeatureId id() const override
    {
        return FeatureId::gravestone;
    }

    std::string name(const Article article) const override;

    void set_inscription(const std::string& str)
    {
        inscr_ = str;
    }

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    std::string inscr_;
};

class ChurchBench: public Rigid
{
public:
    ChurchBench(const P& p);

    ChurchBench() = delete;

    ~ChurchBench() {}

    FeatureId id() const override
    {
        return FeatureId::church_bench;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

enum class StatueType
{
    common,
    ghoul
};

class Statue: public Rigid
{
public:
    Statue(const P& p);
    Statue() = delete;
    ~Statue() {}

    FeatureId id() const override
    {
        return FeatureId::statue;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    StatueType type_;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    int base_shock_when_adj() const override;
};

class Stalagmite: public Rigid
{
public:
    Stalagmite(const P& p);
    Stalagmite() = delete;
    ~Stalagmite() {}

    FeatureId id() const override
    {
        return FeatureId::stalagmite;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Stairs: public Rigid
{
public:
    Stairs(const P& p);
    Stairs() = delete;
    ~Stairs() {}

    FeatureId id() const override
    {
        return FeatureId::stairs;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    void on_new_turn_hook() override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Bridge : public Rigid
{
public:
    Bridge(const P& p) :
        Rigid(p),
        axis_(Axis::hor) {}
    Bridge() = delete;
    ~Bridge() {}

    FeatureId id() const override
    {
        return FeatureId::bridge;
    }

    std::string name(const Article article) const override;
    TileId tile() const override;
    char glyph() const override;

    void set_axis(const Axis axis)
    {
        axis_ = axis;
    }

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    Axis axis_;
};

class LiquidShallow: public Rigid
{
public:
    LiquidShallow(const P& p);
    LiquidShallow() = delete;
    ~LiquidShallow() {}

    FeatureId id() const override
    {
        return FeatureId::liquid_shallow;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    LiquidType type_;

private:
    Clr clr_default() const override;

    Clr clr_bg_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class LiquidDeep: public Rigid
{
public:
    LiquidDeep(const P& p);
    LiquidDeep() = delete;
    ~LiquidDeep() {}

    FeatureId id() const override
    {
        return FeatureId::liquid_deep;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    LiquidType type_;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Chasm: public Rigid
{
public:
    Chasm(const P& p);
    Chasm() = delete;
    ~Chasm() {}

    FeatureId id() const override
    {
        return FeatureId::chasm;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Lever: public Rigid
{
public:
    Lever(const P& p);

    Lever() = delete;

    ~Lever() {}

    FeatureId id() const override
    {
        return FeatureId::lever;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    void toggle();

    void bump(Actor& actor_bumping) override;

    bool is_left_pos() const
    {
        return is_left_pos_;
    }

    bool is_linked_to(const Rigid& feature) const
    {
        return linked_feature_ == &feature;
    }

    void set_linked_feature(Rigid& feature)
    {
        linked_feature_ = &feature;
    }

    void unlink()
    {
        linked_feature_ = nullptr;
    }

    // Levers linked to the same feature
    void add_sibbling(Lever* const lever)
    {
        sibblings_.push_back(lever);
    }

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    bool is_left_pos_;

    Rigid* linked_feature_;

    std::vector<Lever*> sibblings_;
};

class Altar: public Rigid
{
public:
    Altar(const P& p);
    Altar() = delete;
    ~Altar() {}

    FeatureId id() const override
    {
        return FeatureId::altar;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

class Tree: public Rigid
{
public:
    Tree(const P& p);
    Tree() = delete;
    ~Tree() {}

    FeatureId id() const override
    {
        return FeatureId::tree;
    }

    std::string name(const Article article) const override;

    WasDestroyed on_finished_burning() override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;
};

// NOTE: In some previous versions, it was possible to inspect the tomb and get
//       a hint about its trait ("It has an aura of unrest", "There are
//       foreboding carved signs", etc). This is currently not possible - you
//       open the tomb and any "trap" it has will trigger. Therefore the
//       TombTrait type could be removed, and instead an effect is just
//       randomized when the tomb is opened. But it should be kept the way it
//       is; it could be useful. Maybe some sort of hint will be re-implemented
//       (e.g. via the "Detect Traps" spell).
enum class TombTrait
{
    ghost,
    other_undead,   // Zombies, Mummies, ...
    stench,         // Fumes, Ooze-type monster
    cursed,
    END
};

enum class TombAppearance
{
    common,     // Common items
    ornate,     // Minor treasure
    marvelous,  // Major treasure
    END
};

class Tomb: public Rigid
{
public:
    Tomb(const P& pos);
    Tomb() = delete;
    ~Tomb() {}

    FeatureId id() const override
    {
        return FeatureId::tomb;
    }

    std::string name(const Article article) const override;
    TileId tile() const override;
    void bump(Actor& actor_bumping) override;
    DidOpen open(Actor* const actor_opening) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    DidTriggerTrap trigger_trap(Actor* const actor) override;

    void player_loot();

    bool is_open_, is_trait_known_;

    int push_lid_one_in_n_;
    TombAppearance appearance_;
    bool is_random_appearance_;
    TombTrait trait_;
};

enum class ChestMatl
{
    wood,
    iron,
    END
};

class Chest: public Rigid
{
public:
    Chest(const P& pos);
    Chest() = delete;
    ~Chest() {}

    FeatureId id() const override
    {
        return FeatureId::chest;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    void bump(Actor& actor_bumping) override;

    DidOpen open(Actor* const actor_opening) override;

    void hit(const int dmg,
             const DmgType dmg_type,
             const DmgMethod dmg_method,
             Actor* const actor) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void player_loot();

    bool is_open_, is_locked_;

    ChestMatl matl_;
};

class Cabinet: public Rigid
{
public:
    Cabinet(const P& pos);
    Cabinet() = delete;
    ~Cabinet() {}

    FeatureId id() const override
    {
        return FeatureId::cabinet;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    void bump(Actor& actor_bumping) override;

    DidOpen open(Actor* const actor_opening) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void player_loot();

    bool is_open_;
};

class Bookshelf: public Rigid
{
public:
    Bookshelf(const P& pos);
    Bookshelf() = delete;
    ~Bookshelf() {}

    FeatureId id() const override
    {
        return FeatureId::bookshelf;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void player_loot();

    bool is_looted_;
};

class AlchemistBench: public Rigid
{
public:
    AlchemistBench(const P& pos);
    AlchemistBench() = delete;
    ~AlchemistBench() {}

    FeatureId id() const override
    {
        return FeatureId::alchemist_bench;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void player_loot();

    bool is_looted_;
};

enum class FountainEffect
{
    refreshing,
    xp,

    START_OF_BAD_EFFECTS,
    curse,
    disease,
    poison,
    frenzy,
    paralyze,
    blind,
    faint,
    END
};

class Fountain: public Rigid
{
public:
    Fountain(const P& pos);

    Fountain() = delete;

    ~Fountain() {}

    FeatureId id() const override
    {
        return FeatureId::fountain;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    bool has_drinks_left() const
    {
        return has_drinks_left_;
    }

    FountainEffect effect() const
    {
        return fountain_effect_;
    }

    void set_effect(const FountainEffect effect)
    {
        fountain_effect_ = effect;
    }

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    FountainEffect fountain_effect_;
    bool has_drinks_left_;
};

class Cocoon: public Rigid
{
public:
    Cocoon(const P& pos);

    Cocoon() = delete;

    ~Cocoon() {}

    FeatureId id() const override
    {
        return FeatureId::cocoon;
    }

    std::string name(const Article article) const override;

    TileId tile() const override;

    void bump(Actor& actor_bumping) override;

    DidOpen open(Actor* const actor_opening) override;

private:
    Clr clr_default() const override;

    void on_hit(const int dmg,
                const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void player_loot();

    DidTriggerTrap trigger_trap(Actor* const actor) override;

    bool is_trapped_;
    bool is_open_;
};

#endif
