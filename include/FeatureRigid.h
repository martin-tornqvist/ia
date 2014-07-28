#ifndef FEATURE_RIGID_H
#define FEATURE_RIGID_H

#include "Feature.h"

enum class BurnState {notBurned, burning, hasBurned};

class Rigid: public Feature {
public:
  Rigid(Pos pos);

  Rigid() = delete;

  virtual FeatureId   getId()                         const override = 0;
  virtual std::string getName(const Article article)  const override = 0;
  virtual void        onNewTurn()                           override;
  Clr                 getClr()                        const override final;
  virtual Clr         getDefClr()                     const = 0;
  virtual Clr         getClrBg()                      const override final;

  virtual void hit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor* actor = nullptr);

  void tryPutGore();

  TileId  getGoreTile()   const {return goreTile_;}
  char    getGoreGlyph()  const {return goreGlyph_;}

  void clearGore();

  virtual bool open() {return false;}
  virtual void disarm();
  virtual void examine();

  void mkBloody() {isBloody_ = true;}

  void setHasBurned() {burnState_ = BurnState::hasBurned;}

protected:
  void setHitEffect(const DmgType dmgType, const DmgMethod dmgMethod,
                    const std::function<void (Actor* const actor)>& effect);

  void tryStartBurning(const bool IS_MSG_ALLOWED);

  virtual void onFinishedBurning() {}

  BurnState getBurnState() const {return burnState_;}

  virtual void triggerTrap(Actor& actor) {(void)actor;}

  TileId goreTile_;
  char goreGlyph_;

private:
  std::function<void(Actor* const actor)>onHit[int(DmgType::END)][int(DmgMethod::END)];

  bool isBloody_;

  BurnState burnState_;
};

enum class FloorType {cmn, cave, stonePath};

class Floor: public Rigid {
public:
  Floor(Pos pos);
  Floor() = delete;
  ~Floor() {}

  FeatureId getId() const override {return FeatureId::floor;}

  TileId      getTile()                       const override;
  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  FloorType type_;
};

class Carpet: public Rigid {
public:
  Carpet(Pos pos);
  Carpet() = delete;
  ~Carpet() {}

  FeatureId getId() const override {return FeatureId::carpet;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        onFinishedBurning()                   override;
};

enum class GrassType {cmn, withered};

class Grass: public Rigid {
public:
  Grass(Pos pos);
  Grass() = delete;
  ~Grass() {}

  FeatureId getId() const override {return FeatureId::grass;}

  TileId      getTile()                       const override;
  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  GrassType type_;
};

class Bush: public Rigid {
public:
  Bush(Pos pos);
  Bush() = delete;
  ~Bush() {}

  FeatureId getId() const override {return FeatureId::bush;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        onFinishedBurning()                   override;

  GrassType type_;
};

class Brazier: public Rigid {
public:
  Brazier(Pos pos) : Rigid(pos) {}
  Brazier() = delete;
  ~Brazier() {}

