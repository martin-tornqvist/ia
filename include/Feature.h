#ifndef FEATURE_H
#define FEATURE_H

#include <vector>
#include <string>

#include "CmnTypes.h"
#include "FeatureData.h"

class Actor;

class Feature {
public:
  Feature(const Pos& pos) : hasBlood_(false), pos_(pos) {}

  virtual ~Feature() {}

  virtual FeatureId     getId()   const = 0;
  const   FeatureDataT& getData() const;

  //Note: "hit()" should never directly destroy the object, only call "destroy()"
  virtual void          hit(const DmgType dmgType, const DmgMethod dmgMethod);
  virtual void          destroy(const DmgType dmgType);
  virtual void          bump(Actor& actorBumping);
  virtual void          newTurn() {}
  virtual bool          canMoveCmn()                                    const;
  virtual bool          canMove(const std::vector<PropId>& actorsProps) const;
  virtual bool          isSoundPassable()                               const;
  virtual bool          isVisionPassable()                              const;
  virtual bool          isProjectilePassable()                          const;
  virtual bool          isSmokePassable()                               const;
  virtual bool          isBottomless()                                  const;
  virtual std::string   getDescr(const bool DEFINITE_ARTICLE)           const;
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

  int getShockWhenAdj() const;
  Pos getPos()          const {return pos_;}

  bool hasBlood_;

protected:
  Pos pos_;
};

#endif
