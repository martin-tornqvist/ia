#ifndef FEATURE_RIGID_H
#define FEATURE_RIGID_H

#include "Feature.h"

enum class BurnState        {notBurned, burning, hasBurned};

enum class WasDestroyed     {no, yes};
enum class DidTriggerTrap   {no, yes};
enum class DidOpen          {no, yes};

class Rigid: public Feature
{
public:
    Rigid(Pos pos);

    Rigid() = delete;

    virtual ~Rigid() {}

    virtual FeatureId   getId()                         const override = 0;
    virtual std::string getName(const Article article)  const override = 0;
    virtual void        onNewTurn()                           override final;
    Clr                 getClr()                        const override final;
    virtual Clr         getClrBg()                      const override final;

    virtual void hit(const DmgType dmgType, const DmgMethod dmgMethod,
                     Actor* const actor = nullptr);

    virtual int getShockWhenAdj() const;

    void tryPutGore();

    TileId  getGoreTile()   const {return goreTile_;}
    char    getGoreGlyph()  const {return goreGlyph_;}

    void clearGore();

    virtual DidOpen   open(Actor* const actorOpening);
    virtual void      disarm();

    void mkBloody() {isBloody_ = true;}

    void      setHasBurned()        {burnState_ = BurnState::hasBurned;}
    BurnState getBurnState() const  {return burnState_;}

protected:
    virtual void            onNewTurn_() {}

    virtual void            onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                                  Actor* const actor) = 0;

    virtual Clr             getClr_()   const = 0;
    virtual Clr             getClrBg_() const {return clrBlack;}

    void                    tryStartBurning(const bool IS_MSG_ALLOWED);
    virtual WasDestroyed    onFinishedBurning();
    virtual DidTriggerTrap  triggerTrap(Actor* const actor);

    TileId  goreTile_;
    char    goreGlyph_;

private:
    bool      isBloody_;
    BurnState burnState_;
};

enum class FloorType {cmn, cave, stonePath};

class Floor: public Rigid
{
public:
    Floor(Pos pos);
    Floor() = delete;
    ~Floor() {}

    FeatureId getId() const override {return FeatureId::floor;}

    TileId      getTile()                       const override;
    std::string getName(const Article article)  const override;

