#ifndef FEATURE_STATIC_H
#define FEATURE_STATIC_H

#include "Feature.h"

enum class BurnState {none, burning, hasBurned};

class FeatureStatic: public Feature {
public:
  FeatureStatic(Pos pos);

  FeatureStatic() = delete;

  virtual FeatureId getId() const override = 0;

  virtual void hit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor* actor = nullptr);

  virtual Clr         getClr()                              const override;
  virtual Clr         getClrBg()                            const override;
  virtual std::string getDescr(const bool DEFINITE_ARTICLE) const override;

  void tryPutGore();

  TileId  getGoreTile()   const {return goreTile_;}
  char    getGoreGlyph()  const {return goreGlyph_;}

  void clearGore();

  virtual bool open() {return false;}
  virtual void disarm();
  virtual void examine();

protected:
  virtual void triggerTrap(Actor& actor) {(void)actor;}

  std::function<void(Actor* const actor)>onHit[int(DmgType::END)][int(DmgMethod::END)];

  TileId goreTile_;
  char goreGlyph_;

  BurnState burnState_;
};

enum class FloorType {cmn, cave};

class Floor: public FeatureStatic {
public:
  Floor(Pos pos) : FeatureStatic(pos), type_(FloorType::cmn) {}
  Floor() = delete;
  ~Floor() {}

  FeatureId getId() const override {return FeatureId::floor;}

  FloorType type_;
};

class Carpet: public FeatureStatic {
public:
  Carpet(Pos pos) : FeatureStatic(pos) {}
  Carpet() = delete;
  ~Carpet() {}

  FeatureId getId() const override {return FeatureId::carpet;}
};

enum class GrassType {cmn, withered};

class Grass: public FeatureStatic {
public:
  Grass(Pos pos);
  Grass() = delete;
  ~Grass() {}

  FeatureId getId() const override {return FeatureId::grass;}

  GrassType type_;
};

enum class BushType {cmn, withered};

class Bush: public FeatureStatic {
public:
  Bush(Pos pos) : FeatureStatic(pos), type_(BushType::cmn) {}
  Bush() = delete;
  ~Bush() {}

  FeatureId getId() const override {return FeatureId::bush;}

  BushType type_;
};

class Brazier: public FeatureStatic {
public:
  Brazier(Pos pos) : FeatureStatic(pos) {}
  Brazier() = delete;
  ~Brazier() {}

  FeatureId getId() const override {return FeatureId::brazier;}
};

enum class WallType {cmn, cmnAlt, cave, egypt};

class Wall: public FeatureStatic {
public:
  Wall(Pos pos);
  Wall() = delete;
  ~Wall() {}

  FeatureId getId() const override {return FeatureId::wall;}

  std::string getDescr(const bool DEFINITE_ARTICLE) const override;
  Clr   getClr()                                    const;
  char        getGlyph()                            const;
  TileId      getFrontWallTile()                    const;
  TileId      getTopWallTile()                      const;

  void setRandomNormalWall();
  void setRandomIsMossGrown();

  WallType type_;
  bool isMossy_;

  static bool isTileAnyWallFront(const TileId tile);
  static bool isTileAnyWallTop(const TileId tile);

private:
  void destrAdjDoors() const;
};

class RubbleLow: public FeatureStatic {
public:
  RubbleLow(Pos pos) : FeatureStatic(pos) {}
  RubbleLow() = delete;
  ~RubbleLow() {}

  FeatureId getId() const override {return FeatureId::rubbleLow;}
};

class RubbleHigh: public FeatureStatic {
public:
  RubbleHigh(Pos pos) : FeatureStatic(pos) {}
  RubbleHigh() = delete;
  ~RubbleHigh() {}

  FeatureId getId() const override {return FeatureId::rubbleHigh;}
};

class GraveStone: public FeatureStatic {
public:
  GraveStone(Pos pos) : FeatureStatic(pos) {}
  GraveStone() = delete;
  ~GraveStone() {}

  FeatureId getId() const override {return FeatureId::gravestone;}

  std::string getDescr(const bool DEFINITE_ARTICLE) const override;

  void setInscription(const std::string& str) {inscr_ = str;}

  void bump(Actor& actorBumping) override;

private:
  std::string inscr_;
};

class ChurchBench: public FeatureStatic {
public:
  ChurchBench(Pos pos) : FeatureStatic(pos) {}
  ChurchBench() = delete;
  ~ChurchBench() {}

  FeatureId getId() const override {return FeatureId::churchBench;}
};

class Statue: public FeatureStatic {
public:
  Statue(Pos pos) : FeatureStatic(pos) {}
  Statue() = delete;
  ~Statue() {}

  FeatureId getId() const override {return FeatureId::statue;}
};

class Pillar: public FeatureStatic {
public:
  Pillar(Pos pos) : FeatureStatic(pos) {}
  Pillar() = delete;
  ~Pillar() {}

  FeatureId getId() const override {return FeatureId::pillar;}
};

class Stairs: public FeatureStatic {
public:
  Stairs(Pos pos) : FeatureStatic(pos) {}
  Stairs() = delete;
  ~Stairs() {}

  FeatureId getId() const override {return FeatureId::stairs;}

  void bump(Actor& actorBumping) override;
};

class Bridge : public FeatureStatic {
public:
  Bridge(Pos pos) : FeatureStatic(pos) {}
  Bridge() = delete;
  ~Bridge() {}

  FeatureId getId() const override {return FeatureId::bridge;}

  TileId  getTile()   const override;
  char    getGlyph()  const override;

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

  void bump(Actor& actorBumping) override;

  LiquidType type_;
};

class LiquidDeep: public FeatureStatic {
public:
  LiquidDeep(Pos pos) : FeatureStatic(pos), type_(LiquidType::water) {}
  LiquidDeep() = delete;
  ~LiquidDeep() {}

  FeatureId getId() const override {return FeatureId::liquidDeep;}

  void bump(Actor& actorBumping) override;

  LiquidType type_;
};

class Chasm: public FeatureStatic {
public:
  Chasm(Pos pos) : FeatureStatic(pos) {}
  Chasm() = delete;
  ~Chasm() {}

  FeatureId getId() const override {return FeatureId::chasm;}
};

class Door;

class Lever: public FeatureStatic {
public:
  Lever(Pos pos) :
    FeatureStatic(pos), isPositionLeft_(true), doorLinkedTo_(nullptr)  {}

  Lever() = delete;

  ~Lever() {}

  FeatureId getId() const override {return FeatureId::lever;}

  Clr getClr()  const;
  TileId    getTile() const;
  void      examine() override;

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
};

class Tree: public FeatureStatic {
public:
  Tree(Pos pos) : FeatureStatic(pos) {}
  Tree() = delete;
  ~Tree() {}

  FeatureId getId() const override {return FeatureId::tree;}
};

#endif
