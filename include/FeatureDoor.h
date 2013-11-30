#ifndef FEATURE_DOOR_H
#define FEATURE_DOOR_H

#include "Feature.h"

using namespace std;

enum DoorSpawnState_t {
  doorSpawnState_open,
  doorSpawnState_closed,
  doorSpawnState_stuck,
  doorSpawnState_broken,
  doorSpawnState_secret,
  doorSpawnState_secretAndStuck
};

enum DoorMaterial_t {
  doorMaterial_wood,
  doorMaterial_metal
};

class Engine;
class DoorSpawnData;

class Door: public FeatureStatic {
public:
  ~Door() {
  }

  void bump(Actor& actorBumping);
  bool isMovePassable(Actor* const actorMoving) const;
  bool isBodyTypePassable(const BodyType_t bodyType) const;
  bool isVisionPassable() const;
  bool isProjectilesPassable() const;
  bool isSmokePassable() const;
  SDL_Color getColor() const;
  char getGlyph() const;
  Tile_t getTile() const;

  void tryOpen(Actor* actorTrying);
  void tryClose(Actor* actorTrying);
  void tryBash(Actor* actorTrying);
  bool trySpike(Actor* actorTrying);

  bool isOpen() const {
    return isOpen_;
  }
  bool isSecret() const {
    return isSecret_;
  }
  bool isStuck() const {
    return isStuck_;
  }
  bool isOpenedAndClosedExternally() const {
    return isOpenedAndClosedExternally_;
  }

  string getDescription(const bool DEFINITE_ARTICLE) const;

  MaterialType_t getMaterialType() const;

  void reveal(const bool ALLOW_MESSAGE);
  void clue();

  void setToSecret() {
    isOpen_ = false;
    isSecret_ = true;
  }

  virtual bool openFeature();

  static bool isTileAnyDoor(const Tile_t tile) {
    return
      tile == tile_doorBroken ||
      tile == tile_doorClosed ||
      tile == tile_doorOpen;
  }

protected:
  friend class FeatureFactory;
  friend class MapGenBsp;
  friend class FeatureLever;
  Door(Feature_t id, Pos pos, Engine* engine, DoorSpawnData* spawnData);

  const FeatureData* const mimicFeature_;
  int nrSpikes_;

  bool isOpen_, isBroken_, isStuck_, isSecret_, isClued_, isOpenedAndClosedExternally_;

  DoorMaterial_t material_;

  friend class Player;
  void playerTrySpotHidden();
  void playerTryClueHidden();
};

#endif

