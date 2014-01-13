#ifndef FEATURE_H
#define FEATURE_H

#include <vector>
#include <string>

#include <iostream>

#include "Colors.h"
#include "CommonData.h"
#include "ActorData.h"
#include "FeatureData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;
class Actor;
class FeatureFactory;
class FeatureSpawnData;

class Feature {
public:
  virtual ~Feature() {}

  virtual void bump(Actor& actorBumping);
  virtual void newTurn();
  virtual bool isBodyTypePassable(const BodyType_t bodyType) const;
  virtual bool isVisionPassable() const;
  virtual bool isProjectilesPassable() const;
  virtual bool isSmokePassable() const;
  virtual bool isBottomless() const;
  virtual string getDescr(const bool DEFINITE_ARTICLE) const;
  virtual void hit(const int DMG, const DmgTypes_t dmgType);
  virtual SDL_Color getColor() const;
  virtual SDL_Color getColorBg() const;
  virtual char getGlyph() const;
  virtual Tile_t getTile() const;
  virtual void addLight(bool light[MAP_W][MAP_H]) const;
  virtual bool canHaveCorpse() const;
  virtual bool canHaveStaticFeature() const;
  virtual bool canHaveBlood() const;
  virtual bool canHaveGore() const;
  virtual bool canHaveItem() const;
  bool hasBlood() const;
  void setHasBlood(const bool HAS_BLOOD);
  Feature_t getId() const;
  virtual int getDodgeModifier() const;
  int getShockWhenAdjacent() const;
  virtual void examine();
  virtual MaterialType_t getMaterialType() const;
protected:
  friend class Map;
  Feature(Feature_t id, Pos pos, Engine& engine,
          FeatureSpawnData* spawnData = NULL);

  Pos pos_;
  Engine& eng;
  const FeatureData* const data_;
  bool hasBlood_;
};

class FeatureMob: public Feature {
public:
  Pos getPos()  const {return pos_;}
  int getX()    const {return pos_.x;}
  int getY()    const {return pos_.y;}

  //For smoke etc
  bool shouldBeDeleted() {return shouldBeDeleted_;}

protected:
  friend class FeatureFactory;
  FeatureMob(Feature_t id, Pos pos, Engine& engine,
             FeatureSpawnData* spawnData = NULL) :
    Feature(id, pos, engine), shouldBeDeleted_(false) {
    (void)spawnData;
  }

  bool shouldBeDeleted_;
};

class FeatureStatic: public Feature {
public:
  virtual string getDescr(const bool DEFINITE_ARTICLE) const override;

  void setGoreIfPossible();

  inline Tile_t getGoreTile() const {return goreTile_;}

  inline char getGoreGlyph()  const {return goreGlyph_;}

  inline void clearGore() {
    goreTile_ = tile_empty;
    goreGlyph_ = ' ';
    hasBlood_ = false;
  }

  void tryBash(Actor& actorTrying);
  virtual void specificTryBash(Actor& actorTrying) {(void)actorTrying;}

  virtual void playerTryDisarm();

  virtual bool openFeature() { return false;}

protected:
  friend class FeatureFactory;
  friend class Map;
  FeatureStatic(Feature_t id, Pos pos, Engine& engine,
                FeatureSpawnData* spawnData = NULL) :
    Feature(id, pos, engine), goreTile_(tile_empty), goreGlyph_(' ') {
    (void)spawnData;
  }

  Tile_t goreTile_;
  char goreGlyph_;
};

#endif