  FeatureId getId() const override {return FeatureId::brazier;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

enum class WallType {cmn, cmnAlt, cave, egypt, cliff, lengMonestary};

class Wall: public Rigid {
public:
  Wall(Pos pos);
  Wall() = delete;
  ~Wall() {}

  FeatureId getId() const override {return FeatureId::wall;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
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
  void destrAdjDoors() const;
  void mkLowRubbleAndRocks(); //Note: Will destroy object
};

class RubbleLow: public Rigid {
public:
  RubbleLow(Pos pos);
  RubbleLow() = delete;
  ~RubbleLow() {}

  FeatureId getId() const override {return FeatureId::rubbleLow;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

class RubbleHigh: public Rigid {
public:
  RubbleHigh(Pos pos);
  RubbleHigh() = delete;
  ~RubbleHigh() {}

  FeatureId getId() const override {return FeatureId::rubbleHigh;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

private:
  void mkLowRubbleAndRocks(); //Note: Will destroy object
};

class GraveStone: public Rigid {
public:
  GraveStone(Pos pos);
  GraveStone() = delete;
  ~GraveStone() {}

  FeatureId getId() const override {return FeatureId::gravestone;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  void setInscription(const std::string& str) {inscr_ = str;}

  void bump(Actor& actorBumping) override;

private:
  std::string inscr_;
};

class ChurchBench: public Rigid {
public:
  ChurchBench(Pos pos);
  ChurchBench() = delete;  ~ChurchBench() {}

  FeatureId getId() const override {return FeatureId::churchBench;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

enum class StatueType {cmn, ghoul};

class Statue: public Rigid {
public:
  Statue(Pos pos);
  Statue() = delete;
  ~Statue() {}

  FeatureId getId() const override {return FeatureId::statue;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  StatueType type_;
};

class Pillar: public Rigid {
public:
  Pillar(Pos pos);
  Pillar() = delete;
  ~Pillar() {}

  FeatureId getId() const override {return FeatureId::pillar;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

class Stairs: public Rigid {
public:
  Stairs(Pos pos);
  Stairs() = delete;
  ~Stairs() {}

  FeatureId getId() const override {return FeatureId::stairs;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  void bump(Actor& actorBumping) override;
};

class Bridge : public Rigid {
public:
  Bridge(Pos pos) : Rigid(pos) {}
  Bridge() = delete;
  ~Bridge() {}

  FeatureId getId() const override {return FeatureId::bridge;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  TileId      getTile()                       const override;
  char        getGlyph()                      const override;

  void setDir(const HorizontalVertical dir) {dir_ = dir;}

private:
  HorizontalVertical dir_;
};

enum class LiquidType {water, mud, blood, acid, lava};

class LiquidShallow: public Rigid {
public:
  LiquidShallow(Pos pos);
  LiquidShallow() = delete;
  ~LiquidShallow() {}

  FeatureId getId() const override {return FeatureId::liquidShallow;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  void bump(Actor& actorBumping) override;

  LiquidType type_;
};

class LiquidDeep: public Rigid {
public:
  LiquidDeep(Pos pos);
  LiquidDeep() = delete;
  ~LiquidDeep() {}

  FeatureId getId() const override {return FeatureId::liquidDeep;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;

  void bump(Actor& actorBumping) override;

  LiquidType type_;
};

class Chasm: public Rigid {
public:
  Chasm(Pos pos);
  Chasm() = delete;
  ~Chasm() {}

  FeatureId getId() const override {return FeatureId::chasm;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

class Door;

class Lever: public Rigid {
public:
  Lever(Pos pos);

  Lever() = delete;

  ~Lever() {}

  FeatureId getId() const override {return FeatureId::lever;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  TileId      getTile()                       const override;
  void        examine()                             override;

  void setLinkedDoor(Door* const door) {doorLinkedTo_ = door;}

protected:
  void pull();

  bool isPositionLeft_;
  Door* doorLinkedTo_;
};

class Altar: public Rigid {
public:
  Altar(Pos pos);
  Altar() = delete;
  ~Altar() {}

  FeatureId getId() const override {return FeatureId::altar;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

class Tree: public Rigid {
public:
  Tree(Pos pos);
  Tree() = delete;
  ~Tree() {}

  FeatureId getId() const override {return FeatureId::tree;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
};

class Item;

class ItemContainer {
public:
  ItemContainer();

  ~ItemContainer();

  void setRandomItemsForFeature(const FeatureId featureId,
                                const int NR_ITEMS_TO_ATTEMPT);

  void dropItems(const Pos& pos);

  void destroySingleFragile();

  std::vector<Item*> items_;
};

enum class TombTrait {
  stench,                 //Fumes, Ooze-type monster
  auraOfUnrest,           //Ghost-type monster
  forebodingCarvedSigns,  //Cursed
  END
};

enum class TombAppearance {
  common,
  ornate,     //Good items
  marvelous,  //Excellent items
  END
};

class Tomb: public Rigid {
public:
  Tomb(const Pos& pos);
  Tomb() = delete;
  ~Tomb() {}

  FeatureId getId() const override {return FeatureId::tomb;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        bump(Actor& actorBumping)             override;
  bool        open()                                override;
  void        examine()                             override;

private:
  void triggerTrap(Actor& actor) override;

  void trySprainPlayer();

  bool isContentKnown_, isTraitKnown_;

  ItemContainer itemContainer_;

  int pushLidOneInN_;
  TombAppearance appearance_;
  TombTrait trait_;
};

enum class ChestMatl {wood, iron, END};

class Chest: public Rigid {
public:
  Chest(const Pos& pos);
  Chest() = delete;
  ~Chest() {}

  FeatureId getId() const override {return FeatureId::chest;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        bump(Actor& actorBumping)             override;
  bool        open()                                override;
  void        examine()                             override;
  void        disarm()                              override;

private:
  void triggerTrap(Actor& actor) override;

  void trySprainPlayer();

  ItemContainer itemContainer_;

  bool isContentKnown_;
  bool isLocked_, isTrapped_, isTrapStatusKnown_;
  ChestMatl matl_;
};

class Cabinet: public Rigid {
public:
  Cabinet(const Pos& pos);
  Cabinet() = delete;
  ~Cabinet() {}

  FeatureId getId() const override {return FeatureId::cabinet;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        bump(Actor& actorBumping)             override;
  bool        open()                                override;

private:
  ItemContainer itemContainer_;
  bool isContentKnown_;
};

enum class FountainType {
  dry, //This must be at first position!
  tepid,
  refreshing,
  bless,
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
  rConfusion,
  END
};

enum class FountainMatl {stone, gold};

class Fountain: public Rigid {
public:
  Fountain(const Pos& pos);
  Fountain() = delete;
  ~Fountain() {}

  FeatureId getId() const override {return FeatureId::fountain;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        bump(Actor& actorBumping)             override;

private:
  FountainType fountainType_;
  FountainMatl fountainMatl_;
};

class Cocoon: public Rigid {
public:
  Cocoon(const Pos& pos);
  Cocoon() = delete;
  ~Cocoon() {}

  FeatureId getId() const override {return FeatureId::cocoon;}

  std::string getName(const Article article)  const override;
  Clr         getDefClr()                     const override;
  void        bump(Actor& actorBumping)             override;
  bool        open()                                override;

private:
  void triggerTrap(Actor& actor) override;

  bool isContentKnown_;

  ItemContainer itemContainer_;
};

#endif
