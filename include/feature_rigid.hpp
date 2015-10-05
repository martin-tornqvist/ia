#ifndef FEATURE_RIGID_H
#define FEATURE_RIGID_H

#include "feature.hpp"

enum class Burn_state
{
    not_burned,
    burning,
    has_burned
};

enum class Was_destroyed    {no, yes};
enum class Did_trigger_trap {no, yes};
enum class Did_open         {no, yes};

class Rigid: public Feature
{
public:
    Rigid(const Pos& feature_pos);

    Rigid() = delete;

    virtual ~Rigid() {}

    virtual Feature_id id() const override = 0;

    virtual std::string name(const Article article) const override = 0;

    virtual void on_new_turn() override final;

    Clr clr() const override final;

    virtual Clr clr_bg() const override final;

    virtual void hit(const Dmg_type dmg_type,
                     const Dmg_method dmg_method,
                     Actor* const actor = nullptr);

    virtual int shock_when_adj() const;

    void try_put_gore();

    Tile_id gore_tile() const
    {
        return gore_tile_;
    }

    char gore_glyph() const
    {
        return gore_glyph_;
    }

    void clear_gore();

    virtual Did_open open(Actor* const actor_opening);

    virtual void disarm();

    void add_light(bool light[MAP_W][MAP_H]) const override final;

    void mk_bloody()
    {
        is_bloody_ = true;
    }

    void set_has_burned()
    {
        burn_state_ = Burn_state::has_burned;
    }

    Burn_state burn_state() const
    {
        return burn_state_;
    }

protected:
    virtual void on_new_turn_hook() {}

    virtual void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                        Actor* const actor) = 0;

    virtual Clr clr_default() const = 0;

    virtual Clr clr_bg_default() const
    {
        return clr_black;
    }

    void try_start_burning(const bool IS_MSG_ALLOWED);

    virtual Was_destroyed on_finished_burning();

    virtual Did_trigger_trap trigger_trap(Actor* const actor);

    virtual void add_light_hook(bool light[MAP_W][MAP_H]) const
    {
        (void)light;
    }

    Tile_id gore_tile_;
    char gore_glyph_;

private:
    bool is_bloody_;
    Burn_state burn_state_;
};

enum class Floor_type {cmn, cave, stone_path};

class Floor: public Rigid
{
public:
    Floor(const Pos& feature_pos);

    Floor() = delete;

    ~Floor() {}

    Feature_id id() const override
    {
        return Feature_id::floor;
    }

    Tile_id tile() const override;
    std::string name(const Article article) const override;

