#ifndef FEATURE_STATIC_H
#define FEATURE_STATIC_H

#include "Feature.h"

enum class BurnState {notBurned, burning, hasBurned};

class FeatureStatic: public Feature {
public:
  FeatureStatic(Pos pos);

  FeatureStatic() = delete;

  virtual FeatureId   getId()                         const override = 0;
  virtual std::string getName(const Article article)  const override = 0;
  virtual void        onNewTurn()                           override;
  Clr                 getClr()                        const override final;
  virtual Clr         getClr_()                       const = 0;
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

protected:
  void setHitEffect(const DmgType dmgType, const DmgMethod dmgMethod,
                    const std::function<void (Actor* const actor)>& effect);

  void tryStartBurning(const bool IS_MSG_ALLOWED);

  BurnState getBurnState() const {return burnState_;}

  virtual void triggerTrap(Actor& actor) {(void)actor;}

  TileId goreTile_;
  char goreGlyph_;

private:
  std::function<void(Actor* const actor)>onHit[int(DmgType::END)][int(DmgMethod::END)];

  BurnState burnState_;
};

enum class FloorType {cmn, cave};

class Floor: public FeatureStatic {
public:
  Floor(Pos pos);
  Floor() = delete;
  ~Floor() {}

  FeatureId getId() const override {return FeatureId::floor;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  FloorType type_;
};

class Carpet: public FeatureStatic {
public:
  Carpet(Pos pos);
  Carpet() = delete;
  ~Carpet() {}

