#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>

#include "CommonData.h"

#include "ActorData.h"
#include "Sound.h"
#include "Config.h"
#include "MapTests.h"
#include "Art.h"

using namespace std;

class Engine;

class StatusHandler;
class ResHandler;
class TimedEntity;
class Inventory;

class Actor {
public:
  Actor() {}
  virtual ~Actor();

  inline StatusHandler* getStatusHandler() {return statusHandler_;}

  inline ActorDef* getDef() {return def_;}

  Pos pos;
  ActorDeadState_t deadState;

  Inventory* getInventory() {return inventory_;}

  void place(const Pos& pos_, ActorDef* const actorDefinition,
             Engine* engine);

  bool hit(int dmg, const DmgTypes_t dmgType);
  void hitSpi(const int DMG);

  bool restoreHp(const int HP_RESTORED, const bool ALLOW_MESSAGES);
  bool restoreSpi(const int SPI_RESTORED, const bool ALLOW_MESSAGES);
  void changeMaxHp(const int CHANGE, const bool ALLOW_MESSAGES);
  void changeMaxSpi(const int CHANGE, const bool ALLOW_MESSAGES);

  void die(const bool IS_MANGLED, const bool ALLOW_GORE,
           const bool ALLOW_DROP_ITEMS);

  void newTurn();

  virtual void act() = 0;

  virtual void hearSound(const Sound& sound) = 0;

  virtual void updateColor();

  //Function taking into account FOV, invisibility, status, etc
  //This is the final word on wether an actor can visually percieve
  //another actor.
  bool checkIfSeeActor(
    const Actor& other,
    const bool visionBlockingCells[MAP_X_CELLS][MAP_Y_CELLS]) const;

  void getSpotedEnemies(vector<Actor*>& vectorToFill);

  inline ActorId_t getId()  const {return def_->id;}
  inline int getHp()        const {return hp_;}
  inline int getSpi()       const {return spi_;}
  virtual int getHpMax(const bool WITH_MODIFIERS) const {
    (void)WITH_MODIFIERS;
    return hpMax_;
  }
  inline int getSpiMax()    const {return spiMax_;}

  inline string getNameThe() const {return def_->name_the;}
  inline string getNameA() const {return def_->name_a;}
  inline bool isHumanoid() const {return def_->isHumanoid;}
  inline char getGlyph() const {return glyph_;}
  virtual const SDL_Color& getColor() {return clr_;}
  inline const Tile_t& getTile() const {return tile_;}
  inline MoveType_t getMoveType() const {return def_->moveType;}

  void addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const;

  virtual void actorSpecific_addLight(
    bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {
    (void)light;
  }

  Engine* eng;

  void teleport(const bool MOVE_TO_POS_AWAY_FROM_MONSTERS);

  virtual void actorSpecificOnStandardTurn() {}

protected:
  //TODO Try to get rid of these friend declarations
  friend class AbilityValues;
  friend class DungeonMaster;
  friend class Dynamite;
  friend class Molotov;
  friend class Flare;
  friend class StatusDiseased;

  virtual void actorSpecificDie() {}
  virtual void actorSpecific_hit(const int DMG) {(void)DMG;}
  virtual void actorSpecific_spawnStartItems() = 0;

  virtual void monsterHit(int& dmg) {(void)dmg;}
  virtual void monsterDeath() {}

  SDL_Color clr_;
  char glyph_;
  Tile_t tile_;

  int hp_, hpMax_, spi_, spiMax_;

  Pos lairCell_;

  ResHandler*     resHandler_;
  StatusHandler*  statusHandler_;
  ActorDef*       def_;
  Inventory*      inventory_;
};

#endif
