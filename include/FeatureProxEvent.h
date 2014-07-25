#ifndef FEATURE_PROX_EVENT_H
#define FEATURE_PROX_EVENT_H

#include "FeatureMob.h"

class ProxEvent: public Mob {
public:
  ~ProxEvent() {}

  void onNewTurn() override;

protected:
  ProxEvent(Pos pos) : Mob(pos) {}

  virtual void playerIsNear() = 0;
};

class ProxEventWallCrumble: public ProxEvent {
public:
  ProxEventWallCrumble(Pos pos, std::vector<Pos>& walls, std::vector<Pos>& inner) :
    ProxEvent(pos), wallCells_(walls), innerCells_(inner) {}

  //Spawn by id compliant ctor (do not use for normal cases):
  ProxEventWallCrumble(const Pos& pos) : ProxEvent(pos) {}

  FeatureId getId() const override {return FeatureId::proxEventWallCrumble;}

  std::string getName(const Article article) const override {
    (void)article;
    return "";
  }
  Clr getClr() const override {return clrBlack;}

private:
  void playerIsNear();

  std::vector<Pos> wallCells_;
  std::vector<Pos> innerCells_;
};

#endif