  FeatureId getId() const override {return FeatureId::carpet;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

enum class GrassType {cmn, withered};

class Grass: public FeatureStatic {
public:
  Grass(Pos pos);
  Grass() = delete;
  ~Grass() {}

  FeatureId getId() const override {return FeatureId::grass;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  GrassType type_;
};

enum class BushType {cmn, withered};

class Bush: public FeatureStatic {
public:
  Bush(Pos pos);
  Bush() = delete;
  ~Bush() {}

  FeatureId getId() const override {return FeatureId::bush;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  BushType type_;
};

class Brazier: public FeatureStatic {
public:
  Brazier(Pos pos) : FeatureStatic(pos) {}
  Brazier() = delete;
  ~Brazier() {}

  FeatureId getId() const override {return FeatureId::brazier;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

enum class WallType {cmn, cmnAlt, cave, egypt};

class Wall: public FeatureStatic {
public:
  Wall(Pos pos);
  Wall() = delete;
  ~Wall() {}

  FeatureId getId() const override {return FeatureId::wall;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
  char        getGlyph()                      const override;
  TileId      getFrontWallTile()              const;
  TileId      getTopWallTile()                const;

  void setRandomNormalWall();
  void setRandomIsMossGrown();

  WallType type_;
  bool isMossy_;

  static bool isTileAnyWallFront(const TileId tile);
  static bool isTileAnyWallTop(const TileId tile);

private:
  void destrAdjDoors() const;
  void mkLowRubbleAndRocks(); //Note: Will destroy object
};

class RubbleLow: public FeatureStatic {
public:
  RubbleLow(Pos pos) : FeatureStatic(pos) {}
  RubbleLow() = delete;
  ~RubbleLow() {}

  FeatureId getId() const override {return FeatureId::rubbleLow;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

class RubbleHigh: public FeatureStatic {
public:
  RubbleHigh(Pos pos);
  RubbleHigh() = delete;
  ~RubbleHigh() {}

  FeatureId getId() const override {return FeatureId::rubbleHigh;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

private:
  void mkLowRubbleAndRocks(); //Note: Will destroy object
};

class GraveStone: public FeatureStatic {
public:
  GraveStone(Pos pos) : FeatureStatic(pos) {}
  GraveStone() = delete;
  ~GraveStone() {}

  FeatureId getId() const override {return FeatureId::gravestone;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  void setInscription(const std::string& str) {inscr_ = str;}

  void bump(Actor& actorBumping) override;

private:
  std::string inscr_;
};

class ChurchBench: public FeatureStatic {
public:
  ChurchBench(Pos pos) : FeatureStatic(pos) {}
  ChurchBench() = delete;  ~ChurchBench() {}

  FeatureId getId() const override {return FeatureId::churchBench;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

class Statue: public FeatureStatic {
public:
  Statue(Pos pos);
  Statue() = delete;
  ~Statue() {}

  FeatureId getId() const override {return FeatureId::statue;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

class Pillar: public FeatureStatic {
public:
  Pillar(Pos pos) : FeatureStatic(pos) {}
  Pillar() = delete;
  ~Pillar() {}

  FeatureId getId() const override {return FeatureId::pillar;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

class Stairs: public FeatureStatic {
public:
  Stairs(Pos pos) : FeatureStatic(pos) {}
  Stairs() = delete;
  ~Stairs() {}

  FeatureId getId() const override {return FeatureId::stairs;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  void bump(Actor& actorBumping) override;
};

class Bridge : public FeatureStatic {
public:
  Bridge(Pos pos) : FeatureStatic(pos) {}
  Bridge() = delete;
  ~Bridge() {}

  FeatureId getId() const override {return FeatureId::bridge;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
  TileId      getTile()                       const override;
  char        getGlyph()                      const override;

  void setDir(const HorizontalVertical dir) {dir_ = dir;}

private:
  HorizontalVertical dir_;
};

enum class LiquidType {water, mud, blood, acid, lava};

class LiquidShallow: public FeatureStatic {
public:
  LiquidShallow(Pos pos) : FeatureStatic(pos), type_(LiquidType::water) {}
  LiquidShallow() = delete;
  ~LiquidShallow() {}

  FeatureId getId() const override {return FeatureId::liquidShallow;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  void bump(Actor& actorBumping) override;

  LiquidType type_;
};

class LiquidDeep: public FeatureStatic {
public:
  LiquidDeep(Pos pos) : FeatureStatic(pos), type_(LiquidType::water) {}
  LiquidDeep() = delete;
  ~LiquidDeep() {}

  FeatureId getId() const override {return FeatureId::liquidDeep;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;

  void bump(Actor& actorBumping) override;

  LiquidType type_;
};

class Chasm: public FeatureStatic {
public:
  Chasm(Pos pos) : FeatureStatic(pos) {}
  Chasm() = delete;
  ~Chasm() {}

  FeatureId getId() const override {return FeatureId::chasm;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

class Door;

class Lever: public FeatureStatic {
public:
  Lever(Pos pos) :
    FeatureStatic(pos), isPositionLeft_(true), doorLinkedTo_(nullptr)  {}

  Lever() = delete;

  ~Lever() {}

  FeatureId getId() const override {return FeatureId::lever;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
  TileId      getTile()                       const override;
  void        examine()                             override;

  void setLinkedDoor(Door* const door) {doorLinkedTo_ = door;}

protected:
  void pull();

  bool isPositionLeft_;
  Door* doorLinkedTo_;
};

class Altar: public FeatureStatic {
public:
  Altar(Pos pos) : FeatureStatic(pos) {}
  Altar() = delete;
  ~Altar() {}

  FeatureId getId() const override {return FeatureId::altar;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
};

class Tree: public FeatureStatic {
public:
  Tree(Pos pos);
  Tree() = delete;
  ~Tree() {}

  FeatureId getId() const override {return FeatureId::tree;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
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

class Tomb: public FeatureStatic {
public:
  Tomb(const Pos& pos);
  Tomb() = delete;
  ~Tomb() {}

  FeatureId getId() const override {return FeatureId::tomb;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
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

class Chest: public FeatureStatic {
public:
  Chest(const Pos& pos);
  Chest() = delete;
  ~Chest() {}

  FeatureId getId() const override {return FeatureId::chest;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
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
  ChestMatl matl;
};

class Cabinet: public FeatureStatic {
public:
  Cabinet(const Pos& pos);
  Cabinet() = delete;
  ~Cabinet() {}

  FeatureId getId() const override {return FeatureId::cabinet;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
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

class Fountain: public FeatureStatic {
public:
  Fountain(const Pos& pos);
  Fountain() = delete;
  ~Fountain() {}

  FeatureId getId() const override {return FeatureId::fountain;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
  void bump(Actor& actorBumping)                    override;

private:
  FountainType fountainType_;
  FountainMatl fountainMatl_;
};

class Cocoon: public FeatureStatic {
public:
  Cocoon(const Pos& pos);
  Cocoon() = delete;
  ~Cocoon() {}

  FeatureId getId() const override {return FeatureId::cocoon;}

  std::string getName(const Article article)  const override;
  Clr         getClr_()                       const override;
  void        bump(Actor& actorBumping)             override;
  bool        open()                                override;

private:
  void triggerTrap(Actor& actor) override;

  bool isContentKnown_;

  ItemContainer itemContainer_;
};

#endif