    Floor_type type_;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Carpet: public Rigid
{
public:
    Carpet(const Pos& feature_pos);

    Carpet() = delete;

    ~Carpet() {}

    Feature_id id() const override
    {
        return Feature_id::carpet;
    }

    std::string name(const Article article) const override;
    Was_destroyed on_finished_burning() override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

enum class Grass_type {cmn, withered};

class Grass: public Rigid
{
public:
    Grass(const Pos& feature_pos);

    Grass() = delete;

    ~Grass() {}

    Feature_id id() const override
    {
        return Feature_id::grass;
    }

    Tile_id tile() const override;
    std::string name(const Article article) const override;

    Grass_type type_;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Bush: public Rigid
{
public:
    Bush(const Pos& feature_pos);

    Bush() = delete;

    ~Bush() {}

    Feature_id id() const override
    {
        return Feature_id::bush;
    }

    std::string name(const Article article) const override;
    Was_destroyed on_finished_burning() override;

    Grass_type type_;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Brazier: public Rigid
{
public:
    Brazier(const Pos& feature_pos) : Rigid(feature_pos) {}

    Brazier() = delete;

    ~Brazier() {}

    Feature_id id() const override
    {
        return Feature_id::brazier;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    void add_light_hook(bool light[MAP_W][MAP_H]) const override;
};

enum class Wall_type {cmn, cmn_alt, cave, egypt, cliff, leng_monestary};

class Wall: public Rigid
{
public:
    Wall(const Pos& feature_pos);

    Wall() = delete;

    ~Wall() {}

    Feature_id id() const override
    {
        return Feature_id::wall;
    }

    std::string name(const Article article) const override;
    char glyph() const override;
    Tile_id front_wall_tile() const;
    Tile_id top_wall_tile() const;

    void set_rnd_cmn_wall();
    void set_random_is_moss_grown();

    Wall_type type_;
    bool is_mossy_;

    static bool is_tile_any_wall_front(const Tile_id tile);
    static bool is_tile_any_wall_top(const Tile_id tile);

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Rubble_low: public Rigid
{
public:
    Rubble_low(const Pos& feature_pos);

    Rubble_low() = delete;

    ~Rubble_low() {}

    Feature_id id() const override
    {
        return Feature_id::rubble_low;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Bones: public Rigid
{
public:
    Bones(const Pos& feature_pos);

    Bones() = delete;

    ~Bones() {}

    Feature_id id() const override
    {
        return Feature_id::bones;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Rubble_high: public Rigid
{
public:
    Rubble_high(const Pos& feature_pos);

    Rubble_high() = delete;

    ~Rubble_high() {}

    Feature_id id() const override
    {
        return Feature_id::rubble_high;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Grave_stone: public Rigid
{
public:
    Grave_stone(const Pos& feature_pos);

    Grave_stone() = delete;

    ~Grave_stone() {}

    Feature_id id() const override
    {
        return Feature_id::gravestone;
    }

    std::string name(const Article article) const override;

    void set_inscription(const std::string& str)
    {
        inscr_ = str;
    }

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    std::string inscr_;
};

class Church_bench: public Rigid
{
public:
    Church_bench(const Pos& feature_pos);

    Church_bench() = delete;

    ~Church_bench() {}

    Feature_id id() const override
    {
        return Feature_id::church_bench;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

enum class Statue_type {cmn, ghoul};

class Statue: public Rigid
{
public:
    Statue(const Pos& feature_pos);
    Statue() = delete;
    ~Statue() {}

    Feature_id id() const override
    {
        return Feature_id::statue;
    }

    int shock_when_adj() const override;

    std::string name(const Article article) const override;

    Tile_id tile() const override;

    Statue_type type_;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Pillar: public Rigid
{
public:
    Pillar(const Pos& feature_pos);
    Pillar() = delete;
    ~Pillar() {}

    Feature_id id() const override
    {
        return Feature_id::pillar;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Monolith: public Rigid
{
public:
    Monolith(const Pos& feature_pos);
    Monolith() = delete;
    ~Monolith() {}

    Feature_id id() const override
    {
        return Feature_id::monolith;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Stalagmite: public Rigid
{
public:
    Stalagmite(const Pos& feature_pos);
    Stalagmite() = delete;
    ~Stalagmite() {}

    Feature_id id() const override
    {
        return Feature_id::stalagmite;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Stairs: public Rigid
{
public:
    Stairs(const Pos& feature_pos);
    Stairs() = delete;
    ~Stairs() {}

    Feature_id id() const override
    {
        return Feature_id::stairs;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    void on_new_turn_hook() override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Bridge : public Rigid
{
public:
    Bridge(const Pos& feature_pos) :
        Rigid(feature_pos),
        axis_(Axis::hor) {}
    Bridge() = delete;
    ~Bridge() {}

    Feature_id id() const override
    {
        return Feature_id::bridge;
    }

    std::string name(const Article article) const override;
    Tile_id tile() const override;
    char glyph() const override;

    void set_axis(const Axis axis)
    {
        axis_ = axis;
    }

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    Axis axis_;
};

class Liquid_shallow: public Rigid
{
public:
    Liquid_shallow(const Pos& feature_pos);
    Liquid_shallow() = delete;
    ~Liquid_shallow() {}

    Feature_id id() const override
    {
        return Feature_id::liquid_shallow;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    Liquid_type type_;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Liquid_deep: public Rigid
{
public:
    Liquid_deep(const Pos& feature_pos);
    Liquid_deep() = delete;
    ~Liquid_deep() {}

    Feature_id id() const override
    {
        return Feature_id::liquid_deep;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

    Liquid_type type_;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Chasm: public Rigid
{
public:
    Chasm(const Pos& feature_pos);
    Chasm() = delete;
    ~Chasm() {}

    Feature_id id() const override
    {
        return Feature_id::chasm;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Door;

class Lever: public Rigid
{
public:
    Lever(const Pos& feature_pos);

    Lever() = delete;

    ~Lever() {}

    Feature_id id() const override
    {
        return Feature_id::lever;
    }

    std::string name(const Article article) const override;
    Tile_id tile() const override;

    void set_linked_door(Door* const door) {door_linked_to_ = door;}

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    void pull();

    bool is_position_left_;
    Door* door_linked_to_;
};

class Altar: public Rigid
{
public:
    Altar(const Pos& feature_pos);
    Altar() = delete;
    ~Altar() {}

    Feature_id id() const override
    {
        return Feature_id::altar;
    }

    std::string name(const Article article) const override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Tree: public Rigid
{
public:
    Tree(const Pos& feature_pos);
    Tree() = delete;
    ~Tree() {}

    Feature_id id() const override
    {
        return Feature_id::tree;
    }

    std::string name(const Article article) const override;

    Was_destroyed on_finished_burning() override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;
};

class Item;

class Item_container
{
public:
    Item_container();

    ~Item_container();

    void init(const Feature_id feature_id, const int NR_ITEMS_TO_ATTEMPT);

    void open(const Pos& feature_pos, Actor* const actor_opening);

    void destroy_single_fragile();

    std::vector<Item*> items_;
};

//NOTE: In some previous versions, it was possible to inspect the tomb and get a hint
//about its trait ("It has an aura of unrest", "There are foreboding carved signs", etc).
//This is currently not possible - you open the tomb and any "trap" it has will trigger.
//Therefore the Tomb_trait type could be removed, and instead an effect is just randomized
//when the tomb is opened. But it should be kept the way it is; it could be useful.
//Maybe some sort of hint will be re-implemented (e.g. via the "Detect Traps" spell).
enum class Tomb_trait
{
    ghost,
    other_undead,   //Zombies, Mummies, ...
    stench,         //Fumes, Ooze-type monster
    cursed,
    END
};

enum class Tomb_appearance
{
    common,     //Common items
    ornate,     //Minor treasure
    marvelous,  //Major treasure
    END
};

class Tomb: public Rigid
{
public:
    Tomb(const Pos& pos);
    Tomb() = delete;
    ~Tomb() {}

    Feature_id id() const override
    {
        return Feature_id::tomb;
    }

    std::string name(const Article article) const override;
    Tile_id tile() const override;
    void bump(Actor& actor_bumping) override;
    Did_open open(Actor* const actor_opening) override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    Did_trigger_trap trigger_trap(Actor* const actor) override;

    void player_loot();

    void try_sprain_player();

    bool is_open_, is_trait_known_;

    Item_container item_container_;

    int push_lid_one_in_n_;
    Tomb_appearance appearance_;
    bool is_random_appearance_;
    Tomb_trait trait_;
};

enum class Chest_matl {wood, iron, END};

class Chest: public Rigid
{
public:
    Chest(const Pos& pos);
    Chest() = delete;
    ~Chest() {}

    Feature_id id() const override
    {
        return Feature_id::chest;
    }

    std::string name(const Article article) const override;
    Tile_id tile() const override;
    void bump(Actor& actor_bumping) override;
    Did_open open(Actor* const actor_opening) override;
    void disarm() override;

    void hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
             Actor* const actor) override;


private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    void try_find_trap();

    Did_trigger_trap trigger_trap(Actor* const actor) override;

    void player_loot();

    void try_sprain_player();

    Item_container item_container_;

    bool is_open_, is_locked_, is_trapped_, is_trap_status_known_;

    Chest_matl matl_;

//How hard the trap is to detect (0 - 2)
// 0: Requires nothing
// 1: Requires "Observant"
// 2: Requires "Perceptive"
    const int TRAP_DET_LVL;
};

class Cabinet: public Rigid
{
public:
    Cabinet(const Pos& pos);
    Cabinet() = delete;
    ~Cabinet() {}

    Feature_id id() const override
    {
        return Feature_id::cabinet;
    }

    std::string name(const Article article) const override;
    Tile_id tile() const override;
    void bump(Actor& actor_bumping) override;
    Did_open open(Actor* const actor_opening) override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    void player_loot();

    Item_container item_container_;
    bool is_open_;
};

enum class Fountain_effect
{
    refreshing,
    curse,
    spirit,
    vitality,
    disease,
    poison,
    frenzy,
    paralyze,
    blind,
    faint,
    rFire,
    rElec,
    rFear,
    rConf,
    END
};

enum class Fountain_matl
{
    stone, gold
};

class Fountain: public Rigid
{
public:
    Fountain(const Pos& pos);

    Fountain() = delete;

    ~Fountain() {}

    Feature_id id() const override
    {
        return Feature_id::fountain;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor) override;

    std::vector<Fountain_effect> fountain_effects_;
    Fountain_matl fountain_matl_;
    int nr_drinks_left_;
};

class Cocoon: public Rigid
{
public:
    Cocoon(const Pos& pos);

    Cocoon() = delete;

    ~Cocoon() {}

    Feature_id id() const override
    {
        return Feature_id::cocoon;
    }

    std::string name(const Article article) const override;

    Tile_id tile() const override;

    void bump(Actor& actor_bumping) override;

    Did_open open(Actor* const actor_opening) override;

private:
    Clr clr_default() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor) override;

    void player_loot();

    Did_trigger_trap trigger_trap(Actor* const actor) override;

    bool is_trapped_;
    bool is_open_;

    Item_container item_container_;
};

#endif
