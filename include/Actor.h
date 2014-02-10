#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>

#include "CommonData.h"

#include "ActorData.h"
#include "Sound.h"
#include "Config.h"
#include "Art.h"

using namespace std;

class Engine;

class PropHandler;
class Inventory;

class Actor {
public:
  Actor(Engine& engine);
  virtual ~Actor();

  inline PropHandler& getPropHandler()  {return *propHandler_;}
  inline ActorData&   getData()         {return *data_;}
  inline Inventory&   getInv()          {return *inv_;}

  //This function is not concerned with whether the parameter actor is within
  //FOV, or if the actor is actually hidden or not. It merely tests the sneak
  //skill of the actor, and various conditions such as light/dark.
  //It has no side effects - it merely does a randomized check.
  bool isSpottingHiddenActor(Actor& actor);

  void place(const Pos& pos_, ActorData& data);
  virtual void place_() {}

  bool hit(int dmg, const DmgTypes dmgType, const bool ALLOW_WOUNDS);
  bool hitSpi(const int DMG);

  bool restoreHp(const int HP_RESTORED,
                 const bool ALLOW_MESSAGES = true,
                 const bool IS_ALLOWED_ABOVE_MAX = false);
  bool restoreSpi(const int SPI_RESTORED,
                  const bool ALLOW_MESSAGES = true,
                  const bool IS_ALLOWED_ABOVE_MAX = false);
  void changeMaxHp(const int CHANGE, const bool ALLOW_MESSAGES);
  void changeMaxSpi(const int CHANGE, const bool ALLOW_MESSAGES);

  void die(const bool IS_MANGLED, const bool ALLOW_GORE,
           const bool ALLOW_DROP_ITEMS);

  virtual void onActorTurn() {}
  virtual void onStandardTurn() {}

  virtual void moveDir(Dir dir) = 0;

  virtual void updateColor();

  //Function taking into account FOV, invisibility, status, etc
  //This is the final word on wether an actor can visually percieve
  //another actor.
  bool isSeeingActor(
    const Actor& other,
    const bool visionBlockingCells[MAP_W][MAP_H]) const;

  void getSpottedEnemies(vector<Actor*>& vectorRef);

  inline ActorId getId()  const {return data_->id;}
  inline int getHp()        const {return hp_;}
  inline int getSpi()       const {return spi_;}
  int getHpMax(const bool WITH_MODIFIERS) const;
  inline int getSpiMax()    const {return spiMax_;}

  inline string getNameThe() const {return data_->name_the;}
  inline string getNameA() const {return data_->name_a;}
  inline bool isHumanoid() const {return data_->isHumanoid;}
  inline char getGlyph() const {return glyph_;}
  virtual const SDL_Color& getClr() {return clr_;}
  inline const TileId& getTile() const {return tile_;}

  void addLight(bool light[MAP_W][MAP_H]) const;

  virtual void addLight_(
    bool light[MAP_W][MAP_H]) const {
    (void)light;
  }

  void teleport(const bool MOVE_TO_POS_AWAY_FROM_MONSTERS);

  Pos pos;
  ActorDeadState deadState;

  Engine& eng;

protected:
  //TODO Try to get rid of these friend declarations
  friend class AbilityValues;
  friend class DungeonMaster;
  friend class Dynamite;
  friend class Molotov;
  friend class Flare;
  friend class PropDiseased;

  virtual void die_() {}
  virtual void hit_(int& dmg, const bool ALLOW_WOUNDS) {
    (void)dmg;
    (void)ALLOW_WOUNDS;
  }
  virtual void spawnStartItems() = 0;

  SDL_Color clr_;
  char glyph_;
  TileId tile_;

  int hp_, hpMax_, spi_, spiMax_;

  Pos lairCell_;

  PropHandler*  propHandler_;
  ActorData*    data_;
  Inventory*    inv_;
};

#endif
