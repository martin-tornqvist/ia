#ifndef FEATURE_DOOR_H
#define FEATURE_DOOR_H

#include "FeatureRigid.h"

enum class DoorSpawnState
{
  open,
  closed,
  stuck,
  secret,
  secretAndStuck,
  any
};

class Door: public Rigid
{
public:
  Door(const Pos& pos, const Rigid* const mimicFeature,
       DoorSpawnState spawnState = DoorSpawnState::any);

  //Spawn by id compliant ctor (do not use for normal cases):
  Door(const Pos& pos) : Rigid(pos), mimicFeature_(nullptr), nrSpikes_(0) {}

  Door() = delete;

  ~Door() override {}

  FeatureId getId() const override {return FeatureId::door;}

  std::string getName(const Article article)                  const override;
  WasDestroyed onFinishedBurning()                                  override;
  char        getGlyph()                                      const override;
  TileId      getTile()                                       const override;
  void        bump(Actor& actorBumping)                             override;
  bool        canMoveCmn()                                    const override;
  bool        canMove(const bool actorsPropIds[endOfPropIds]) const override;
  bool        isLosPassable()                                 const override;
  bool        isProjectilePassable()                          const override;
  bool        isSmokePassable()                               const override;

  void tryOpen(Actor* actorTrying);
  void tryClose(Actor* actorTrying);
  bool trySpike(Actor* actorTrying);

  bool isOpen()   const {return isOpen_;}
  bool isSecret() const {return isSecret_;}
  bool isStuck()  const {return isStuck_;}
  bool isHandledExternally() const {return isHandledExternally_;}

  Matl getMatl() const;

  void reveal(const bool ALLOW_MESSAGE);

  void setToSecret() {isOpen_ = isSecret_ = false;}

  virtual DidOpen open(Actor* const actorOpening) override;

  static bool isTileAnyDoor(const TileId tile)
  {
    return tile == TileId::doorClosed || tile == TileId::doorOpen;
  }

  void playerTrySpotHidden();

  const Rigid* getMimic() const {return mimicFeature_;}

private:
  Clr getClr_() const override;

  void onHit(const DmgType dmgType, const DmgMethod dmgMethod,
             Actor* const actor) override;

  const Rigid* const mimicFeature_;
  int nrSpikes_;

  bool isOpen_, isStuck_, isSecret_, isHandledExternally_;

  Matl matl_;
};

#endif

