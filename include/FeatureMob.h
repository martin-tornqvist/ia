#ifndef FEATURE_MOB_H
#define FEATURE_MOB_H

#include "Feature.h"

class FeatureMob: public Feature {
public:
  FeatureMob(Pos pos) : Feature(pos) {}

  FeatureMob() = delete;

  virtual FeatureId getId() const override = 0;

  virtual ~FeatureMob() {}
};

class Smoke: public FeatureMob {
public:
  Smoke(Pos pos, int nrTurnsLeft) : FeatureMob(pos), nrTurnsLeft_(nrTurnsLeft) {}

  //Spawn by id compliant ctor (do not use for normal cases):
  Smoke(const Pos& pos) : FeatureMob(pos), nrTurnsLeft_(-1) {}

  Smoke() = delete;

  ~Smoke() {}

  FeatureId getId() const override {return FeatureId::smoke;}

  void newTurn();

protected:
  int nrTurnsLeft_;
};

class LitDynamite: public FeatureMob {
public:
  LitDynamite(Pos pos, int nrTurnsLeft) : FeatureMob(pos), nrTurnsLeft_(nrTurnsLeft) {}

  //Spawn by id compliant ctor (do not use for normal cases):
  LitDynamite(const Pos& pos) : FeatureMob(pos), nrTurnsLeft_(-1) {}

  LitDynamite() = delete;

  ~LitDynamite() {}

  FeatureId getId() const override {return FeatureId::litDynamite;}

  //TODO Lit dynamite should add light on their own cell (just one cell)
  //void addLight(bool light[MAP_W][MAP_H]) const;

  void newTurn();

private:
  int nrTurnsLeft_;
};

class LitFlare: public FeatureMob {
public:
  LitFlare(Pos pos, int nrTurnsLeft) : FeatureMob(pos), nrTurnsLeft_(nrTurnsLeft) {}

  //Spawn by id compliant ctor (do not use for normal cases):
  LitFlare(const Pos& pos) : FeatureMob(pos), nrTurnsLeft_(-1) {}

  LitFlare() = delete;

  ~LitFlare() {}

  FeatureId getId() const override {return FeatureId::litFlare;}

  void newTurn();

  void addLight(bool light[MAP_W][MAP_H]) const;

private:
  int nrTurnsLeft_;
};

#endif