    FloorType type_;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Carpet: public Rigid
{
public:
    Carpet(Pos pos);
    Carpet() = delete;
    ~Carpet() {}

    FeatureId getId() const override {return FeatureId::carpet;}

    std::string getName(const Article article)  const override;
    WasDestroyed onFinishedBurning()                   override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

enum class GrassType {cmn, withered};

class Grass: public Rigid
{
public:
    Grass(Pos pos);
    Grass() = delete;
    ~Grass() {}

    FeatureId getId() const override {return FeatureId::grass;}

    TileId      getTile()                       const override;
    std::string getName(const Article article)  const override;

    GrassType type_;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Bush: public Rigid
{
public:
    Bush(Pos pos);
    Bush() = delete;
    ~Bush() {}

    FeatureId getId() const override {return FeatureId::bush;}

    std::string getName(const Article article)  const override;
    WasDestroyed onFinishedBurning()                   override;

    GrassType type_;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Brazier: public Rigid
{
public:
    Brazier(Pos pos) : Rigid(pos) {}
    Brazier() = delete;
    ~Brazier() {}

    FeatureId getId() const override {return FeatureId::brazier;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

enum class WallType {cmn, cmnAlt, cave, egypt, cliff, lengMonestary};

class Wall: public Rigid
{
public:
    Wall(Pos pos);
    Wall() = delete;
    ~Wall() {}

    FeatureId getId() const override {return FeatureId::wall;}

    std::string getName(const Article article)  const override;
    char        getGlyph()                      const override;
    TileId      getFrontWallTile()              const;
    TileId      getTopWallTile()                const;

    void setRndCmnWall();
    void setRandomIsMossGrown();

    WallType type_;
    bool isMossy_;

    static bool isTileAnyWallFront(const TileId tile);
    static bool isTileAnyWallTop(const TileId tile);

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class RubbleLow: public Rigid
{
public:
    RubbleLow(Pos pos);
    RubbleLow() = delete;
    ~RubbleLow() {}

    FeatureId getId() const override {return FeatureId::rubbleLow;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Bones: public Rigid
{
public:
    Bones(Pos pos);
    Bones() = delete;
    ~Bones() {}

    FeatureId getId() const override {return FeatureId::bones;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class RubbleHigh: public Rigid
{
public:
    RubbleHigh(Pos pos);
    RubbleHigh() = delete;
    ~RubbleHigh() {}

    FeatureId getId() const override {return FeatureId::rubbleHigh;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class GraveStone: public Rigid
{
public:
    GraveStone(Pos pos);
    GraveStone() = delete;
    ~GraveStone() {}

    FeatureId getId() const override {return FeatureId::gravestone;}

    std::string getName(const Article article)  const override;

    void setInscription(const std::string& str) {inscr_ = str;}

    void bump(Actor& actorBumping) override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    std::string inscr_;
};

class ChurchBench: public Rigid
{
public:
    ChurchBench(Pos pos);
    ChurchBench() = delete;  ~ChurchBench() {}

    FeatureId getId() const override {return FeatureId::churchBench;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

enum class StatueType {cmn, ghoul};

class Statue: public Rigid
{
public:
    Statue(Pos pos);
    Statue() = delete;
    ~Statue() {}

    FeatureId getId() const override {return FeatureId::statue;}

    int getShockWhenAdj() const override;

    std::string getName(const Article article)  const override;

    TileId getTile() const override;

    StatueType type_;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Pillar: public Rigid
{
public:
    Pillar(Pos pos);
    Pillar() = delete;
    ~Pillar() {}

    FeatureId getId() const override {return FeatureId::pillar;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Monolith: public Rigid
{
public:
    Monolith(Pos pos);
    Monolith() = delete;
    ~Monolith() {}

    FeatureId getId() const override {return FeatureId::monolith;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Stalagmite: public Rigid
{
public:
    Stalagmite(Pos pos);
    Stalagmite() = delete;
    ~Stalagmite() {}

    FeatureId getId() const override {return FeatureId::stalagmite;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Stairs: public Rigid
{
public:
    Stairs(Pos pos);
    Stairs() = delete;
    ~Stairs() {}

    FeatureId getId() const override {return FeatureId::stairs;}

    std::string getName(const Article article)  const override;

    void bump(Actor& actorBumping) override;

    void onNewTurn_() override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Bridge : public Rigid
{
public:
    Bridge(Pos pos) :
        Rigid (pos),
        dir_  (HorizontalVertical::hor) {}
    Bridge() = delete;
    ~Bridge() {}

    FeatureId getId() const override {return FeatureId::bridge;}

    std::string getName(const Article article)  const override;
    TileId      getTile()                       const override;
    char        getGlyph()                      const override;

    void setDir(const HorizontalVertical dir) {dir_ = dir;}

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    HorizontalVertical dir_;
};

enum class LiquidType {water, mud, blood, acid, lava};

class LiquidShallow: public Rigid
{
public:
    LiquidShallow(Pos pos);
    LiquidShallow() = delete;
    ~LiquidShallow() {}

    FeatureId getId() const override {return FeatureId::liquidShallow;}

    std::string getName(const Article article)  const override;

    void bump(Actor& actorBumping) override;

    LiquidType type_;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class LiquidDeep: public Rigid
{
public:
    LiquidDeep(Pos pos);
    LiquidDeep() = delete;
    ~LiquidDeep() {}

    FeatureId getId() const override {return FeatureId::liquidDeep;}

    std::string getName(const Article article)  const override;

    void bump(Actor& actorBumping) override;

    LiquidType type_;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Chasm: public Rigid
{
public:
    Chasm(Pos pos);
    Chasm() = delete;
    ~Chasm() {}

    FeatureId getId() const override {return FeatureId::chasm;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Door;

class Lever: public Rigid
{
public:
    Lever(Pos pos);

    Lever() = delete;

    ~Lever() {}

    FeatureId getId() const override {return FeatureId::lever;}

    std::string getName(const Article article)  const override;
    TileId      getTile()                       const override;

    void setLinkedDoor(Door* const door) {doorLinkedTo_ = door;}

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    void pull();

    bool isPositionLeft_;
    Door* doorLinkedTo_;
};

class Altar: public Rigid
{
public:
    Altar(Pos pos);
    Altar() = delete;
    ~Altar() {}

    FeatureId getId() const override {return FeatureId::altar;}

    std::string getName(const Article article)  const override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Tree: public Rigid
{
public:
    Tree(Pos pos);
    Tree() = delete;
    ~Tree() {}

    FeatureId getId() const override {return FeatureId::tree;}

    std::string getName(const Article article)  const override;

    WasDestroyed onFinishedBurning() override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;
};

class Item;

class ItemContainer
{
public:
    ItemContainer();

    ~ItemContainer();

    void init(const FeatureId featureId, const int NR_ITEMS_TO_ATTEMPT);

    void open(const Pos& featurePos, Actor* const actorOpening);

    void destroySingleFragile();

    std::vector<Item*> items_;
};

//Note: In some previous versions, it was possible to inspect the tomb and get a hint
//about its trait ("It has an aura of unrest", "There are foreboding carved signs", etc).
//This is currently not possible - you open the tomb and any "trap" it has will trigger.
//Therefore the TombTrait type could be removed, and instead an effect is just randomized
//when the tomb is opened. But it should be kept the way it is; it could be useful.
//Maybe some sort of hint will be re-implemented (e.g. via the "Detect Traps" spell).
enum class TombTrait
{
    stench,   //Fumes, Ooze-type monster
    ghost,    //Any ghost-type monster
    cursed,
    END
};

enum class TombAppearance
{
    common,
    ornate,     //Good items
    marvelous,  //Excellent items
    END
};

class Tomb: public Rigid
{
public:
    Tomb(const Pos& pos);
    Tomb() = delete;
    ~Tomb() {}

    FeatureId getId() const override {return FeatureId::tomb;}

    std::string getName(const Article article)  const override;
    TileId      getTile()                       const override;
    void        bump(Actor& actorBumping)             override;
    DidOpen     open(Actor* const actorOpening)       override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    DidTriggerTrap triggerTrap(Actor* const actor) override;

    void playerLoot();

    void trySprainPlayer();

    bool isOpen_, isTraitKnown_;

    ItemContainer itemContainer_;

    int pushLidOneInN_;
    TombAppearance appearance_;
    TombTrait trait_;
};

enum class ChestMatl {wood, iron, END};

class Chest: public Rigid
{
public:
    Chest(const Pos& pos);
    Chest() = delete;
    ~Chest() {}

    FeatureId getId() const override {return FeatureId::chest;}

    std::string getName(const Article article)  const override;
    TileId      getTile()                       const override;
    void        bump(Actor& actorBumping)             override;
    DidOpen     open(Actor* const actorOpening)       override;
    void        disarm()                              override;

    void hit(const DmgType dmgType, const DmgMethod dmgMethod,
             Actor* const actor) override;


private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    void tryFindTrap();

    DidTriggerTrap triggerTrap(Actor* const actor) override;

    void playerLoot();

    void trySprainPlayer();

    ItemContainer itemContainer_;

    bool isOpen_, isLocked_, isTrapped_, isTrapStatusKnown_;

    const ChestMatl matl_;

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

    FeatureId getId() const override {return FeatureId::cabinet;}

    std::string getName(const Article article)  const override;
    TileId      getTile()                       const override;
    void        bump(Actor& actorBumping)             override;
    DidOpen     open(Actor* const actorOpening)       override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    void playerLoot();

    ItemContainer itemContainer_;
    bool isOpen_;
};

enum class FountainEffect
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
    rCold,
    rElec,
    rFear,
    rConf,
    END
};

enum class FountainMatl {stone, gold};

class Fountain: public Rigid
{
public:
    Fountain(const Pos& pos);
    Fountain() = delete;
    ~Fountain() {}

    FeatureId getId() const override {return FeatureId::fountain;}

    std::string getName(const Article article)  const override;
    void        bump(Actor& actorBumping)             override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    std::vector<FountainEffect> fountainEffects_;
    FountainMatl                fountainMatl_;
    int                         nrDrinksLeft_;
};

class Cocoon: public Rigid
{
public:
    Cocoon(const Pos& pos);
    Cocoon() = delete;
    ~Cocoon() {}

    FeatureId getId() const override {return FeatureId::cocoon;}

    std::string getName(const Article article)  const override;
    TileId      getTile()                       const override;
    void        bump(Actor& actorBumping)             override;
    DidOpen     open(Actor* const actorOpening)       override;

private:
    Clr getClr_() const override;

    void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
               Actor* const actor) override;

    void playerLoot();

    DidTriggerTrap triggerTrap(Actor* const actor) override;

    bool isTrapped_;
    bool isOpen_;

    ItemContainer itemContainer_;
};

#endif
