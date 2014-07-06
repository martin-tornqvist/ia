#ifndef FEATURE_H
#define FEATURE_H

#include <vector>
#include <string>

#include <iostream>

#include "Colors.h"
#include "CmnData.h"
#include "ActorData.h"
#include "FeatureData.h"
#include "CmnTypes.h"

class Actor;
class FeatureSpawnData;

class Feature {
public:
  Feature(FeatureId id, Pos pos, FeatureSpawnData* spawnData = nullptr);

  virtual ~Feature() {}

  virtual void          bump(Actor& actorBumping);
  virtual void          newTurn();
  virtual bool          canMoveCmn()                                    const;
  virtual bool          canMove(const std::vector<PropId>& actorsProps) const;
  virtual bool          isSoundPassable()                               const;
  virtual bool          isVisionPassable()                              const;
  virtual bool          isProjectilePassable()                          const;
  virtual bool          isSmokePassable()                               const;
  virtual bool          isBottomless()                                  const;
  virtual std::string   getDescr(const bool DEFINITE_ARTICLE)           const;
  virtual void          hit(const int DMG, const DmgType dmgType);
  virtual SDL_Color     getClr()                                        const;
  virtual SDL_Color     getClrBg()                                      const;
  virtual char          getGlyph()                                      const;
  virtual TileId        getTile()                                       const;
  virtual void          addLight(bool light[MAP_W][MAP_H])              const;
  virtual bool          canHaveCorpse()                                 const;
  virtual bool          canHaveStaticFeature()                          const;
  virtual bool          canHaveBlood()                                  const;
  virtual bool          canHaveGore()                                   const;
  virtual bool          canHaveItem()                                   const;
  virtual int           getDodgeModifier()                              const;
  virtual MaterialType  getMaterialType()                               const;

  FeatureId getId()           const;
  int       getShockWhenAdj() const;
  bool      hasBlood()        const;
  void      setHasBlood(const bool HAS_BLOOD);

protected:
  Pos pos_;

  const FeatureDataT* const data_;
  bool hasBlood_;
};

class FeatureMob: public Feature {
public:
  FeatureMob(FeatureId id, Pos pos, FeatureSpawnData* spawnData = nullptr) :
    Feature(id, pos), shouldBeDeleted_(false) {(void)spawnData;}

  Pos getPos()  const {return pos_;}
  int getX()    const {return pos_.x;}
  int getY()    const {return pos_.y;}

  //For smoke etc
  bool shouldBeDeleted() {return shouldBeDeleted_;}

protected:
  bool shouldBeDeleted_;
};

class FeatureStatic: public Feature {
public:
  FeatureStatic(FeatureId id, Pos pos, FeatureSpawnData* spawnData = nullptr) :
    Feature(id, pos), goreTile_(TileId::empty), goreGlyph_(' ') {
    (void)spawnData;
  }

  virtual std::string getDescr(const bool DEFINITE_ARTICLE) const override;

  void setGoreIfPossible();

  TileId getGoreTile() const {return goreTile_;}

  char getGoreGlyph()  const {return goreGlyph_;}

  void clearGore();

  virtual void bash(Actor& actorTrying);
  virtual void bash_(Actor& actorTrying);
  virtual bool open() {return false;}
  virtual void disarm();
  virtual void examine();

protected:
  virtual void triggerTrap(Actor& actor) {(void)actor;}

  TileId goreTile_;
  char goreGlyph_;
};

enum class WallType {common, alt1, cave, egypt};

class Wall: public FeatureStatic {
public:
  Wall(FeatureId id, Pos pos);
  ~Wall() {}

  std::string getDescr(const bool DEFINITE_ARTICLE) const override;
  SDL_Color getClr() const;
  char getGlyph() const;

  TileId getFrontWallTile() const;
  TileId getTopWallTile() const;

  void setRandomNormalWall();
  void setRandomIsMossGrown();

  WallType wallType_;
  bool isMossGrown_;

  static bool isTileAnyWallFront(const TileId tile);

  static bool isTileAnyWallTop(const TileId tile);
};

class Grave: public FeatureStatic {
public:
  Grave(FeatureId id, Pos pos) : FeatureStatic(id, pos) {}
  ~Grave() {}

  std::string getDescr(const bool DEFINITE_ARTICLE) const override;

  void setInscription(const std::string& str) {inscription_ = str;}

  void bump(Actor& actorBumping) override;

private:
  std::string inscription_;
};

class Stairs: public FeatureStatic {
public:
  Stairs(FeatureId id, Pos pos) : FeatureStatic(id, pos) {}
  ~Stairs() {}

  void bump(Actor& actorBumping) override;
};

class Bridge : public FeatureStatic {
public:
  Bridge(FeatureId id, Pos pos) : FeatureStatic(id, pos) {}
  ~Bridge() {}

  TileId getTile()  const override {return dir_ == hor ? TileId::tomb : TileId::bat;}

  char getGlyph()   const override {return dir_ == hor ? '|' : '=';}

  void setDir(const HorizontalVertical dir) {dir_ = dir;}

private:
  HorizontalVertical dir_;
};

#endif
