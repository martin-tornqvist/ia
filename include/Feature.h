#ifndef FEATURE_H
#define FEATURE_H

#include <vector>
#include <string>

#include "CmnTypes.h"
#include "FeatureData.h"

class Actor;

class Feature {
public:
  Feature(const Pos& pos) : pos_(pos) {}

  virtual ~Feature() {}

  virtual FeatureId     getId()                         const = 0;
  virtual std::string   getName(const Article article)  const = 0;
  virtual Clr           getClr()                        const = 0;
  virtual Clr           getClrBg()                      const = 0;

  const FeatureDataT&   getData() const;

  virtual void hit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* const actor);

  virtual void          bump(Actor& actorBumping);
  virtual void          onNewTurn() {}
  virtual bool          canMoveCmn()                                    const;
  virtual bool          canMove(const std::vector<PropId>& actorsProps) const;
  virtual bool          isSoundPassable()                               const;
  virtual bool          isVisionPassable()                              const;
  virtual bool          isProjectilePassable()                          const;
  virtual bool          isSmokePassable()                               const;
  virtual bool          isBottomless()                                  const;
  virtual char          getGlyph()                                      const;
  virtual TileId        getTile()                                       const;
  virtual void          addLight(bool light[MAP_W][MAP_H])              const;
  virtual bool          canHaveCorpse()                                 const;
  virtual bool          canHaveRigid()                                  const;
  virtual bool          canHaveBlood()                                  const;
  virtual bool          canHaveGore()                                   const;
  virtual bool          canHaveItem()                                   const;
  virtual int           getDodgeModifier()                              const;
  virtual Matl          getMatl()                                       const;

  int getShockWhenAdj() const;
  Pos getPos()          const {return pos_;}

protected:
  Pos pos_;
};

#endif
