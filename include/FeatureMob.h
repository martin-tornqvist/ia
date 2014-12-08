#ifndef FEATURE_MOB_H
#define FEATURE_MOB_H

#include "Feature.h"

class Mob: public Feature
{
public:
  Mob(const Pos& pos) : Feature(pos) {}

  Mob() = delete;

  virtual FeatureId   getId()                         const override = 0;
  virtual std::string getName(const Article article)  const override = 0;
  Clr                 getClr()                        const override = 0;

  Clr getClrBg() const override final {return clrBlack;}

  virtual ~Mob() {}
};

class Smoke: public Mob
{
public:
  Smoke(const Pos& pos, const int NR_TURNS) :
    Mob(pos), nrTurnsLeft_(NR_TURNS) {}

  //Spawn-by-id compliant ctor (do not use for normal cases):
  Smoke(const Pos& pos) : Mob(pos), nrTurnsLeft_(-1) {}

  Smoke() = delete;
  ~Smoke() {}

  FeatureId getId() const override {return FeatureId::smoke;}

  std::string getName(const Article article)  const override;
  Clr         getClr()                        const override;

  void onNewTurn() override;

protected:
  int nrTurnsLeft_;
};

class LitDynamite: public Mob
{
public:
  LitDynamite(const Pos& pos, const int NR_TURNS) :
    Mob(pos), nrTurnsLeft_(NR_TURNS) {}

  //Spawn-by-id compliant ctor (do not use for normal cases):
  LitDynamite(const Pos& pos) : Mob(pos), nrTurnsLeft_(-1) {}

  LitDynamite() = delete;

  ~LitDynamite() {}

  FeatureId getId() const override {return FeatureId::litDynamite;}

  std::string getName(const Article article)  const override;
  Clr         getClr()                        const override;

  //TODO Lit dynamite should add light on their own cell (just one cell)
  //void addLight(bool light[MAP_W][MAP_H]) const;

  void onNewTurn() override;

private:
  int nrTurnsLeft_;
};

class LitFlare: public Mob
{
public:
  LitFlare(const Pos& pos, const int NR_TURNS) :
    Mob(pos), nrTurnsLeft_(NR_TURNS) {}

  //Spawn-by-id compliant ctor (do not use for normal cases):
  LitFlare(const Pos& pos) : Mob(pos), nrTurnsLeft_(-1) {}

  LitFlare() = delete;

  ~LitFlare() {}

  FeatureId getId() const override {return FeatureId::litFlare;}

  std::string getName(const Article article)  const override;
  Clr         getClr()                        const override;

  void onNewTurn() override;

  void addLight(bool light[MAP_W][MAP_H]) const;

private:
  int nrTurnsLeft_;
};

#endif
