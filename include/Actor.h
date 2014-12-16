#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>

#include "CmnData.h"

#include "ActorData.h"
#include "Sound.h"
#include "Config.h"
#include "Art.h"

class PropHandler;
class Inventory;

enum class ActorDied {no, yes};

class Actor
{
public:
  Actor();
  virtual ~Actor();

  PropHandler& getPropHandler()  {return *propHandler_;}
  ActorDataT&  getData()         {return *data_;}
  Inventory&   getInv()          {return *inv_;}

  //This function is not concerned with whether the parameter actor is within
  //FOV, or if the actor is actually hidden or not. It merely tests the sneak
  //skill of the actor, and various conditions such as light/dark.
  //It has no side effects - it merely does a randomized check.
  bool isSpottingHiddenActor(Actor& actor);

  void place(const Pos& pos_, ActorDataT& data);
  virtual void place_() {}

  ActorDied hit(int dmg, const DmgType dmgType, const DmgMethod method = DmgMethod::END);
  ActorDied hitSpi(const int DMG, const bool ALLOW_MSG);

  bool restoreHp   (const int HP_RESTORED, const bool ALLOW_MSG = true,
                    const bool IS_ALLOWED_ABOVE_MAX = false);
  bool restoreSpi  (const int SPI_RESTORED, const bool ALLOW_MSG = true,
                    const bool IS_ALLOWED_ABOVE_MAX = false);
  void changeMaxHp (const int CHANGE, const bool ALLOW_MSG);
  void changeMaxSpi(const int CHANGE, const bool ALLOW_MSG);

  void die(const bool IS_DESTROYED, const bool ALLOW_GORE,
           const bool ALLOW_DROP_ITEMS);

  virtual void onActorTurn() {}
  virtual void onStdTurn() {}

  virtual void moveDir(Dir dir) = 0;

  virtual void updateClr();

  //Function taking into account FOV, invisibility, status, etc
  //This is the final word on whether an actor can visually perceive
  //another actor.
  bool isSeeingActor(const Actor& other, const bool blockedLos[MAP_W][MAP_H]) const;

  void getSeenFoes(std::vector<Actor*>& out);

  ActorId getId()                  const {return data_->id;}
  int getHp()                      const {return hp_;}
  int getSpi()                     const {return spi_;}
  int getHpMax(const bool WITH_MODIFIERS) const;
  int getSpiMax()                  const {return spiMax_;}

  std::string getNameThe()         const {return data_->nameThe;}
  std::string getNameA()           const {return data_->nameA;}
  std::string getCorpseNameA()     const {return data_->corpseNameA;}
  std::string getCorpseNameThe()   const {return data_->corpseNameThe;}
  bool isHumanoid()                const {return data_->isHumanoid;}
  char getGlyph()                  const {return glyph_;}
  virtual const Clr& getClr()                   {return clr_;}
  const TileId& getTile()          const {return tile_;}

  void addLight(bool lightMap[MAP_W][MAP_H]) const;

  virtual void addLight_(bool light[MAP_W][MAP_H]) const {(void)light;}

  void teleport(const bool MOVE_TO_POS_AWAY_FROM_MONSTERS);

  bool       isAlive ()  const {return state_ == ActorState::alive;}
  bool       isCorpse()  const {return state_ == ActorState::corpse;}
  ActorState getState()  const {return state_;}

  virtual bool isLeaderOf(const Actor* const actor)       const = 0;
  virtual bool isActorMyLeader(const Actor* const actor)  const = 0;

  bool isPlayer() const;

  Pos pos;

protected:
  //TODO Try to get rid of these friend declarations
  friend class AbilityVals;
  friend class PropDiseased;
  friend class PropPossessedByZuul;
  friend class Trap;

  virtual void die_() {}
  virtual void hit_(int& dmg) {(void)dmg;}
  virtual void mkStartItems() = 0;

  ActorState  state_;
  Clr         clr_;
  char        glyph_;
  TileId      tile_;

  int hp_, hpMax_, spi_, spiMax_;

  Pos lairCell_;

  PropHandler*  propHandler_;
  ActorDataT*   data_;
  Inventory*    inv_;
};

#endif
