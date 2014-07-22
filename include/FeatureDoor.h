#ifndef FEATURE_DOOR_H
#define FEATURE_DOOR_H

#include "FeatureStatic.h"

enum class DoorSpawnState {
  open,
  closed,
  stuck,
  secret,
  secretAndStuck
};

class Door: public FeatureStatic {
public:
  Door(const Pos& pos, const FeatureStatic* const mimicFeature);

  //Spawn by id compliant ctor (do not use for normal cases):
  Door(const Pos& pos) : FeatureStatic(pos), mimicFeature_(nullptr), nrSpikes_(0) {}

  Door() = delete;

  ~Door() override {}

  FeatureId getId() const override {return FeatureId::door;}

  std::string getName(const Article article)                  const override;
  Clr         getClr_()                                       const override;
  char        getGlyph()                                      const override;
  TileId      getTile()                                       const override;
  void        bump(Actor& actorBumping)                             override;
  bool        canMoveCmn()                                    const override;
  bool        canMove(const std::vector<PropId>& actorsProps) const override;
  bool        isVisionPassable()                              const override;
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

  virtual bool open() override;

  static bool isTileAnyDoor(const TileId tile) {
    return tile == TileId::doorClosed || tile == TileId::doorOpen;
  }

  void playerTrySpotHidden();

protected:
  const FeatureStatic* const mimicFeature_;
  int nrSpikes_;

  bool isOpen_, isStuck_, isSecret_, isHandledExternally_;

  Matl matl_;
};

#endif

